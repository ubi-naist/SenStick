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

#include "twi_slave_humidity_sensor.h"

#include "senstick_sensor_base_data.h"
#include "senstick_io_definition.h"

// 仕様書
// Datasheet SHT20
// Humidity and Temperature Sensor IC
// Release Date: Version 4 – May 2014

typedef enum {
    TriggerTempMeasurementHoldMaster  = 0xe3, //1110_0011
    TriggerRHMeasurementHoldMaster    = 0xe5, //1110_0101
    TriggerTempMeasurement  = 0xf3, //1111_0011
    TriggerRHMeasurement    = 0xf5, //1111_0101
    WriteUserRegister       = 0xe6, //1110_0110
    ReadUserRegister        = 0xe7, //1110_0111
    SoftReset               = 0xfe, //1111_1110
} SHT20Command_t;

/**
 * Private methods
 */

static bool writeToSHT20(const SHT20Command_t command, const uint8_t *p_data, const uint8_t data_length)
{
    return writeToTwiSlave(TWI_SHT20_ADDRESS, (uint8_t)command, p_data, data_length);
}
static bool readFromSHT20(const SHT20Command_t command, uint8_t *data, const uint8_t data_length)
{
    return readFromTwiSlave(TWI_SHT20_ADDRESS, (uint8_t)command, data, data_length);
}

/**
* public methods
*/
bool initHumiditySensor(void)
{
    // ソフトウェア・リセット
    writeToSHT20(SoftReset, NULL, 0);
    nrf_delay_ms(15); // リセット処理待ち
    
    // ユーザレジスタを設定する。[5:3]は、リセット後からのデフォルト値のまま保持しなければならない。
    // Bit
    // 7,0
    // RH           Temperature
    //    ‘00’ 12 bit    14 bit
    //    ‘01’ 8  bit    12 bit
    //    ‘10’ 10 bit    13 bit
    //    ‘11’ 11 bit    11 bit
    // 6    Status: End of battery1
    //      ‘0’: VDD > 2.25V
    //      ‘1’: VDD < 2.25V
    // 3,4,5    Reserved.
    // 2    Enable on-chip heater
    // 1    Disable OTP Reload
    
    uint8_t user_reg[2];
    
    // レジスタ読み出し, レジスタ値+チェックサム
    readFromSHT20( ReadUserRegister, user_reg, 2);
    
    // レジスタを設定
    // RH 12-bit T 14-bit, Disable on-chip heater, Disable OTP Reload
    // FIXME RH 12-bit T 14-bit モード ('00')だと、データが1バイトしか読み出されない。なぜ?
//    user_reg[0] = (user_reg[0] & ~0x81) | 0x80; // ~(1000_0001), bit 7,0 を00 (RH 12-bit, T 14-bit)に
    user_reg[0] = (user_reg[0] & ~0x81) | 0x81; // ~(1000_0001), bit 7,0 を11 (RH 11-bit, T 11-bit)に
    user_reg[0] = (user_reg[0] & ~0x04) | 0x00; // Enable on-chip heater 0
    user_reg[0] |= 0x02;  // Diable OTP Reload 1
    
    // レジスタの値を書き込み
    bool result = writeToSHT20( WriteUserRegister, &(user_reg[0]), 1);
    
    return result;
}

// 計測時間中はI2Cバスを離さない。
// 相対湿度計測 12-bit精度 typ 22ミリ秒 max 30ミリ秒
// 温度計測 14-bit精度 typ 66ミリ秒 max 85ミリ秒
void getHumidityData(HumidityData_t *p_data)
{
    uint8_t buffer[3];
    readFromSHT20(TriggerRHMeasurementHoldMaster, buffer, 3);
    
    // データをデコードする
    *p_data = ((uint16_t)buffer[0] << 8) | ((uint16_t)buffer[0] & 0xfc);
//NRF_LOG_PRINTF_DEBUG("getHumidityData() 0x%04x\n", *p_data);
    /*
    ret_code_t err_code;
    
    // 読み出しターゲットアドレスを設定
    buffer[0] = TriggerRHMeasurementHoldMaster;
    err_code = nrf_drv_twi_tx(p_context->p_twi, TWI_SHT20_ADDRESS, buffer, 1, true);
    APP_ERROR_CHECK(err_code);
    
    // データを読み出し
//    err_code = nrf_drv_twi_rx(p_context->p_twi, TWI_SHT20_ADDRESS, buffer, 3, false);
        err_code = nrf_drv_twi_rx(p_context->p_twi, TWI_SHT20_ADDRESS, buffer, 3, false);
    APP_ERROR_CHECK(err_code);
    
//    *p_data = (uint16_t)(buffer[1] & 0x3f) << 8 | (uint16_t)buffer[0];
     */
}

void getTemperatureData(TemperatureData_t *p_data)
{
    uint8_t buffer[3];
    readFromSHT20( TriggerTempMeasurementHoldMaster, buffer, 3);
    
    // データをデコードする
    *p_data = ((uint16_t)buffer[0] << 8) | ((uint16_t)buffer[0] & 0xfc);
//NRF_LOG_PRINTF_DEBUG("getTemperatureData() 0x%04x\n", *p_data);
}
