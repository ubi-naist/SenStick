#ifndef twi_slave_pressure_sensor_h
#define twi_slave_pressure_sensor_h

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_twi.h"
#include "senstick_data_models.h"

// 構造体の宣言
typedef struct {
    nrf_drv_twi_t *p_twi;
} pressure_sensor_context_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
void initPressureSensor(pressure_sensor_context_t *p_context, nrf_drv_twi_t *p_twi);

void getPressureData(pressure_sensor_context_t *p_context, AirPressureData_t *p_data);

// バス上にセンサーがあるかどうかを判定します。
bool isPressureSensor(nrf_drv_twi_t *p_twi);

#endif /* twi_slave_pressure_sensor_h */
