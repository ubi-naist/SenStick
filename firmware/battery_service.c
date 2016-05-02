#include <string.h>

#include <ble_bas.h>
#include <nordic_common.h>

#include <app_error.h>
#include <sdk_errors.h>

#include "battery_service.h"

static ble_bas_t batter_service_context;

void init_battery_service()
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
    bas_init.initial_batt_level   = 100;
    
    err_code = ble_bas_init(&batter_service_context, &bas_init);
    APP_ERROR_CHECK(err_code);
}

void handle_battery_service_ble_event(ble_evt_t * p_ble_evt)
{
    ble_bas_on_ble_evt(&batter_service_context, p_ble_evt);
}

void update_battery_service_battery_value(uint8_t battery_level)
{
    ret_code_t err_code;
    err_code = ble_bas_battery_level_update(&batter_service_context, battery_level);
    APP_ERROR_CHECK(err_code);
}
