#include "twi_slave_uv_sensor.h"

#include <string.h>

#include "nrf_delay.h"
#include "nrf_drv_twi.h"
#include "app_error.h"

#include "twi_slave_utility.h"
#include "senstick_io_definitions.h"

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
void initUVSensor(uv_sensor_context_t *p_context, nrf_drv_twi_t *p_twi)
{
    memset(p_context, 0, sizeof(uv_sensor_context_t));
    p_context->p_twi = p_twi;
    
    // レジスタの初期設定
    ret_code_t err_code;
    uint8_t data;
    
    // Bit
    // 7,6  Reserved ('00')
    // 5    ACK, Acknowledge activity setting
    // 4    ACK_THD Acknowledge threshold window setting for byte mode usage
    // 3,2  IT, Integration time setting
    // 1    Reserved ('1')
    // 0    SD, Shutdown mode setting

    // ACK disable, Shutdown mode disable, Integration time '01' 1T, 0000_0110, 0x06
    // 外付け抵抗が560kohm, 1T = 250ミリ秒位
    data = 0x06;
    err_code = nrf_drv_twi_tx(p_context->p_twi, TWI_VEML6070_ADDRESS, &data, 1, false);
    APP_ERROR_CHECK(err_code);
}

void getUVSensorData(uv_sensor_context_t *p_context, UltraVioletData_t *p_data)
{
    /*
     • Slave addresses (8 bits) for data read: 0x71 and 0x73
     • Data reading sequence for the host:
     -Set read command to 0x73, read MSB 8 bits of 16 bits light data (sequence 1)
     -Set read command to 0x71, read LSB 8 bits of 16 bits light data for completing data structure (sequence 2)
     */
    ret_code_t err_code;

    uint8_t data0;
    err_code = nrf_drv_twi_rx(p_context->p_twi, TWI_VEML6070_RD_ADDRESS, &data0, 1, false);
    APP_ERROR_CHECK(err_code);

    uint8_t data1;
    err_code = nrf_drv_twi_rx(p_context->p_twi, TWI_VEML6070_ADDRESS, &data1, 1, false);
    APP_ERROR_CHECK(err_code);
    
    *p_data = ((uint16_t)data0 << 8) | (uint16_t)data1 ;
}
