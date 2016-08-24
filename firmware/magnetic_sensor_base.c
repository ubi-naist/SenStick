#include <stdint.h>
#include <stdbool.h>
#include "value_types.h"
#include "twi_slave_nine_axes_sensor.h"

#include "magnetic_sensor_base.h"

#include "senstick_sensor_base_data.h"
#include "senstick_flash_address_definition.h"

// センサーの初期化。
static bool initSensorHandler(void)
{
    return initNineAxesSensor();
}

// センサーのwakeup/sleepを指定します
static void setSensorWakeupHandler(bool shouldWakeUp, const sensor_service_setting_t *p_setting)
{
    awakeNineAxesSensor();

    uint8_t buf[sizeof(MagneticFieldData_t)];
    getMagneticFieldData(buf);
}

// センサーの値を読み込みます。
static uint8_t getSensorDataHandler(uint8_t *p_buffer)
{
    getMagneticFieldData(p_buffer);
    return sizeof(MagneticFieldData_t);
}

// srcとdstのセンサデータの最大値/最小値をp_srcに入れます。p_srcは破壊されます。
static void getMaxMinValueHandler(bool isMax, uint8_t *p_src, uint8_t *p_dst)
{
    // TBD
}

// センサ構造体データをBLEのシリアライズしたバイナリ配列に変換します。
static uint8_t getBLEDataHandler(uint8_t *p_dst, uint8_t *p_src)
{
    MagneticFieldData_t data;
    memcpy(&data, p_src, sizeof(MagneticFieldData_t));
    int16ToByteArrayLittleEndian(&(p_dst[0]), data.x);
    int16ToByteArrayLittleEndian(&(p_dst[2]), data.y);
    int16ToByteArrayLittleEndian(&(p_dst[4]), data.z);
    
    return 6;
}

const senstick_sensor_base_t magneticSensorBase =
{
    sizeof(MagneticFieldData_t), // sizeof(センサデータの構造体)
    (2 * 3),                    // BLEでやり取りするシリアライズされたデータのサイズ
    {
        MAGNETIC_SENSOR_STORAGE_START_ADDRESS, // スタートアドレス
        MAGNETIC_SENSOR_STORAGE_SIZE           // サイズ
    },
    initSensorHandler,
    setSensorWakeupHandler,
    getSensorDataHandler,
    getMaxMinValueHandler,
    getBLEDataHandler
};

