//
//  ble_activity_service.c
//  senstick
//
//  Created by AkihiroUehara on 2015/12/17.
//
//

#include <string.h>

#include <nordic_common.h>

#include <ble.h>
#include <ble_gatts.h>

#include <sdk_errors.h>
#include <app_error.h>

#include "ble_activity_service.h"

const ble_uuid128_t ble_activity_base_uuid128 = {
    {
        //DAA6XXXX-5A61-46B3-8192-C0DAD49EBC03
        0x03,0xBC,0x9E,0xD4,0xDA,0xC0
        //
        ,0x92,0x81
        //
        ,0xB3,0x46
        //
        ,0x61,0x5A,0x00,0x00,0xA6,0xDA
    }
};

// 16ビット(ベースUUIDのXX部分)
#define BLE_UUID_SERVICE_SUB                0xe72a
#define BLE_UUID_NINE_AXIS_CHAR_SUB         0xe72b
#define BLE_UUID_BRIGHTNESS_CHAR_SUB        0xe72c
#define BLE_UUID_TEMP_HUMIDITY_CHAR_SUB     0xe72d
#define BLE_UUID_PRESSURE_CHAR_SUB          0xe72e
#define BLE_UUID_SETTING_CHAR_SUB           0xe730
#define BLE_UUID_CONTROL_CHAR_SUB           0xe72f

// キャラクタリスティクスのデータ長
#define BLE_NINE_AXIS_CHAR_VALUE_LENGTH     18
#define BLE_BRIGHTNESS_CHAR_VALUE_LENGTH    6
#define BLE_TEMP_HUMIDITY_CHAR_VALUE_LENGTH 4
#define BLE_PRESSURE_CHAR_VALUE_LENGTH      3

// GATTレイヤでの最大データ長。ATT MTU23で、20。
#define GATT_MAX_DATA_LENGTH 20

/**
 * Private methods
 */

static void on_write(ble_activity_service_t *p_context, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // CCCDへの書き込み確認
    if((p_evt_write->len == 2) && (p_evt_write->handle == p_context->nine_axis_sensor_char_handle.cccd_handle)) {
        p_context->should_notify_nine_axis_sensor = ble_srv_is_notification_enabled(p_evt_write->data);
        return;
    } else if((p_evt_write->len == 2) && (p_evt_write->handle == p_context->brightness_char_handle.cccd_handle)) {
        p_context->should_notify_brightness_data = ble_srv_is_notification_enabled(p_evt_write->data);
        return;
    } else if((p_evt_write->len == 2) && (p_evt_write->handle == p_context->temp_humidity_char_handle.cccd_handle)) {
        p_context->should_notify_temp_humidity_data = ble_srv_is_notification_enabled(p_evt_write->data);
        return;
    } else if((p_evt_write->len == 2) && (p_evt_write->handle == p_context->pressure_char_handle.cccd_handle)) {
        p_context->should_notify_pressure_data = ble_srv_is_notification_enabled(p_evt_write->data);;
        return;
    }
    
    // 書き込まれた値を取り出す。
    ret_code_t err_code;
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    
    ble_gatts_value_t gatts_value;
    memset(&gatts_value, 0 , sizeof(gatts_value));
    gatts_value.len     = MIN(sizeof(buffer), p_evt_write->len); // 長さを制約
    gatts_value.offset  = 0;
    gatts_value.p_value = buffer;
    err_code = sd_ble_gatts_value_get(p_context->connection_handle, p_evt_write->handle, &gatts_value);
    APP_ERROR_CHECK(err_code);
    
    // 設定、制御への書き込みを通知する
    if((p_evt_write->handle == p_context->setting_char_handle.value_handle) || (p_evt_write->handle == p_context->control_char_handle.value_handle)) {
        ble_activity_service_event_t event;
        // イベントタイプを設定。今は2つしかないので3項演算子で設定。
        event.event_type  = (p_evt_write->handle == p_context->setting_char_handle.value_handle) ? BLE_ACTIVITY_SERVICE_SETTING_WRITTEN : BLE_ACTIVITY_SERVICE_CONTROL_WRITTEN;
        event.p_data      = buffer;
        event.data_length = gatts_value.len;
        // イベントハンドラを呼び出す
        (p_context->event_handler)(p_context, &event);
    }
}

static uint32_t add_notification_characteristics(ble_gatts_char_handles_t *p_handle, const ble_activity_service_t *p_context, const uint16_t uuid_sub, const int value_len)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t attribute_md;
    ble_gatts_attr_t    attr_value;
    ble_gatts_attr_md_t cccd_md;
    
    // UUIDを用意
    ble_uuid_t characteristics_uuid;
    characteristics_uuid.type = p_context->uuid_type;
    characteristics_uuid.uuid = uuid_sub;
    
    // クライアント・キャラクタリスティクス・コンフィグレーション・ディスクリプタ メタデータ
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(cccd_md.read_perm));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(cccd_md.write_perm));
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    // ノーティフィケーション・キャラクタリスティクスのメタデータ
    memset(&char_md, 0, sizeof(char_md));
//    char_md.char_props.read     = 1;    // 読み出し、ノーティフィケーションのフラグを立てる
    char_md.char_props.notify   = 1;
    char_md.p_cccd_md           = &cccd_md; // CCCDのメタデータを設定する
    
    memset(&attribute_md, 0, sizeof(attribute_md));
    attribute_md.vloc = BLE_GATTS_VLOC_STACK;  // Attributeの値のメモリ領域は、スタックに管理させる
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(attribute_md.read_perm)); // 書き込みおよび読み出しパーミションは、オープン。
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(attribute_md.write_perm));
    
    memset(&attr_value, 0, sizeof(attr_value));
    attr_value.p_uuid    = &characteristics_uuid;
    attr_value.p_attr_md = &attribute_md;
    attr_value.max_len   = value_len;

    return sd_ble_gatts_characteristic_add(p_context->service_handle, &char_md, &attr_value, p_handle);
}

static uint32_t add_write_characteristics(ble_gatts_char_handles_t *p_handle, const ble_activity_service_t *p_context, const uint16_t uuid_sub, const int value_len)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t attribute_md;
    ble_gatts_attr_t    attr_value;
    
    // UUIDを用意
    ble_uuid_t characteristics_uuid;
    characteristics_uuid.type = p_context->uuid_type;
    characteristics_uuid.uuid = uuid_sub;
    
    // ノーティフィケーション・キャラクタリスティクスのメタデータ
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write = 1;
    
    memset(&attribute_md, 0, sizeof(attribute_md));
    attribute_md.vloc = BLE_GATTS_VLOC_STACK;  // Attributeの値のメモリ領域は、スタックに管理させる
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(attribute_md.write_perm));
    
    memset(&attr_value, 0, sizeof(attr_value));
    attr_value.p_uuid    = &characteristics_uuid;
    attr_value.p_attr_md = &attribute_md;
    attr_value.max_len   = value_len;
    
    return sd_ble_gatts_characteristic_add(p_context->service_handle, &char_md, &attr_value, p_handle);
}

static void add_characteristics(ble_activity_service_t *p_context)
{
    ret_code_t err_code;
    
    err_code = add_notification_characteristics(&(p_context->nine_axis_sensor_char_handle), p_context, BLE_UUID_NINE_AXIS_CHAR_SUB, BLE_NINE_AXIS_CHAR_VALUE_LENGTH);
    APP_ERROR_CHECK(err_code);
    
    err_code = add_notification_characteristics(&(p_context->brightness_char_handle), p_context, BLE_UUID_BRIGHTNESS_CHAR_SUB, BLE_BRIGHTNESS_CHAR_VALUE_LENGTH);
    APP_ERROR_CHECK(err_code);
    
    err_code = add_notification_characteristics(&(p_context->temp_humidity_char_handle), p_context, BLE_UUID_TEMP_HUMIDITY_CHAR_SUB, BLE_TEMP_HUMIDITY_CHAR_VALUE_LENGTH);
    APP_ERROR_CHECK(err_code);
    
    err_code = add_notification_characteristics(&(p_context->pressure_char_handle), p_context, BLE_UUID_PRESSURE_CHAR_SUB, BLE_PRESSURE_CHAR_VALUE_LENGTH);
    APP_ERROR_CHECK(err_code);

    err_code = add_write_characteristics(&(p_context->setting_char_handle), p_context, BLE_UUID_SETTING_CHAR_SUB, 1);
    APP_ERROR_CHECK(err_code);

    err_code = add_write_characteristics(&(p_context->control_char_handle), p_context, BLE_UUID_CONTROL_CHAR_SUB, 1);
    APP_ERROR_CHECK(err_code);
}

/**
 * Public methods
 */

uint32_t ble_activity_service_init(ble_activity_service_t *p_context, const ble_activity_service_init_t *p_init)
{
    if(p_context == NULL || p_init == NULL) {
        return NRF_ERROR_NULL;
    }
    
    uint32_t   err_code;
    
    // サービス構造体を初期化
    memset(p_context, 0, sizeof(ble_activity_service_t));

    p_context->event_handler     = p_init->event_handler;
    p_context->connection_handle = BLE_CONN_HANDLE_INVALID;
    
    // ベースUUIDを登録
    err_code = sd_ble_uuid_vs_add(&ble_activity_base_uuid128, &(p_context->uuid_type));
    APP_ERROR_CHECK(err_code);

    // サービスを登録
    ble_uuid_t service_uuid;
    service_uuid.uuid = BLE_UUID_SERVICE_SUB;
    service_uuid.type = p_context->uuid_type;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &(p_context->service_handle));
    APP_ERROR_CHECK(err_code);
    
    // UUIDをコピー
    BLE_UUID_COPY_INST(p_context->service_uuid, service_uuid);
    
    // キャラクタリスティクスを追加
    add_characteristics(p_context);
    
    return NRF_SUCCESS;
}

void ble_activity_service_on_ble_event(ble_activity_service_t *p_context, ble_evt_t * p_ble_evt)
{
    if( p_context == NULL || p_ble_evt == NULL) {
        return;
    }
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            p_context->connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            p_context->connection_handle = BLE_CONN_HANDLE_INVALID;
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_context, p_ble_evt);
            break;
        default:
            break;
    }
}
