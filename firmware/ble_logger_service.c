#include "ble_logger_service.h"
#include <string.h>

#include <nordic_common.h>
#include <nrf_log.h>
#include <nrf_assert.h>

#include <ble.h>
#include <ble_gatts.h>
#include <ble_srv_common.h>

#include <sdk_errors.h>
#include <app_error.h>

#include "ble_logger_service.h"

#define GATT_MAX_DATA_LENGTH 20

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


// 書き込みサービスの、コントロールポイントの読み出しをサポートします。
void process_writing_control_point_read_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t *p_length)
{
    *p_length = 3;
    // 書込しているか否かは、p_writingのポインタがNULLか否かで判定可能
    buffer[0] = (p_context->p_writing != NULL);
}
void process_writing_control_point_write_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t length)
{
    if(buffer[0] == 0x01) {
        bleLoggerServiceStartLogging(p_context);
    }
    if(buffer[1] == 0x01) {
        bleLoggerServiceFormatStorage(p_context);
    }
    if(buffer[2] == 0x01) {
        // DFU開始を要求, ロギングを停止してから。
        bleLoggerServiceStopLogging(p_context);
        (p_context->event_handler)(p_context, REQUEST_DFU);
    }
}

// ストレージのステータス読み出し
void process_storage_status_read_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t *p_length)
{
    *p_length = 1 + NUM_OF_SENSOR_DEVICES;
    // ストレージのステータスを読みだし
    storageGetRemainingCapacity(&(p_context->flash_stream), &(buffer[0]), &(buffer[1]));
}

// RTCの設定
void process_rtc_write_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t length)
{
    ble_date_time_t date;
    ble_date_time_decode(&date, buffer);
    setRTCDateTime(p_context->p_sensor_manager_context, &date);
}
void process_rtc_read_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t *p_length)
{
    *p_length = 7;
    
    ble_date_time_t date;
    getRTCDateTime(p_context->p_sensor_manager_context, &date);
    ble_date_time_encode(&date, buffer);
}

// 読み出しターゲットの更新
void process_reading_target_write_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t length)
{
    // ターゲットのデータのIDをチェック
    // メタ情報のキャラクタリスティクスの値を更新
    // サンプリング設定、日時、概要
}

void process_reading_status_read_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t *p_length)
{
    // 現在のターゲットの、サンプル数および読み出し位置の読み出し対応
    *p_length = 2;
}

void process_reading_control_point_write_request(ble_logger_service_t *p_context, uint8_t *buffer, uint8_t length)
{
    // 読み出し動作を設定。もしものcccdが立っているなら、notificationをキック。
}

// on the flyでの読み出し処理
static void on_rw_auth_req(ble_logger_service_t *p_context, ble_evt_t *p_ble_evt)
{
    ret_code_t err_code;
    ble_gatts_evt_rw_authorize_request_t *p_auth_req = &p_ble_evt->evt.gatts_evt.params.authorize_request;

    // バッファを用意
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    uint8_t length = 0;
    memset(buffer, 0, sizeof(buffer));
    
    ble_gatts_rw_authorize_reply_params_t reply_params;
    memset(&reply_params, 0, sizeof(reply_params));
    
    if(p_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_READ) {
        // ハンドルで判別して読み出し処理
        if( p_auth_req->request.read.handle == p_context->writing_control_point_char_handles.value_handle) {
            process_writing_control_point_read_request(p_context, buffer, &length);
        } else if ( p_auth_req->request.read.handle == p_context->storage_status_char_handles.value_handle) {
            process_storage_status_read_request(p_context, buffer, &length);
        } else if ( p_auth_req->request.read.handle == p_context->rtc_char_handles.value_handle) {
            process_rtc_read_request(p_context, buffer, &length);
        } else if ( p_auth_req->request.read.handle == p_context->reading_status_char_handles.value_handle) {
            process_reading_status_read_request(p_context, buffer, &length);
        } else {
//            NRF_LOG_PRINTF_DEBUG("on_rw_auth_req(), unexpected handler 0x%04x.\n", p_auth_req->request.read.handle);
        }
        
        reply_params.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
        reply_params.params.read.gatt_status   = BLE_GATT_STATUS_SUCCESS;
        reply_params.params.read.update        = (length != 0);
        reply_params.params.read.offset        = 0;
        reply_params.params.read.len           = length;
        reply_params.params.read.p_data        = buffer;
    } else {
        reply_params.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        reply_params.params.write.gatt_status = BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
        NRF_LOG_PRINTF_DEBUG("on_rw_auth_req(), auth write\n");
    }
    
    err_code = sd_ble_gatts_rw_authorize_reply(p_context->connection_handle, &reply_params);
    APP_ERROR_CHECK(err_code);
}

static void onWrite(ble_logger_service_t *p_context, ble_evt_t * p_ble_evt)
{
    ret_code_t err_code;
    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    uint8_t buffer[GATT_MAX_DATA_LENGTH];
    uint8_t length = (uint8_t) MIN(GATT_MAX_DATA_LENGTH, p_evt_write->len); // 長さを指定値に制約
    
    ble_gatts_value_t gatts_value;
    memset(&gatts_value, 0, sizeof(gatts_value));
    gatts_value.len     = length;
    gatts_value.offset  = 0;
    gatts_value.p_value = buffer;
    err_code = sd_ble_gatts_value_get(p_context->connection_handle,
                                      p_evt_write->handle,
                                      &gatts_value);
    APP_ERROR_CHECK(err_code);

    // value handle への書き込み
    if(p_evt_write->handle == p_context->writing_control_point_char_handles.value_handle) {
        process_writing_control_point_write_request(p_context, buffer, length);
    } else if ( p_evt_write->handle == p_context->rtc_char_handles.value_handle) {
        process_rtc_write_request(p_context, buffer, length);
    } else if (p_evt_write->handle == p_context->reading_target_char_handles.value_handle) {
        process_reading_target_write_request(p_context, buffer, length);
    } else if (p_evt_write->handle == p_context->reading_control_point_char_handles.value_handle) {
        process_reading_control_point_write_request(p_context, buffer, length);
    } else  {
//        NRF_LOG_PRINTF_DEBUG("onWrite(), unexpected handler 0x%02x.\n", p_evt_write->handle);
    }
        /*
        hoge()
    } else if((p_evt_write->handle == p_context->notification_characteristic_handle.cccd_handle) && (p_evt_write->len == 2)) {
        // cccd書き込み
        p_context->is_notification_enabled = ble_srv_is_notification_enabled(p_evt_write->data);
    }
    else*/
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
    // 値はスタック側
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
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = true;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_OPEN;
    err_code = characteristic_add(p_context->writing_service_handle, &params, &p_context->writing_control_point_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // ステータス
    params.uuid         = 0xbb02;
    params.max_len      = 8;
    params.is_var_len   = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->writing_service_handle, &params, &p_context->storage_status_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // RTC
    params.uuid         = 0xbb03;
    params.max_len      = 7;
    params.is_var_len   = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->writing_service_handle, &params, &p_context->rtc_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // アブストラクト
    params.uuid         = 0xbb04;
    params.max_len      = 20;
    params.is_var_len   = true;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
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
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_target_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // 読み出しコントロールポイント
    params.uuid         = 0xcc02;
    params.max_len      = 3;
    params.is_var_len   = false;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = true;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_OPEN;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_control_point_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // ステータス
    params.uuid         = 0xcc03;
    params.max_len      = 8;
    params.is_var_len   = false;
    params.is_defered_read   = true;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_status_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // データポイント
    params.uuid         = 0xcc04;
    params.max_len      = 20;
    params.is_var_len   = true;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->reading_data_point_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // サンプリングなどセンサ設定
    params.uuid         = 0xcc05;
    params.max_len      = 16;
    params.is_var_len   = false;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->meta_data_sensor_setting_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // 時間
    params.uuid         = 0xcc06;
    params.max_len      = 7;
    params.is_var_len   = false;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->meta_data_datetime_char_handles);
    APP_ERROR_CHECK(err_code);
    
    // アブストラクト
    params.uuid         = 0xcc07;
    params.max_len      = 20;
    params.is_var_len   = true;
    params.is_defered_read   = false;
    params.is_defered_write  = false;
    params.char_props.read   = true;
    params.char_props.write  = false;
    params.char_props.notify = false;
    params.read_access       = SEC_OPEN;
    params.write_access      = SEC_NO_ACCESS;
    params.cccd_write_access = SEC_NO_ACCESS;
    err_code = characteristic_add(p_context->reading_service_handle, &params, &p_context->meta_data_abstract_char_handles);
    APP_ERROR_CHECK(err_code);
}

/**
 * Public methods
 */

uint32_t bleLoggerServiceInit(ble_logger_service_t *p_context, sensor_manager_t *p_sensor_manager_context, ble_logger_service_event_handler_t logger_event_handler)
{
    if( logger_event_handler == NULL) {
        return NRF_ERROR_NULL;
    }
    
    uint32_t   err_code;
    
    // サービス構造体を初期化
    memset(p_context, 0, sizeof(ble_logger_service_t));

    p_context->p_sensor_manager_context = p_sensor_manager_context;
    p_context->event_handler            = logger_event_handler;
    p_context->connection_handle        = BLE_CONN_HANDLE_INVALID;
    
    // ベースUUIDを登録
    err_code = sd_ble_uuid_vs_add(&ble_logger_base_uuid128, &(p_context->uuid_type));
    APP_ERROR_CHECK(err_code);
    
    // サービスを追加
    addServices(p_context);
    
    // ストリームを開く
    initFlashStream(&(p_context->flash_stream));
    
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
        case BLE_EVT_TX_COMPLETE:
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            on_rw_auth_req(p_context, p_ble_evt);
        default:
            break;
    }
}

// センサー設定。ロギングを開始する前に、必ずこれを呼び出します。
void bleLoggerSetSensorSetting(ble_logger_service_t *p_context, sensorSetting_t *p_setting)
{
    p_context->p_setting = p_setting;
}

void bleLoggerServiceStartLogging(ble_logger_service_t *p_context)
{
    // 現在読みだしているストリームが、書き込み途中のストリームであるなら、割当バッファを切り替える
    bool isReadingRecordingData = (p_context->p_writing != NULL) && (p_context->p_writing == p_context->p_reading);
    if(isReadingRecordingData) {
        p_context->writing_storage_index = (p_context->writing_storage_index +1 ) % 2;
    }
    
    // 今のロギングを停止する
    bleLoggerServiceStopLogging(p_context);
    
    // 現在日時を取得
    ble_date_time_t date;
    getRTCDateTime(p_context->p_sensor_manager_context, &date);
    
    // ストレージを割り当てる
    p_context->p_writing = &(p_context->writing_storage[p_context->writing_storage_index]);
    // 新しいストリームを開く
    bool result = storageOpen(p_context->p_writing, 0xff, &(p_context->flash_stream), p_context->p_setting, &date, p_context->abstract);
    if( ! result) {
        p_context->p_writing = NULL;
    }
    
    // イベント通知
    (p_context->event_handler)(p_context, START_RECORDING);
}

void bleLoggerServiceStopLogging(ble_logger_service_t *p_context)
{
    if(p_context->p_writing == NULL) {
        return;
    }
    
    storageClose(p_context->p_writing);
    p_context->p_writing = NULL;
    
    // イベント通知
    (p_context->event_handler)(p_context, STOP_RECORDING);
}

void bleLoggerServiceWrite(ble_logger_service_t *p_context, const SensorData_t *p_sensorData)
{
    if(p_context->p_writing == NULL) {
        return;
    }
    
    int size = storageWrite(p_context->p_writing, p_sensorData);
    if(size == 0) {
        // DISK FULLイベント通知
        (p_context->event_handler)(p_context, STORAGE_FULL);
        bleLoggerServiceStopLogging(p_context);
    }
}

void bleLoggerServiceFormatStorage(ble_logger_service_t *p_context)
{
    // TBD
}
