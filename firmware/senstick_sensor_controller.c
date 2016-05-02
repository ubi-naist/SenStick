#include <app_timer.h>
#include <nrf_assert.h>

#include "senstick_sensor_controller.h"

#include "senstick_ble_definitions.h"
#include "senstick_sensor_base.h"
#include "sensor_service.h"

#include "acceleration_sensor_base.h"

//#define NUM_OF_SENSORS  7
#define NUM_OF_SENSORS  1
#define TIMER_PERIOD_MS 100

// センサー個別のアクセスベースのポインタ、無効なのはNULL
static bool m_isSensorAvailable[NUM_OF_SENSORS];
const senstick_sensor_base_t *m_p_sensor_bases[] = {&accelerationSensorBase};
    
// センサーのサービス
static  sensor_service_t m_services[NUM_OF_SENSORS];
// センサーの設定
static sensor_service_setting_t m_sensorSetting[NUM_OF_SENSORS];
// センサーのログID
static sensor_service_logID_t m_sensorLogID[NUM_OF_SENSORS];
    
APP_TIMER_DEF(m_timer_id);
static  bool m_isSensorWorking;
static  samplingDurationType m_sensorSampling[NUM_OF_SENSORS];

/**
 * Private methods
 */
static void sensor_notify_raw_data(sensor_device_t deviceType, uint8_t *p_raw_data, uint8_t data_length)
{
    // BLEで送るシリアライズされたデータに変換
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    const senstick_sensor_base_t *ptr = m_p_sensor_bases[deviceType];
    uint8_t length = (ptr->getBLEDataHandler)(buffer, p_raw_data);
    // 通知
    sensorServiceNotifyRealtimeData(&(m_services[deviceType]), buffer, length);
}

static void sensor_log_raw_data(sensor_device_t deviceType, uint8_t *p_data, uint8_t length)
{
    // TBD
}

static void sensor_timer_handler(void *p_arg)
{
    uint8_t buffer[MAX_SENSOR_RAW_DATA_SIZE];
    
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        // 時間を増分
        m_sensorSampling[i] += TIMER_PERIOD_MS;
        // しきい値を超えていたら
        if(m_sensorSampling[i] >= m_sensorSetting[i].samplingDuration) {
            m_sensorSampling[i] -= m_sensorSetting[i].samplingDuration;
            //データ取得、そして通知とロギング
            if( m_isSensorAvailable[i] ) {
                const senstick_sensor_base_t *ptr = m_p_sensor_bases[i];
                uint8_t length = (ptr->getSensorDataHandler)(buffer);
                // 通知, ログ保存
                sensor_service_command_t command = m_sensorSetting[i].command;
                // 通知
                if((command & 0x01) != 0) {
                    sensor_notify_raw_data((sensor_device_t)i, buffer, length);
                }
                // ログ保存
                if((command & 0x02) != 0) {
                    sensor_log_raw_data((sensor_device_t)i, buffer, length);
                }
            }
        }
    }
}

static void setSensorShoudlWork(bool shouldWakeup)
{
    // センサーの電源をon/off。
    for(int i=0 ; i < NUM_OF_SENSORS; i++) {
        const senstick_sensor_base_t *ptr = m_p_sensor_bases[i];
        if( m_isSensorAvailable[i] ) {
            (ptr->setSensorWakeupHandler)(shouldWakeup, &(m_sensorSetting[i]));
        }
    }

    // TBD 停止の場合は、センシングなどの現在の処理を完了させる
    // TBD ログの開始処理
    // TBD センシングのタイマー開始
    ret_code_t err_code;
    if( shouldWakeup ) {
        err_code = app_timer_start(m_timer_id,
                                   APP_TIMER_TICKS(TIMER_PERIOD_MS, APP_TIMER_PRESCALER),
                                   NULL);
        APP_ERROR_CHECK(err_code);

    } else {
        app_timer_stop(m_timer_id);
    }
    m_isSensorWorking = shouldWakeup;
}

static void startToFormatStorage(void)
{
    setSensorShoudlWork(false);
    // TBD ログのセッションを初期化。ヘッダとデータの最初の部分。
}

/**
 * Public methods
 */
ret_code_t initSenstickSensorController(uint8_t uuid_type)
{
    ret_code_t err_code;
    
    // メモリ初期化
//    memset(&context, 0, sizeof(senstick_sensor_controller_t));
    memset(m_p_sensor_bases, 0, sizeof(senstick_sensor_base_t *) * NUM_OF_SENSORS);
    memset(m_services,       0, sizeof(sensor_service_t)         * NUM_OF_SENSORS);
    memset(m_sensorSetting,  0, sizeof(sensor_service_setting_t) * NUM_OF_SENSORS);
    memset(m_sensorLogID,    0, sizeof(sensor_service_logID_t)   * NUM_OF_SENSORS);
    m_isSensorWorking = false;
    // 設定、周期を300ミリ秒に初期化。
    for(int i = 0; i < NUM_OF_SENSORS; i++) {
        m_sensorSetting[i].samplingDuration = 300; // 300ミリ秒
    }
    
    // タイマーの初期化
    err_code = app_timer_create(&(m_timer_id), APP_TIMER_MODE_REPEATED, sensor_timer_handler);
    APP_ERROR_CHECK(err_code);
    
    // センサの初期化と、配列を初期化。初期化の成否は、センサ有効フラグに収める。
    for(int i =0; i < NUM_OF_SENSORS; i++) {
        bool result = (m_p_sensor_bases[i]->initSensorHandler)();
        m_isSensorAvailable[i] = result;
    }
    
    // TBD 永続化していたデフォルト設定値を読み込み
    // サービスを初期化
    for(int i=0; i < NUM_OF_SENSORS; i++) {
        // 初期化に失敗したセンサーのサービスは構築しない
        if( ! m_isSensorAvailable[i] ) {
            ret_code_t err_code = initSensorService(&(m_services[i]), uuid_type, (sensor_device_t)i);
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
    return serializesensor_service_setting(p_buffer, &(m_sensorSetting[device_type]));
}

uint8_t senstickSensorControllerReadLogID(sensor_device_t device_type, uint8_t *p_buffer, uint8_t length)
{
    ASSERT(device_type < NUM_OF_SENSORS);
    ASSERT(length >= 7);
    return serializeSensorServiceLogID(p_buffer, &(m_sensorLogID[device_type]));
}

uint8_t senstickSensorControllerReadMetaData(sensor_device_t device_type, uint8_t *p_buffer, uint8_t length)
{
    // TBD EEPROMからメタデータを読みだす。
    return 0;
}

bool senstickSensorControllerWriteSetting(sensor_device_t device_type, uint8_t *p_data, uint8_t length)
{
    // センシング動作中はfalseを返す
    if(m_isSensorWorking) {
        return false;
    }
    
    // 設定更新
    deserializesensor_service_setting(&(m_sensorSetting[device_type]), p_data);
    return true;
}

void senstickSensorControllerWriteLogID(sensor_device_t device_type, uint8_t *p_data, uint8_t length)
{
    // TBD ログ読み出し中は、停止してからこの値で更新。
}

/**
 *  observer
 */
void senstickSensorController_observeControlCommand(senstick_control_command_t command)
{
    switch(command) {
        case sensorShouldSleep:
            setSensorShoudlWork(false);
            break;
        case sensorShouldWork:
            setSensorShoudlWork(true);
            break;
        case formattingStorage:
            startToFormatStorage();
            break;
        case enterDeepSleep:
        case enterDFUmode:
            setSensorShoudlWork(false);
            // TBD センサ設定情報の永続化処理
            break;
        default:
            // 意図しないケース
            ASSERT(false);
            break;
    }
}
                                    

void senstickSensorController_handleBLEEvent(ble_evt_t * p_ble_evt)
{
    // サービスにイベントを渡す。初期化に失敗したセンサーのサービスは無視する。
    for(int i=0; i < NUM_OF_SENSORS; i++) {
        if( m_isSensorAvailable[i] ) {
            sensorService_handleBLEEvent(&(m_services[i]), p_ble_evt);
        }
    }
    // TBD ログの通知。スタックのバッファが埋まるまで。
}
