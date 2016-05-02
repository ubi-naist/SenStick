#include "twi_slave_brightness_sensor.h"

#include <string.h>

#include "nrf_delay.h"
#include "nrf_drv_twi.h"
#include "app_error.h"

#include "twi_slave_utility.h"
#include "senstick_io_definitions.h"

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

static void writeToBH1780GLI(brightness_sensor_context_t *p_context, const BH1780GLI_RegisterType target_reg, const uint8_t data)
{
    uint8_t buffer[2];
    
    // コマンドレジスタ
    buffer[0] = 0x80 | (uint8_t)target_reg;
    buffer[1] = data;
    
    // I2C書き込み
    ret_code_t err_code;
    err_code = nrf_drv_twi_tx(p_context->p_twi, TWI_BH1780GLI_ADDRESS, buffer, 2, false);
    APP_ERROR_CHECK(err_code);
}

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
void initBrightnessSensor(brightness_sensor_context_t *p_context, nrf_drv_twi_t *p_twi)
{
    memset(p_context, 0, sizeof(brightness_sensor_context_t));
    p_context->p_twi = p_twi;
    
    // レジスタの初期設定
    // CONTROLレジスタ
    // 7:2  0
    // 1:0
    //      "00" : Power down
    //      "01" : Resv
    //      "10" : Resv
    //      "11" : Power up

    // Power up
    writeToBH1780GLI(p_context, Control, 0x03);
}

void getBrightnessData(brightness_sensor_context_t *p_context, BrightnessData_t *p_data)
{
    // 読み出しレジスタのアドレスを設定
    ret_code_t err_code;
    uint8_t data = 0x80 | (uint8_t)DataLow; // 1000_1100
    err_code = nrf_drv_twi_tx(p_context->p_twi, TWI_BH1780GLI_ADDRESS, &data, 1, false);
    APP_ERROR_CHECK(err_code);

    // データを読み出し
    uint8_t buffer[2];
    err_code = nrf_drv_twi_rx(p_context->p_twi, TWI_BH1780GLI_ADDRESS, buffer, 2, false);
    APP_ERROR_CHECK(err_code);
    
    *p_data = (uint16_t)buffer[1] << 8 | (uint16_t)buffer[0];
}
