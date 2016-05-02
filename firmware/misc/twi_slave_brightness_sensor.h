#ifndef twi_slave_brightness_sensor_h
#define twi_slave_brightness_sensor_h

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_twi.h"
#include "senstick_data_models.h"

// 構造体の宣言
typedef struct {
    nrf_drv_twi_t *p_twi;
} brightness_sensor_context_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
void initBrightnessSensor(brightness_sensor_context_t *p_context, nrf_drv_twi_t *p_twi);

// 単位 lx
// 変換時間約150ミリ秒
void getBrightnessData(brightness_sensor_context_t *p_context, BrightnessData_t *p_data);

#endif /* twi_slave_brightness_sensor_h */
