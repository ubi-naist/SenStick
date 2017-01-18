#ifndef senstick_rtc_h
#define senstick_rtc_h

// 時計機能を提供します。

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"

// 時間計測の初期処理。
void initSenstickRTC(void);

// 時計時刻を設定/取得します。
void setSenstickRTCDateTime(ble_date_time_t *p_date);
void getSenstickRTCDateTime(ble_date_time_t *p_date);

#endif /* senstick_rtc_h */
