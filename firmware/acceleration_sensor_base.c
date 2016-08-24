#include <stdint.h>
#include <stdbool.h>
#include "value_types.h"
#include "twi_slave_nine_axes_sensor.h"

#include "acceleration_sensor_base.h"

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
    setNineAxesSensorAccelerationRange((AccelerationRange_t)p_setting->measurementRange);
    
    uint8_t buf[sizeof(AccelerationData_t)];
    getAccelerationData(buf);
}

// センサーの値を読み込みます。
static uint8_t getSensorDataHandler(uint8_t *p_buffer)
{
    getAccelerationData(p_buffer);
    return sizeof(AccelerationData_t);
}

// srcとdstのセンサデータの最大値/最小値をp_srcに入れます。p_srcは破壊されます。
static void getMaxMinValueHandler(bool isMax, uint8_t *p_src, uint8_t *p_dst)
{
    // TBD
}

// センサ構造体データをBLEのシリアライズしたバイナリ配列に変換します。
static uint8_t getBLEDataHandler(uint8_t *p_dst, uint8_t *p_src)
{
    AccelerationData_t data;
    memcpy(&data, p_src, sizeof(AccelerationData_t));
    int16ToByteArrayLittleEndian(&(p_dst[0]), data.x);
    int16ToByteArrayLittleEndian(&(p_dst[2]), data.y);
    int16ToByteArrayLittleEndian(&(p_dst[4]), data.z);
    
    return 6;
}

const senstick_sensor_base_t accelerationSensorBase =
{
    sizeof(AccelerationData_t), // sizeof(センサデータの構造体)
    (2 * 3),                    // BLEでやり取りするシリアライズされたデータのサイズ
    {
        ACCELERATION_SENSOR_STORAGE_START_ADDRESS, // スタートアドレス
        ACCELERATION_SENSOR_STORAGE_SIZE           // サイズ
    },
    initSensorHandler,
    setSensorWakeupHandler,
    getSensorDataHandler,
    getMaxMinValueHandler,
    getBLEDataHandler
};
