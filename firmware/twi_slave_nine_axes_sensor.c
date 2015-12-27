//
//  twi_slave_nine_axes_sensor.c
//  senstick
//
//  Created by AkihiroUehara on 2015/12/17.
//
//

#include <string.h>

#include "nrf_delay.h"
#include "twi_slave_nine_axes_sensor.h"
#include "nrf_drv_twi.h"
#include "app_error.h"

#include "senstick_io_definitions.h"

// レジスタ仕様書
// MPU-9250
// Register Map and Descriptions Revision 1.4
// Document Number: RM-MPU-9250A-00
// Revision: 1.4
// Release Date: 9/9/2013

// レジスタアドレスの列挙型
typedef enum {
    // 加速度、ジャイロセンサー
    PWR_MGMT_1 = 0x6b,
    PWR_MGMT_2 = 0x6c,
    ACCEL_XOUT_H = 0x3b,
    GYRO_XOUT_H = 0x43,

    // 磁気センサー
    HXL   = 0x03,
    CNTL1 = 0x0a,
    CNTL2 = 0x0b,
} MPU9250Register_t;

/**
 * private methods
 */

// MPU9250に書き込みます。
// TWI_MPU9250_ADDRESS は senstick_io_definitions.h で定義されているI2Cバスのアドレスです。
static void writeToMPU9250(nine_axes_sensor_context_t *p_context, MPU9250Register_t target_register, const uint8_t *data, uint8_t data_length)
{
    ret_code_t err_code;
    
    // 先頭バイトは、レジスタアドレス
    uint8_t buffer[data_length + 1];
    buffer[0] = (uint8_t)target_register;
    memcpy(&(buffer[1]), data, data_length);
    
    // I2C書き込み
    err_code = nrf_drv_twi_tx(p_context->p_twi, TWI_MPU9250_ADDRESS, buffer, (data_length + 1), false);
    APP_ERROR_CHECK(err_code);
}

// MPU9250から読み込みます。
static void readFromMPU9250(nine_axes_sensor_context_t *p_context, MPU9250Register_t target_register, uint8_t *data, uint8_t data_length)
{
    ret_code_t err_code;
    
    // 読み出しターゲットアドレスを設定
    uint8_t buffer0 = (uint8_t)target_register;
    err_code = nrf_drv_twi_tx(p_context->p_twi, TWI_MPU9250_ADDRESS, &buffer0, 1, true);
    APP_ERROR_CHECK(err_code);

    // データを読み出し
    err_code = nrf_drv_twi_rx(p_context->p_twi, TWI_MPU9250_ADDRESS, data, data_length, false);
    APP_ERROR_CHECK(err_code);
}

static void getAccelerationData(nine_axes_sensor_context_t *p_context, AccelerationData_t *p_acceleration)
{
    uint8_t buffer[6];
    // 加速度のデータは一連の連続するアドレスから読み出す。
    // ACCEL_XOUT_H 0x3b
    // ACCEL_XOUT_L
    // ACCEL_YOUT_H
    // ACCEL_YOUT_L
    // ACCEL_ZOUT_H
    // ACCEL_ZOUT_L
    readFromMPU9250(p_context, ACCEL_XOUT_H, buffer, sizeof(buffer));
    p_acceleration->x = readUInt16AsBigEndian(&(buffer[0]));
    p_acceleration->y = readUInt16AsBigEndian(&(buffer[1]));
    p_acceleration->z = readUInt16AsBigEndian(&(buffer[2]));
}

static void getRotationRateData(nine_axes_sensor_context_t *p_context, RotationRateData_t *p_rotaionRate)
{
    uint8_t buffer[6];
    // ジャイロのデータは一連の連続するアドレスから読み出す。
    // GYRO_XOUT_H 0x43
    // GYRO_XOUT_L
    // GYRO_YOUT_H
    // GYRO_YOUT_L
    // GYRO_ZOUT_H
    // GYRO_ZOUT_L
    readFromMPU9250(p_context, GYRO_XOUT_H, buffer, sizeof(buffer));

    p_rotaionRate->x = readUInt16AsBigEndian(&(buffer[0]));
    p_rotaionRate->y = readUInt16AsBigEndian(&(buffer[1]));
    p_rotaionRate->z = readUInt16AsBigEndian(&(buffer[2]));
}

static void getMagneticFieldData(nine_axes_sensor_context_t *p_context, MagneticFieldData_t *p_magneticField)
{
    uint8_t buffer[6];
    // 地磁気のデータは一連の連続するアドレスから読み出す。
    // HXL 0x03
    // HXH
    // HYL
    // HYH
    // HZL
    // HZH
    readFromMPU9250(p_context, HXL, buffer, sizeof(buffer));
    
    p_magneticField->x = readUInt16AsLittleEndian(&(buffer[0]));
    p_magneticField->y = readUInt16AsLittleEndian(&(buffer[1]));
    p_magneticField->z = readUInt16AsLittleEndian(&(buffer[2]));
}

/**
 * public methods
 */

void initNineAxesSensor(nine_axes_sensor_context_t *p_context, nrf_drv_twi_t *p_twi)
{
    // コンテクストの初期設定
    memset(p_context, 0,sizeof(nine_axes_sensor_context_t));
    p_context->p_twi = p_twi;
    
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
    writeToMPU9250(p_context, PWR_MGMT_1, data, sizeof(data));
    
    // CNTL1
    // D4: BIT              0   0: 14-bit output, 1: 16-bit output
    // D3: Mode[3:0]        x   "0000": Power-down mode
    //                          "0001": Single measurement mode
    //                          "0010" Continuous measurement mode 1, 8 Hz sample rates.
    //                          "0110" Continuous measurement mode 2, 100 Hz sample rates.
    // D2: =
    // D1: =
    // D0: =
    const uint8_t data1[] = {0x01};
    writeToMPU9250(p_context, CNTL1, data1, sizeof(data1));
}

void sleepNineAxesSensor(nine_axes_sensor_context_t *p_context)
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
    writeToMPU9250(p_context, PWR_MGMT_2, data, sizeof(data));
    
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
    writeToMPU9250(p_context, PWR_MGMT_1, data2, sizeof(data2));
    
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
    writeToMPU9250(p_context, CNTL1, data3, sizeof(data3));
}

void awakeNineAxesSensor(nine_axes_sensor_context_t *p_context)
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
    writeToMPU9250(p_context, PWR_MGMT_1, data2, sizeof(data2));
    
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
    writeToMPU9250(p_context, PWR_MGMT_2, data, sizeof(data));
    
    // CNTL1
    // D4: BIT              0   0: 14-bit output, 1: 16-bit output
    // D3: Mode[3:0]        x   "0000": Power-down mode
    //                          "0001": Single measurement mode
    //                          "0010" Continuous measurement mode 1, 8 Hz sample rates.
    //                          "0110" Continuous measurement mode 2, 100 Hz sample rates.
    // D2: =
    // D1: =
    // D0: =
    const uint8_t data1[] = {0x01};
    writeToMPU9250(p_context, CNTL1, data1, sizeof(data1));
}

void getNineAxesData(nine_axes_sensor_context_t *p_context, MotionSensorData_t *sensor_data)
{
    getAccelerationData(p_context,  &(sensor_data->acceleration));
    getRotationRateData(p_context,  &(sensor_data->rotaionRate));
    getMagneticFieldData(p_context, &(sensor_data->magneticField));
    
}
