//
//  twi_slave_utility.c
//  senstick
//
//  Created by AkihiroUehara on 2015/12/27.
//
//

#include "twi_slave_utility.h"

#include <string.h>
#include "sdk_errors.h"
#include "app_error.h"

void writeToTwiSlave(nrf_drv_twi_t *p_twi, uint8_t twi_address, uint8_t target_register, const uint8_t *data, uint8_t data_length)
{
    ret_code_t err_code;
    
    // 先頭バイトは、レジスタアドレス
    uint8_t buffer[data_length + 1];
    buffer[0] = target_register;
    memcpy(&(buffer[1]), data, data_length);
    
    // I2C書き込み
    err_code = nrf_drv_twi_tx(p_twi, twi_address, buffer, (data_length + 1), false);
    APP_ERROR_CHECK(err_code);
}

void readFromTwiSlave(nrf_drv_twi_t *p_twi, uint8_t twi_address, uint8_t target_register, uint8_t *data, uint8_t data_length)
{
    ret_code_t err_code;
    
    // 読み出しターゲットアドレスを設定
    uint8_t buffer0 = target_register;
    err_code = nrf_drv_twi_tx(p_twi, twi_address, &buffer0, 1, true);
    APP_ERROR_CHECK(err_code);
    
    // データを読み出し
    err_code = nrf_drv_twi_rx(p_twi, twi_address, data, data_length, false);
    APP_ERROR_CHECK(err_code);
}
