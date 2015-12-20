
#include "senstick_core_manager.h"

/**
* Senstick IO初期化ルーチン
*/
static init_io(void)
{
    uint32_t err_code;
    nrf_drv_gpiote_out_config_t config;
    
    // gpioteモジュールを初期化する
    if(!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    
    // LEDピンの設定
    config = GPIOTE_CONFIG_OUT_SIMPLE(false); // 引数は init_high。初期値をlowにする。
    err_code = nrf_drv_gpiote_out_config(PIN_NUMBER_LED, &config);
    APP_ERROR_CHECK(err_code);
    
    
    // 割り込み入力ピンの設定
    // P0.20    In      タクトスイッチ接続。スイッチオンでGNDに落とされる。
    // P0.08    In      9軸センサーINT
    // P0.09    In      RTC INTIR
#define PIN_NUMBER_TACT_SWITCH  20
    config = GPIOTE_CONFIG_IN_SENSE_HITOLO; // HIGH to LOW を検出。
    config.pull = NRF_GPIO_PIN_PULLUP; // Pull upする。

    err_code = nrf_drv_gpiote_in_config(10, &config, pin_event_handler);
// センサーのINTは、使わない
//#define PIN_NUMBER_9AXIS_INT    8
//#define PIN_NUMBER_RTC_INT      9
    


        
        
#if SDK_VERSION == 6
        APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
#else // SDK_VERSION == 8
        ret_code_t err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
#endif
        
        init_nrf_gpio_range_cfg_input_disconnect(0 , 30 , NRF_GPIO_PIN_NOPULL);
        
        nrf_gpio_cfg_output(PIN_NUMBER_RESETB);
        nrf_gpio_pin_set(PIN_NUMBER_RESETB);
        
        nrf_gpio_cfg_input(PIN_NUMBER_UV_INT , NRF_GPIO_PIN_NOPULL);
        nrf_gpio_cfg_input(PIN_NUMBER_RTC_INT_PIN , NRF_GPIO_PIN_NOPULL);
    }


    // UART
    // P0.23    Out     シリアルTXD。(CP2104のRxDに接続。)
    // P0.28    In      シリアルRXD。(CP2104のTxDに接続。)
#define PIN_NUMBER_UART_TXD 23
#define PIN_NUMBER_UART_RXD 28
    

    
    // P0.27    In      32kHzクロックが供給される
    // P0.05    In      電源電圧。(470k/220k分圧、0.32倍)
#define PIN_NUMBER_32kHz_CLK            27
#define PIN_NUMBER_SUPPLY_MONITORING    5
    
    // TWI
    // P0.21            SDA
    // P0.24            SCL
    // P0.03    Out     TWIの電源
#define PIN_NUMBER_TWI_SDA   21
#define PIN_NUMBER_TWI_SCL   24
#define PIN_NUMBER_TWI_POWER 3
    
    // SPI
    // P0.00        /CS
    // P0.01        DO
    // P0.11        DI
    // P0.12        CLK
#define PIN_NUMBER_SPI_CS   0
#define PIN_NUMBER_SPI_DO   1
#define PIN_NUMBER_SPI_DI   11
#define PIN_NUMBER_SPI_CLK  12
    
}
