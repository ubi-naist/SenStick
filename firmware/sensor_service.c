
#include "sensor_service.h"
#include "senstick_sensor_controller.h"
/**
 * Private methods
 */

// セッティング、LOGID、メタデータの読み出し処理
static void onRWAuthReq(sensor_service_t *p_context, ble_evt_t *p_ble_evt)
{
    ret_code_t err_code;
    ble_gatts_evt_rw_authorize_request_t *p_auth_req = &p_ble_evt->evt.gatts_evt.params.authorize_request;
    
    // バッファを用意
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    uint8_t length = 0;
    memset(buffer, 0, sizeof(buffer));
    
    // リプライを用意
    ble_gatts_rw_authorize_reply_params_t reply_params;
    memset(&reply_params, 0, sizeof(reply_params));
    
    // 実際の読み出し処理
    if(p_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_READ) {
        if( p_auth_req->request.read.handle == p_context->sensor_setting_char_handle.value_handle){
            length = senstickSensorControllerReadSetting(p_context->device_type, buffer, GATT_MAX_DATA_LENGTH);
        } else if( p_auth_req->request.read.handle == p_context->sensor_log_metadata_char_handle.value_handle){
            length = senstickSensorControllerReadMetaData(p_context->device_type, buffer, GATT_MAX_DATA_LENGTH);
        } else {
            // 一致しないハンドラ
            return;
        }
        // リプライ
        reply_params.type                    = BLE_GATTS_AUTHORIZE_TYPE_READ;
        reply_params.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
        reply_params.params.read.update      = (length != 0);
        reply_params.params.read.len         = length;
        reply_params.params.read.offset      = 0;
        reply_params.params.read.p_data      = buffer;
        
        err_code = sd_ble_gatts_rw_authorize_reply(p_context->connection_handle, &reply_params);
        APP_ERROR_CHECK(err_code);
        
    } else if(p_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
        bool result = true;
        if( p_auth_req->request.write.handle == p_context->sensor_setting_char_handle.value_handle){
            result = senstickSensorControllerWriteSetting(p_context->device_type, p_auth_req->request.write.data, p_auth_req->request.write.len);
        } else if( p_auth_req->request.write.handle == p_context->sensor_logid_char_handle.value_handle) {
            senstickSensorControllerWriteLogID(p_context->device_type, p_auth_req->request.write.data, p_auth_req->request.write.len);
        } else {
            // 一致しないハンドラ
            return;
        }
        // リプライ
                // nRf51ではリプライのフィールドには、gatt_statusのみがある。SDK12 S132v3では、gatt_status, update, offset, len, p_dataの5フィールド。
        reply_params.type                      = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        // iOSアプリ側でBLEのアクセスでAUTH_ERRORがあると、一連の処理が破棄されるのか?、みたいな振る舞い。読み出せるべき値が読み出せないとか。なので、ここはスルー。
        reply_params.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
#ifdef NRF52
        // このパラメータが1になっていなければ、invalid parameterでsd_ble_gatts_rw_authorize_reply()が落ちる。
        reply_params.params.write.update = 1;
#endif
//        reply_params.params.read.gatt_status   = result ? BLE_GATT_STATUS_SUCCESS : BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
        err_code = sd_ble_gatts_rw_authorize_reply(p_context->connection_handle, &reply_params);
        APP_ERROR_CHECK(err_code);
        
        senstickSensorControllerNotifyLogData();
    }
}

static void onWrite(sensor_service_t *p_context, ble_evt_t * p_ble_evt)
{
    
    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // CCCDへの書き込み確認
    if(p_evt_write->len == 2) {
        if(p_evt_write->handle == p_context->sensor_realtime_data_char_handle.cccd_handle) {
            p_context->is_sensor_realtime_data_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        } else if(p_evt_write->handle == p_context->sensor_log_data_char_handle.cccd_handle) {
            p_context->is_sensor_log_data_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            senstickSensorControllerNotifyLogData(); // ログ通知開始
            return;
        }
    }
    
}

static void addService(sensor_service_t *p_context, uint8_t uuid_type)
{
    ret_code_t err_code;
    ble_add_char_params_t params;
    
    // サービスを登録
    ble_uuid_t uuid;
    uuid.uuid = SENSOR_SERVICE_UUID + (uint16_t)p_context->device_type;
    uuid.type = uuid_type;
    err_code  = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &(p_context->service_handle));
    APP_ERROR_CHECK(err_code);
    
    // params初期設定
    memset(&params, 0 , sizeof(params));
    params.uuid_type      = uuid_type;
    params.is_value_user  = false;
    
    // セッティング
    params.uuid              = SENSOR_SETTING_CHAR_UUID + (uint16_t)p_context->device_type;
    params.max_len           = 5;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.is_var_len        = false;
    params.is_defered_read   = true;
    params.is_defered_write  = true;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->service_handle, &params, &(p_context->sensor_setting_char_handle));
    APP_ERROR_CHECK(err_code);
    
    // リアルタイムデータ
    params.uuid              = SENSOR_REALTIME_DATA_CHAR_UUID + (uint16_t)p_context->device_type;
    params.max_len           = GATT_MAX_DATA_LENGTH;
    params.char_props.read   = false;
    params.char_props.write  = false;
    params.char_props.notify = true;
    params.is_var_len        = true;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.read_access       = SEC_NO_ACCESS;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_OPEN;
    err_code = characteristic_add(p_context->service_handle, &params, &(p_context->sensor_realtime_data_char_handle));
    APP_ERROR_CHECK(err_code);
    
    // LOGID
    params.uuid              = SENSOR_LOGID_CHAR_UUID + (uint16_t)p_context->device_type;
    params.max_len           = 7;
    params.char_props.read   = false;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.is_var_len        = false;
    params.is_defered_read   = false;
    params.is_defered_write  = true;
    params.read_access       = SEC_NO_ACCESS;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->service_handle, &params, &(p_context->sensor_logid_char_handle));
    APP_ERROR_CHECK(err_code);
    
    // ログデータ
    params.uuid              = SENSOR_LOG_DATA_CHAR_UUID + (uint16_t)p_context->device_type;
    params.max_len           = GATT_MAX_DATA_LENGTH;
    params.char_props.read   = false;
    params.char_props.write  = false;
    params.char_props.notify = true;
    params.is_var_len        = true;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.read_access       = SEC_NO_ACCESS;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_OPEN;
    err_code = characteristic_add(p_context->service_handle, &params, &(p_context->sensor_log_data_char_handle));
    APP_ERROR_CHECK(err_code);
    
    // メタデータ
    params.uuid              = SENSOR_METADATA_CHAR_UUID + (uint16_t)p_context->device_type;
    params.max_len           = 17;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.is_var_len        = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->service_handle, &params, &(p_context->sensor_log_metadata_char_handle));
    APP_ERROR_CHECK(err_code);
}

/**
 * Public methods
 */

// 初期化します
ret_code_t initSensorService(sensor_service_t *p_context, uint8_t uuid_type, sensor_device_t deviceType)
{
    // サービス構造体を初期化
    memset(p_context, 0, sizeof(sensor_service_t));
    // デフォルト値を設定
    p_context->connection_handle = BLE_CONN_HANDLE_INVALID;
    p_context->device_type       = deviceType;
    
    // サービスを追加
    addService(p_context, uuid_type);
    
    return NRF_SUCCESS;
}

// BLEイベントを受け取ります。
void sensorService_handleBLEEvent(sensor_service_t *p_context, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            p_context->connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            p_context->connection_handle = BLE_CONN_HANDLE_INVALID;
            p_context->is_sensor_realtime_data_notifying = false;
            p_context->is_sensor_log_data_notifying      = false;
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            onRWAuthReq(p_context, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            onWrite(p_context, p_ble_evt);
            break;
        default:
            break;
    }
}

void sensorServiceNotifyRealtimeData(sensor_service_t *p_context, uint8_t *p_data, uint16_t length)
{
    if( ! p_context->is_sensor_realtime_data_notifying) {
        return;
    }

    ble_gatts_hvx_params_t hvx_params;
    
    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_context->sensor_realtime_data_char_handle.value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &length;
    hvx_params.p_data = p_data;
    
//    ret_code_t err_code =
    sd_ble_gatts_hvx(p_context->connection_handle, &hvx_params);
}

// ログデータをNotifyします。失敗したらfalseを返します。
bool sensorServiceNotifyLogData(sensor_service_t *p_context, uint8_t *p_data, uint16_t length)
{
    if( ! p_context->is_sensor_log_data_notifying) {
        return false;
    }
    
    ble_gatts_hvx_params_t hvx_params;
    
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_context->sensor_log_data_char_handle.value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &length;
    hvx_params.p_data = p_data;
    
    ret_code_t err_code = sd_ble_gatts_hvx(p_context->connection_handle, &hvx_params);
    
    return (err_code == NRF_SUCCESS);
}
