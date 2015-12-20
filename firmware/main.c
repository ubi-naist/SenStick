/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */
/*
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
*/

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
// Application identifier allocated by device manager
static dm_application_instance_t m_app_handle;
// Handle of the current connection.
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */

/**
 * 関数宣言
 */


/**
 * イベントハンドラ
 */

// Function for handling the Application's BLE Stack events.
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
            
        default:
            // No implementation needed.
            break;
    }
}

// システムイベントをモジュールに分配する。
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}

// BLEスタックからのイベントを、各モジュールに分配する。
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    /*YOUR_JOB add calls to _on_ble_evt functions from each service your application is using
     ble_xxs_on_ble_evt(&m_xxs, p_ble_evt);
     ble_yys_on_ble_evt(&m_yys, p_ble_evt);
     */
}

/**
 * 関数
 */

// assertで違反した時に呼び出される関数
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    LOG("\nassert_nrf_callback: line:%d file:%s", line_num, p_file_name);
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

// Function for initializing the BLE stack.
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    // Initialize the SoftDevice handler module.
    // ハードウェア構成により、クロック設定は異なってくる。使用するモジュールは、32MHzクロック。32kHzクロックはなし。外部のRTCが32kHzクロックを供給する。
    NRF_CLOCK->XTALFREQ = CLOCK_XTALFREQ_XTALFREQ_32MHz;   // Braveridgeのモジュールは、Xtalが32MHz。
    SOFTDEVICE_HANDLER_APPSH_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_4000MS_CALIBRATION, true);
    
    // #if defined(S110) || defined(S130) || defined(S132)
    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    //#if (defined(S130) || defined(S132))
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    //#endif
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    //#endif
    
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
    
    gap_conn_params.min_conn_interval = DEFAULT_MIN_CONN_INTERVAL_MILLISEC;
    gap_conn_params.max_conn_interval = DEFAULT_MAX_CONN_INTERVAL_MILLISEC;
    gap_conn_params.slave_latency     = DEFAULT_SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = DEFAULT_CONN_SUP_TIMEOUT_MILISEC;
    
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**
 * デバイスマネージャー
 */

static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle, dm_event_t const * p_event, ret_code_t event_result)
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

// Function for the Device Manager initialization.
// 内部でpstorageを使うために、これを呼ぶ前にpstorageの初期化を完了させておくこと。
static void device_manager_init(void)
{
    uint32_t               err_code;
    dm_init_param_t        init_param;
    dm_application_param_t register_param;
    
    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);
    
    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);
    
    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));
    // ペアリング/ボンディングは使わない。
    /*
     register_param.sec_param.bond         = SEC_PARAM_BOND;
     register_param.sec_param.mitm         = SEC_PARAM_MITM;
     register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
     register_param.sec_param.oob          = SEC_PARAM_OOB;
     register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
     register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
     register_param.evt_handler            = device_manager_evt_handler;
     register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
     */
    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}

/**
 * コネクション・パラメータの管理
 */

// Function for handling the Connection Parameters Module.
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
/*
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
*/
}

// Function for handling a Connection Parameters error.
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

//  Function for initializing the Connection Parameters module.
static void conn_params_init(void)
{
    uint32_t               err_code;

    // コネクションパラメータモジュールを初期化する。
    ble_conn_params_init_t cp_init;
    memset(&cp_init, 0, sizeof(cp_init));
    
    cp_init.p_conn_params                  = NULL;  // パラメータを与えていないので、モジュールはスタックから取得する。
    cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(FIRST_CONN_PARAMS_UPDATE_DELAY_MILLISEC, APP_TIMER_PRESCALER);;
    cp_init.next_conn_params_update_delay  = APP_TIMER_TICKS(NEXT_CONN_PARAMS_UPDATE_DELAY_MILLISEC,  APP_TIMER_PRESCALER);
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;    // 書き込まれた時に接続パラメータ更新を開始する、CCCDハンドラ。
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**
 * アドバタイジング
 */

static void on_advertising_event(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;
    
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_IDLE:
            LOG("\nBLE_ADV_EVT_IDLE.");
            break;
            
        case BLE_ADV_EVT_DIRECTED:
            LOG("\nBLE_ADV_EVT_DIRECTED.");
            break;
            
        case BLE_ADV_EVT_FAST:
            LOG("\nBLE_ADV_EVT_FAST.");
            break;
            
        case BLE_ADV_EVT_SLOW:
            LOG("\nBLE_ADV_EVT_SLOW.");
            break;
            
        case BLE_ADV_EVT_FAST_WHITELIST:
            LOG("\nBLE_ADV_EVT_FAST_WHITELIST.");
            break;
            
        case BLE_ADV_EVT_SLOW_WHITELIST:
            LOG("\nBLE_ADV_EVT_SLOW_WHITELIST.");
            break;
            
        case BLE_ADV_EVT_WHITELIST_REQUEST:
            LOG("\nBLE_ADV_EVT_WHITELIST_REQUEST.");
            break;
            
            // Direct接続を使わないので、これを処理する必要はない。
        case BLE_ADV_EVT_PEER_ADDR_REQUEST:
            LOG("\nBLE_ADV_EVT_PEER_ADDR_REQUEST.");
            break;
            
        default:
            break;
    }
}

static void on_advertising_error(uint32_t nrf_error)
{
    LOG("\non_advertising_error() error:0x%0x.", nrf_error);
}

void initAdvertisingManager(void)
{
    uint32_t      err_code;
    
    // コンテキストをクリアする
    memset(p_context, 0, sizeof(ble_advertising_manager_t));
    
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
    
    scan_response_data.name_type               = BLE_ADVDATA_FULL_NAME;
    
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
    
    err_code = ble_advertising_init(&advdata, &scan_response_data, &options, on_advertising_event, on_advertsing_error);
    APP_ERROR_CHECK(err_code);
}

void startAdvertising(void)
{
    uint32_t      err_code;
    
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}





// Function for putting the chip into sleep mode.
/*
 static void sleep_mode_enter(void)
 {
 uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
 APP_ERROR_CHECK(err_code);
 
 // Prepare wakeup buttons.
 err_code = bsp_btn_ble_sleep_mode_prepare();
 APP_ERROR_CHECK(err_code);
 
 // Go to system-off mode (this function will not return; wakeup will cause a reset).
 err_code = sd_power_system_off();
 APP_ERROR_CHECK(err_code);
 }
 */

/**@brief Function for handling the YYY Service events.
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service, 
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. \r\n", p_evt->params.char_xx.value.p_str);
            break;
        
        default:
            // No implementation needed.
            break;
    }
}*/

/// brief Function for initializing services that will be used by the application.

//static void services_init(void)
//{
    /* YOUR_JOB: Add code to initialize the services used by the application.
    uint32_t                           err_code;
    ble_xxs_init_t                     xxs_init;
    ble_yys_init_t                     yys_init;

    // Initialize XXX Service.
    memset(&xxs_init, 0, sizeof(xxs_init));

    xxs_init.evt_handler                = NULL;
    xxs_init.is_xxx_notify_supported    = true;
    xxs_init.ble_xx_initial_value.level = 100; 
    
    err_code = ble_bas_init(&m_xxs, &xxs_init);
    APP_ERROR_CHECK(err_code);

    // Initialize YYY Service.
    memset(&yys_init, 0, sizeof(yys_init));
    yys_init.evt_handler                  = on_yys_evt;
    yys_init.ble_yy_initial_value.counter = 0;

    err_code = ble_yy_service_init(&yys_init, &yy_init);
    APP_ERROR_CHECK(err_code);
    */
//}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
/*
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}*/

/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
/*
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), 
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}
*/


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;

    // スタックの初期化。
    ble_stack_init();
    
    // タイマーモジュール設定。
    APP_TIMER_INIT(PRESCALER, OP_QUEUE_SIZE, scheduler_function);
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    // pstorageを初期化。device managerを呼び出す前に、この処理を行わなくてはならない。
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);
    
    // 各モジュールの設定
    device_manager_init();
    
    // Initialize.
    gap_params_init();
    advertising_init();
    services_init();
    conn_params_init();

    // アドバタイジングを開始する。
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    for (;;) {
        // BLEのイベント待ち
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // スケジューラのタスク実行
        app_sched_execute();
    }
}

