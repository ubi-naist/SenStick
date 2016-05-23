#include <string.h>

#include <ble_bas.h>
#include <nordic_common.h>
#include <nrf_adc.h>

#include <app_timer_appsh.h>
#include <app_error.h>
#include <sdk_errors.h>

#include "senstick_ble_definitions.h"
#include "senstick_io_definitions.h"
#include "battery_service.h"

static ble_bas_t batter_service_context;

// FIXME Deep sleepへの遷移、タイマーの停止と開始

APP_TIMER_DEF(m_battery_timer_id);

static void update_battery_service_battery_value(void)
{
    uint8_t battery_level = getBatteryLevel();
    
    ret_code_t err_code;
    err_code = ble_bas_battery_level_update(&batter_service_context, battery_level);
    APP_ERROR_CHECK(err_code);
}

static void battery_timer_handler(void *p_arg)
{
    update_battery_service_battery_value();
}

void init_battery_service()
{
    ret_code_t err_code;

    // ADCの初期化
    // 10ビット解像度、スケーリング 1/3、リファレンス バンドギャップ電圧()
    nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG_DEFAULT;
    nrf_adc_configure(&nrf_adc_config);
    nrf_adc_input_select(ADC_INPUT_SUPPLY_MONITORING);

    // BAS初期化
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
    bas_init.initial_batt_level   = getBatteryLevel();
    
    err_code = ble_bas_init(&batter_service_context, &bas_init);
    APP_ERROR_CHECK(err_code);
    
    // タイマーの初期化と開始
    err_code = app_timer_create(&(m_battery_timer_id), APP_TIMER_MODE_REPEATED, battery_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_battery_timer_id,
                               APP_TIMER_TICKS(30 * 1000, APP_TIMER_PRESCALER),
                               NULL);
    APP_ERROR_CHECK(err_code);
}

void handle_battery_service_ble_event(ble_evt_t * p_ble_evt)
{
    ble_bas_on_ble_evt(&batter_service_context, p_ble_evt);
}

uint8_t getBatteryLevel(void)
{
    int32_t voltage;
    int level;
    
    // 電圧を取得, 入力スケール 1/3, BGR 1.2Vがリファレンス、10ビット精度なので、2^10(1024)が3.6V相当。
    voltage = nrf_adc_convert_single(ADC_INPUT_SUPPLY_MONITORING);
    
    // 4.2V - 3.2V 電圧範囲で、残量は電圧にリニアで換算
    // 入力電圧は、input-470k/220k-gnd で抵抗分圧されている。
    voltage *= 3; // 抵抗分圧分、元に戻す. 3.14だけど整数で。
    //    int level = (voltage - (int32_t)(3.2 / 3.6 * 1024)) / (int32_t) ((4.0 - 3.2)/3.6  * 1024);
    level = (voltage - 910) * 100 / 228;
    // 値域を100-0にする
    level = MIN(level, 100);
    level = MAX(level, 0);
    
    return (uint8_t)level;
}

