#ifndef twi_slave_humidity_sensor_h
#define twi_slave_humidity_sensor_h

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"

// 湿度データ
// 変換式、仕様書10ページ、RH = -6 + 125 * SRH / 2^(16)
typedef uint16_t HumidityData_t;

// 温度データ
// 変換式、仕様書10ページ、T = -46.85 + 175.72 * St/ 2^(16)
typedef uint16_t TemperatureData_t;

// 温度と湿度データ
typedef struct {
    HumidityData_t humidity;
    TemperatureData_t temperature;
} HumidityAndTemperatureData_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
bool initHumiditySensor(void);

// 計測時間中はI2Cバスを離さない。
// 相対湿度計測 11-bit精度 typ 12ミリ秒 max 15ミリ秒
// 変換式、仕様書10ページ、RH = -6 + 125 * SRH / 2^(16)
void triggerHumidityMeasurement(void);
void getHumidityData(HumidityData_t *p_data);

// 温度計測 11-bit typ. 9ミリ秒 max 11ミリ秒
// 変換式、仕様書10ページ、T = -46.85 + 175.72 * St/ 2^(16)
void triggerTemperatureMeasurement(void);
void getTemperatureData(TemperatureData_t *p_data);

#endif /* twi_slave_humidity_sensor_h */
