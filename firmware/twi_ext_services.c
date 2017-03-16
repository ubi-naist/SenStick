//
//  twi_ext_services.c
//  senstick
//
//  Created by AkihiroUehara on 2017/02/24.
//
//

#include "twi_ext_services.h"

#include "twi_ext_motor.h"

static twi_ext_services_t m_context;

/**
 * Private methods
 */

// セッティング、LOGID、メタデータの読み出し処理
static void onRWAuthReq(twi_ext_services_t *p_context, ble_evt_t *p_ble_evt)
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
        if( p_auth_req->request.read.handle == p_context->twi_ext_motor_fault_status_char_handle.value_handle){
            // フォールトレジスタの読み出し処理。
            buffer[0] = getDRV8830MotorDriverFaultRegister();
            length = 1;
            // faultレジスタの値をクリアします。
            clearDRV8830MotorDriverFaultRegister();
//            length = senstickSensorControllerReadSetting(p_context->device_type, buffer, GATT_MAX_DATA_LENGTH);
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
        if( p_auth_req->request.write.handle == p_context->twi_ext_motor_control_char_handle.value_handle){
            // コントロールレジスタへの書き込み処理
            //p_auth_req->request.write.data
            //p_auth_req->request.write.len
            setDRV8830MotorDriverControlRegister(p_auth_req->request.write.data[0]);
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
    }
}

static void onWrite(twi_ext_services_t *p_context, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // CCCDへの書き込み確認
    if(p_evt_write->len == 2) {
        /*
        if(p_evt_write->handle == p_context->sensor_realtime_data_char_handle.cccd_handle) {
            p_context->is_sensor_realtime_data_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        } else if(p_evt_write->handle == p_context->sensor_log_data_char_handle.cccd_handle) {
            p_context->is_sensor_log_data_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            senstickSensorControllerNotifyLogData(); // ログ通知開始
            return;
        }
         */
    }
}

static void addTwiExtMotorService(twi_ext_services_t *p_context, uint8_t uuid_type)
{
    ret_code_t err_code;
    ble_add_char_params_t params;
    
    // モーターサービスを登録
    ble_uuid_t uuid;
    uuid.uuid = TWI_EXT_MOTOR_SERVICE_UUID;
    uuid.type = uuid_type;
    err_code  = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &(p_context->twi_ext_motor_service_handle));
    APP_ERROR_CHECK(err_code);
    
    // params初期設定
    memset(&params, 0 , sizeof(params));
    params.uuid_type      = uuid_type;
    params.is_value_user  = false;
    
    // 制御レジスタ
    params.uuid              = TWI_EXT_MOTOR_CONTROL_CHAR_UUID;
    params.max_len           = 1;
    params.char_props.read   = false;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.is_var_len        = false;
    params.is_defered_read   = false;
    params.is_defered_write  = true;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->twi_ext_motor_service_handle, &params, &(p_context->twi_ext_motor_control_char_handle));
    APP_ERROR_CHECK(err_code);
    
    // フォールトレジスタ
    params.uuid              = TWI_EXT_MOTOR_FAULT_STATUS_CHAR_UUID;
    params.max_len           = 1;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.is_var_len        = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->twi_ext_motor_service_handle, &params, &(p_context->twi_ext_motor_fault_status_char_handle));
    APP_ERROR_CHECK(err_code);
}

/**
 * Public methods
 */

// 初期化します
ret_code_t initTwiExtService(uint8_t uuid_type)
{
    // サービス構造体を初期化
    memset(&m_context, 0, sizeof(twi_ext_services_t));

    // デフォルト値を設定
    m_context.connection_handle = BLE_CONN_HANDLE_INVALID;
    
    // モータのサービスを追加
    m_context.is_morter_available = initDRV8830MotorDriver();
    if(m_context.is_morter_available) {
        addTwiExtMotorService(&m_context, uuid_type);
    }
    
    return NRF_SUCCESS;
}

// BLEイベントを受け取ります。
void twiExtServices_handleBLEEvent(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            m_context.connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            m_context.connection_handle = BLE_CONN_HANDLE_INVALID;
            // モータをオフに
            if(m_context.is_morter_available) {
                setDRV8830MotorDriverControlRegister((0x2a << 2) | 0x00); // 3.3V, high-z
            }
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            onRWAuthReq(&m_context, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            onWrite(&m_context, p_ble_evt);
            break;
        default:
            break;
    }
}
