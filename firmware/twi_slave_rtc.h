#ifndef twi_slave_rtc_h
#define twi_slave_rtc_h

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"

// RTCとのバス機能を提供します。

// 電源ON時点、タイマー動作、割り込みレベル。initはタイマー一致フラグをクリアしない。明示的に呼び出すべし。
void initRTC(void);

// 時計時刻を設定/取得します。
void setTWIRTCDateTime(ble_date_time_t *p_date);
void getTWIRTCDateTime(ble_date_time_t *p_date);

// アラーム時刻を設定/取得します。
void setRTCAlarmDateTime(ble_date_time_t *p_date);
void getRTCAlarmDateTime(ble_date_time_t *p_date);

// アラーム一致によるINT出力を、クリアします
//void clearRTCAlarm(rtc_context_t *p_context);

// アラームの有効/無効
//void setRTCAlarmEnable(rtc_context_t *p_context, bool flag);
//bool getRTCAlarmEnable(rtc_context_t *p_context);

#endif /* twi_slave_rtc_h */
