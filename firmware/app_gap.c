#include <string.h>

#include <ble_gap.h>
#include <ble_hci.h>
#include <sdk_errors.h>

#include <app_error.h>
#include <app_util.h>

#include "app_gap.h"

#include "ble_parameters_config.h"
#include "senstick_device_definition.h"

static uint16_t m_conn_handle;

void init_app_gap(void)
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

void app_gap_on_ble_event(ble_evt_t * p_ble_evt)
{
    ret_code_t err_code;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
            
            // Disconnect on GATT Server and Client timeout events.
        case BLE_GATTC_EVT_TIMEOUT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_GATTS_EVT_TIMEOUT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
    }
}
