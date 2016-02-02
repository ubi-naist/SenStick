#include "senstick_tests.h"

/*
 static void test_twi_slaves(sensor_manager_t *p_context)
 {
 // 値取得、デバッグ
 while(1) {
 MotionSensorData_t sensor_data;
 getNineAxesData(&(p_context->nine_axes_sensor_context), &sensor_data);
 debugLogAccerationData(&(sensor_data.acceleration));
 
 AirPressureData_t pressure_data;
 getPressureData(&(p_context->pressure_sensor_context), &pressure_data);
 NRF_LOG_PRINTF_DEBUG("Pressure, %d.\n", pressure_data); // 0.01hPa resolution
 
 HumidityData_t humidity_data;
 getHumidityData(&(p_context->humidity_sensor_context), &humidity_data);
 NRF_LOG_PRINTF_DEBUG("Humidity, %d.\n", humidity_data);
 
 UltraVioletData_t uv_data;
 getUVSensorData(&(p_context->uv_sensor_context), &uv_data);
 NRF_LOG_PRINTF_DEBUG("UV, %d.\n", uv_data);
 
 BrightnessData_t brightness_data;
 getBrightnessData(&(p_context->brightness_sensor_context), &brightness_data);
 NRF_LOG_PRINTF_DEBUG("Brightness, %d.\n", brightness_data);
 
 NRF_LOG_PRINTF_DEBUG("\n");
 nrf_delay_ms(500);
 }
 }
 */