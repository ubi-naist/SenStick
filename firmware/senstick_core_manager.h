#ifndef senstick_core_manager_h
#define senstick_core_manager_h

#include "twi_slave_nine_axes_sensor.h"

// 構造体の宣言
typedef struct senstick_core_s {
    nrf_drv_twi_t twi;
    nine_axes_sensor_context_t nine_axes_sensor_context;
} senstick_core_t;

// Senstickの、TWIおよびGPIOの統合動作を提供します。
// 例えば、センサーからデータを読みだして、フラッシュメモリに書き出す一連のシーケンスのように、周辺IOを束ねた逐次動作を提供します。

void init_senstick_core_manager(senstick_core_t *p_context);

// LEDの点灯/消灯
// TWIバスのリセット(電源落として、再起動)
// バッテリー電圧の取得


#endif /* senstick_core_manager_h */
