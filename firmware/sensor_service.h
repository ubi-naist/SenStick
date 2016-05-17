#ifndef sensor_service_h
#define sensor_service_h

#include "service_util.h"
#include "senstick_sensor_base_data.h"

/**
 * BLEの、センサーのサービスを提供します。
 * BLE側との、センサー設定、ログIDの読み書き機能を提供します。データの読み書きなどの処理には、sensor_base_controllerのメソッドを呼び出します。
 */

#define SENSOR_SERVICE_UUID            0x2100
#define SENSOR_SETTING_CHAR_UUID       0x7100
#define SENSOR_REALTIME_DATA_CHAR_UUID 0x7200
#define SENSOR_LOGID_CHAR_UUID         0x7300
#define SENSOR_METADATA_CHAR_UUID      0x7400
#define SENSOR_LOG_DATA_CHAR_UUID      0x7500

// サービスのコンテキスト構造体。サービスの実行に必要な情報が含まれる。
typedef struct sensor_service_s {
    uint16_t connection_handle;
    
    uint16_t service_handle;
    
    ble_gatts_char_handles_t sensor_setting_char_handle;
    ble_gatts_char_handles_t sensor_realtime_data_char_handle;
    ble_gatts_char_handles_t sensor_logid_char_handle;
    ble_gatts_char_handles_t sensor_log_metadata_char_handle;
    ble_gatts_char_handles_t sensor_log_data_char_handle;
    
    bool is_sensor_realtime_data_notifying;
    bool is_sensor_log_data_notifying;
    
    sensor_device_t device_type;
} sensor_service_t;

// 初期化します
ret_code_t initSensorService(sensor_service_t *p_context, uint8_t uuid_type, sensor_device_t device_type);

// BLEイベントを受け取ります。
void sensorService_handleBLEEvent(sensor_service_t *p_context, ble_evt_t * p_ble_evt);

// リアルタイムセンサーデータをNotifyします。
void sensorServiceNotifyRealtimeData(sensor_service_t *p_context, uint8_t *p_data, uint16_t length);

// ログデータをNotifyします。失敗したらfalseを返します。
bool sensorServiceNotifyLogData(sensor_service_t *p_context, uint8_t *p_data, uint16_t length);

#endif /* sensor_service_h */
