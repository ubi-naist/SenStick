#ifndef ble_logger_service_h
#define ble_logger_service_h

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#include "senstick_data_models.h"
#include "sensor_data_storage.h"

// イベントハンドラ
typedef struct ble_logger_service_s ble_logger_service_t;
typedef void (*ble_logger_service_event_handler_t) (ble_logger_service_t * p_context);

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
    
    uint16_t connection_handle;
    uint8_t	 uuid_type;                // ベンダーUUIDを登録した時に取得される、UUIDタイプ
};// ble_logger_service_t;

// 初期化関数。
// サービスを使う間に必ずこの関数を呼び出すこと。
uint32_t bleLoggerServiceInit(ble_logger_service_t *p_context, ble_logger_service_event_handler_t logger_event_handler);

// BLEイベント通知。mainはこのメソッドを通じてBLEイベントを伝えます。
void bleLoggerServiceOnBleEvent(ble_logger_service_t *p_context, ble_evt_t *p_ble_evt);

// ロギングを開始します。すでにロギングしているときは、なにもしません。
void bleLoggerServiceStartLogging(ble_logger_service_t *p_context, const sensorSetting_t *p_setting);
// // ロギングを停止します。すでに停止しているときは、なにもしません。
void bleLoggerServiceStopLogging(ble_logger_service_t *p_context);

// データ書き込み
void bleLoggerServiceWrite(ble_logger_service_t *p_context, const SensorData_t *p_sensorData);

#endif /* ble_logger_service_h */
