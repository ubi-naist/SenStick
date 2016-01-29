#ifndef twi_slave_uv_sensor_h
#define twi_slave_uv_sensor_h

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_twi.h"
#include "senstick_data_models.h"

// 構造体の宣言
typedef struct {
    nrf_drv_twi_t *p_twi;
} uv_sensor_context_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
void initUVSensor(uv_sensor_context_t *p_context, nrf_drv_twi_t *p_twi);

// UVA sensitivity, RSET =240kΩ,IT =1T, typ. 5 μW/cm2/step
// 560kohm 1T、サンプリング周期275ミリ秒。 2.14 uW/cm2/step。
void getUVSensorData(uv_sensor_context_t *p_context, UltraVioletData_t *p_data);

#endif /* twi_slave_uv_sensor_h */
