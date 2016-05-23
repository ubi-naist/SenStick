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

#include "twi_slave_brightness_sensor.h"

#include "senstick_sensor_base_data.h"
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

static bool writeToBH1780GLI(const BH1780GLI_RegisterType target_reg, uint8_t data)
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
    bool result = writeToBH1780GLI(Control, 0x03);
    
    return result;
}

void getBrightnessData(BrightnessData_t *p_data)
{
    uint8_t buff[2];
    
    readFromTwiSlave(TWI_BH1780GLI_ADDRESS, DataLow, buff, 2);
    
    *p_data = (uint16_t)buff[1] << 8 | (uint16_t)buff[0];
}
