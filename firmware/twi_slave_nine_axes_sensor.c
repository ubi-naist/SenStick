#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <nrf_delay.h>
#include <nrf_assert.h>
#include <app_error.h>
#include <sdk_errors.h>

#include "value_types.h"
#include "twi_manager.h"
#include "twi_slave_nine_axes_sensor.h"

#include "senstick_sensor_base_data.h"
#include "senstick_io_definition.h"

// レジスタ仕様書
// MPU-9250
// Register Map and Descriptions Revision 1.4
// Document Number: RM-MPU-9250A-00
// Revision: 1.4
// Release Date: 9/9/2013

// レジスタアドレスの列挙型
typedef enum {
    // 加速度、ジャイロセンサー
    GYRO_CONFIG     = 0x1b,
    ACCEL_CONFIG    = 0x1c,
    PWR_MGMT_1      = 0x6b,
    PWR_MGMT_2      = 0x6c,
    ACCEL_XOUT_H    = 0x3b,
    GYRO_XOUT_H     = 0x43,

    I2C_MST_CTRL    = 0x24,
    I2C_SLV0_ADDR   = 0x25,
    I2C_SLV0_REG    = 0x26,
    I2C_SLV0_CTRL   = 0x27,
    EXT_SENS_DATA_00= 0x49,
    
    USER_CTRL = 0x6a,
    INT_PIN_CFG = 0x37,
    
} MPU9250Register_t;

typedef enum {
    ST1   = 0x02,
    HXL   = 0x03,
    ST2   = 0x09,
    CNTL1  = 0x0a,
} AK8963Register_t;

/**
 * private methods
 */

// MPU9250に書き込みます。
// TWI_MPU9250_ADDRESS は senstick_io_definitions.h で定義されているI2Cバスのアドレスです。
static bool writeToMPU9250(MPU9250Register_t target_register, const uint8_t *data, uint8_t data_length)
{
    return writeToTwiSlave(TWI_MPU9250_ADDRESS, (uint8_t)target_register, data, data_length);
}

// MPU9250から読み込みます。
static void readFromMPU9250(MPU9250Register_t target_register, uint8_t *data, uint8_t data_length)
{
    readFromTwiSlave(TWI_MPU9250_ADDRESS, (uint8_t)target_register, data, data_length);
}

static void writeToAK8963(AK8963Register_t target_register, const uint8_t *data, uint8_t data_length)
{
    writeToTwiSlave(TWI_AK8963_ADDRESS, (uint8_t)target_register, data, data_length);
}

// MPU9250から読み込みます。
static void readFromAK8963(AK8963Register_t target_register, uint8_t *data, uint8_t data_length)
{
    readFromTwiSlave(TWI_AK8963_ADDRESS, (uint8_t)target_register, data, data_length);
}

/**
 * public methods
 */

bool initNineAxesSensor(void)
{
    
    // PWR_MGMT_1
    // D7: H_RESET          1   1- 内部レジスタをリセットしてデフォルト値にする。
    // D6: SLEEP            0   セットすれば、チップはスリープモードに入る。
    // D5: CYCLE            0   SLEEPとSTANDBYがセットされてなくて、これがセットされれば、シングルサンプルとSLEEPをサイクルで繰り返す。
    // D4: GYRO_STANDBY     0   セットされれば、ジャイロのドライバとPLLが有効、しかし検出回路はディセーブル、になる。素早い有効化ができる低消費電力モードに使う。
    // D3: PD_PTAT          0   PTAT(proportional to absolute temperature)電圧生成とPTAT ADCのパワーをダウンする。もしも1がセットされれば。
    // D2: CLKSEL[2:0]      0x0 0x0: 内部20MHz発振回路。
    // D1: =
    // D0: =
    const uint8_t data[] = {0x80};
    bool result = writeToMPU9250( PWR_MGMT_1, data, sizeof(data));
    if(! result ) {
        return false;
    }

    // INT Pin / Bypass Enable Configuration
    // 7: ACTL
    //          1 – The logic level for INT pin is active low. 0 – The logic level for INT pin is active high.
    // 6: OPEN
    //          1 – INT pin is configured as open drain. 0 – INT pin is configured as push-pull.
    // 5: LATCH_INT_EN
    //          1 – INT pin level held until interrupt status is cleared. 0 – INT pin indicates interrupt pulse’s is width 50us.
    // 4: INT_ANYRD_2CLEAR
    //          1 – Interrupt status is cleared if any read operation is performed.
    //          0 – Interrupt status is cleared only by reading INT_STATUS register
    // 3: ACTL_FSYNC
    //          1 – The logic level for the FSYNC pin as an interrupt is active low. 0 – The logic level for the FSYNC pin as an interrupt is active high.
    // 2: FSYNC_INT_MODE_EN
    //          1 – This enables the FSYNC pin to be used as an interrupt. A transition to the active level described by the ACTL_FSYNC bit will cause an interrupt.
    //              The status of the interrupt is read in the I2C Master Status register PASS_THROUGH bit.
    //          0 – This disables the FSYNC pin from causing an interrupt.
    // 1: BYPASS_EN
    //          When asserted, the i2c_master interface pins(ES_CL and ES_DA) will go into ‘bypass mode’ when the i2c master interface is disabled.
    //          The pins will float high due to the internal pull-up if not enabled and the i2c master interface is disabled.
    // 0:       RESERVED
    //
    // enable I2C BYPASS_EN
    const uint8_t data2[] = {0x02};
    writeToMPU9250( INT_PIN_CFG, data2, sizeof(data2));
    
    // CNTL1
    // D4: BIT              1   0: 14-bit output, 1: 16-bit output
    // D3: Mode[3:0]        x   "0000": Power-down mode
    //                          "0001": Single measurement mode
    //                          "0010" Continuous measurement mode 1, 8 Hz sample rates.
    //                          "0110" Continuous measurement mode 2, 100 Hz sample rates.
    // D2: =
    // D1: =
    // D0: =
    const uint8_t data3[] = {0x16};
    writeToAK8963( CNTL1, data3, sizeof(data3));
    
    return true;
}

void sleepNineAxesSensor(void)
{
    // PWR_MGMT_2
    // D7: -
    // D6: -
    // D5: DIS_XA   1   X加速度はdisabled
    // D4: DIS_YA   1   Y加速度はdisabled
    // D3: DIS ZA   1   Z加速度はdisabled
    // D2: DIS_XG   1   Xジャイロ disabled
    // D1: DIS_YG   1
    // D0: DIS_ZG   1
    const uint8_t data[] = {0x3f};
    writeToMPU9250( PWR_MGMT_2, data, sizeof(data));
    
    // PWR_MGMT_1
    // D7: DEVICE_RESET     0   1- 内部レジスタをリセットしてデフォルト値にする。
    // D6: SLEEP            1   セットすれば、チップはスリープモードに入る。
    // D5: CYCLE            0   SLEEPとSTANDBYがセットされてなくて、これがセットされれば、シングルサンプルとSLEEPをサイクルで繰り返す。
    // D4: CYRO_STANDBY     0   セットされれば、ジャイロのドライバとPLLが有効、しかし検出回路はディセーブル、になる。素早い有効化ができる低消費電力モードに使う。
    // D3: TEMP_DIS         0   もしも1がセットされれば、温度センサーを無効化する。
    // D2: CLKSEL[2:0]      0    0x0 0x0: 内部20MHz発振回路。
    // D1:                  0
    // D0:                  0
    const uint8_t data2[] = {0x40};
    writeToMPU9250( PWR_MGMT_1, data2, sizeof(data2));
    
    // CNTL1
    // D4: BIT              0   0: 14-bit output, 1: 16-bit output
    // D3: Mode[3:0]        x   "0000": Power-down mode
    //                          "0001": Single measurement mode
    //                          "0010" Continuous measurement mode 1, 8 Hz sample rates.
    //                          "0110" Continuous measurement mode 2, 100 Hz sample rates.
    // D2: =
    // D1: =
    // D0: =
    const uint8_t data3[] = {0x00};
    writeToAK8963( CNTL1, data3, sizeof(data3));
}

void awakeNineAxesSensor(void)
{
    // PWR_MGMT_1
    // D7: DEVICE_RESET     0   1- 内部レジスタをリセットしてデフォルト値にする。
    // D6: SLEEP            0   セットすれば、チップはスリープモードに入る。
    // D5: CYCLE            0   SLEEPとSTANDBYがセットされてなくて、これがセットされれば、シングルサンプルとSLEEPをサイクルで繰り返す。
    // D4: CYRO_STANDBY     0   セットされれば、ジャイロのドライバとPLLが有効、しかし検出回路はディセーブル、になる。素早い有効化ができる低消費電力モードに使う。
    // D3: TEMP_DIS         0   もしも1がセットされれば、温度センサーを無効化する。
    // D2: CLKSEL[2:0]      0    0x0 0x0: 内部20MHz発振回路。
    // D1:                  0
    // D0:                  0
    const uint8_t data2[] = {0x00};
    writeToMPU9250( PWR_MGMT_1, data2, sizeof(data2));
    
    // PWR_MGMT_2
    // D7
    // D6
    // D5: DIS_XA   0   X加速度はdisabled
    // D4: DIS_YA   0   Y加速度はdisabled
    // D3: DIS ZA   0   Z加速度はdisabled
    // D2: DIS_XG   0   Xジャイロ disabled
    // D1: DIS_YG   0
    // D0: DIS_ZG   0
    const uint8_t data[] = {0x00};
    writeToMPU9250( PWR_MGMT_2, data, sizeof(data));
    
    // CNTL1
    // D4: BIT              0   0: 14-bit output, 1: 16-bit output
    // D3: Mode[3:0]        x   "0000": Power-down mode
    //                          "0001": Single measurement mode
    //                          "0010" Continuous measurement mode 1, 8 Hz sample rates.
    //                          "0110" Continuous measurement mode 2, 100 Hz sample rates.
    // D2: =
    // D1: =
    // D0: =
    const uint8_t data1[] = {0x16};
    writeToAK8963( CNTL1, data1, sizeof(data1));
}

void setNineAxesSensorAccelerationRange(AccelerationRange_t range)
{
    //    Register 28 – Accelerometer Configuration
    //    ACCEL_CONFIG    = 0x1c,
    //
    //    7     ax_st_en, X Accel self-test//
    //    6     ay_st_en, Y Accel self-test
    //    5     az_st_en, Z Accel self-test
    //    [4:3] ACCEL_FS_SEL[1:0]
    //              Accel Full Scale Select:
    //              ±2g (00), ±4g (01), ±8g (10), ±16g (11)
    //    [2:0]
    
    uint8_t value = 0;
    
    switch (range) {
        case ACCELERATION_RANGE_2G: value = 0x00 << 3; break;
        case ACCELERATION_RANGE_4G: value = 0x01 << 3; break;
        case ACCELERATION_RANGE_8G: value = 0x02 << 3; break;
        case ACCELERATION_RANGE_16G:value = 0x03 << 3; break;
        default: break;
    }
    
    uint8_t data[] = {value};
    writeToMPU9250( ACCEL_CONFIG, data, sizeof(data));
}

void setNineAxesSensorRotationRange(RotationRange_t range)
{
//    Register 27 – Gyroscope Configuration
//    GYRO_CONFIG     = 0x1b,
//    7     XGYRO_Cten, X Gyro self-test
//    6     YGYRO_Cten, Y Gyro self-test
//    5     ZGYRO_Cten, Z Gyro self-test
//    4:3   GYRO_FS_SEL[1:0]
//              Gyro Full Scale Select:
//              00 = +250dps
//              01= +500 dps
//              10 = +1000 dps
//              11 = +2000 dps
//    2         -, Reserved
//    1:0       Fchoice_b[1:0], Used to bypass DLPF as shown in table 1 above. NOTE: Register is Fchoice_b (inverted version of Fchoice), table 1 uses Fchoice (which is the inverted version of this register).
    uint8_t value = 0;
    
    switch (range) {
        case ROTATION_RANGE_250DPS: value = 0x00 << 3; break;
        case ROTATION_RANGE_500DPS: value = 0x01 << 3; break;
        case ROTATION_RANGE_1000DPS: value = 0x02 << 3; break;
        case ROTATION_RANGE_2000DPS: value = 0x03 << 3; break;
        default: break;
    }
    
    uint8_t data[] = {value};
    writeToMPU9250(GYRO_CONFIG, data, sizeof(data));
}

void getAccelerationData(uint8_t *p_data)
{
    AccelerationData_t *p_acceleration = (AccelerationData_t *)p_data;
    
    uint8_t buffer[6];
    // 加速度のデータは一連の連続するアドレスから読み出す。
    // ACCEL_XOUT_H 0x3b
    // ACCEL_XOUT_L
    // ACCEL_YOUT_H
    // ACCEL_YOUT_L
    // ACCEL_ZOUT_H
    // ACCEL_ZOUT_L
    readFromMPU9250( ACCEL_XOUT_H, buffer, sizeof(buffer));
    p_acceleration->x = readInt16AsBigEndian(&(buffer[0]));
    p_acceleration->y = readInt16AsBigEndian(&(buffer[2]));
    p_acceleration->z = readInt16AsBigEndian(&(buffer[4]));
}

void getRotationRateData(uint8_t *p_data)
{
    RotationRateData_t *p_rotationRate = (RotationRateData_t *)p_data;
    
    uint8_t buffer[6];
    // ジャイロのデータは一連の連続するアドレスから読み出す。
    // GYRO_XOUT_H 0x43
    // GYRO_XOUT_L
    // GYRO_YOUT_H
    // GYRO_YOUT_L
    // GYRO_ZOUT_H
    // GYRO_ZOUT_L
    readFromMPU9250( GYRO_XOUT_H, buffer, sizeof(buffer));
    
    p_rotationRate->x = readInt16AsBigEndian(&(buffer[0]));
    p_rotationRate->y = readInt16AsBigEndian(&(buffer[2]));
    p_rotationRate->z = readInt16AsBigEndian(&(buffer[4]));
}

void getMagneticFieldData(uint8_t *p_data)
{
    MagneticFieldData_t *p_magneticField = (MagneticFieldData_t *)p_data;
    
    uint8_t buffer[7];
    // 地磁気のデータは一連の連続するアドレスから読み出す。
    // HXL 0x03
    // HXH
    // HYL
    // HYH
    // HZL
    // HZH
    //
    // 内部データはHZHの次のアドレスにあるST2を読みだすことで、読み出しロックが解除されて、次のデータに更新される。そのため7バイトを読みだす。
    readFromAK8963( HXL, buffer, sizeof(buffer));
    
    p_magneticField->x = readInt16AsLittleEndian(&(buffer[0]));
    p_magneticField->y = readInt16AsLittleEndian(&(buffer[2]));
    p_magneticField->z = readInt16AsLittleEndian(&(buffer[4]));
}
