#include <stdint.h>
#include <stdbool.h>
#include "value_types.h"
#include "twi_slave_brightness_sensor.h"

#include "brightness_sensor_base.h"

#include "senstick_sensor_base_data.h"
#include "senstick_flash_address_definition.h"

// センサーの初期化。
static bool initSensorHandler(void)
{
    return initBrightnessSensor();
}

// センサーのwakeup/sleepを指定します
static void setSensorWakeupHandler(bool shouldWakeUp, const sensor_service_setting_t *p_setting)
{
    // データ読み捨て
    BrightnessData_t data;
    getBrightnessData(&data);
}

// センサーの値を読み込みます。
static uint8_t getSensorDataHandler(uint8_t *p_buffer)
{
    getBrightnessData((BrightnessData_t *)p_buffer);
    return sizeof(BrightnessData_t);
}

// srcとdstのセンサデータの最大値/最小値をp_srcに入れます。p_srcは破壊されます。
static void getMaxMinValueHandler(bool isMax, uint8_t *p_src, uint8_t *p_dst)
{
    // TBD
}

// センサ構造体データをBLEのシリアライズしたバイナリ配列に変換します。
static uint8_t getBLEDataHandler(uint8_t *p_dst, uint8_t *p_src)
{
    uint16ToByteArrayLittleEndian(&(p_dst[0]), *((BrightnessData_t *)p_src));
    
    return 2;
}

const senstick_sensor_base_t brightnessSensorBase =
{
    sizeof(BrightnessData_t), // sizeof(センサデータの構造体)
    (2),                         // BLEでやり取りするシリアライズされたデータのサイズ
    {
        BRIGHTNESS_SENSOR_STORAGE_START_ADDRESS, // スタートアドレス
        BRIGHTNESS_SENSOR_STORAGE_SIZE           // サイズ
    },
    initSensorHandler,
    setSensorWakeupHandler,
    getSensorDataHandler,
    getMaxMinValueHandler,
    getBLEDataHandler
};

