#include <app_timer_appsh.h>
#include <nrf_log.h>
#include <nrf_assert.h>

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

#define NUM_OF_SENSORS  7
#define TIMER_PERIOD_MS 100

static const senstick_sensor_base_t *m_p_sensor_bases[] = {
    &accelerationSensorBase,
    &gyroSensorBase,
    &magneticSensorBase,
    &brightnessSensorBase,
    &uvSensorBase,
    &humiditySensorBase,
    &pressureSensorBase
};

APP_TIMER_DEF(m_timer_id);

typedef struct {
    // センサー個別のアクセスベースのポインタ、無効なのはNULL
    bool isSensorAvailable[NUM_OF_SENSORS];
    
    // センサーのサービス
    sensor_service_t services[NUM_OF_SENSORS];
    // センサーの設定
    sensor_service_setting_t sensorSetting[NUM_OF_SENSORS];
    
    // センサ動作状態フラグ
    bool isSensorWorking;
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
    // センサ情報を書き込み
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
    if(p_log->readPosition >= p_log->header.size) {
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
//            context.p_readingLogContext[device_type] = NULL;
            break;
        }
    }

//    NRF_LOG_PRINTF_DEBUG("  didNotify: %d, count: %d.\n", didNotified, count);
    
    return didNotified;
}

static void sensor_notify_raw_data(sensor_device_t deviceType, uint8_t *p_raw_data, uint8_t data_length)
{
    // BLEで送るシリアライズされたデータに変換
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    const senstick_sensor_base_t *ptr = m_p_sensor_bases[deviceType];
    uint8_t length = (ptr->getBLEDataHandler)(buffer, p_raw_data);
    // 通知
    sensorServiceNotifyRealtimeData(&(context.services[deviceType]), buffer, length);
}

static void sensor_timer_handler(void *p_arg)
{
    uint8_t buffer[MAX_SENSOR_RAW_DATA_SIZE];
    
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        // 時間を増分
        context.sensorSampling[i] += TIMER_PERIOD_MS;
        // しきい値を超えていたら
        if(context.sensorSampling[i] >= context.sensorSetting[i].samplingDuration) {
            context.sensorSampling[i] -= context.sensorSetting[i].samplingDuration;
            //データ取得、そして通知とロギング
            if( context.isSensorAvailable[i] ) {
                const senstick_sensor_base_t *ptr = m_p_sensor_bases[i];
                uint8_t length = (ptr->getSensorDataHandler)(buffer);
                // 通知, ログ保存
                sensor_service_command_t command = context.sensorSetting[i].command;
                // 通知
                if((command & 0x01) != 0) {
                    sensor_notify_raw_data((sensor_device_t)i, buffer, length);
                }
                // ログ保存
                if((command & 0x02) != 0) {
                    writeLog(&(context.writingLogContext[i]), buffer, length);
                    // TBD 頻度高すぎないか?
                    senstickSensorControllerNotifyLogData();
                }
            }
        }
    }
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

static void setSensorShoudlWork(bool shouldWakeup, uint8_t new_log_id)
{
    ret_code_t err_code;
    
    // 状態が同じなら何もする必要はない。
    if(shouldWakeup == context.isSensorWorking) {
        return;
    }
    
    // センサーの電源をon/off。
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        const senstick_sensor_base_t *ptr = m_p_sensor_bases[i];
        if( context.isSensorAvailable[i] ) {
            (ptr->setSensorWakeupHandler)(shouldWakeup, &(context.sensorSetting[i]));
        }
    }
    
    // ログ及びタイマーの開始と停止
    if(shouldWakeup) {
        startLogging(new_log_id);
        err_code = app_timer_start(m_timer_id,
                                   APP_TIMER_TICKS(TIMER_PERIOD_MS, APP_TIMER_PRESCALER),
                                   NULL);
        APP_ERROR_CHECK(err_code);
    } else {
        app_timer_stop(m_timer_id);
        stopLogging();
    }
    
    // 状態保存
    context.isSensorWorking = shouldWakeup;
}

/**
 * Public methods
 */
ret_code_t initSenstickSensorController(uint8_t uuid_type)
{
    ret_code_t err_code;
    
    // 初期化
    memset(&context, 0, sizeof(seenstick_sensor_controller_context_t));
    // 設定、周期を300ミリ秒に初期化。
    for(int i = 0; i < NUM_OF_SENSORS; i++) {
        context.sensorSetting[i].samplingDuration = 300; // 300ミリ秒
    }
    // 永続化していたデフォルト設定値を読み込み
    loadSensorSetting();
    
    // タイマーの初期化
    err_code = app_timer_create(&(m_timer_id), APP_TIMER_MODE_REPEATED, sensor_timer_handler);
    APP_ERROR_CHECK(err_code);
    
    // センサの初期化と、配列を初期化。初期化の成否は、センサ有効フラグに収める。
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        bool result = (m_p_sensor_bases[i]->initSensorHandler)();
        context.isSensorAvailable[i] = result;
        if( ! result) {
            NRF_LOG_PRINTF_DEBUG("Faled to init sensor %d.\n", i);
        }
    }
    
    // サービスを初期化
    for(int i=0; i < NUM_OF_SENSORS; i++) {
        // 初期化に失敗したセンサーのサービスは構築しない
        if( context.isSensorAvailable[i] ) {
            ret_code_t err_code = initSensorService(&(context.services[i]), uuid_type, (sensor_device_t)i);
            APP_ERROR_CHECK(err_code);
        }
    }
    return NRF_SUCCESS;
}

// sensor serviceが呼び出す、データの読み書きメソッド
uint8_t senstickSensorControllerReadSetting(sensor_device_t device_type, uint8_t *p_buffer, uint8_t length)
{
    ASSERT(device_type < NUM_OF_SENSORS);
    ASSERT(length >= 6);
    return serializesensor_service_setting(p_buffer, &(context.sensorSetting[device_type]));
}

// 次のsenstickSensorControllerReadMetaData()でのみ使用可能なメソッド。
// 書き込み中は読み出せない。
uint32_t readSampleCount(sensor_device_t device_type)
{
    const senstick_sensor_base_t *p_base = m_p_sensor_bases[device_type];
    uint8_t log_count = senstick_getCurrentLogCount();
    
    if(log_count == 0) { // ログがない, すべて空いている
        return p_base->address_info.size / p_base->rawSensorDataSize;
    } else { // 最後のログヘッダを読みだし、サイズを求める
        log_context_t context;
        openLog(&context, (log_count -1),  &(p_base->address_info));
        uint32_t storage_last_address = (p_base->address_info.startAddress + p_base->address_info.size);
        uint32_t data_last_address    = (context.header.startAddress + context.header.size);
        return (storage_last_address - data_last_address) / p_base->rawSensorDataSize;
    }
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
    
    // デシリアライズ
    sensor_service_setting_t setting;
    deserializesensor_service_setting(&setting, p_data);
    // 値の正当性確認
    if( ! isValidSensorServiceCommand((uint8_t)setting.command)) {
        return false;
    }
    // TBD サンプリング周期、レンジ設定の正当性確認(センサーごとの)
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

void senstickSensorControllerNotifyLogData(void)
{
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        bool didNotified = notifyLogDataOfDevice((sensor_device_t) i);
        if(didNotified) {
            break;
        }
    }
}


/**
 *  observer
 */
void senstickSensorController_observeControlCommand(senstick_control_command_t command, uint8_t new_log_id)
{
    switch(command) {
        case sensorShouldSleep:
            setSensorShoudlWork(false, new_log_id);
            break;
        case sensorShouldWork:
            setSensorShoudlWork(true, new_log_id);
            break;
        case formattingStorage:
            senstickSensorControllerFormatStorage();
            break;
        case enterDeepSleep:
        case enterDFUmode:
            setSensorShoudlWork(false, new_log_id);
            // センサ設定情報の永続化処理
            saveSensorSetting();
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
    setSensorShoudlWork(false, 0);
    memset(context.p_readingLogContext, 0, sizeof(log_context_t *) * NUM_OF_SENSORS);
    
    // 各センサーのストレージ初期化
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        formatLog(&(m_p_sensor_bases[i]->address_info));
    }
}
