#include <stdint.h>
#include <stdbool.h>
#include <nrf_log.h>
#include "value_types.h"

#include "twi_slave_humidity_sensor.h"

#include "humidity_sensor_base.h"

#include "senstick_sensor_base_data.h"
#include "senstick_flash_address_definition.h"

static uint8_t _state;
static HumidityAndTemperatureData_t _sensorData;

// センサーの初期化。
static bool initSensorHandler(void)
{
    _state = 0;
    return initHumiditySensor();
}

// センサーのwakeup/sleepを指定します
static void setSensorWakeupHandler(bool shouldWakeUp, const sensor_service_setting_t *p_setting)
{
    _state = 0;
}

// センサーの値を読み込みます。
static uint8_t getSensorDataHandler(uint8_t *p_buffer, samplingDurationType duration_ms)
{
    switch(_state) {
        case 0: // 湿度をトリガー。
            _state = 1;
            triggerHumidityMeasurement();
            return 0;
            
        case 1: // 湿度の取得時間を待つ。15ミリ秒。
            if(duration_ms > 20) {
                _state = 2;
            }
            return 0;
            
        case 2: // 湿度のデータ取得。温度のトリガー。
            _state = 3;
            getHumidityData(&_sensorData.humidity);
            triggerTemperatureMeasurement();
            return 0;
            
        case 3: // 温度の取得時間を待つ。11ミリ秒。
            if(duration_ms > (20 + 1 + 11)) {
                _state = 4;
            }
            return 0;
            
        case 4: // 温度のデータを取得。
            _state = 0;
            getTemperatureData(&_sensorData.temperature);
            memcpy(p_buffer, &_sensorData, sizeof(HumidityAndTemperatureData_t));
            //NRF_LOG_PRINTF_DEBUG("humidity:H0x%04x T0x%04x.\n", p_data->humidity, p_data->temperature);
            return sizeof(HumidityAndTemperatureData_t);
            
        default:
            _state = 0;
            return 0;
    }
}

// srcとdstのセンサデータの最大値/最小値をp_srcに入れます。p_srcは破壊されます。
static void getMaxMinValueHandler(bool isMax, uint8_t *p_src, uint8_t *p_dst)
{
    // TBD
}

// センサ構造体データをBLEのシリアライズしたバイナリ配列に変換します。
static uint8_t getBLEDataHandler(uint8_t *p_dst, uint8_t *p_src)
{
    HumidityAndTemperatureData_t data;
    memcpy(&data, p_src, sizeof(HumidityAndTemperatureData_t));
    uint16ToByteArrayLittleEndian(&(p_dst[0]), data.humidity);
    uint16ToByteArrayLittleEndian(&(p_dst[2]), data.temperature);
//NRF_LOG_PRINTF_DEBUG("humidity:H0x%04x T0x%04x.\n", p_data->humidity, p_data->temperature);
    return 4;
}

const senstick_sensor_base_t humiditySensorBase =
{
    sizeof(HumidityAndTemperatureData_t), // sizeof(センサデータの構造体)
    (2 * 2),                         // BLEでやり取りするシリアライズされたデータのサイズ
    {
        HUMIDITY_SENSOR_STORAGE_START_ADDRESS, // スタートアドレス
        HUMIDITY_SENSOR_STORAGE_SIZE           // サイズ
    },
    initSensorHandler,
    setSensorWakeupHandler,
    getSensorDataHandler,
    getMaxMinValueHandler,
    getBLEDataHandler
};

