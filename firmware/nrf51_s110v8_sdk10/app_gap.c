#include <string.h>

#include <nordic_common.h>
#include <ble_gap.h>
#include <ble_hci.h>
#include <sdk_errors.h>

#include <app_error.h>
#include <app_util.h>

#include <pstorage.h>

#include "app_gap.h"

#include "ble_parameters_config.h"
#include "senstick_device_definition.h"

#define STORAGE_BLOCK_SIZE (sizeof(uint32_t) * 6)

static uint16_t m_conn_handle;
static pstorage_handle_t m_flash_handle;
static uint8_t m_device_name[STORAGE_BLOCK_SIZE];

static void app_gap_pstorage_callback(pstorage_handle_t * handle, uint8_t op_code, uint32_t reason, uint8_t *p_data, uint32_t param_len)
{
    if (reason != NRF_SUCCESS)
    {
        NRF_LOG_PRINTF_DEBUG("error: app_gap_pstorage_callback\n");
        NRF_LOG_PRINTF_DEBUG("\top_code:0x%02x reason:0x%08x.\n", op_code, reason);
    }
}

void init_app_gap(void)
{
    ret_code_t err_code;
    
    // pstorageに登録
    pstorage_module_param_t param;
    param.block_count = 1;
    param.block_size  = STORAGE_BLOCK_SIZE; // GATT経由で書き込める最大長20までで十分だが、ブロックサイズの単位はuint32_tなので、それで設定。
    
    param.cb          = app_gap_pstorage_callback;
    err_code = pstorage_register(&param, &m_flash_handle);
    
    // デバイス名の初期値設定。
    memset(m_device_name, 0, sizeof(m_device_name));
    uint8_t buffer[STORAGE_BLOCK_SIZE];
    err_code = pstorage_load(buffer, &m_flash_handle, STORAGE_BLOCK_SIZE, 0); // sizeof(uint32_t)単位、文字列終端まで含めるため、GATT書き込み最大長20バイト+4。
    if(err_code == NRF_SUCCESS && buffer[0] != 0xff) {
        strncpy((char *)m_device_name, (char *)buffer, STORAGE_BLOCK_SIZE);
    } else {
        strcpy((char *)m_device_name, DEVICE_NAME);
        // 永続化
        err_code = pstorage_store(&m_flash_handle, m_device_name, STORAGE_BLOCK_SIZE, 0);
        APP_ERROR_CHECK(err_code);
    }

    // セキュリティモードおよびデバイス名。
    // セキュリティモード1,レベル1。require no protection, open link.
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    uint16_t length = strlen((char *)m_device_name);
    err_code = sd_ble_gap_device_name_set(&sec_mode, m_device_name, length);
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

void app_gap_set_device_name(uint8_t *p_device_name, uint16_t length)
{
    ret_code_t err_code;

    // スタティック変数に保存
    memset(m_device_name, 0, sizeof(m_device_name));
    strncpy((char *)m_device_name, (char *)p_device_name, MIN(length, sizeof(m_device_name) -1));
    
    // pstorageで永続化
    err_code = pstorage_update(&m_flash_handle, m_device_name, STORAGE_BLOCK_SIZE, 0);
    APP_ERROR_CHECK(err_code);
    
    // セキュリティモードとレベルの設定
    // セキュリティモード1,レベル1。require no protection, open link.
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    length = strlen((char *)m_device_name);
    err_code = sd_ble_gap_device_name_set(&sec_mode, m_device_name, length);
    APP_ERROR_CHECK(err_code);
}

uint16_t app_gap_get_device_name(uint8_t *p_device_name, uint16_t length)
{
    ret_code_t err_code;
    
    err_code = sd_ble_gap_device_name_get(p_device_name, &length);
    APP_ERROR_CHECK(err_code);

    return length;
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
