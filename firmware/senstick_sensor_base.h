#ifndef senstick_sensor_base_h
#define senstick_sensor_base_h

#include <stdint.h>
#include <stdbool.h>

#include "senstick_types.h"
#include "senstick_sensor_base_data.h"

#define MAX_SENSOR_RAW_DATA_SIZE 6

// センサーの初期化。
typedef bool (* initSensorHandlerType)(void);
// センサーのwakeup/sleepを指定します
typedef void (* setSensorWakeupHandlerType)(bool shouldWakeUp, const sensor_service_setting_t *p_setting);
// センサーの値を読み込みます。
// duration_msはセンサ開始からの経過時間をミリ秒で、0カラスタートしてタイマ割り込み周期刻みで与えます。
// 返り値は、有効なデータ長を示します。センサの値読み込み処理が継続中であれば、0を返します。
// 呼び出し側は、時間をおいて、0以外の値が返ってくるまで、このメソッドを呼び出し続けます。
typedef uint8_t (* getSensorDataHandlerType)(uint8_t *p_buffer, samplingDurationType duration_ms);
// srcとdstのセンサデータの最大値/最小値をp_srcに入れます。p_srcは破壊されます。
typedef void (* getMaxMinValueHandlerType)(bool isMax, uint8_t *p_src, uint8_t *p_dst);
// センサ構造体データをBLEのシリアライズしたバイナリ配列に変換します。
typedef uint8_t (* getBLEDataHandlerType)(uint8_t *p_dst, uint8_t *p_src);

typedef struct {
    uint8_t rawSensorDataSize;           // sizeof(センサデータの構造体)
    uint8_t bleSerializedSensorDataSize; // BLEでやり取りするシリアライズされたデータのサイズ
    
    flash_address_info_t address_info;   // フラッシュの割当領域情報
    
    initSensorHandlerType       initSensorHandler;
    setSensorWakeupHandlerType  setSensorWakeupHandler;
    getSensorDataHandlerType    getSensorDataHandler;
    getMaxMinValueHandlerType   getMaxMinValueHandler;
    getBLEDataHandlerType       getBLEDataHandler;
} senstick_sensor_base_t;

#endif /* senstick_sensor_base_h */
