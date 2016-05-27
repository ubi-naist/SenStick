#include <string.h>

#include <nordic_common.h>
#include <ble_srv_common.h>
#include <nrf_log.h>
#include <nrf_assert.h>

#include <ble.h>
#include <ble_gatts.h>

#include <sdk_errors.h>
#include <app_error.h>

#include "senstick_control_service.h"
#include "senstick_data_model.h"

//コンテキスト構造体。
typedef struct senstick_control_service_s {
    uint16_t service_handle;
    
    ble_gatts_char_handles_t control_point_char_handle;
    ble_gatts_char_handles_t available_log_count_char_handle;
    ble_gatts_char_handles_t storage_status_char_handle;
    ble_gatts_char_handles_t rtc_char_handle;
    ble_gatts_char_handles_t abstract_text_char_handle;
    
    uint16_t connection_handle;
} senstick_control_service_t;
static senstick_control_service_t context;

/**
 * Private methods
 */
static void onWriteRTC(ble_gatts_value_t *p_gatts_value)
{
    ble_date_time_t date_time;
    uint8_t decode_len = ble_date_time_decode(&date_time, p_gatts_value->p_value);
    if(decode_len == p_gatts_value->len) {
        senstick_setCurrentDateTime(&date_time);
    }
}

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
    if(p_evt_write->handle == context.control_point_char_handle.value_handle) {
        
        senstick_control_command_t currentValue = senstick_getControlCommand();
        if(currentValue != (senstick_control_command_t)gatts_value.p_value[0]) {
            senstick_setControlCommand((senstick_control_command_t)gatts_value.p_value[0]);
        }
    } else if(p_evt_write->handle == context.rtc_char_handle.value_handle) {
        onWriteRTC(&gatts_value);
    } else if(p_evt_write->handle == context.abstract_text_char_handle.value_handle) {
        senstick_setCurrentLogAbstractText((char *)gatts_value.p_value, gatts_value.len);
    }
}

static uint8_t onRWAuthReq_rtc_char(uint8_t *p_buffer, uint16_t length)
{
    ASSERT(length >= 7);

    ble_date_time_t date_time;
    senstick_getCurrentDateTime(&date_time);
    return ble_date_time_encode(&date_time, p_buffer);
}

static uint8_t onRWAuthReq_abstract_txt(uint8_t *p_buffer, uint8_t length)
{
    return senstick_getCurrentLogAbstractText((char *)p_buffer, length);
}

static void onRWAuthReq(ble_evt_t *p_ble_evt)
{
    ret_code_t err_code;
    ble_gatts_evt_rw_authorize_request_t *p_auth_req = &p_ble_evt->evt.gatts_evt.params.authorize_request;
    
    // バッファを用意
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    uint8_t length = 0;
    memset(buffer, 0, sizeof(buffer));
    
    // 実際の読み出し処理, ハンドラが一致しない場合は、終了。
    if(p_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_READ) {
        if( p_auth_req->request.read.handle == context.rtc_char_handle.value_handle){
            length = onRWAuthReq_rtc_char(buffer, GATT_MAX_DATA_LENGTH);
        } else if( p_auth_req->request.read.handle == context.abstract_text_char_handle.value_handle){
            length = onRWAuthReq_abstract_txt(buffer, GATT_MAX_DATA_LENGTH);
        } else {
            return; // ハンドラが一致しない、ここで終了。
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
    uuid.uuid = CONTROL_SERVICE_UUID;
    uuid.type = uuid_type;
    err_code  = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &context.service_handle);
    APP_ERROR_CHECK(err_code);
    
    // params初期設定
    memset(&params, 0 , sizeof(params));
    // UUID
    params.uuid_type = uuid_type;
    // 値はスタック側
    params.is_value_user     = false;
    
    // コントロールポイント
    params.uuid              = CONTROL_POINT_CHAR_UUID;
    params.max_len           = 1;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = true;
    params.is_var_len        = false;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_OPEN;
    err_code = characteristic_add(context.service_handle, &params, &context.control_point_char_handle);
    APP_ERROR_CHECK(err_code);
    
    // ストレージステータス
    params.uuid              = STORAGE_STATUS_CHAR_UUID;
    params.max_len           = 1;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = true;
    params.is_var_len        = false;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_OPEN;
    err_code = characteristic_add(context.service_handle, &params, &context.storage_status_char_handle);
    APP_ERROR_CHECK(err_code);
    
    // 有効ログの数
    params.uuid              = AVAILABLE_LOG_COUNT_CHAR_UUID;
    params.max_len           = 1;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = true;
    params.is_var_len        = false;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_OPEN;
    err_code = characteristic_add(context.service_handle, &params, &context.available_log_count_char_handle);
    APP_ERROR_CHECK(err_code);
    
    // メタ属性:時間
    params.uuid              = CONTROL_RTC_CHAR_UUID;
    params.max_len           = 7;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.is_var_len        = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(context.service_handle, &params, &context.rtc_char_handle);
    APP_ERROR_CHECK(err_code);
    
    // メタ属性:概要
    params.uuid              = CONTROL_ABSTRACT_TEXT_CHAR_UUID;
    params.max_len           = 20;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.is_var_len        = true;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(context.service_handle, &params, &context.abstract_text_char_handle);
    APP_ERROR_CHECK(err_code);
}

/**
 * Public methods
 */
// 初期化します
uint32_t initSenstickControlService(uint8_t uuid_type)
{        
    // サービス構造体を初期化
    memset(&context, 0, sizeof(senstick_control_service_t));
    context.connection_handle = BLE_CONN_HANDLE_INVALID;
    
    // サービスを追加
    addService(uuid_type);
    
    return NRF_SUCCESS;
}

// BLEイベントを受け取ります。
void senstickControlService_handleBLEEvent(ble_evt_t * p_ble_evt)
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


static bool is_notification_enabled(uint16_t connection_handle, uint16_t cccd_handle)
{
     uint32_t err_code;
    uint8_t  cccd_value_buf[BLE_CCCD_VALUE_LEN];
    ble_gatts_value_t gatts_value;
 
    bool result = false;
 
    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));
 
    gatts_value.len     = BLE_CCCD_VALUE_LEN;
    gatts_value.offset  = 0;
    gatts_value.p_value = cccd_value_buf;
 
     err_code = sd_ble_gatts_value_get(connection_handle, cccd_handle, &gatts_value);
     if (err_code == NRF_SUCCESS) {
         result = ble_srv_is_notification_enabled(cccd_value_buf);
     }
    return result;
 }
 
static void setValueAndNotify(uint16_t connection_handle, uint16_t value_handle, uint16_t cccd_handle, uint8_t *p_data, uint16_t length)
{
     setCharacteristicsValue(connection_handle, value_handle, p_data, length);
    if( is_notification_enabled(connection_handle, cccd_handle) ) {
        notifyToClient(connection_handle, value_handle, p_data, length);
    }
}
void senstickControlService_observeControlCommand(senstick_control_command_t command)
{
    setValueAndNotify(context.connection_handle,
                                     context.control_point_char_handle.value_handle,
                                     context.control_point_char_handle.cccd_handle,
                                     &command, sizeof(uint8_t));
}

void senstickControlService_observeCurrentLogCount(uint8_t count)
{
    setValueAndNotify(context.connection_handle,
                                     context.available_log_count_char_handle.value_handle,
                                     context.available_log_count_char_handle.cccd_handle,
                                     &count, sizeof(uint8_t));
}

void senstickControlService_observeDiskFull(bool flag)
{
    uint8_t val = (uint8_t)flag;
    setValueAndNotify(context.connection_handle,
                                     context.storage_status_char_handle.value_handle,
                                     context.storage_status_char_handle.cccd_handle,
                                     &val, sizeof(uint8_t));
}
