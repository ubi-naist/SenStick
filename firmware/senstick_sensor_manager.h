#ifndef senstick_sensor_manager_h
#define senstick_sensor_manager_h

#include "nrf_drv_twi.h"
#include "nrf_drv_spi.h"

#include "app_timer.h"

#include "twi_slave_nine_axes_sensor.h"
#include "twi_slave_pressure_sensor.h"
#include "twi_slave_humidity_sensor.h"
#include "twi_slave_uv_sensor.h"
#include "twi_slave_brightness_sensor.h"
#include "spi_slave_mx25_flash_memory.h"
#include "senstick_logger.h"

// センサデータの更新コールバック
typedef void (*sampling_callback_handler_t) (SensorType_t sensorType, const SensorData_t *p_sensorData);

// コンテキストの構造体宣言
typedef struct senstick_core_s {
    bool is_sampling;
    int  sampling_count;

    app_timer_t timer_id_data;
    app_timer_id_t timer_id;
//    APP_TIMER_DEF(timer_id);
    
    nrf_drv_twi_t twi;
    nrf_drv_spi_t spi;
    
    nine_axes_sensor_context_t  nine_axes_sensor_context;
    pressure_sensor_context_t   pressure_sensor_context;
    humidity_sensor_context_t   humidity_sensor_context;
    uv_sensor_context_t         uv_sensor_context;
    brightness_sensor_context_t brightness_sensor_context;
    
    flash_memory_context_t      flash_memory_context;
    senstick_logger_t           logger_context;

    sensorSetting_t             sensorSetting;

    sampling_callback_handler_t sampling_callback_handler;
    
} senstick_core_t;

// Senstickの、TWIおよびGPIOの統合動作を提供します。
// 例えば、センサーからデータを読みだして、フラッシュメモリに書き出す一連のシーケンスのように、周辺IOを束ねた逐次動作を提供します。

void initSenstickCoreManager(senstick_core_t *p_context, sampling_callback_handler_t samplingCallback);

// サンプリングレートの設定
void setSensorSetting(senstick_core_t *p_context, const sensorSetting_t *p_setting);

// サンプリング中?
bool isSampling(senstick_core_t *p_context);

// サンプリングの開始
void startSampling(senstick_core_t *p_context);

// サンプリングの停止
void stopSampling(senstick_core_t *p_context);

#endif /* senstick_sensor_manager_h */
