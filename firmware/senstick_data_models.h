#ifndef senstick_data_models_h
#define senstick_data_models_h

#include <stdint.h>
#include <stdbool.h>

// Senstickのデータ・モデル
// BLEのパケット・データと構造体との、シリアライズおよびデシリアライズを提供する。また、値の検証機能を提供する。

/**
 * センサー・データ構造
 */

// 加速度のデータ構造体
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} AccelerationData_t;

// ジャイロのデータ構造体
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} RotationRateData_t;

// 磁界のデータ構造体
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} MagneticFieldData_t;

// 9軸センサーのデータ構造体
typedef struct {
    AccelerationData_t  acceleration;
    MagneticFieldData_t magneticField;
    RotationRateData_t  rotaionRate;
} MotionSensorData_t;

// 輝度データ
typedef uint16_t BrightnessData_t;

// UVデータ
typedef uint16_t UltraVioletData_t;

// 気圧データ
typedef uint16_t AirPressureData_t;

// 湿度データ
typedef uint16_t HumidityData_t;

// 温度データ
typedef uint16_t TemperatureData_t;

/**
 * センサー設定データ
 */

// 加速度センサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
typedef {
    ACCELERATION_RANGE_2G, // +- 2g
    ACCELERATION_RANGE_4G, // +- 4g
    ACCELERATION_RANGE_8G, // +- 8g
    ACCELERATION_RANGE_16G, // +- 16g
} AccelerationRange_t;

// ジャイロセンサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
typedef {
    ROTATION_RANGE_250DPS,
    ROTATION_RANGE_500DPS,
    ROTATION_RANGE_1000DPS,
    ROTATION_RANGE_2000DPS,
} RotationRange_t;

// サンプリング・レート
typedef {
  SAMPLING_RATE_0_1_Hz, // 0.1Hz
  SAMPLING_RATE_1_Hz,   // 1Hz
  SAMPLING_10_Hz        // 10Hz
} SamplingRate_t;

/**
 * RTCのデータ構造
 */

typedef struct {
    // RTCの数値はすべてBCD(Binary-coded decimal)。16進表記したときの各桁が0-9の値を表す。
    uint8_t second;
    uint8_t minute;
    uint8_t hour;       // BCD 24時間表記。
    uint8_t dayOfWeek;  // 曜日(1が日曜日 7が土曜日)
    uint8_t day;
    uint8_t month;
    uint8_t year;       // BCD 西暦の下2桁
} rtcSettingCommand_t;

// RTC_ALARM_SETTING_COMMAND
typedef struct {
    // RTCの数値はすべてBCD
    uint8_t minute;
    uint8_t hour;
    uint8_t dayOfWeekBitFields;  // 曜日(1が日曜日 7が土曜日)
} rtcAlarmSettingCommand_t;

#endif /* senstick_data_models_h */
