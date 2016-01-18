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
typedef struct AccelerationData_s {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} AccelerationData_t;

// ジャイロのデータ構造体
typedef struct RotationRateData_s {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} RotationRateData_t;

// 磁界のデータ構造体
typedef struct MagneticFieldData_s {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} MagneticFieldData_t;

// 9軸センサーのデータ構造体
typedef struct MotionSensorData_s {
    AccelerationData_t  acceleration;
    MagneticFieldData_t magneticField;
    RotationRateData_t  rotaionRate;
} MotionSensorData_t;

// 輝度データ
typedef uint16_t BrightnessData_t;

// UVデータ
typedef uint16_t UltraVioletData_t;

// 気圧データ
typedef uint32_t AirPressureData_t;

// 湿度データ
typedef uint16_t HumidityData_t;

// 温度データ
typedef uint16_t TemperatureData_t;

/**
 * センサー設定データ
 */

// 加速度センサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
typedef enum {
    ACCELERATION_RANGE_2G   = 0x00, // +- 2g
    ACCELERATION_RANGE_4G   = 0x01, // +- 4g
    ACCELERATION_RANGE_8G   = 0x02, // +- 8g
    ACCELERATION_RANGE_16G  = 0x03, // +- 16g
} AccelerationRange_t;

// ジャイロセンサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
typedef enum {
    ROTATION_RANGE_250DPS   = 0x00,
    ROTATION_RANGE_500DPS   = 0x01,
    ROTATION_RANGE_1000DPS  = 0x02,
    ROTATION_RANGE_2000DPS  = 0x03,
} RotationRange_t;

// サンプリング・レート
typedef enum {
  SAMPLING_RATE_0_1_Hz  = 0x00, // 0.1Hz
  SAMPLING_RATE_1_Hz    = 0x01, // 1Hz
  SAMPLING_RATE_10_Hz   = 0x02, // 10Hz
} SamplingRate_t;

/**
 * RTCのデータ構造
 */

typedef struct rtcSettingCommand_s {
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
typedef struct rtcAlarmSettingCommand_s {
    // RTCの数値はすべてBCD
    uint8_t minute;
    uint8_t hour;
    uint8_t dayOfWeekBitFields;  // 曜日(1が日曜日 7が土曜日)
} rtcAlarmSettingCommand_t;

/**
 * GATTサービス、シリアライズ
 */
typedef struct sensorSettings_s {
    AccelerationRange_t accelerationRange;
    RotationRange_t     rotationRange;

    SamplingRate_t nineAxesSensorSamplingRate;
    SamplingRate_t humiditySensorSamplingRate;
    SamplingRate_t pressureSensorSamplingRate;
    SamplingRate_t illuminationSensorSamplingRate;
} sensorSetting_t;

// 関数
uint16_t readUInt16AsBigEndian(uint8_t *ptr);
uint16_t readUInt16AsLittleEndian(uint8_t *ptr);

uint32_t readUInt32AsLittleEndian(uint8_t *ptr);

// UInt32をバイトアレイに展開します。ビッグエンディアン。配列は4バイトの領域を確保していなければなりません。
void uint32ToByteArrayBigEndian(uint8_t *p_dst, uint32_t src);

// センサー設定をバイナリ配列に展開します。このバイナリ配列はGATTの設定キャラクタリスティクスにそのまま使用できます。 バイト配列は7バイトの領域が確保されていなければなりません。
void serializeSensorSetting(uint8_t *p_dst, const sensorSetting_t *p_setting);
// バイト配列をセンサー設定情報に展開します。不正な値があった場合は、処理は完了せず、falseを返します。 バイト配列は7バイトの領域が確保されていなければなりません。
bool deserializeSensorSetting(sensorSetting_t *p_setting, uint8_t *p_src );


// デバッグ用ログ関数
void debugLogAccerationData(const AccelerationData_t *data);

#endif /* senstick_data_models_h */
