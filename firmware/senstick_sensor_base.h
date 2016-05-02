#ifndef senstick_sensor_base_h
#define senstick_sensor_base_h

#include <stdint.h>
#include <stdbool.h>

#include "senstick_sensor_base_data.h"

#define MAX_SENSOR_RAW_DATA_SIZE 6

// センサーの初期化。
typedef bool (* initSensorHandlerType)(void);
// センサーのwakeup/sleepを指定します
typedef void (* setSensorWakeupHandlerType)(bool shouldWakeUp, const sensor_service_setting_t *p_setting);
// センサーの値を読み込みます。
typedef uint8_t (* getSensorDataHandlerType)(uint8_t *p_buffer);
// srcとdstのセンサデータの最大値/最小値をp_srcに入れます。p_srcは破壊されます。
typedef void (* getMaxMinValueHandlerType)(bool isMax, uint8_t *p_src, uint8_t *p_dst);
// センサ構造体データをBLEのシリアライズしたバイナリ配列に変換します。
typedef uint8_t (* getBLEDataHandlerType)(uint8_t *p_dst, uint8_t *p_src);

typedef struct {
    uint8_t rawSensorDataSize;           // sizeof(センサデータの構造体)
    uint8_t bleSerializedSensorDataSize; // BLEでやり取りするシリアライズされたデータのサイズ
    
    initSensorHandlerType       initSensorHandler;
    setSensorWakeupHandlerType  setSensorWakeupHandler;
    getSensorDataHandlerType    getSensorDataHandler;
    getMaxMinValueHandlerType   getMaxMinValueHandler;
    getBLEDataHandlerType       getBLEDataHandler;
} senstick_sensor_base_t;

#endif /* senstick_sensor_base_h */
