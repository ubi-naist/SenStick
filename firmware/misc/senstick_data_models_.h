#ifndef senstick_data_models_h
#define senstick_data_models_h

#include <stdint.h>
#include <stdbool.h>

/*

// 輝度データ
// 単位 lx
// 変換時間約150ミリ秒
typedef uint16_t BrightnessData_t;

// UVデータ
// サンプリング周期275ミリ秒、単位換算 2.14 uW/cm2/step
typedef uint16_t UltraVioletData_t;

// 気圧データ
// 24-bit, 4096 LSB/ hPa
typedef uint32_t AirPressureData_t;

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

**
 * センサー設定データ
 *


*/

#endif /* senstick_data_models_h */
