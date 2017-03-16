//
//  twi_ext_motor.c
//  senstick
//
//  Created by AkihiroUehara on 2017/02/23.
//
//

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

#include "twi_ext_motor.h"
#include "senstick_io_definition.h"


// 仕様書 http://www.tij.co.jp/product/jp/DRV8830/
// SLVSAB2G – MAY 2010 – REVISED DECEMBER 2015


// IOポートのアドレス定義, I2Cバス上の、スレーブ・アドレス
#define TWI_DRV8830_ADDRESS     0x64 // 110_0100 最下位4ビットはピン値で与えられる。この基板はA1/A0がオープン。

// レジスタアドレスの列挙型
typedef enum {
    DRV8830_CONTROL_REG = 0x00,
    DRV8830_FAULT_REG   = 0x01
} DRV8830Register_t;


/**
 * private methods
 */

static bool writeToDRV8830(const DRV8830Register_t target_register, const uint8_t *data, const uint8_t data_length)
{
    return writeToTwiSlave(TWI_DRV8830_ADDRESS, (uint8_t)target_register, data, data_length);
}

static bool readFromDRV8830(const DRV8830Register_t target_register, uint8_t *data, const uint8_t data_length)
{
    return readFromTwiSlave(TWI_DRV8830_ADDRESS, (uint8_t)target_register, data, data_length);
}

/**
 * public methods
 */

bool initDRV8830MotorDriver(void)
{
    // Control register
    //  7..2    VSET
    //  1       IN2
    //  0       IN1
    //
    //  VSETは出力電圧を決める、0x06 ~ 0x3Fまでの値。
    //  4 x VREF x (VSET +1) / 64, where VREF is the internal 1.285-V reference.
    // つまり (VSET +1) * 0.8 V
    //
    //  IN2     IN1     OUT1    OUT2
    //  0       0       Z       Z       Standby/coast
    //  1       0       L       H       Reverse
    //  0       1       H       L       Forward
    //  1       1       H       H       Break
    //

    // VSET 0x2a(3.37V), 0x03 break
    const uint8_t data = (0x2a << 2) | 0x00;
    bool result = writeToDRV8830( DRV8830_CONTROL_REG, &data, sizeof(data));
    // faultレジスタをクリア。
    if(result) {
        clearDRV8830MotorDriverFaultRegister();
    }
    return result;
}

// controlレジスタを設定します。
void setDRV8830MotorDriverControlRegister(uint8_t value)
{
    writeToDRV8830(DRV8830_CONTROL_REG, &value, sizeof(value));
}

// faultレジスタを読み出します。
uint8_t getDRV8830MotorDriverFaultRegister(void)
{
    uint8_t value;
    readFromDRV8830( DRV8830_FAULT_REG, &value, sizeof(value));
    return value;
}

// faultレジスタの値をクリアします。
void clearDRV8830MotorDriverFaultRegister(void)
{
    // Falut register
    // 7        clear
    // 6..5     not used
    // 4        ilimit, current limit
    // 3        ots, over temperature
    // 2        uvlo, under voltate
    // 1        ocp, over current protection.
    // 0        fault
    const uint8_t value = 0x80;
    writeToDRV8830( DRV8830_FAULT_REG, &value, sizeof(value));
}
