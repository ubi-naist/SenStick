#ifndef twi_slave_nine_axes_sensor_h
#define twi_slave_nine_axes_sensor_h

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_twi.h"
#include "senstick_data_models.h"

// 構造体の宣言
typedef struct {
    nrf_drv_twi_t *p_twi;
} nine_axes_sensor_context_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
void initNineAxesSensor(nine_axes_sensor_context_t *p_context, nrf_drv_twi_t *p_twi);

void getNineAxesData(nine_axes_sensor_context_t *p_context, MotionSensorData_t *sensor_data);

void sleepNineAxesSensor(nine_axes_sensor_context_t *p_context);
void awakeNineAxesSensor(nine_axes_sensor_context_t *p_context);

void setNineAxesSensorAccelerationRange(nine_axes_sensor_context_t *p_context, AccelerationRange_t range);
void setNineAxesSensorRotationRange(nine_axes_sensor_context_t *p_context, RotationRange_t range);

#endif /* twi_slave_nine_axes_sensor_h */
