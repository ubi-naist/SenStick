#ifndef senstick_core_manager_h
#define senstick_core_manager_h

#include "nrf_drv_twi.h"
#include "nrf_drv_spi.h"

#include "twi_slave_nine_axes_sensor.h"
#include "twi_slave_pressure_sensor.h"
#include "twi_slave_humidity_sensor.h"
#include "twi_slave_uv_sensor.h"
#include "twi_slave_brightness_sensor.h"
#include "spi_slave_mx25_flash_memory.h"

// 構造体の宣言
typedef struct senstick_core_s {
    nrf_drv_twi_t twi;
    nrf_drv_spi_t spi;
    
    nine_axes_sensor_context_t  nine_axes_sensor_context;
    pressure_sensor_context_t   pressure_sensor_context;
    humidity_sensor_context_t   humidity_sensor_context;
    uv_sensor_context_t         uv_sensor_context;
    brightness_sensor_context_t brightness_sensor_context;
    
    flash_memory_context_t      flash_memory_context;
} senstick_core_t;

// Senstickの、TWIおよびGPIOの統合動作を提供します。
// 例えば、センサーからデータを読みだして、フラッシュメモリに書き出す一連のシーケンスのように、周辺IOを束ねた逐次動作を提供します。

void init_senstick_core_manager(senstick_core_t *p_context);

// LEDの点灯/消灯
// TWIバスのリセット(電源落として、再起動)
// バッテリー電圧の取得


#endif /* senstick_core_manager_h */
