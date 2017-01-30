#include <app_timer_appsh.h>
#include <nrf_log.h>
#include <nrf_assert.h>
#include <app_mailbox.h>
#include <nrf_soc.h>
#include <app_util_platform.h>
#include <app_scheduler.h>

#include "senstick_util.h"

#include "log_controller.h"

#include "senstick_sensor_controller.h"

#include "senstick_ble_definition.h"
#include "senstick_sensor_base.h"
#include "sensor_service.h"

#include "senstick_flash_address_definition.h"
#include "spi_slave_mx25_flash_memory.h"

#include "acceleration_sensor_base.h"
#include "gyro_sensor_base.h"
#include "magnetic_sensor_base.h"
#include "brightness_sensor_base.h"
#include "uv_sensor_base.h"
#include "humidity_sensor_base.h"
#include "pressure_sensor_base.h"

#define NUM_OF_SENSORS     7
#define MAILBOX_ITEM_SIZE  (MAX_SENSOR_RAW_DATA_SIZE +2)

#ifdef NRF51
#define MAILBOX_QUEUE_SIZE 40
#else // NRF52
// 消去max. 120ミリ秒 x 3センサー分、つまり360ミリ秒のうちに、3センサーx10ミリ秒サンプリング = 3 * 36 = 108
#define MAILBOX_QUEUE_SIZE 255
#endif

// TIMER割り込みプリスケーラ。16MHz / 2^4 = 1MHz。
#define TIMER_PRESCALERS_1US  (4)

// 割り込み周期(かつサンプリング周期の最小量)
#define TIMER_PERIOD_MS 10

static const senstick_sensor_base_t *m_p_sensor_bases[] = {
    &accelerationSensorBase,
    &gyroSensorBase,
    &magneticSensorBase,
    &brightnessSensorBase,
    &uvSensorBase,
    &humiditySensorBase,
    &pressureSensorBase
};


//APP_TIMER_DEF(m_timer_id);
// メールボックスのバイナリ配列のフォーマットは、[sensor_device_t, length, シリアライズされた構造体]
APP_MAILBOX_DEF(m_mailbox, MAILBOX_QUEUE_SIZE, MAILBOX_ITEM_SIZE);

typedef struct {
    // センサー個別のアクセスベースのポインタ、無効なのはNULL
    bool isSensorAvailable[NUM_OF_SENSORS];
    
    // センサーのサービス
    sensor_service_t services[NUM_OF_SENSORS];
    // センサーの設定
    sensor_service_setting_t sensorSetting[NUM_OF_SENSORS];
    
    // センサ動作状態フラグ
    bool isSensorWorking;

    // ログ吐き出しタスク読みだしフラグ
    bool isDequeueTaskRunning;
    
    // 通知フラグ
    bool isNotificationRunning;
    
    // センサのサンプリング周期積算カウンタ
    samplingDurationType sensorSampling[NUM_OF_SENSORS];
    
    log_context_t writingLogContext[NUM_OF_SENSORS];
    log_context_t readingLogContext[NUM_OF_SENSORS];
    
    log_context_t *p_readingLogContext[NUM_OF_SENSORS];
} seenstick_sensor_controller_context_t;

static seenstick_sensor_controller_context_t context;

/**
 * Private methods
 */

// もしもフラッシュに有効なセンサ情報があれば、読み込みます
#define MAGIC_WORD 0xabcd
void loadSensorSetting(void)
{
    // マジックワードを確認
    uint32_t i;
    readFlash(SENSOR_SETTING_STORAGE_START_ADDRESS, (uint8_t *)&i, sizeof(uint32_t));
    if( i!= MAGIC_WORD) {
        return;
    }
    // センサ情報を読み込み
    for(int i=0; i < NUM_OF_SENSORS; i++) {
        readFlash(SENSOR_SETTING_STORAGE_START_ADDRESS + sizeof(uint32_t) + i * sizeof(sensor_service_setting_t), (uint8_t *)&context.sensorSetting[i], sizeof(sensor_service_setting_t));
    }
}

void saveSensorSetting(void)
{
    // セクタを消去
    formatFlash(SENSOR_SETTING_STORAGE_START_ADDRESS, SENSOR_SETTING_STORAGE_SIZE);
    // マジックワードを書き込む
    uint32_t format_id = MAGIC_WORD;
    writeFlash(SENSOR_SETTING_STORAGE_START_ADDRESS, (uint8_t *)&format_id, sizeof(uint32_t));
    // センサ情報を書き込み
    for(int i=0; i < NUM_OF_SENSORS; i++) {
        writeFlash(SENSOR_SETTING_STORAGE_START_ADDRESS + sizeof(uint32_t) + i * sizeof(sensor_service_setting_t), (uint8_t *)&context.sensorSetting[i], sizeof(sensor_service_setting_t));
    }
}

void formatSensorSetting(void)
{
    // セクタを消去
    formatFlash(SENSOR_SETTING_STORAGE_START_ADDRESS, SENSOR_SETTING_STORAGE_SIZE);
}

// BLEサービスにログデータを通知します。読み込んだバイト数を返します。
// 先頭バイトは、有効なデータユニットの数、その後センサデータが並びます。
static uint8_t fillBLESensorData(uint8_t *p_data, uint8_t length, sensor_device_t device_type)
{
    const senstick_sensor_base_t *p_base = m_p_sensor_bases[device_type];
    log_context_t *p_log = context.p_readingLogContext[device_type];
    uint8_t buff[20]; // TBD マジックワード。センサーの生データ最大値を指定すべき。

    ASSERT(p_base->rawSensorDataSize < sizeof(buff));
    
    p_data[0] = 0;
    // データを読み込み設定していく
    const uint8_t s = p_base->bleSerializedSensorDataSize;
    uint8_t pt = 1;
    do {
        uint8_t read_length = readLog(p_log, buff, p_base->rawSensorDataSize);
        if(read_length == 0) {
            break;
        }
        (p_base->getBLEDataHandler)(&(p_data[pt]), buff);
        (p_data[0])++;
        pt += s;
    } while (pt < (length - s));
    
    return pt;
}

// 指定されたセンサーデータをGATTのバッファが尽きるまで通知する。
static bool notifyLogDataOfDevice(sensor_device_t device_type)
{
    uint8_t buff[GATT_MAX_DATA_LENGTH];
    
    bool didNotified = false;
    
//    NRF_LOG_PRINTF_DEBUG("notifyLogDataOfDevice(), device_type:%d.\n", device_type);
    
    // ログデータの通知がONになっていないなら、スキップ
    sensor_service_t *p_service = &context.services[device_type];
    if( ! p_service->is_sensor_log_data_notifying) {
        return false;
    }
    // ログデータ読み出しが有効ではないなら、スキップ
    log_context_t *p_log = context.p_readingLogContext[device_type];
    if(p_log == NULL) {
        return false;
    }
    // EndOfDataパケットを送信済
    if( p_log->didSendEndOfDataPacket ) { //&& (p_log->readPosition >= p_log->header.size)) {
        return false;
    }

    int count = 0;
    // データ読み出し
    for(;;) {
        uint32_t read_position = p_log->readPosition;
        uint8_t length = fillBLESensorData(buff, sizeof(buff), device_type);
        // 終端パケットなら -> 書き込み時は何もしない, 読み込み時なら通知&終了(ポインタをNULL)
        // 普通のパケットなら->通知するだけ
        if(p_log->canWrite && length == 1) {
            break;
        }
        
        bool notified = sensorServiceNotifyLogData(p_service, buff, length);
        // 通知失敗なら終了。読み出し位置を戻す。
        if( ! notified ) {
            seekLog(p_log, read_position);
            break;
        } else {
            didNotified = true;
            count++;
        }
        // もしも最後のパケット通知なら、読み出しを終了する。
        if(length == 1) {
            p_log->didSendEndOfDataPacket = true;
//            context.p_readingLogContext[device_type] = NULL;
            break;
        }
    }

//    NRF_LOG_PRINTF_DEBUG("  didNotify: %d, count: %d.\n", didNotified, count);
    
    return didNotified;
}

static bool sensor_notify_raw_data(sensor_device_t deviceType, uint8_t *p_raw_data, uint8_t data_length)
{
    // BLEで送るシリアライズされたデータに変換
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    const senstick_sensor_base_t *ptr = m_p_sensor_bases[deviceType];
    uint8_t length = (ptr->getBLEDataHandler)(buffer, p_raw_data);
    // 通知
    return sensorServiceNotifyRealtimeData(&(context.services[deviceType]), buffer, length);
}

static void setSensorShoudlWork(bool shouldWakeup, bool shouldLogging, uint8_t new_log_id);
static void flash_mailbox(void)
{
    ret_code_t err_code;
    uint8_t buffer[MAILBOX_ITEM_SIZE];

//    uint32_t prev_time = app_timer_cnt_get();
    
    while(true) {
        // キューの深さ表示
/*
        static uint32_t queue_length = 1;
        uint32_t length = app_mailbox_length_get (&m_mailbox);
        if( length > queue_length ) {
            queue_length = length;
            NRF_LOG_PRINTF_DEBUG("queue: %d.\n", queue_length);
        }
*/
        // デキュー
        err_code = app_mailbox_get(&m_mailbox, buffer);
        if(err_code == NRF_ERROR_NO_MEM) {
            break;
        }
        
        sensor_service_command_t command = context.sensorSetting[buffer[0]].command;
        // BLEリアルタイム通知
        if((command & 0x01) != 0) {
            sensor_notify_raw_data((sensor_device_t)buffer[0], &buffer[2], buffer[1]);
        }
        // ログ保存とBLE通知
        if((command & 0x02) != 0) {
            int wlen = writeLog(&(context.writingLogContext[buffer[0]]), &buffer[2], buffer[1]);
            senstickSensorControllerNotifyLogData();
            // 書き込みサイズが指定と違う、つまりログがいっぱいだったら、ロギングの停止、ディスクフルフラグを立てる
            // このメソッドは内部でflash_mailbox()を呼び出すので、再帰されても大丈夫なように、あらかじめメイルボックスをフラッシュしておく。
            if( wlen != buffer[1]) {
                do {
                    err_code = app_mailbox_get(&m_mailbox, buffer);
                } while(err_code == NRF_SUCCESS);
                // ロギング停止
                senstick_setControlCommand(sensorShouldSleep);
                senstick_setDiskFull(true);    // ディスクフルフラグを立てる。
            }
        }
    }

    // 処理時間表示
    /*
    uint32_t now_time = app_timer_cnt_get();
    uint32_t dur;
    app_timer_cnt_diff_compute(now_time, prev_time, &dur);
    prev_time = now_time;
    NRF_LOG_PRINTF_DEBUG("\n  takes: %d.", dur);
    */
    
    // タスクフラグをクリア
    CRITICAL_REGION_ENTER();
    context.isDequeueTaskRunning = false;
    CRITICAL_REGION_EXIT();
}

static void sched_event_handler(void *p_event_data, uint16_t event_size)
{
    flash_mailbox();
}

// Timer2 interrupt handler
void TIMER2_IRQHandler(void)
{
    // clear event flag
    NRF_TIMER2->EVENTS_COMPARE[0] = 0;
    //  LOG("\nTIMER2_IRQHandler(), CC[0] %d", NRF_TIMER2->CC[0]);
    
    ret_code_t err_code;
    uint8_t buffer[MAX_SENSOR_RAW_DATA_SIZE];
    uint8_t mailbox_buffer[MAILBOX_ITEM_SIZE];
    bool did_enqueue = false;
    
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        // データ取得および通知とロギング対象?
        sensor_service_command_t command = context.sensorSetting[i].command;
        if(!(context.isSensorAvailable[i] && (command & 0x03) != 0)) {
            continue;
        }
        // 時間を増分
        context.sensorSampling[i] += TIMER_PERIOD_MS;
        // しきい値を超えていたら
        if(context.sensorSampling[i] >= context.sensorSetting[i].samplingDuration) {
            // データ取得
            // センサ取得トリガー時間からの差分時間。
            samplingDurationType duration = context.sensorSampling[i] - context.sensorSetting[i].samplingDuration;
            const senstick_sensor_base_t *ptr = m_p_sensor_bases[i];
            uint8_t length = (ptr->getSensorDataHandler)(buffer, duration);
            // データが取得できれば、メールボックスにデータを保存して、次のサンプリングに。
            if( length > 0) {
                // 次のサンプリング時間。
                context.sensorSampling[i] -= context.sensorSetting[i].samplingDuration;
                // メールボックスに格納
                did_enqueue       = true;
                mailbox_buffer[0] = i;
                mailbox_buffer[1] = length;
                memcpy(&mailbox_buffer[2], buffer, length);
                err_code = app_mailbox_put (&m_mailbox, mailbox_buffer);
                APP_ERROR_CHECK(err_code);
            }
        }
    }
    // メールボックスに何かしらデータが入っている、かつ、タスクにメールボックス吐き出すがないならば、タスクを積む。
    CRITICAL_REGION_ENTER();
    if( did_enqueue && !context.isDequeueTaskRunning) {
        context.isDequeueTaskRunning = true;
        err_code = app_sched_event_put(NULL, 0, sched_event_handler);
    }
    CRITICAL_REGION_EXIT();
}

static void startLogging(uint8_t new_log_id)
{
    // ログを開き、メタデータを、先頭要素として書き込み。
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        createLog(&(context.writingLogContext[i]), new_log_id,
                  context.sensorSetting[i].samplingDuration, context.sensorSetting[i].measurementRange,
                  &(m_p_sensor_bases[i]->address_info));
    }
}

static void stopLogging(void)
{
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        closeLog(&(context.writingLogContext[i]));
    }
    
    // 読込中のがいたら、それを書き込みログから読み込みログに切り替える。
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        if(context.p_readingLogContext[i] == &(context.writingLogContext[i])) {
            context.p_readingLogContext[i] = &(context.readingLogContext[i]);
            reOpenLog(context.p_readingLogContext[i], &(context.writingLogContext[i]));
        }
    }
}

static void setSensorPower(bool isPowerOn)
{
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        const senstick_sensor_base_t *ptr = m_p_sensor_bases[i];
        if( context.isSensorAvailable[i] ) {
            (ptr->setSensorWakeupHandler)(isPowerOn, &(context.sensorSetting[i]));
        }
    }
}

static void setSensorShoudlWork(bool shouldWakeup, bool shouldLogging, uint8_t new_log_id)
{
    // 状態が同じなら何もする必要はない。
    if(shouldWakeup == context.isSensorWorking) {
        return;
    }
    
    // ログ及びタイマーの開始と停止
    if(shouldWakeup) {
        // センサースタート
        setSensorPower(true);
        // ログスタート
        if( shouldLogging ) {
            startLogging(new_log_id);
        }
        // タイマーをスタート
        NRF_TIMER2->TASKS_CLEAR = 1;
        NRF_TIMER2->CC[0]       = TIMER_PERIOD_MS * 1000; // prescalerは1us。1msec = 1,000us
        NRF_TIMER2->TASKS_START = 1;
    } else {
        // タイマーをシャットダウン
        NRF_TIMER2->TASKS_SHUTDOWN = 1;
        // メールボックスをフラッシュ。
        flash_mailbox();
        // ログを閉じる
        if(shouldLogging) {
            stopLogging();
        }
        // センサ設定情報の永続化処理
        saveSensorSetting();
        // センサーの電源を落とす
        setSensorPower(false);
    }
    
    // 状態保存
    context.isSensorWorking = shouldWakeup;
}

static void init_timer(void)
{
    uint32_t err_code;
    
    err_code = sd_nvic_ClearPendingIRQ(TIMER2_IRQn);
    APP_ERROR_CHECK(err_code);

#ifdef NRF52
    err_code = sd_nvic_SetPriority(TIMER2_IRQn, APP_IRQ_PRIORITY_HIGH); //APP_IRQ_PRIORITY_HIGH       // Replaces NRF_APP_PRIORITY_HIGH
    APP_ERROR_CHECK(err_code);
#else // NRF51
    err_code = sd_nvic_SetPriority(TIMER2_IRQn, NRF_APP_PRIORITY_HIGH);
    APP_ERROR_CHECK(err_code);
#endif
    
    err_code = sd_nvic_EnableIRQ(TIMER2_IRQn);
    APP_ERROR_CHECK(err_code);
    
    // TIMERは、16MHzのHCLKをソースにする。分周比は2^x。もしもソースが1MHz以下の場合は、消費電流が低減される。
    NRF_TIMER2->MODE        = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->BITMODE     = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER2->PRESCALER   = TIMER_PRESCALERS_1US;
    
    // Set a shortcut register to clear a counter register by a compare match event.
    NRF_TIMER2->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
    
    // Interrupt setup. Enable interuptions by CC0;
    NRF_TIMER2->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
}

/**
 * Public methods
 */
ret_code_t initSenstickSensorController(uint8_t uuid_type)
{
    ret_code_t err_code;
    
    // 初期化
    memset(&context, 0, sizeof(seenstick_sensor_controller_context_t));
    // 設定、周期を200ミリ秒に初期化。
    for(int i = 0; i < NUM_OF_SENSORS; i++) {
        context.sensorSetting[i].samplingDuration = 200; // 200ミリ秒
    }
    // 永続化していたデフォルト設定値を読み込み
    loadSensorSetting();
    
    // タイマーの初期化
    init_timer();
    
    // センサの初期化と、配列を初期化。初期化の成否は、センサ有効フラグに収める。
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        bool result = (m_p_sensor_bases[i]->initSensorHandler)();
        context.isSensorAvailable[i] = result;
        if( ! result) {
            NRF_LOG_PRINTF_DEBUG("Faled to init sensor %d.\n", i);
        }
    }
    
    // サービスを初期化, 初期化に失敗したセンサーでもBLEのサービスは構築する
    for(int i=0; i < NUM_OF_SENSORS; i++) {
        // 初期化に失敗したセンサーのサービスは構築しない
//        if( context.isSensorAvailable[i] ) {
            ret_code_t err_code = initSensorService(&(context.services[i]), uuid_type, (sensor_device_t)i);
            APP_ERROR_CHECK(err_code);
//        }
    }
    
    // メールボックスを用意
    err_code = app_mailbox_create(&m_mailbox);
    APP_ERROR_CHECK(err_code);
    
    return NRF_SUCCESS;
}

uint8_t senstickSensorControllerGetNumOfActiveSensor(void)
{
    uint8_t count = 0;
    
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        sensor_service_command_t command = context.sensorSetting[i].command;
        if( context.isSensorAvailable[i] && (command != sensorServiceCommand_stop)) {
            count++;
        }
    }
    return count;
    
}

uint8_t senstickSensorControllerGetNumOfLoggingReadySensor(void)
{
    uint8_t count = 0;
    
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        sensor_service_command_t command = context.sensorSetting[i].command;
        if( context.isSensorAvailable[i] && (command & sensorServiceCommand_logging) != 0) {
            count++;
        }
    }
    return count;
}

// sensor serviceが呼び出す、データの読み書きメソッド
uint8_t senstickSensorControllerReadSetting(sensor_device_t device_type, uint8_t *p_buffer, uint8_t length)
{
    ASSERT(device_type < NUM_OF_SENSORS);
    ASSERT(length >= 6);
    return serializesensor_service_setting(p_buffer, &(context.sensorSetting[device_type]));
}

// 残りサンプル数を読みだす
uint32_t readSampleCount(sensor_device_t device_type)
{
    const senstick_sensor_base_t *p_base = m_p_sensor_bases[device_type];
    uint8_t log_count = senstick_getCurrentLogCount();
    
    if(log_count == 0) { // ログがない, すべて空いている
        return p_base->address_info.size / p_base->rawSensorDataSize;
    }
    
    // 最後のログヘッダを読みだし、サイズを求める

    // ログコンテキストを読みだす。書き込み中であればそれをコピー。停止中であれば末尾のログコンテキストを読みだす。
    log_context_t log_context;
    if(context.isSensorWorking) {
        log_context = context.writingLogContext[device_type];
    } else {
        openLog(&log_context, (log_count -1),  &(p_base->address_info));
    }

    uint32_t storage_last_address = (p_base->address_info.startAddress + p_base->address_info.size);
    uint32_t data_last_address    = (log_context.header.startAddress + log_context.header.size);
    return (storage_last_address - data_last_address) / p_base->rawSensorDataSize;
}

uint8_t senstickSensorControllerReadMetaData(sensor_device_t device_type, uint8_t *p_buffer, uint8_t length)
{
    ASSERT(length >= 17);
    
    const senstick_sensor_base_t *p_base = m_p_sensor_bases[device_type];
    const log_context_t *p_log           = context.p_readingLogContext[device_type];
    
    sensor_metadata_t metadata;
    memset(&metadata, 0, sizeof(sensor_metadata_t));
    metadata.logID = 0xff;
    if(p_log != NULL) {
        metadata.logID            = p_log->header.logID;
        metadata.samplingDuration = p_log->header.samplingDuration;
        metadata.measurementRange = p_log->header.measurementRange;
        metadata.position         = p_log->readPosition / p_base->rawSensorDataSize; // 単位はサンプル数
        if(p_log->canWrite) {
            // 書き込み中、有効なサンプル数は、書き込みサイズで決まる
            metadata.sampleCount  = p_log->writePosition / p_base->rawSensorDataSize; // 単位はサンプル数
            // 書き込み中、残り容量はこのヘッダから計算できる
            metadata.remainingStorage = (p_log->header.size - p_log->writePosition) / p_base->rawSensorDataSize;
        } else {
            metadata.sampleCount = p_log->header.size    / p_base->rawSensorDataSize;
            // 読み込み時は、最後のヘッダを読みだして、残り残量を求めるs
            metadata.remainingStorage = readSampleCount(device_type);
        }
    } else {
        metadata.remainingStorage = readSampleCount(device_type);
    }
    
    uint8_t len = serializeSensorMetaData(p_buffer, &metadata);
    return len;
}

bool senstickSensorControllerWriteSetting(sensor_device_t device_type, uint8_t *p_data, uint8_t length)
{
    // センシング動作中はfalseを返す
    if(context.isSensorWorking) {
        return false;
    }
    
    // センサーが使用不可能なときはfalseを返す
    if(!context.isSensorAvailable[device_type]) {
        return false;
    }
    
    // デシリアライズ
    sensor_service_setting_t setting;
    deserializesensor_service_setting(&setting, p_data);
    // 値の正当性確認
    if( ! isValidSensorServiceCommand((uint8_t)setting.command)) {
        return false;
    }
    
    // センササンプリング周期の制約条件。
    // 本来はここにベタ書きするものではない。本来はセンサごとに処理を委譲すべき。
    // I2Cバスを共有している都合、センサー単体で値の領域判定ができない部分があるので、それはここで処理する。
    switch(device_type) {
        case AccelerationSensor:  // I2Cバスを330マイクロ秒使う。
        case GyroSensor:          // I2Cバスを330マイクロ秒使う。
        case MagneticFieldSensor: // I2Cバスを360マイクロ秒使う。
#ifdef NRF52
            // これらのセンサーは10ミリ秒以上の周期。
            if( setting.samplingDuration < 10) {
                return false;
            }
#else // NRF51, 16kB
            // フラッシュのセクタ消去Typ.30ミリ秒の条件で、30ミリ秒サンプリングでメールボックスの深さは10に達する。
            // ワースト120ミリ秒、またメールボックスは40までなので、サンプリング周期の上限は30ミリ秒が限度。
            if( setting.samplingDuration < 30) {
                return false;
            }
            
#endif
            break;

        case UltraVioletSensor:            // I2Cバスを170マイクロ秒使う。
        case AirPressureSensor:            // I2Cバスを500マイクロ秒使う。
        case BrightnessSensor:             // 変換処理に150ミリ秒かかかる。
        case HumidityAndTemperatureSensor: // 変換処理に21ミリ秒かかる。
            // 周期は200ミリ秒以上
            if (setting.samplingDuration < 200) {
                return false;
            }
            break;
            
        default:
            // 未知のデバイスタイプは除外
            return false;
    }
    
    // 代入
    context.sensorSetting[device_type] = setting;
    return true;
}

void senstickSensorControllerWriteLogID(sensor_device_t device_type, uint8_t *p_data, uint8_t length)
{
    // 現在の読み出しをキャンセル
    context.p_readingLogContext[device_type] = NULL;
    
    // デシリアライズ
    sensor_service_logID_t log_id;
    deserializeSensorServiceLogID(&log_id, p_data);
    
    // ログの範囲が外れているなら、ここで終了
    if(log_id.logID >= senstick_getCurrentLogCount()) {
        return;
    }

    // 読み出しログポインタを開く
    if(context.isSensorWorking && context.writingLogContext[device_type].header.logID == log_id.logID) {
        // もしも書き込み中の読み出しならば、それを参照
        context.p_readingLogContext[device_type] = &(context.writingLogContext[device_type]);
    } else {
        // 読み出しのみならばそれを開く
        context.p_readingLogContext[device_type] = &context.readingLogContext[device_type];
        openLog(context.p_readingLogContext[device_type], log_id.logID, &(m_p_sensor_bases[device_type]->address_info));
    }
    NRF_LOG_PRINTF_DEBUG("reading log, id:%d.\n", log_id.logID);
    // TBD 縮退数の保存
    
    // 読み出し位置を設定
    seekLog(context.p_readingLogContext[device_type], log_id.position * m_p_sensor_bases[device_type]->rawSensorDataSize);
}

// BLEイベントと、TIMER割り込みイベントから呼ばれるため、スレッドセーフにしておく。
void senstickSensorControllerNotifyLogData(void)
{
    if( context.isNotificationRunning ) {
        return;
    }
    
    // タスクフラグをセット
    CRITICAL_REGION_ENTER();
    context.isNotificationRunning = true;
    CRITICAL_REGION_EXIT();
    
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        bool didNotified = notifyLogDataOfDevice((sensor_device_t) i);
        if(didNotified) {
            break;
        }
    }

    // タスクフラグをクリア
    CRITICAL_REGION_ENTER();
    context.isNotificationRunning = false;
    CRITICAL_REGION_EXIT();
}

// データ領域がいっぱいかを返します。
bool senstickSensorControllerIsDataFull(uint8_t logID)
{
    log_context_t log_context;
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        openLog(&log_context, logID, &(m_p_sensor_bases[i]->address_info));

        // 末尾がデータ領域を超えていないか?
        // センサ構造体は最大で6バイト。余裕を見て128サンプルくらいが空いているかを確認。
        if( (log_context.header.startAddress + log_context.header.size + 6 * 128) > (m_p_sensor_bases[i]->address_info.startAddress + m_p_sensor_bases[i]->address_info.size) ) {
            NRF_LOG_PRINTF_DEBUG("storage over: sensor:%d.\n", i);
            return true;
        }
    }
    return false;
}

/**
 *  observer
 */
void senstickSensorController_observeControlCommand(senstick_control_command_t command, bool shouldStartLogging, uint8_t new_log_id)
{
    switch(command) {
        case sensorShouldSleep:
            setSensorShoudlWork(false, shouldStartLogging, new_log_id);
            break;
        case sensorShouldWork:
            setSensorShoudlWork(true, shouldStartLogging, new_log_id);
            break;
        case formattingStorage:
            senstickSensorControllerFormatStorage();
            formatSensorSetting();
            break;
        case shouldDeviceSleep:
        case enterDFUmode:
            setSensorShoudlWork(false, shouldStartLogging, new_log_id);
            break;
        default:
            break;
    }
}

void senstickSensorController_handleBLEEvent(ble_evt_t * p_ble_evt)
{
    // サービスにイベントを渡す。初期化に失敗したセンサーのサービスは無視する。
    for(int i=0; i < NUM_OF_SENSORS; i++) {
        if( context.isSensorAvailable[i] ) {
            sensorService_handleBLEEvent(&(context.services[i]), p_ble_evt);
        }
    }
    // ログの通知。スタックのバッファが埋まるまで。
    if(p_ble_evt->header.evt_id == BLE_EVT_TX_COMPLETE) {
//        NRF_LOG_PRINTF_DEBUG("BLE_EVT_TX_COMPLETE, notifyLogData().\n");
        senstickSensorControllerNotifyLogData();
    }
}

void senstickSensorControllerFormatStorage(void)
{
    setSensorShoudlWork(false, false, 0);
    memset(context.p_readingLogContext, 0, sizeof(log_context_t *) * NUM_OF_SENSORS);
    
    // 各センサーのストレージ初期化
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        formatLog(&(m_p_sensor_bases[i]->address_info));
    }
}
