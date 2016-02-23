#include <stdint.h>
#include <string.h>

#include <nordic_common.h>

#include <nrf_drv_gpiote.h>
#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_drv_config.h>
#include <nrf_adc.h>
#include <app_error.h>

#include "senstick_definitions.h"
#include "senstick_io_definitions.h"
#include "senstick_data_models.h"

#include "gpio_manager.h"

/*
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

/*
 * Private methods
 */

// IO初期化ルーチン
static void init_gpio(void)
{
    ret_code_t err_code;
    
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
    
    // 以下のピンは、今は利用しない
    // P0.08    In      9軸センサーINT
    // P0.09    In      RTC INTIR
    // P0.27    In      32kHzクロックが供給される
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

/*
 * Public methods
 */
void initGPIOManager(gpio_manager_t *p_context)
{
    memset(p_context, 0, sizeof(gpio_manager_t));
    
    init_gpio();
    init_adc();
    
    LEDDriverInit(&(p_context->led_driver_context));
}

void setTWIPowerSupply(gpio_manager_t *p_context, bool available)
{
    if(available) {
        nrf_drv_gpiote_out_set(PIN_NUMBER_TWI_POWER);
    } else {
        nrf_drv_gpiote_out_clear(PIN_NUMBER_TWI_POWER);
    }
}

uint8_t getBatteryLevel(gpio_manager_t *p_context)
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

void setLEDBlink(gpio_manager_t *p_context, uint16_t period, uint16_t blink)
{
    setLEDDriverBlinkMode(&(p_context->led_driver_context), period, blink);
}
