#include <stdint.h>
#include "nrf_drv_gpiote.h"
#include "nrf_adc.h"
#include "app_error.h"

#include "senstick_core_manager.h"
#include "senstick_pin_definitions.h"
/**
 * イベントハンドラ
 */
static void gpio_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    switch(pin) {
        case PIN_NUMBER_TACT_SWITCH:
            break;
        default:
            break;
    }
}

/**
* Private メソッド
*/

// IO初期化ルーチン
static void init_gpio(void)
{
    uint32_t err_code;
    nrf_drv_gpiote_out_config_t out_config;
    nrf_drv_gpiote_in_config_t in_config;
    
    // gpioteモジュールを初期化する
    if(!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    
    // LEDピンの設定
    // 引数は init_high。初期値をlowにする。
//    out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
    out_config.task_pin = false;
    err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_LED, &out_config);
    APP_ERROR_CHECK(err_code);
    
    // タクトスイッチの設定
    // P0.20    In      タクトスイッチ接続。スイッチオンでGNDに落とされる。
    // 引数は high accuracy。精度はいらないのでfalseを指定。
//    in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    in_config.is_watcher  = false;
    in_config.hi_accuracy = false;
    in_config.sense       = NRF_GPIOTE_POLARITY_HITOLO;
    in_config.pull        = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(PIN_NUMBER_TACT_SWITCH, &in_config, gpio_event_handler);
    APP_ERROR_CHECK(err_code);

    // TWIのデバイス電源設定
    // 引数は init_high。初期値をhighにする。
//    out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH;
    out_config.task_pin = false;
    err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_TWI_POWER, &out_config);
    APP_ERROR_CHECK(err_code);
    // ドライブストレンクスを"強"に
    nrf_gpio_cfg(PIN_NUMBER_TWI_POWER,
                     NRF_GPIO_PIN_DIR_OUTPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_H0H1,   // 0にハイドライブ、1にハイドライブ
                     NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_set(PIN_NUMBER_TWI_POWER);
    
    // SPIのnCS
//    out_config = GPIOTE_CONFIG_OUT_SIMPLE(false); // 引数は init_high。初期値をlowにする。
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
    out_config.task_pin = false;
    err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_SPI_nCS, &out_config);
    APP_ERROR_CHECK(err_code);
    
    // 以下のピンは、今は利用しない
    // P0.08    In      9軸センサーINT
    // P0.09    In      RTC INTIR
    // P0.27    In      32kHzクロックが供給される
}

static uint16_t getAdcValue(void)
{
    uint16_t value;
    
    // Read ADC Battery Level
    NRF_ADC->ENABLE         = ADC_ENABLE_ENABLE_Enabled;
    NRF_ADC->EVENTS_END     = 0;    // Stop any running conversions.
    NRF_ADC->TASKS_START    = 1;
    
    while(NRF_ADC->EVENTS_END == 0); // 変換完了待ち
    
    NRF_ADC->EVENTS_END     = 0;
    value                   = NRF_ADC->RESULT;
    NRF_ADC->TASKS_STOP     = 1;
    NRF_ADC->ENABLE         = ADC_ENABLE_ENABLE_Disabled;
    
    return value;
}

// ADC初期化関数
static void init_adc(void)
{
    // 10ビット解像度、スケーリング 1/3、リファレンス バンドギャップ電圧()
    nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG_DEFAULT;
    
    // Initialize and configure ADC
    nrf_adc_configure(&nrf_adc_config);
    nrf_adc_input_select(ADC_INPUT_SUPPLY_MONITORING);
}

/**
 * Public関数
 */
void init_senstick_core_manager(void)
{
    init_gpio();
    init_adc();

}

