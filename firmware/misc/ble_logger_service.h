#ifndef ble_logger_service_h
#define ble_logger_service_h

#include "service_utility.h"

#include "senstick_data_models.h"
#include "sensor_data_storage.h"

#include "twi_sensor_manager.h"

//
typedef enum {
    REQUEST_DFU,
    START_RECORDING,
    STOP_RECORDING,
    STORAGE_FULL,    
} logger_service_event_type_t;

// イベントハンドラ
typedef struct ble_logger_service_s ble_logger_service_t;
typedef void (*ble_logger_service_event_handler_t) (ble_logger_service_t * p_context, logger_service_event_type_t event);

struct ble_logger_service_s {
    ble_logger_service_event_handler_t event_handler;
    
    ble_uuid_t base_uuid;

    // サービスハンドラ
    uint16_t writing_service_handle;
    ble_gatts_char_handles_t writing_control_point_char_handles;
    ble_gatts_char_handles_t storage_status_char_handles;
    ble_gatts_char_handles_t rtc_char_handles;
    ble_gatts_char_handles_t writing_abstract_char_handles;
    
    uint16_t reading_service_handle;
    ble_gatts_char_handles_t reading_target_char_handles;
    ble_gatts_char_handles_t reading_control_point_char_handles;
    ble_gatts_char_handles_t reading_status_char_handles;
    ble_gatts_char_handles_t reading_data_point_char_handles;
    ble_gatts_char_handles_t meta_data_sensor_setting_char_handles;
    ble_gatts_char_handles_t meta_data_datetime_char_handles;
    ble_gatts_char_handles_t meta_data_abstract_char_handles;
    
    uint16_t streaming_service_handle;
    ble_gatts_char_handles_t streaming_sensor_setting_char_handles;
    ble_gatts_char_handles_t streaming_data_point_char_handles;
    
    uint16_t connection_handle;
    uint8_t	 uuid_type;                // ベンダーUUIDを登録した時に取得される、UUIDタイプ
    
    bool is_streaming_data_point_notifying;
    
    sensor_manager_t *p_sensor_manager_context;
    
    // ストレージ
    flash_stream_context_t flash_stream;
    
    sensor_data_storage_t  writing_storage[2];
    uint8_t writing_storage_index;
    sensor_data_storage_t  reading_storage;
    sensor_data_storage_t  *p_writing;
    sensor_data_storage_t  *p_reading;

    char abstract[21];
    
    sensorSetting_t *p_setting;
    
};// ble_logger_service_t;

// 初期化関数。
// サービスを使う間に必ずこの関数を呼び出すこと。
uint32_t bleLoggerServiceInit(ble_logger_service_t *p_context, sensor_manager_t *p_sensor_manager_context, ble_logger_service_event_handler_t logger_event_handler);

// BLEイベント通知。mainはこのメソッドを通じてBLEイベントを伝えます。
void bleLoggerServiceOnBleEvent(ble_logger_service_t *p_context, ble_evt_t *p_ble_evt);

// ロギングを開始します。すでにロギングしているときは、なにもしません。
void bleLoggerServiceStartLogging(ble_logger_service_t *p_context);

// // ロギングを停止します。すでに停止しているときは、なにもしません。
void bleLoggerServiceStopLogging(ble_logger_service_t *p_context);

// センサー設定。ロギングを開始する前に、必ずこれを呼び出します。
void bleLoggerSetSensorSetting(ble_logger_service_t *p_context, sensorSetting_t *p_setting);

// データ更新
// ロギングしているか否かに関係なく、センサデータが更新されている限りは、常にこれを呼び出します
void bleLoggerServiceWrite(ble_logger_service_t *p_context, const SensorData_t *p_sensorData);

// ロギングを開始します。すでにロギングしているときは、なにもしません。
void bleLoggerServiceFormatStorage(ble_logger_service_t *p_context);

#endif /* ble_logger_service_h */
