#ifndef twi_slave_brightness_sensor_h
#define twi_slave_brightness_sensor_h

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"

// 輝度データ
// 単位 lx
// 変換時間約150ミリ秒
typedef uint16_t BrightnessData_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
bool initBrightnessSensor(void);

// 単位 lx
// 変換時間約150ミリ秒
void getBrightnessData(BrightnessData_t *p_data);

#endif /* twi_slave_brightness_sensor_h */
