#ifndef senstick_sensor_base_data_h
#define senstick_sensor_base_data_h

#include "senstick_types.h"
#include "senstick_data_model.h"

// 物理的なセンサーの種別
typedef enum {
    AccelerationSensor              = 0,
    GyroSensor                      = 1,
    MagneticFieldSensor             = 2,
    BrightnessSensor                = 3,
    UltraVioletSensor               = 4,
    HumidityAndTemperatureSensor    = 5,
    AirPressureSensor               = 6,
} sensor_device_t;

typedef enum {
    sensorServiceCommand_stop                = 0x00,
    sensorServiceCommand_sensing             = 0x01,
    sensorServiceCommand_sensing_and_logging = 0x03,
} sensor_service_command_t;

// センサーのサンプリング周期
typedef uint16_t samplingDurationType;

// 設定キャラクタリスティクスのデータモデル
typedef struct {
    sensor_service_command_t command;           // センサーの動作指定を示します。停止/センシング/センシング&ロギング。
    samplingDurationType     samplingDuration;  // サンプリング周期(ミリ秒)
    uint16_t                 measurementRange;  // 測定レンジ。値の意味は、センサごとに異なります。
} sensor_service_setting_t;

// logidキャラクタリスティクスのデータモデル
typedef struct {
    uint8_t  logID;           // 読み出し対象のログIDを指定します。
    uint16_t skipCount;       // スキップするカウント数です。0もしくは1ならば通常の呼び出し、2以上ならばその範囲で最大次にその範囲で最小を返します。
    uint32_t position;        // 読み出し位置。単位は、データのサンプル数です。
} sensor_service_logID_t;

// logメタデータ
typedef struct {
    samplingDurationType samplingDuration;
    uint16_t             measurementRange;
    uint32_t             sampleCount;       // 有効なサンプル数。
    uint32_t             position;          // 現在の読み出し位置。
    uint32_t             remainingStorage; // ストレージの空き領域(サンプル数)
} sensor_metadata_t;

// バイナリ配列に変換します。バッファは長さ6バイト以上。
uint8_t serializesensor_service_setting(uint8_t *p_dst, sensor_service_setting_t *p_src);
void deserializesensor_service_setting(sensor_service_setting_t *p_dst, uint8_t *p_src);
// 7バイト以上
uint8_t serializeSensorServiceLogID(uint8_t *p_dst, sensor_service_logID_t *p_src);
void deserialize_sensorServiceLogID(sensor_service_logID_t *p_dst, uint8_t *p_src);
// 13バイト以上
uint8_t serializeSensorMetaData(uint8_t *p_dst, sensor_metadata_t *p_src);
void deserializeSensorMetaData(sensor_metadata_t *p_dst, uint8_t *p_src);

#endif /* senstick_sensor_base_data_h */
