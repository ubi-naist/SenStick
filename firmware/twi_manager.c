#include <string.h>

#include <nrf_drv_twi.h>
#include <nrf_drv_gpiote.h>
#include <nrf_delay.h>
#include <sdk_errors.h>
#include <app_error.h>

#include "senstick_io_definitions.h"

static nrf_drv_twi_t twi;

ret_code_t TwiSlave_TX(uint8_t address, uint8_t const *p_data, uint32_t length, bool xfer_pending)
{
    return nrf_drv_twi_tx(&twi, address, p_data, length, xfer_pending);
}

ret_code_t TwiSlave_RX(uint8_t address, uint8_t *p_data, uint32_t length, bool xfer_pending)
{
    return nrf_drv_twi_rx(&twi, address, p_data, length, xfer_pending);
}


bool writeToTwiSlave(uint8_t twi_address, uint8_t target_register, const uint8_t *data, uint8_t length)
{
    ret_code_t err_code;
    
    // 先頭バイトは、レジスタアドレス
    uint8_t buffer[length + 1];
    buffer[0] = target_register;
    memcpy(&(buffer[1]), data, length);
    
    // I2C書き込み
    err_code = nrf_drv_twi_tx(&twi, twi_address, buffer, (length + 1), false);
    return (err_code == NRF_SUCCESS);
}

bool readFromTwiSlave(uint8_t twi_address, uint8_t target_register, uint8_t *data, uint8_t length)
{
    ret_code_t err_code;
    
    // 読み出しターゲットアドレスを設定
    uint8_t buffer0 = target_register;
    err_code = nrf_drv_twi_tx(&twi, twi_address, &buffer0, 1, true);
    if(err_code != NRF_SUCCESS) {
        return false;
    }
    
    // データを読み出し
    err_code = nrf_drv_twi_rx(&twi, twi_address, data, length, false);
    return (err_code == NRF_SUCCESS);
}

void initTWIManager(void)
{
    ret_code_t err_code;
    
    // TWIインタフェース TWI1を使用。
    twi.p_reg        = NRF_TWI1;
    twi.irq          = TWI1_IRQ;
    twi.instance_id  = TWI1_INSTANCE_INDEX;
    
    err_code = nrf_drv_twi_init(&twi, NULL, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&twi);
    
    // gpioteモジュールを初期化する
    if(!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    
    // TWIの電源のIOピン設定
    // out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);
    nrf_drv_gpiote_out_config_t out_config;
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH;
    out_config.task_pin   = false;
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

    // センサーの電源があがる200ミリ秒を待つ。
    nrf_delay_ms(200);
}
