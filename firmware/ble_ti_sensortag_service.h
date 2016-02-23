#ifndef ble_ti_sensortag_service_h
#define ble_ti_sensortag_service_h

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#include "senstick_data_models.h"

// イベントハンドラ
typedef struct ble_sensortag_service_s ble_sensortag_service_t;
typedef void (*ble_sensortag_service_event_handler_t) (ble_sensortag_service_t * p_context, sensorSetting_t *p_setting);

// アクティビティサービスのコンテキスト構造体。サービスの実行に必要な情報が含まれる。
struct ble_sensortag_service_s {
    ble_sensortag_service_event_handler_t setting_changed_event_handler;     // イベントハンドラ
    
    ble_uuid_t base_uuid;

    // IR温度計。このサービスは、使わない。
    uint16_t ir_temperature_service_handle;
    ble_gatts_char_handles_t ir_temperature_value_char_handle;
    ble_gatts_char_handles_t ir_temperature_configration_char_handle;
    ble_gatts_char_handles_t ir_temperature_period_char_handle;
    
    uint16_t accelerometer_service_handle;
    ble_gatts_char_handles_t accelerometer_value_char_handle;
    ble_gatts_char_handles_t accelerometer_configration_char_handle;
    ble_gatts_char_handles_t accelerometer_period_char_handle;
    
    uint16_t humidity_service_handle;
    ble_gatts_char_handles_t humidity_value_char_handle;
    ble_gatts_char_handles_t humidity_configration_char_handle;
    ble_gatts_char_handles_t humidity_period_char_handle;
    
    uint16_t magnetometer_service_handle;
    ble_gatts_char_handles_t magnetometer_value_char_handle;
    ble_gatts_char_handles_t magnetometer_configration_char_handle;
    ble_gatts_char_handles_t magnetometer_period_char_handle;
    
    uint16_t barometer_service_handle;
    ble_gatts_char_handles_t barometer_value_char_handle;
    ble_gatts_char_handles_t barometer_configration_char_handle;
    ble_gatts_char_handles_t barometer_calibration_char_handle;
    ble_gatts_char_handles_t barometer_period_char_handle;

    uint16_t gyroscope_service_handle;
    ble_gatts_char_handles_t gyroscope_value_char_handle;
    ble_gatts_char_handles_t gyroscope_configration_char_handle;
    ble_gatts_char_handles_t gyroscope_period_char_handle;
    
    uint16_t connection_handle;
    uint8_t	 uuid_type;                // ベンダーUUIDを登録した時に取得される、UUIDタイプ

    sensorSetting_t sensor_setting;    // センサーの設定
    
    // cccdの値保存
    bool is_temperature_notifying;
    bool is_accelerometer_notifying;
    bool is_humidity_notifying;
    bool is_magnetrometer_notifying;
    bool is_barometer_notifying;
    bool is_gyroscope_notifying;
    bool is_illumination_notifying;
    // サンプリング設定
    bool is_accelerometer_sampling;
    bool is_humidity_sampling;
    bool is_magnetrometer_sampling;
    bool is_barometer_sampling;
    bool is_gyroscope_sampling;
    bool is_illumination_sampling;
    uint8_t gyroSamplingPeriod;
    
};// ble_sensortag_service_t;

// 初期化関数
// このサービスを使う前に、必ずこの関数を呼び出すこと。
uint32_t bleSensorTagServiceInit(ble_sensortag_service_t *p_context, const sensorSetting_t *p_setting, ble_sensortag_service_event_handler_t setting_changed_event_handler);

// BLEイベント通知関数
// BLEのイベントをこの関数に通知します。
void bleSensorTagServiceOnBLEEvent(ble_sensortag_service_t *p_context, ble_evt_t * p_ble_evt);

// 通知メソッド
void notifySensorData(ble_sensortag_service_t *p_context, const SensorData_t *p_sensorData);

// センサーの設定を設定します。BLEのサービスの値に反映されます。
void setSensorTagSetting(ble_sensortag_service_t *p_context, const sensorSetting_t *p_dst);

// センサーの設定を取得します
void getSensorTagSetting(ble_sensortag_service_t *p_context, sensorSetting_t *p_dst);

#endif /* ble_ti_sensortag_service_h */

