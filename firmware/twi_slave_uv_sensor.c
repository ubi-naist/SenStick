#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_assert.h>
#include <app_error.h>
#include <sdk_errors.h>

#include "value_types.h"
#include "twi_manager.h"

#include "twi_slave_uv_sensor.h"

#include "senstick_sensor_base_data.h"
#include "senstick_io_definitions.h"

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
bool initUVSensor(void)
{
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
    err_code = TwiSlave_TX(TWI_VEML6070_ADDRESS, &data, 1, false);
    
    return (err_code == NRF_SUCCESS);
}

void getUVSensorData(UltraVioletData_t *p_data)
{
    /*
     • Slave addresses (8 bits) for data read: 0x71 and 0x73
     • Data reading sequence for the host:
     -Set read command to 0x73, read MSB 8 bits of 16 bits light data (sequence 1)
     -Set read command to 0x71, read LSB 8 bits of 16 bits light data for completing data structure (sequence 2)
     */
    ret_code_t err_code;

    uint8_t data0;
    err_code = TwiSlave_RX(TWI_VEML6070_RD_ADDRESS, &data0, 1, false);
    APP_ERROR_CHECK(err_code);

    uint8_t data1;
    err_code = TwiSlave_RX(TWI_VEML6070_ADDRESS, &data1, 1, false);
    APP_ERROR_CHECK(err_code);
    
    *p_data = ((uint16_t)data0 << 8) | (uint16_t)data1 ;
}
