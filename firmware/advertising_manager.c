#include <nrf_log.h>
#include <app_error.h>

#include "senstick_util.h"
#include "ble_parameters_config.h"

#include "advertising_manager.h"

static ble_uuid_t *p_service_uuid;

static void on_advertising_event(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;
    
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_IDLE:
            // 再度 slow でアドバタイジングを再開する。
            err_code = ble_advertising_start(BLE_ADV_MODE_SLOW);
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_ADV_EVT_DIRECTED:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_DIRECTED.\n");
            break;
            
        case BLE_ADV_EVT_FAST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_FAST.\n");
            break;
            
        case BLE_ADV_EVT_SLOW:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_SLOW.\n");
            break;
            
        case BLE_ADV_EVT_FAST_WHITELIST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_FAST_WHITELIST.\n");
            break;
            
        case BLE_ADV_EVT_SLOW_WHITELIST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_SLOW_WHITELIST.\n");
            break;
            
        case BLE_ADV_EVT_WHITELIST_REQUEST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_WHITELIST_REQUEST.\n");
            break;
            
            // Direct接続を使わないので、これを処理する必要はない。
        case BLE_ADV_EVT_PEER_ADDR_REQUEST:
            NRF_LOG_PRINTF_DEBUG("\nBLE_ADV_EVT_PEER_ADDR_REQUEST.\n");
            break;
            
        default:
            break;
    }
}

static void on_advertising_error(uint32_t nrf_error)
{
    NRF_LOG_PRINTF_DEBUG("\non_advertising_error() error:0x%0x.", nrf_error);
}

void init_advertising_manager(ble_uuid_t *p_uuid)
{
    p_service_uuid = p_uuid;
}

void startAdvertising(void)
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
    if(p_service_uuid != NULL) {
        scan_response_data.uuids_complete.uuid_cnt = 1;
        scan_response_data.uuids_complete.p_uuids  = p_service_uuid;
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
    
    // アドバタイジングを開始
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

void stopAdvertising(void)
{
    /*
    uint32_t err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);
     */
    sd_ble_gap_adv_stop();    
}
