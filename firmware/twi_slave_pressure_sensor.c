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

#include "twi_slave_pressure_sensor.h"

#include "senstick_sensor_base_data.h"
#include "senstick_io_definition.h"

// 仕様書
// LPS25HB
// MEMS pressure sensor: 260-1260 hPa absolute digital output barometer
// Document Number: DocID027112 Rev 2
// Release Date: May/2015

// レジスタアドレスの列挙型
typedef enum {
    // 制御レジスタ
    CTRL_REG1 = 0x20,
    CTRL_REG2 = 0x21,
    CTRL_REG3 = 0x22,
    CTRL_REG4 = 0x23,

    // 圧力データ
    PRESS_OUT_XL = (0x28 | 0x80), // 読み出し時のアドレス自動インクリメントのため、最上位ビットを1にしている
    PRESS_OUT_L  = (0x29 | 0x80),
    PRESS_OUT_H  = (0x2a | 0x80),
} LPS25HBRegister_t;

/**
 * private methods
 */

static bool writeToLPS25HB(const LPS25HBRegister_t target_register, const uint8_t *data, const uint8_t data_length)
{
   return writeToTwiSlave(TWI_LPS25HB_ADDRESS, (uint8_t)target_register, data, data_length);
}

static bool readFromLPS25HB(const LPS25HBRegister_t target_register, uint8_t *data, const uint8_t data_length)
{
   return readFromTwiSlave(TWI_LPS25HB_ADDRESS, (uint8_t)target_register, data, data_length);
}

// 指定されたビットを立てます
/*
static void setRegisterBitOfLPS25HB( const LPS25HBRegister_t target_register, const uint8_t bit_mask, bool set_active)
{
    // ターゲットの値を読みだす
    uint8_t reg_value;
    readFromLPS25HB( target_register, &reg_value, 1);
    
    // ビットをいじる
    if(set_active) {
        reg_value |= bit_mask;
    } else {
        reg_value &= ~bit_mask;
    }
    
    // 書き戻す
    writeToLPS25HB( target_register, &reg_value, 1);
}
*/

/**
 * public methods
 */
bool initPressureSensor(void)
{
// CTRL_REG1
// 7   PD        Power-down control. Default value: 0 (0: power-down mode; 1: active mode)
// 6:4 ODR [2:0] Output data rate selection. Default value: 000 Refer to Table 19.
// 3   DIFF_EN   Interrupt generation enable. Default value: 0
//              (0: interrupt generation disabled; 1: interrupt generation enabled)
// 2   BDU       Block data update. Default value: 0
//              (0: continuous update;
//              1: output registers not updated until MSB and LSB have been read)
// 1   RESET_AZ  Reset Autozero function. Default value: 0. (0: normal mode; 1: reset Autozero function)
// 0   SIM       SPI Serial Interface Mode selection.Default value: 0 (0: 4-wire interface; 1: 3-wire interface)

    // Active, one-shot mode,  1_000_0000
    const uint8_t data_CTRL_REG1[] = {0x80};
    bool result = writeToLPS25HB( CTRL_REG1, data_CTRL_REG1, sizeof(data_CTRL_REG1));
    
    return result;
}

void getPressureData(AirPressureData_t *p_data)
{
    // 変換済のはずの値を読みだす
    // データは、PRESS_OUT_XLから一連の連続するアドレスから読み出す。
    uint8_t buffer[4];
    memset(buffer, 0, sizeof(buffer));
    readFromLPS25HB( PRESS_OUT_XL, buffer, 3);
    
    *p_data = readUInt32AsLittleEndian(buffer);
    
 // 次の変換
// CTRL_REG2
// 7   BOOT      Reboot memory content. Default value: 0.
//              (0: normal mode; 1: reboot memory content). The bit is self-cleared when the BOOT is completed.
// 6   FIFO_EN   FIFO enable. Default value: 0. (0: disable; 1: enable)
// 5   STOP_ON_FTH   Stop on FIFO threshold. Enable FIFO watermark level use. Default value 0 (0: disable; 1: enable).
// 4   FIFO_MEAN_DEC Register address automatically incremented during a multiple byte access with a serial interface (I2C or SPI). Default value 1.
//                  (0: disable; 1 enable)
// 3   I2C_EN    I2C interface enabled. Default value 0. (0: I2C enabled;1: I2C disabled)
// 2   SWRESET   Software reset. Default value: 0.
//              (0: normal mode; 1: software reset).
//              The bit is self-cleared when the reset is completed.
// 1   AUTOZERO  Autozero enable. Default value: 0.
//              (0: normal mode; 1: Autozero enabled)
// 0   ONE_SHOT  One shot mode enable. Default value: 0. (0: idle mode; 1: a new dataset is acquired)

    // 0001_0001
    const uint8_t data_CTRL_REG2[] = {0x11};
    writeToLPS25HB( CTRL_REG2, data_CTRL_REG2, sizeof(data_CTRL_REG2));
}
/*
bool isPressureSensor(void)
{
    ret_code_t err_code;
    
    // I2Cバスのアドレス TWI_LPS25HB_ADDRESS に、ターゲットのレジスタ CTRL_REG1 に 1バイト 0x80 を書き込む。
    // 先頭バイトは、レジスタアドレス
    uint8_t buffer[1 + 1]; // data_length + 1
    buffer[0] = CTRL_REG1;
    buffer[1] = 0x80;
    
    // I2C書き込み
    err_code = nrf_drv_twi_tx(p_twi, TWI_LPS25HB_ADDRESS, buffer, sizeof(buffer), false);
    return (err_code == NRF_SUCCESS);
}
*/
