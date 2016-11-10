#include <string.h>

#include <ble.h>
#include <app_error.h>
#include <sdk_errors.h>
#include <softdevice_handler_appsh.h>

#include "ble_stack.h"

// Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device
#define IS_SRVC_CHANGED_CHARACT_PRESENT  0

// セントラル及びペリフェラルとしての、接続数の定義
#define CENTRAL_LINK_COUNT    0
#define PERIPHERAL_LINK_COUNT 1

// GATT MTUサイズの定義, GATT_MTU_SIZE_DEFAULT は ble_gatt.h で23に定義されている。
#define NRF_BLE_MAX_MTU_SIZE GATT_MTU_SIZE_DEFAULT

void init_ble_stack(sys_evt_handler_t systemHandler, ble_evt_handler_t bleHandler)
{
    ret_code_t err_code;
    
    // Initialize the SoftDevice handler module.
    // LFCLK crystal oscillator. 32kHz xtal外付け。
    nrf_clock_lf_cfg_t clock_lf_cfg = {
        .source        = NRF_CLOCK_LF_SRC_XTAL,
        .rc_ctiv       = 0,
        .rc_temp_ctiv  = 0,
        .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM};

    // SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    SOFTDEVICE_HANDLER_APPSH_INIT(&clock_lf_cfg, NULL);      // BLEのスタックの処理は、スケジューラを使う。
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT, &ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    // #if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
    // S110のみ有効。
    // GATTサーバのメモリが標準量0x700では足りないので、0x680増やす。リンカでメモリスタート位置を0x20002000から0x20002680に変更している。
    //    ble_enable_params.gatts_enable_params.attr_tab_size   = 0x680 + 0x700;
    //    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(bleHandler);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(systemHandler);
    APP_ERROR_CHECK(err_code);
}

