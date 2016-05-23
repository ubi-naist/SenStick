#ifndef twi_slave_pressure_sensor_h
#define twi_slave_pressure_sensor_h

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"

// 気圧データ
// 24-bit, 4096 LSB/ hPa
typedef uint32_t AirPressureData_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
bool initPressureSensor(void);

void getPressureData(AirPressureData_t *p_data);

#endif /* twi_slave_pressure_sensor_h */
