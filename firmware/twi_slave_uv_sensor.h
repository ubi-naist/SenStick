#ifndef twi_slave_uv_sensor_h
#define twi_slave_uv_sensor_h

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"

// UVデータ
// サンプリング周期275ミリ秒、単位換算 2.14 uW/cm2/step
typedef uint16_t UltraVioletData_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。
bool initUVSensor(void);

// UVA sensitivity, RSET =240kΩ,IT =1T, typ. 5 μW/cm2/step
// 560kohm 1T、サンプリング周期275ミリ秒。 2.14 uW/cm2/step。
void getUVSensorData(UltraVioletData_t *p_data);

#endif /* twi_slave_uv_sensor_h */
