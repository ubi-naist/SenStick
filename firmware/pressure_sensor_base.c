#include <stdint.h>
#include <stdbool.h>
#include "value_types.h"
#include "twi_slave_pressure_sensor.h"

#include "pressure_sensor_base.h"

#include "senstick_sensor_base_data.h"
#include "senstick_flash_address_definition.h"

// センサーの初期化。
static bool initSensorHandler(void)
{
    return initPressureSensor();
}

// センサーのwakeup/sleepを指定します
static void setSensorWakeupHandler(bool shouldWakeUp, const sensor_service_setting_t *p_setting)
{
}

// センサーの値を読み込みます。
static uint8_t getSensorDataHandler(uint8_t *p_buffer)
{
    getPressureData((AirPressureData_t *)p_buffer);
    return sizeof(AirPressureData_t);
}

// srcとdstのセンサデータの最大値/最小値をp_srcに入れます。p_srcは破壊されます。
static void getMaxMinValueHandler(bool isMax, uint8_t *p_src, uint8_t *p_dst)
{
    // TBD
}

// センサ構造体データをBLEのシリアライズしたバイナリ配列に変換します。
static uint8_t getBLEDataHandler(uint8_t *p_dst, uint8_t *p_src)
{
    uint32ToByteArrayLittleEndian(&(p_dst[0]), (AirPressureData_t)*p_src);
    
    return 4;
}

const senstick_sensor_base_t pressureSensorBase =
{
    sizeof(AirPressureData_t), // sizeof(センサデータの構造体)
    (4),                       // BLEでやり取りするシリアライズされたデータのサイズ
    {
        PRESSURE_SENSOR_STORAGE_START_ADDRESS, // スタートアドレス
        PRESSURE_SENSOR_STORAGE_SIZE           // サイズ
    },
    initSensorHandler,
    setSensorWakeupHandler,
    getSensorDataHandler,
    getMaxMinValueHandler,
    getBLEDataHandler
};

