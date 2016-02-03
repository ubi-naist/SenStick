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
// 16ビット 符号付き数値。フルスケールは設定レンジ値による。2, 4, 8, 16G。
typedef struct AccelerationData_s {
    int16_t x;
    int16_t y;
    int16_t z;
} AccelerationData_t;

// ジャイロのデータ構造体
// 16ビット 符号付き数値。フルスケールは設定レンジ値による。250, 500, 1000, 2000 DPS。
typedef struct RotationRateData_s {
    int16_t x;
    int16_t y;
    int16_t z;
} RotationRateData_t;

// 磁界のデータ構造体
// 16-bit 符号付き数字 フルスケール f ±4800 μT
typedef struct MagneticFieldData_s {
    int16_t x;
    int16_t y;
    int16_t z;
} MagneticFieldData_t;

// 9軸センサーのデータ構造体
typedef struct MotionSensorData_s {
    AccelerationData_t  acceleration;
    MagneticFieldData_t magneticField;
    RotationRateData_t  rotationRate;
} MotionSensorData_t;

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

// コールバック関数のための統合センサーデータ型
typedef union {
    MotionSensorData_t  motionSensorData;
    BrightnessData_t    brightnessData;
    UltraVioletData_t   ultraVioletData;
    AirPressureData_t   airPressureData;
    HumidityAndTemperatureData_t humidityAndTemperatureData;
} SensorData_t;

// 物理的なセンサーの種別
typedef enum {
    MotionSensor                    = 0,
    BrightnessSensor                = 1,
    UltraVioletSensor               = 2,
    HumidityAndTemperatureSensor    = 3,
    AirPressureSensor               = 4
} SensorDeviceType_t;

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

// サンプリング・レート。ミリ秒単位。
typedef int16_t SamplingRate_t;

// RTCのデータ構造
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

    // サンプリングレートの単位はミリ秒
    SamplingRate_t motionSensorSamplingPeriod;
    SamplingRate_t humidityAndTemperatureSamplingPeriod;
    SamplingRate_t airPressureSamplingPeriod;
    SamplingRate_t brightnessSamplingPeriod;
    SamplingRate_t ultraVioletSamplingPeriod;

    bool is_accelerometer_sampling;
    // ジャイロスコープの設定は、下位3ビットを軸ごとのenableに割り当てる, Z/Y/X
    uint8_t is_gyroscope_sampling;
    bool is_humidity_sampling;
    bool is_temperature_sampling;
    bool is_magnetrometer_sampling;
    bool is_barometer_sampling;
    bool is_illumination_sampling;
    bool is_uv_sampling;
    
} sensorSetting_t;


// 数値とバイト列との変換
uint16_t readUInt16AsBigEndian(uint8_t *ptr);
uint16_t readUInt16AsLittleEndian(uint8_t *ptr);

uint32_t readUInt32AsLittleEndian(uint8_t *ptr);

void int16ToByteArrayBigEndian(uint8_t *p_dst, int16_t src);
void int16ToByteArrayLittleEndian(uint8_t *p_dst, int16_t src);

void uint16ToByteArrayBigEndian(uint8_t *p_dst, uint16_t src);
void uint16ToByteArrayLittleEndian(uint8_t *p_dst, uint16_t src);

void uint32ToByteArrayBigEndian(uint8_t *p_dst, uint32_t src);

// センサー設定の周期を設定します。
bool setSensorSettingPeriod(sensorSetting_t *p_setting, SensorDeviceType_t device_type, int period);

/*
// センサー設定をバイナリ配列に展開します。このバイナリ配列はGATTの設定キャラクタリスティクスにそのまま使用できます。 バイト配列は7バイトの領域が確保されていなければなりません。
void serializeSensorSetting(uint8_t *p_dst, const sensorSetting_t *p_setting);
// バイト配列をセンサー設定情報に展開します。不正な値があった場合は、処理は完了せず、falseを返します。 バイト配列は7バイトの領域が確保されていなければなりません。
bool deserializeSensorSetting(sensorSetting_t *p_setting, uint8_t *p_src );

// モーションデータを18バイトのバイト配列に展開します
void serializeMotionData(uint8_t *p_dst, const MotionSensorData_t *p_data);

// 輝度データを2バイトのバイト配列に展開します
void serializeBrightnessData(uint8_t *p_dst, const BrightnessData_t *p_data);

// 輝度データを4バイトのバイト配列に展開します
void serializeHumidityAndTemperatureData(uint8_t *p_dst, const HumidityAndTemperatureData_t *p_data);

// 圧力データを3バイトのバイト配列に展開します
void serializeAirPressureData(uint8_t *p_dst, const AirPressureData_t *p_data);
*/
// デバッグ用ログ関数
void debugLogAccerationData(const AccelerationData_t *data);

#endif /* senstick_data_models_h */
