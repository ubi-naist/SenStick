#include "nordic_common.h"

#include "nrf_soc.h"
#include "nrf_delay.h"
#include "nrf_log.h"

#include "sdk_errors.h"
#include "nrf51_bitfields.h"

#include "softdevice_handler.h"
#include "ble_advertising.h"
#include "ble_gap.h"
#include "ble_hci.h"
#include "ble_parameters_config.h"

#include "device_manager.h"

#include "app_timer_appsh.h"
#include "app_scheduler.h"
#include "app_error.h"
#include "pstorage.h"

#include "senstick_device_definitions.h"
#include "senstick_definitions.h"
#include "senstick_core_manager.h"

#include "ble_activity_service.h"

/**
 * 定義
 */

// Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device
#define IS_SRVC_CHANGED_CHARACT_PRESENT  1
// Value used as error code on stack dump, can be used to identify stack location on stack unwind.
#define DEAD_BEEF 0xDEADBEEF

/**
 * static変数
 */

static ble_activity_service_t activity_service_context;
static senstick_core_t senstick_core_context;
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
static dm_application_instance_t m_app_handle; // Application identifier allocated by device manager
/**
 * 関数宣言
 */


/**
 * イベントハンドラ
 */

// システムイベントをモジュールに分配する。
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GAP_EVT_CONNECTED");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GAP_EVT_DISCONNECTED");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GAP_EVT_SEC_PARAMS_REQUEST.\n  invoked by SMP Paring request, replying parameters.");
            break;
            
        case BLE_GAP_EVT_CONN_SEC_UPDATE:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GAP_EVT_CONN_SEC_UPDATE.\n  Encrypted with STK.");
            break;
            
        case BLE_GAP_EVT_AUTH_STATUS:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GAP_EVT_AUTH_STATUS.");
            break;
            
        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GAP_EVT_SEC_INFO_REQUEST");
            break;
            
        case BLE_EVT_TX_COMPLETE:
            NRF_LOG_PRINTF_DEBUG("\nBLE_EVT_TX_COMPLETE");
            break;
            
        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GAP_EVT_CONN_PARAM_UPDATE.");
            break;
            
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GATTS_EVT_SYS_ATTR_MISSING.");
            break;
            
        case BLE_GAP_EVT_TIMEOUT:
            break;
            
        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
                        NRF_LOG_PRINTF_DEBUG("\nBLE_GATTC_EVT_PRIM_SRVC_DISC_RSP");
            break;
        case BLE_GATTC_EVT_CHAR_DISC_RSP:
                        NRF_LOG_PRINTF_DEBUG("\nBLE_GATTC_EVT_CHAR_DISC_RSP");
            break;
        case BLE_GATTC_EVT_DESC_DISC_RSP:
                        NRF_LOG_PRINTF_DEBUG("\nBLE_GATTC_EVT_DESC_DISC_RSP");
            break;
        case BLE_GATTC_EVT_WRITE_RSP:
                        NRF_LOG_PRINTF_DEBUG("\nBLE_GATTC_EVT_WRITE_RSP");
            break;
        case BLE_GATTC_EVT_HVX:
                        NRF_LOG_PRINTF_DEBUG("\nBLE_GATTC_EVT_HVX");
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GATTC_EVT_TIMEOUT. disconnecting.");
            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GATTS_EVT_WRITE");
            break;
            
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_PRINTF_DEBUG("\nBLE_GATTS_EVT_TIMEOUT. disconnecting.");
            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
            
        default:
            //No implementation needed
            NRF_LOG_PRINTF_DEBUG("\nunknown event id: 0x%02x.", p_ble_evt->header.evt_id);
            break;
    }
}

// BLEスタックからのイベントを、各モジュールに分配する。
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    
    bleActivityServiceOnBLEEvent(&activity_service_context, p_ble_evt);
    
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}

/**
 * 関数
 */

// アプリケーション内部で致命的なエラーが発生した時に呼び出される関数
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    NRF_LOG_PRINTF_DEBUG("\napp_error: er_code:0x%04x line:%d file:%s", error_code, line_num, p_file_name);
    nrf_delay_ms(500);
    
    sd_nvic_SystemReset();
}

// assertで違反した時に呼び出される関数
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    NRF_LOG_PRINTF_DEBUG("\nassert_nrf_callback: line:%d file:%s", line_num, p_file_name);
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

// デバイスマネージャのイベント処理ハンドラ
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle, dm_event_t const  * p_event, ret_code_t event_result)
{
    APP_ERROR_CHECK(event_result);
/*
#ifdef BLE_DFU_APP_SUPPORT
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        app_context_load(p_handle);
    }
#endif // BLE_DFU_APP_SUPPORT
*/
    return NRF_SUCCESS;
}

// デバイスマネージャの初期化
// このメソッドを呼び出す前にpstorageの初期化が完了していること。
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;
    
    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);
    
    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));
    
    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
    
    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}

// Function for initializing the BLE stack.
static void ble_stack_init(void)
{
    ret_code_t err_code;
    
    // Initialize the SoftDevice handler module.
    // ハードウェア構成により、クロック設定は異なってくる。使用するモジュールは、32MHzクロック。32kHzクロックはなし。外部のRTCが32kHzクロックを供給する。
    // Braveridgeのモジュールは、Xtalが32MHz。
    NRF_CLOCK->XTALFREQ = (uint32_t)((CLOCK_XTALFREQ_XTALFREQ_32MHz << CLOCK_XTALFREQ_XTALFREQ_Pos) & CLOCK_XTALFREQ_XTALFREQ_Msk);
    // BLEのスタックの処理は、スケジューラを使わず割り込みから直接処理に移行する。IO処理などでのブロックを排除するため。
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_4000MS_CALIBRATION, NULL);
    
    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

// GAPを設定
static void gap_params_init(void)
{
    uint32_t                err_code;

    // セキュリティモードとレベルの設定
    // セキュリティモード1,レベル1。require no protection, open link.
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    // デバイス名を設定
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    // アピアランスを設定
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
    APP_ERROR_CHECK(err_code);
    
    // コネクション・パラメータを設定
    ble_gap_conn_params_t   gap_conn_params;
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
//MSEC_TO_UNITS(
    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(DEFAULT_MIN_CONN_INTERVAL_MILLISEC, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(DEFAULT_MAX_CONN_INTERVAL_MILLISEC, UNIT_1_25_MS);
    gap_conn_params.slave_latency     = DEFAULT_SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(DEFAULT_CONN_SUP_TIMEOUT_MILISEC, UNIT_10_MS);
    
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**
 * アドバタイジング
 */

static void on_advertising_event(ble_adv_evt_t ble_adv_evt)
{
//    ret_code_t err_code;
    
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_IDLE:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_IDLE.");
            break;
            
        case BLE_ADV_EVT_DIRECTED:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_DIRECTED.");
            break;
            
        case BLE_ADV_EVT_FAST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_FAST.");
            break;
            
        case BLE_ADV_EVT_SLOW:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_SLOW.");
            break;
            
        case BLE_ADV_EVT_FAST_WHITELIST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_FAST_WHITELIST.");
            break;
            
        case BLE_ADV_EVT_SLOW_WHITELIST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_SLOW_WHITELIST.");
            break;
            
        case BLE_ADV_EVT_WHITELIST_REQUEST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_WHITELIST_REQUEST.");
            break;
            
            // Direct接続を使わないので、これを処理する必要はない。
        case BLE_ADV_EVT_PEER_ADDR_REQUEST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_PEER_ADDR_REQUEST.");
            break;
            
        default:
            break;
    }
}

static void on_advertising_error(uint32_t nrf_error)
{
    NRF_LOG_PRINTF_DEBUG("\non_advertising_error() error:0x%0x.", nrf_error);
}

void initAdvertisingManager(ble_uuid_t *p_uuid)
{
    uint32_t      err_code;
    
    /*    
    // アドバタイジングデータを構築する。
    ble_advdata_t advdata;
    memset(&advdata, 0, sizeof(advdata));
    // Flags,サービスのUUIDを設定する
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = 1;
    advdata.uuids_complete.p_uuids  = p_uuid;
    
    // スキャンレスポンスを構築する。
    // 完全なLocal nameを設定する。
    ble_advdata_t scan_response_data;
    memset(&scan_response_data, 0, sizeof(scan_response_data));
    
    scan_response_data.name_type = BLE_ADVDATA_FULL_NAME;
    */
    
    // アドバタイジングデータを構築する。
    ble_advdata_t advdata;
    memset(&advdata, 0, sizeof(advdata));
    advdata.flags     = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.name_type = BLE_ADVDATA_FULL_NAME;
    
    // スキャンレスポンスを構築する。
    ble_advdata_t scan_response_data;
    memset(&scan_response_data, 0, sizeof(scan_response_data));
    scan_response_data.uuids_complete.uuid_cnt = 1;
    scan_response_data.uuids_complete.p_uuids  = p_uuid;

    // アドバタイジングモードのオプション設定
    ble_adv_modes_config_t options;
    memset(&options, 0, sizeof(ble_adv_modes_config_t));
    
    //directed modeはdisable (0設定)
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = ADV_FAST_INTERVAL_0625UNIT;
    options.ble_adv_fast_timeout  = ADV_FAST_TIMEOUT_SEC;
    
    options.ble_adv_slow_enabled  = true;
    options.ble_adv_slow_interval = ADV_SLOW_INTERVAL_0625UNIT;
    options.ble_adv_slow_timeout  = ADV_SLOW_TIMEOUT_SEC;
    
    err_code = ble_advertising_init(&advdata, &scan_response_data, &options, on_advertising_event, on_advertising_error);
    APP_ERROR_CHECK(err_code);
}

void startAdvertising(void)
{
    uint32_t      err_code;
    
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

/**
 * サービスの処理
 */
// Byte
// 0    Reserved
// 1    Acceleration sensor config, range.
//          0x00: 2G (default)
//          0x01: 4G
//          0x02: 8G
//          0x03: 16G
// 2    Cyro sensor config, range.
//          0x00 250  dps (default)
//          0x01 500  dps
//          0x02 1000 dps
//          0x03 2000 dps
// 3    Nine axes sensor sampling rate.
//          0x00    0.1 sample/sec
//          0x01    1   sample/sec
//          0x03    10  sample/sec (default)
// 4    THA sensor sampling rage.
//          0x00    0.1 sample/sec (default)
//          0x01    1   sample/sec
//          0x03    10  sample/sec
// 5    HPA sensor sampling rage.
//          0x00    0.1 sample/sec
//          0x01    1   sample/sec
//          0x03    10  sample/sec  (default)
// 6    Illumination sensor sampling rage.
//          0x00    0.1 sample/sec
//          0x01    1   sample/sec (default)
//          0x03    10  sample/sec

// 設定及びコントロールのキャラクタリスティクスに書き込まれた設定情報を解釈します
void onActivityServiceEvent(ble_activity_service_t * p_context, const ble_activity_service_event_t * p_event)
{

    switch( p_event->event_type ) {
        case BLE_ACTIVITY_SERVICE_SETTING_WRITTEN:
        {
            sensorSetting_t sensor_setting;
            bool result;
            if( p_event->data_length == 7) {
                result = deserializeSensorSetting(&sensor_setting, p_event->p_data);
                if(result) {
                    setSensorSetting(&senstick_core_context, &sensor_setting);
                }
            }
        }
            break;
        case BLE_ACTIVITY_SERVICE_CONTROL_WRITTEN:
            break;
        default: break;
    }
/*
    event.p_data      = buffer;
    event.data_length = gatts_value.len;
*/
}

/**
 * main関数
 */
int main(void)
{
    ret_code_t err_code;

    // RTTログを有効に
    NRF_LOG_INIT();
    NRF_LOG_PRINTF_DEBUG("Start....\n");
    
    // タイマーモジュール、スケジューラ設定。
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    // pstorageを初期化。device managerを呼び出す前に、この処理を行わなくてはならない。
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);
    
    // スタックの初期化。
    ble_stack_init();
    gap_params_init();
    
    // デバイスマネージャを有効化    
    device_manager_init(true);
    
    // サービス初期化
    bleActivityServiceInit_t activity_service_init;
    activity_service_init.event_handler = onActivityServiceEvent;
    err_code = bleActivityServiceInit(&activity_service_context, &activity_service_init);
    APP_ERROR_CHECK(err_code);
    
    // コアの初期化
    initSenstickCoreManager(&(senstick_core_context));
    
    // アドバタイジングを開始する。
    initAdvertisingManager(&(activity_service_context.service_uuid));
    startAdvertising();
    
    for (;;) {
        // BLEのイベント待ち
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // スケジューラのタスク実行
        app_sched_execute();
    }
}
