#include "nordic_common.h"

#include "nrf_soc.h"
#include "nrf_delay.h"
#include "nrf_log.h"

#include "sdk_errors.h"
#include "nrf51_bitfields.h"

#include "softdevice_handler.h"
#include "ble_advertising.h"
#include "ble_gap.h"
#include "ble_parameters_config.h"

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
static senstick_core_t manager_context;

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

// BLEスタックからのイベントを、各モジュールに分配する。
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_advertising_on_ble_evt(p_ble_evt);
    ble_activity_service_on_ble_event(&activity_service_context, p_ble_evt);
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
void on_activity_service_event(ble_activity_service_t * p_context, const ble_activity_service_event_t * p_event)
{
    
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
    
    // サービス初期化
    ble_activity_service_init_t activity_service_init;
    activity_service_init.event_handler = on_activity_service_event;
    err_code = ble_activity_service_init(&activity_service_context, &activity_service_init);
    APP_ERROR_CHECK(err_code);
    
    // コアの初期化
    init_senstick_core_manager(&(manager_context));
    
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
