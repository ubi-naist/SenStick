#include <string.h>

#include <ble.h>

#include <app_error.h>
#include <sdk_errors.h>

#include "senstick_meta_data_service.h"
#include "metadata_log_controller.h"

//コンテキスト構造体。
typedef struct senstick_metadata_service_s {
    uint16_t service_handle;
    
    ble_gatts_char_handles_t target_log_id_char_handle;
    ble_gatts_char_handles_t target_datetime_char_handle;
    ble_gatts_char_handles_t target_abstract_char_handle;
    
    uint16_t connection_handle;
    
    uint8_t target_log_id;
} senstick_metadata_service_t;
static senstick_metadata_service_t context;

/**
 * Private methods
 */
static void onWrite(ble_evt_t * p_ble_evt)
{
    ret_code_t err_code;
    
    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // 値を取り出す
    uint8_t buf[GATT_MAX_DATA_LENGTH];
    
    ble_gatts_value_t gatts_value;
    memset(&gatts_value, 0 , sizeof(gatts_value));
    gatts_value.len     = p_evt_write->len;
    gatts_value.offset  = 0;
    gatts_value.p_value = buf;
    err_code = sd_ble_gatts_value_get(context.connection_handle, p_evt_write->handle, &gatts_value);
    APP_ERROR_CHECK(err_code);
    
    // キャラクタリスティクスごとの処理に振り分ける
    if(p_evt_write->handle == context.target_log_id_char_handle.value_handle && gatts_value.len == 1) {
        context.target_log_id = buf[0];
    }
}

static uint8_t onRWAuthReq_datetime_char(uint8_t *p_buffer, uint16_t length)
{
    ble_date_time_t date_time;
    memset(&date_time, 0, sizeof(ble_date_time_t));
    metaDataLogReadDateTime(context.target_log_id, &date_time);
    return ble_date_time_encode(&date_time, p_buffer);
}

static uint8_t onRWAuthReq_abstract_char(uint8_t *p_buffer, uint8_t length)
{
    return metaDataLogReadAbstractText(context.target_log_id, (char *)p_buffer, length);
}

static void onRWAuthReq(ble_evt_t *p_ble_evt)
{
    ret_code_t err_code;
    ble_gatts_evt_rw_authorize_request_t *p_auth_req = &p_ble_evt->evt.gatts_evt.params.authorize_request;
    
    // バッファを用意
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    uint8_t length = 0;
    memset(buffer, 0, sizeof(buffer));
    
    // 実際の読み出し処理
    if(p_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_READ) {
        if( p_auth_req->request.read.handle == context.target_datetime_char_handle.value_handle){
            length = onRWAuthReq_datetime_char(buffer, GATT_MAX_DATA_LENGTH);
        } else if( p_auth_req->request.read.handle == context.target_abstract_char_handle.value_handle){
            length = onRWAuthReq_abstract_char(buffer, GATT_MAX_DATA_LENGTH);
        } else {
            // ハンドラの一致なし、ここで終了
            return;
        }
        
        // リプライを用意
        ble_gatts_rw_authorize_reply_params_t reply_params;
        memset(&reply_params, 0, sizeof(reply_params));
        
        reply_params.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
        reply_params.params.read.gatt_status   = BLE_GATT_STATUS_SUCCESS;
        reply_params.params.read.update        = (length != 0);
        reply_params.params.read.offset        = 0;
        reply_params.params.read.len           = length;
        reply_params.params.read.p_data        = buffer;
        
        // リプライ
        err_code = sd_ble_gatts_rw_authorize_reply(context.connection_handle, &reply_params);
        APP_ERROR_CHECK(err_code);
    }
}

static void addService(uint8_t uuid_type)
{
    ret_code_t err_code;
    ble_add_char_params_t params;
    
    // サービスを登録
    ble_uuid_t uuid;
    uuid.uuid = METADATA_SERVICE_UUID;
    uuid.type = uuid_type;
    err_code  = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &context.service_handle);
    APP_ERROR_CHECK(err_code);
    
    // params初期設定
    memset(&params, 0 , sizeof(params));
    // UUID
    params.uuid_type = uuid_type;
    // 値はスタック側
    params.is_value_user     = false;
    
    // ターゲットログ
    params.uuid              = TARGET_LOG_ID_CHAR_UUID;
    params.max_len           = 1;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(context.service_handle, &params, &context.target_log_id_char_handle);
    APP_ERROR_CHECK(err_code);
    
    // メタ属性:時間
    params.uuid              = TARGET_DATETIME_CHAR_UUID;
    params.max_len           = 7; // ble_date_timeのシリアライズされたバイナリサイズ
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.is_var_len        = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(context.service_handle, &params, &context.target_datetime_char_handle);
    APP_ERROR_CHECK(err_code);
    
    // メタ属性:概要
    params.uuid              = TARGET_ABSTRACT_CHAR_UUID;
    params.max_len           = GATT_MAX_DATA_LENGTH;
    params.is_var_len        = true;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.is_var_len        = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(context.service_handle, &params, &context.target_abstract_char_handle);
    APP_ERROR_CHECK(err_code);
}

/**
 * Public methods
 */
// 初期化します
ret_code_t initSenstickMetaDataService(uint8_t uuid_type)
{
    // サービス構造体を初期化
    memset(&context, 0, sizeof(senstick_metadata_service_t));
    context.connection_handle = BLE_CONN_HANDLE_INVALID;
    
    // サービスを追加
    addService(uuid_type);
    
    return NRF_SUCCESS;
}

// BLEイベントを受け取ります。
void senstickMetaDataService_handleBLEEvent(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            context.connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            context.connection_handle = BLE_CONN_HANDLE_INVALID;
            break;
        case BLE_GATTS_EVT_WRITE:
            onWrite(p_ble_evt);
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            onRWAuthReq(p_ble_evt);
        default:
            break;
    }
}

