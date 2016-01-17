#include <stdint.h>
#include <string.h>

#include "nordic_common.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_drv_config.h"
#include "nrf_adc.h"

#include "app_util_platform.h"
#include "app_error.h"

#include "senstick_core_manager.h"
#include "senstick_io_definitions.h"
#include "senstick_data_models.h"

/**
 * 型宣言
 */

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

static void init_spi_slaves(senstick_core_t *p_context)
{
    ret_code_t err_code;
    // SPIインタフェース SPI0を使用。
    
    /*
     #define NRF_DRV_SPI_INSTANCE(id)                        \
     {                                                       \
     .p_registers  = NRF_DRV_SPI_PERIPHERAL(id),         \     #define NRF_DRV_SPI_PERIPHERAL(id)  (void *)CONCAT_2(NRF_SPI, id)
     .irq          = CONCAT_3(SPI, id, _IRQ),            \
     .drv_inst_idx = CONCAT_3(SPI, id, _INSTANCE_INDEX), \
     .use_easy_dma = CONCAT_3(SPI, id, _USE_EASY_DMA)    \
     }     */
    p_context->spi.p_registers  = NRF_SPI0;
    p_context->spi.irq          = SPI0_IRQ;
    p_context->spi.drv_inst_idx = SPI0_INSTANCE_INDEX;
    p_context->spi.use_easy_dma = SPI0_USE_EASY_DMA;
    
//    nrf_drv_spi_config_t config = NRF_DRV_SPI_DEFAULT_CONFIG(0);
    /*
#define NRF_DRV_SPI_DEFAULT_CONFIG(id)                       \
    {                                                            \
        .sck_pin      = CONCAT_3(SPI, id, _CONFIG_SCK_PIN),      \
        .mosi_pin     = CONCAT_3(SPI, id, _CONFIG_MOSI_PIN),     \
        .miso_pin     = CONCAT_3(SPI, id, _CONFIG_MISO_PIN),     \
        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,                \
        .irq_priority = CONCAT_3(SPI, id, _CONFIG_IRQ_PRIORITY), \
        .orc          = 0xFF,                                    \
        .frequency    = NRF_DRV_SPI_FREQ_4M,                     \
        .mode         = NRF_DRV_SPI_MODE_0,                      \
        .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,         \
    }
     */
    nrf_drv_spi_config_t config;
    config.sck_pin      = SPI0_CONFIG_SCK_PIN;
    config.mosi_pin     = SPI0_CONFIG_MOSI_PIN;
    config.miso_pin     = SPI0_CONFIG_MISO_PIN;
    config.ss_pin       = NRF_DRV_SPI_PIN_NOT_USED;
    config.irq_priority = SPI0_CONFIG_IRQ_PRIORITY;
    config.orc          = 0xff;
    config.frequency    = NRF_DRV_SPI_FREQ_4M;
//    config.frequency    = NRF_DRV_SPI_FREQ_250K;
    config.mode         = NRF_DRV_SPI_MODE_0;
    config.bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    
    err_code = nrf_drv_spi_init(&(p_context->spi), &config, NULL);
    APP_ERROR_CHECK(err_code);
    
    initFlashMemory(&(p_context->flash_memory_context), &(p_context->spi));
}

static void init_twi_slaves(senstick_core_t *p_context)
{
    ret_code_t err_code;
    
    // TWIインタフェース TWI1を使用。
    p_context->twi.p_reg        = NRF_TWI1;
    p_context->twi.irq          = TWI1_IRQ;
    p_context->twi.instance_id  = TWI1_INSTANCE_INDEX;

    err_code = nrf_drv_twi_init(&(p_context->twi), NULL, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&(p_context->twi));
    
    // slaveの初期化
    initNineAxesSensor(&(p_context->nine_axes_sensor_context), &(p_context->twi));
    initPressureSensor(&(p_context->pressure_sensor_context),  &(p_context->twi));
    initHumiditySensor(&(p_context->humidity_sensor_context),  &(p_context->twi));
    initUVSensor(&(p_context->uv_sensor_context),  &(p_context->twi));
    initBrightnessSensor(&(p_context->brightness_sensor_context),  &(p_context->twi));
    // 初期化処理完了待ち時間
    nrf_delay_ms(100);
}

static void test_twi_slaves(senstick_core_t *p_context)
{
    // 値取得、デバッグ
    while(1) {
        MotionSensorData_t sensor_data;
        getNineAxesData(&(p_context->nine_axes_sensor_context), &sensor_data);
        debugLogAccerationData(&(sensor_data.acceleration));
        
        AirPressureData_t pressure_data;
        getPressureData(&(p_context->pressure_sensor_context), &pressure_data);
        NRF_LOG_PRINTF_DEBUG("Pressure, %d.\n", pressure_data); // 0.01hPa resolution
        
        HumidityData_t humidity_data;
        getHumidityData(&(p_context->humidity_sensor_context), &humidity_data);
        NRF_LOG_PRINTF_DEBUG("Humidity, %d.\n", humidity_data);
        
        UltraVioletData_t uv_data;
        getUVSensorData(&(p_context->uv_sensor_context), &uv_data);
        NRF_LOG_PRINTF_DEBUG("UV, %d.\n", uv_data);
        
        BrightnessData_t brightness_data;
        getBrightnessData(&(p_context->brightness_sensor_context), &brightness_data);
        NRF_LOG_PRINTF_DEBUG("Brightness, %d.\n", brightness_data);
        
        NRF_LOG_PRINTF_DEBUG("\n");
        nrf_delay_ms(500);
    }
}
/**
 * Public関数
 */
void init_senstick_core_manager(senstick_core_t *p_context)
{
    memset(p_context, 0, sizeof(senstick_core_t));
    
    init_gpio();
    init_adc();
    
    // 周辺デバイスの起動待ち時間。MPU-9250最大100ミリ秒
    nrf_delay_ms(100);
    
    init_spi_slaves(p_context);
//    testFlashMemory(&(p_context->flash_memory_context));
//    testLogger(&(p_context->flash_memory_context));
    
    init_twi_slaves(p_context);
//    test_twi_slaves(p_context);
}

