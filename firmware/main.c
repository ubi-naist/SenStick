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

#include "ble_dis.h"
#include "ble_bas.h"

#include "device_manager.h"

#include "app_timer_appsh.h"
#include "app_scheduler.h"
#include "app_error.h"
#include "pstorage.h"

#include "senstick_device_definitions.h"
#include "senstick_definitions.h"
#include "twi_sensor_manager.h"
#include "gpio_manager.h"

#include "ble_ti_sensortag_service.h"
#include "ble_logger_service.h"

#include "sensor_data_storage.h"
#include "test_storage.h"

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
static ble_bas_t m_bas;
APP_TIMER_DEF(main_app_timer_id);

static gpio_manager_t gpio_manager_context;
static ble_sensortag_service_t sensortag_service_context;
static ble_logger_service_t logger_service_context;
static sensor_manager_t sensor_manager_context;

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
    ret_code_t err_code;
    
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
    
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
    bleSensorTagServiceOnBLEEvent(&sensortag_service_context, p_ble_evt);
    bleLoggerServiceOnBleEvent(&logger_service_context, p_ble_evt);
    
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
    ret_code_t             err_code;
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

// デバイスインフォアメーションサービスを初期化
void initDeviceInformationService(void)
{
    ret_code_t err_code;
    ble_dis_init_t dis_init_obj;
    
    memset(&dis_init_obj, 0, sizeof(dis_init_obj));
    
    // 文字列を設定
    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init_obj.hw_rev_str, HARDWARE_REVISION_STRING);
    ble_srv_ascii_to_utf8(&dis_init_obj.fw_rev_str, FIRMWARE_REVISION_STRING);
    
    // 属性設定
//    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);
    
    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
}

// バッテリサービスを初期化
void initBatteryService(void)
{
    ret_code_t err_code;
    ble_bas_init_t bas_init;
    
    memset(&(bas_init), 0, sizeof(bas_init));
    
    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);
    
    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = getBatteryLevel(&gpio_manager_context);
    
    err_code = ble_bas_init(&m_bas, &bas_init);
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
//    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    // S110のみ有効。
    // GATTサーバのメモリが標準量0x700では足りないので、0x400増やす。リンカでメモリスタート位置を0x20002000から0x20002400に変更している。
    ble_enable_params.gatts_enable_params.attr_tab_size   = 0x300 + 0x700;
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
    if(p_uuid != NULL) {
        scan_response_data.uuids_complete.uuid_cnt = 1;
        scan_response_data.uuids_complete.p_uuids  = p_uuid;
    }

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

// 設定及びコントロールのキャラクタリスティクスに書き込まれた設定情報を解釈します
// FIXME データの内容の解釈処理は、サービスの処理として実装すべきかも。

// 設定したサンプリング周期ごとに、センサーデータが渡されるコールバック関数
static void onSamplingCallbackHandler(SensorDeviceType_t sensorType, const SensorData_t *p_sensorData)
{
    // センサータグサービスに通知する
    notifySensorData(&sensortag_service_context, sensorType, p_sensorData);
    // ロガーに通知、データ書き込み
    bleLoggerServiceWrite(&logger_service_context, sensorType, p_sensorData);
}

// サービス経由で外部から、センサー設定が変更されるたびに呼び出されるコールバック関数
static void onSensorSettingChangedHandler(ble_sensortag_service_t * p_context, sensorSetting_t *p_setting)
{
    // センサー設定を、センサマネージャに反映させる。
    setSensorManagerSetting(&sensor_manager_context, p_setting);
    // ロガーは一旦停止する。
    bleLoggerServiceStopLogging(&logger_service_context);
}

// ロガーから呼び出される、コールバック関数
static void onLoggerHandler(ble_logger_service_t * p_context)
{
    // TBD
}

// 30秒毎に呼び出されるタイマー
static void main_app_timer_handler(void *p_arg)
{
    ret_code_t err_code;
    
    // バッテリー監視
    uint8_t battery_level = getBatteryLevel(&gpio_manager_context);
    if( m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        err_code = ble_bas_battery_level_update(&m_bas,battery_level);
//        APP_ERROR_CHECK(err_code);
    }
}

/**
 * main関数
 */
int main(void)
{
    ret_code_t err_code;

    sensorSetting_t defaultSensorSetting;
    {
        defaultSensorSetting.accelerationRange  = ACCELERATION_RANGE_2G;
        defaultSensorSetting.rotationRange      = ROTATION_RANGE_250DPS;
        
        defaultSensorSetting.motionSensorSamplingPeriod             = 100;
        defaultSensorSetting.humidityAndTemperatureSamplingPeriod   = 500;
        defaultSensorSetting.airPressureSamplingPeriod              = 500;
        defaultSensorSetting.brightnessSamplingPeriod               = 500;
        defaultSensorSetting.ultraVioletSamplingPeriod              = 500;
        
        defaultSensorSetting.is_accelerometer_sampling   = true;
        defaultSensorSetting.is_gyroscope_sampling       = 0x07;
        defaultSensorSetting.is_humidity_sampling        = true;
        defaultSensorSetting.is_temperature_sampling     = true;
        defaultSensorSetting.is_magnetrometer_sampling   = true;
        defaultSensorSetting.is_barometer_sampling       = true;
        defaultSensorSetting.is_illumination_sampling    = true;
    }
    
    // RTTログを有効に
    NRF_LOG_INIT();
    NRF_LOG_PRINTF_DEBUG("Start....\n");
    
    // タイマーモジュール、スケジューラ設定。
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    // pstorageを初期化。device managerを呼び出す前に、この処理を行わなくてはならない。
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);
    
    // スタックの初期化。GAPパラメータを設定
    ble_stack_init();
    gap_params_init();
    
    // BLEデバイスマネージャを初期化
    device_manager_init(true);
    
    // GPIOの初期化
    initGPIOManager(&gpio_manager_context);
    
    // センサマネージャーの初期化
    initSensorManager(&sensor_manager_context, &gpio_manager_context, &defaultSensorSetting,onSamplingCallbackHandler);
    
    // デバイスインフォアメーションサービスを追加
    initDeviceInformationService();

    // バッテリーサービスを追加
    initBatteryService();
    
    // センサータグサービスを追加
    err_code = bleSensorTagServiceInit(&sensortag_service_context, &defaultSensorSetting, onSensorSettingChangedHandler);
    APP_ERROR_CHECK(err_code);
    // ロガーサービスを追加
    err_code = bleLoggerServiceInit(&logger_service_context, onLoggerHandler);
    APP_ERROR_CHECK(err_code);

    /** **/
    // メモリ単体テスト
    //    testFlashMemory(&(stream.flash_context));

    // メモリへの書き込みテスト
    /*
    flash_stream_context_t stream;
    initFlashStream(&stream);
    do_storage_test(&stream);
     */
    /** **/
    
    // アドバタイジングを開始する。
//    initAdvertisingManager(&(sensortag_service_context.service_uuid));
    initAdvertisingManager(NULL); // TBD ロギング等適当なサービスのUUIDを後で入れること。
    startAdvertising();
    
    // アプリのタイマーを開始する。
    err_code = app_timer_create(&main_app_timer_id, APP_TIMER_MODE_REPEATED, main_app_timer_handler );
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(main_app_timer_id,
                               APP_TIMER_TICKS(30 * 1000, APP_TIMER_PRESCALER), // 30秒
                               NULL);
    APP_ERROR_CHECK(err_code);
    
    // センサーセンシング及びロギングを開始
    sensorManagerStartSampling(&sensor_manager_context);
    
    for (;;) {
        // BLEのイベント待ち
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // スケジューラのタスク実行
        app_sched_execute();
    }
}
