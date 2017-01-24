#include <string.h>

#include <ble.h>
#include <app_error.h>
#include <sdk_errors.h>
#include <softdevice_handler_appsh.h>

#include "ble_stack.h"

// Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device
#define IS_SRVC_CHANGED_CHARACT_PRESENT 1

void init_ble_stack(sys_evt_handler_t systemHandler, ble_evt_handler_t bleHandler)
{
    ret_code_t err_code;
    
    // Initialize the SoftDevice handler module.
    // ハードウェア構成により、クロック設定は異なってくる。使用するモジュールは、32MHzクロック。32kHzクロックはなし。外部のRTCが32kHzクロックを供給する。
    // Braveridgeのモジュールは、Xtalが32MHz。
    NRF_CLOCK->XTALFREQ = (uint32_t)((CLOCK_XTALFREQ_XTALFREQ_32MHz << CLOCK_XTALFREQ_XTALFREQ_Pos) & CLOCK_XTALFREQ_XTALFREQ_Msk);
    // 外部32kHzクロック供給。
    NRF_CLOCK->LFCLKSRC = (uint32_t)((CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
    // BLEのスタックの処理は、スケジューラを使う。
    SOFTDEVICE_HANDLER_APPSH_INIT(NRF_CLOCK_LFCLKSRC_XTAL_250_PPM, NULL);
    
    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    //    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    // S110のみ有効。
    // GATTサーバのメモリが標準量0x700では足りないので、0x680増やす。リンカでメモリスタート位置を0x20002000から0x20002680に変更している。
    ble_enable_params.gatts_enable_params.attr_tab_size   = 0x680 + 0x700;
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(bleHandler);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(systemHandler);
    APP_ERROR_CHECK(err_code);
}
