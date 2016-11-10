#include <string.h>

#include <nordic_common.h>
#include <ble_gap.h>
#include <ble_hci.h>
#include <sdk_errors.h>

#include <app_error.h>
#include <app_util.h>

// nRF52 pstorageをfstorageに切り替える。
#include <fstorage.h>

#include "app_gap.h"

#include "ble_parameters_config.h"
#include "senstick_util.h"
#include "senstick_device_definition.h"

// ページサイズはnRF51は256, nRF52は1024
// デバイス名は最大20バイトだから1ページでよい。

// 本来は20バイトで十分だが末尾の0を入れるため21バイトのバッファが必要。またワードサイズにアライメントするため、24(=4*6)とする。
#define DEVICE_NAME_LENGTH (sizeof(uint32_t) *6)

static uint16_t m_conn_handle;
static uint8_t m_device_name[DEVICE_NAME_LENGTH];

// 関数宣言
static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result);
// fstorageの領域確保とイベントハンドラの設定
FS_REGISTER_CFG(fs_config_t fs_config) =
{
    .callback  = fs_evt_handler, // Function for event callbacks.
    .num_pages = 1,              // Number of physical flash pages required.
    .priority  = 0xFE            // Priority for flash usage.
};

static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result)
{
    if (result != FS_SUCCESS)
    {
        // An error occurred.
        NRF_LOG_PRINTF_DEBUG("error: fs_evt_handler, result:%d.\n", result);
    }
}
//p_start_addr;
void init_app_gap(void)
{
    ret_code_t err_code;
    
    // デバイス名の初期値設定。
    memset(m_device_name, 0, sizeof(m_device_name));
    
    // フラッシュのアドレスを取得。先頭がマジックワードでなければ、デバイス名として取得。
    uint8_t *buffer = (uint8_t *)fs_config.p_start_addr;
    if(buffer[0] != 0xff) {
        strncpy((char *)m_device_name, (char *)buffer, DEVICE_NAME_LENGTH);
    } else {
        strcpy((char *)m_device_name, DEVICE_NAME);
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
    
    // デバイス名の永続化。fstorageのページを消去、後に保存。
    fs_ret_t ret;
    ret = fs_erase(&fs_config, fs_config.p_start_addr, 1, NULL);
    APP_ERROR_CHECK(ret); // FS_SUCCESSは0。APP_ERROR_CHECKで代用する。
    ret = fs_store(&fs_config, fs_config.p_start_addr, (uint32_t *)m_device_name, sizeof(m_device_name) / sizeof(uint32_t), NULL);
    APP_ERROR_CHECK(ret); // FS_SUCCESSは0。APP_ERROR_CHECKで代用する。
    
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
