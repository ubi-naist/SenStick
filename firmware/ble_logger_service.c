#include "ble_logger_service.h"
#include <string.h>

#include <nordic_common.h>

#include <ble.h>
#include <ble_gatts.h>
#include <ble_srv_common.h>

#include <sdk_errors.h>
#include <app_error.h>

#include "ble_logger_service.h"

const ble_uuid128_t ble_logger_base_uuid128 = {
    {
        //F000XXXX-0451-4000-B000-000000000000
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xB0,
        0x00, 0x40,
        0x51, 0x04,
        0x00, 0x00, 0x00, 0xF0
    }
};

/**
 * Private methods
 */


static void onWrite(ble_logger_service_t *p_context, ble_evt_t * p_ble_evt)
{
//    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
}

static void onDisconnect(ble_logger_service_t *p_context, ble_evt_t * p_ble_evt)
{

}

static void addServices(ble_logger_service_t *p_context)
{
    ret_code_t err_code;
    ble_add_char_params_t params;
    
    // 書き込みサービスを登録
    ble_uuid_t uuid;
    uuid.uuid = 0xbb00;
    uuid.type = p_context->uuid_type;
    err_code  = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &(p_context->writing_service_handle));
    APP_ERROR_CHECK(err_code);

    // params初期設定
    memset(&params, 0 , sizeof(params));
    // UUID
    params.uuid_type = p_context->uuid_type;
    // 値の長さ
    params.is_value_user = false;
    // 初期値
    //params.init_len = 0;
    //params.p_init_value = NULL;
    // 認証
    params.is_defered_read  = false;
    params.is_defered_write = false;
    
    // 書き込みコントロールポイント
    params.uuid         = 0xbb01;
    params.max_len      = 1;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_OPEN;
    err_code = characteristic_add(p_context->writing_service_handle, &params, &p_context->writing_control_point_char_handles);
    APP_ERROR_CHECK(err_code);

    // ステータス
    params.uuid         = 0xbb02;
    params.max_len      = 8;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->writing_service_handle, &params, &p_context->storage_status_char_handles);
    APP_ERROR_CHECK(err_code);

    // RTC
    params.uuid         = 0xbb03;
    params.max_len      = 7;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->writing_service_handle, &params, &p_context->rtc_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // アブストラクト
    params.uuid         = 0xbb04;
    params.max_len      = 20;
    params.is_var_len   = true;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->writing_service_handle, &params, &p_context->writing_abstract_char_handles);
    APP_ERROR_CHECK(err_code);

    
    // 読み出しのサービスを登録
    uuid.uuid = 0xcc00;
    uuid.type = p_context->uuid_type;
    err_code  = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &(p_context->reading_service_handle));
    APP_ERROR_CHECK(err_code);
    
    // 読み出し対象
    params.uuid         = 0xcc01;
    params.max_len      = 2;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_target_char_handles);
    APP_ERROR_CHECK(err_code);

    // 読み出しコントロールポイント
    params.uuid         = 0xcc02;
    params.max_len      = 3;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_control_point_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // ステータス
    params.uuid         = 0xcc03;
    params.max_len      = 8;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_status_char_handles);
    APP_ERROR_CHECK(err_code);

    // データポイント
    params.uuid         = 0xcc04;
    params.max_len      = 20;
    params.is_var_len   = true;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_data_point_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // サンプリングなどセンサ設定
    params.uuid         = 0xcc05;
    params.max_len      = 16;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->meta_data_sensor_setting_char_handles);
    APP_ERROR_CHECK(err_code);

    // 時間
    params.uuid         = 0xcc06;
    params.max_len      = 7;
    params.is_var_len   = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->meta_data_datetime_char_handles);
    APP_ERROR_CHECK(err_code);

    // アブストラクト
    params.uuid         = 0xcc07;
    params.max_len      = 20;
    params.is_var_len   = true;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->meta_data_abstract_char_handles);
    APP_ERROR_CHECK(err_code);
}

/**
 * Public methods
 */

uint32_t bleLoggerServiceInit(ble_logger_service_t *p_context, ble_logger_service_event_handler_t logger_event_handler)
{
    if( logger_event_handler == NULL) {
        return NRF_ERROR_NULL;
    }
    
    uint32_t   err_code;
    
    // サービス構造体を初期化
    memset(p_context, 0, sizeof(ble_logger_service_t));
    
    p_context->event_handler     = logger_event_handler;
    p_context->connection_handle = BLE_CONN_HANDLE_INVALID;
    
    // ベースUUIDを登録
    err_code = sd_ble_uuid_vs_add(&ble_logger_base_uuid128, &(p_context->uuid_type));
    APP_ERROR_CHECK(err_code);
    
    // サービスを追加
    addServices(p_context);
    
    return NRF_SUCCESS;
}


void bleLoggerServiceOnBleEvent(ble_logger_service_t *p_context, ble_evt_t * p_ble_evt)
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
            onDisconnect(p_context, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            onWrite(p_context, p_ble_evt);
            break;
        default:
            break;
    }
}

void bleLoggerServiceStartLogging(ble_logger_service_t *p_context, const sensorSetting_t *p_setting)
{}

void bleLoggerServiceStopLogging(ble_logger_service_t *p_context)
{}

void bleLoggerServiceWrite(ble_logger_service_t *p_context, SensorDeviceType_t sensorType, const SensorData_t *p_sensorData)
{
}

