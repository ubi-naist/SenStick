#ifndef ble_activity_service_h
#define ble_activity_service_h

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#include "senstick_data_models.h"

// SenstickのGATTサービスを提供します。

// キャラクタリスティクスのデータ長
#define BLE_NINE_AXIS_CHAR_VALUE_LENGTH     18
#define BLE_BRIGHTNESS_CHAR_VALUE_LENGTH    6
#define BLE_TEMP_HUMIDITY_CHAR_VALUE_LENGTH 4
#define BLE_PRESSURE_CHAR_VALUE_LENGTH      3

// イベントタイプ
typedef enum {
    BLE_ACTIVITY_SERVICE_SETTING_WRITTEN, // 設定情報が書き込まれた
    BLE_ACTIVITY_SERVICE_CONTROL_WRITTEN, // 制御情報が書き込まれた
} ble_activity_service_event_type_t;

// イベントのデータ構造体
typedef struct {
    ble_activity_service_event_type_t event_type;
    
    uint8_t *p_data; // データへのポインタ
    int data_length; // 長さ
} ble_activity_service_event_t;

// 構造体の宣言
typedef struct ble_activity_service_s ble_activity_service_t;

// イベントハンドラ
typedef void (*ble_activity_service_event_handler_t) (ble_activity_service_t * p_context, const ble_activity_service_event_t * p_event);

// アクティビティサービス初期化構造体。この構造体は初期化に必要なすべてのデータとオプションを含む。
typedef struct {
    ble_activity_service_event_handler_t event_handler;
} bleActivityServiceInit_t;

// アクティビティサービスのコンテキスト構造体。サービスの実行に必要な情報が含まれる。
struct ble_activity_service_s {
    ble_activity_service_event_handler_t event_handler;     // イベントハンドラ

    ble_uuid_t service_uuid;
    
    uint16_t service_handle;                                // サービスのGATTデータベースのHandle。
    ble_gatts_char_handles_t nine_axis_sensor_char_handle;  // 9軸センサのキャラクタリスティクスのハンドル
    ble_gatts_char_handles_t brightness_char_handle;        // 照度データ
    ble_gatts_char_handles_t temp_humidity_char_handle;     // 温湿度データ
    ble_gatts_char_handles_t pressure_char_handle;          // 圧力データ
    ble_gatts_char_handles_t setting_char_handle;           // 設定
    ble_gatts_char_handles_t control_char_handle;           // コントロール
    
    uint16_t connection_handle;
    uint8_t	 uuid_type;                // ベンダーUUIDを登録した時に取得される、UUIDタイプ

    bool should_notify_nine_axis_sensor;
    bool should_notify_brightness_data;
    bool should_notify_temp_humidity_data;
    bool should_notify_pressure_data;
};

// 初期化関数
// このサービスを使う前に、必ずこの関数を呼び出すこと。
uint32_t bleActivityServiceInit(ble_activity_service_t *p_context, const bleActivityServiceInit_t *p_init);

// BLEイベント通知関数
// BLEのイベントをこの関数に通知します。
void bleActivityServiceOnBLEEvent(ble_activity_service_t *p_context, ble_evt_t * p_ble_evt);

// 通知メソッド
void notifySensorData(ble_activity_service_t *p_context, SensorType_t sensorType, const SensorData_t *p_sensorData);

#endif /* ble_activity_service_h */
