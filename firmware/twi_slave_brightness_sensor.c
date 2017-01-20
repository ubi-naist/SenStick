#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_drv_twi.h>
#include <nrf_assert.h>
#include <app_error.h>
#include <sdk_errors.h>

#include "value_types.h"
#include "twi_manager.h"

#include "twi_slave_brightness_sensor.h"

#include "senstick_sensor_base_data.h"
#include "senstick_io_definition.h"

typedef enum {
    Control         = 0x0,
    PartID          = 0x0a,
    ManufactureID   = 0x0b,
    DataLow         = 0x0c,
    DataHigh        = 0x0d,
} BH1780GLI_RegisterType;
/**
 * Private methods
 */

static bool writeToBH1780GLI(uint8_t target_reg, uint8_t data)
{
    return writeToTwiSlave(TWI_BH1780GLI_ADDRESS, target_reg, &data, 1);
}

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
bool initBrightnessSensor(void)
{
    // レジスタの初期設定
    // CONTROLレジスタ
    // 7:2  0
    // 1:0
    //      "00" : Power down
    //      "01" : Resv
    //      "10" : Resv
    //      "11" : Power up

    // Power up
    bool result = writeToBH1780GLI((uint8_t)Control | (uint8_t)0x80, 0x03);
    
    return result;
}

void triggerBrightnessData(void)
{
    // 読み出しレジスタのアドレスを設定
    ret_code_t err_code;
    uint8_t data = 0x80 | (uint8_t)DataLow; // 1000_1100
    err_code = nrf_drv_twi_tx(&twi, TWI_BH1780GLI_ADDRESS, &data, 1, false);
    APP_ERROR_CHECK(err_code);
}

void getBrightnessData(BrightnessData_t *p_data)
{
    /*
    uint8_t buff[2];
    
    readFromTwiSlave(TWI_BH1780GLI_ADDRESS, (uint8_t)DataLow | 0x80, buff, 2);
    
    *p_data = (uint16_t)buff[1] << 8 | (uint16_t)buff[0];
     */
    /*
    // 読み出しレジスタのアドレスを設定
    ret_code_t err_code;
    uint8_t data = 0x80 | (uint8_t)DataLow; // 1000_1100
    err_code = nrf_drv_twi_tx(&twi, TWI_BH1780GLI_ADDRESS, &data, 1, false);
    APP_ERROR_CHECK(err_code);
    
    nrf_delay_ms(150); // 150ミリ秒待つ
    */
    // データを読み出し
    ret_code_t err_code;
    uint8_t buffer[2];
#ifdef NRF51
    err_code = nrf_drv_twi_rx(&twi, TWI_BH1780GLI_ADDRESS, buffer, 2, false);
    APP_ERROR_CHECK(err_code);
#else // NRF52
    err_code = nrf_drv_twi_rx(&twi, TWI_BH1780GLI_ADDRESS, buffer, 2);
    APP_ERROR_CHECK(err_code);
#endif
    
    *p_data = (uint16_t)buffer[1] << 8 | (uint16_t)buffer[0];
    
//    NRF_LOG_PRINTF_DEBUG("getBrightnessData() 0x%02x\n", *p_data);
}

/*
#include "twi_slave_brightness_sensor.h"

#include <string.h>

#include "nrf_delay.h"
#include "nrf_drv_twi.h"
#include "app_error.h"

#include "senstick_io_definition.h"
*/

/*
static bool writeToBH1780GLI(const BH1780GLI_RegisterType target_reg, const uint8_t data)
{
    uint8_t buffer[2];
    
    // コマンドレジスタ
    buffer[0] = 0x80 | (uint8_t)target_reg;
    buffer[1] = data;
    
    // I2C書き込み
    ret_code_t err_code;
    err_code = nrf_drv_twi_tx(&twi, TWI_BH1780GLI_ADDRESS, buffer, 2, false);
    return (err_code == NRF_SUCCESS);
}

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
bool initBrightnessSensor(void)
{
    // レジスタの初期設定
    // CONTROLレジスタ
    // 7:2  0
    // 1:0
    //      "00" : Power down
    //      "01" : Resv
    //      "10" : Resv
    //      "11" : Power up
    
    // Power up
    return writeToBH1780GLI(Control, 0x03);
}

void getBrightnessData(BrightnessData_t *p_data)
{
    // 読み出しレジスタのアドレスを設定
    ret_code_t err_code;
    uint8_t data = 0x80 | (uint8_t)DataLow; // 1000_1100
    err_code = nrf_drv_twi_tx(&twi, TWI_BH1780GLI_ADDRESS, &data, 1, false);
    APP_ERROR_CHECK(err_code);
    
    // データを読み出し
    uint8_t buffer[2];
    err_code = nrf_drv_twi_rx(&twi, TWI_BH1780GLI_ADDRESS, buffer, 2, false);
    APP_ERROR_CHECK(err_code);
    
    *p_data = (uint16_t)buffer[1] << 8 | (uint16_t)buffer[0];
}
*/
