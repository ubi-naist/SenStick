#ifndef twi_slave_rtc_h
#define twi_slave_rtc_h

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_twi.h"
#include "senstick_data_models.h"

typedef struct {
    nrf_drv_twi_t *p_twi;
} rtc_init_t;

typedef struct {
    nrf_drv_twi_t *p_twi;
} rtc_context_t;

// RTCとのバス機能を提供します。

// 電源ON時点、タイマー動作、割り込みレベル。initはタイマー一致フラグをクリアしない。明示的に呼び出すべし。
void initRTC(rtc_context_t *p_context, const rtc_init_t *p_init);

// 時計時刻を設定/取得します。
void setRTCDateTime(rtc_context_t *p_context, const rtcSettingCommand_t *p_setting);
void getRTCDateTime(rtc_context_t *p_context, rtcSettingCommand_t *p_setting);

// アラーム時刻を設定/取得します。
void setRTCAlarmDateTime(rtc_context_t *p_context, const rtcAlarmSettingCommand_t *p_setting);
void getRTCAlarmDateTime(rtc_context_t *p_context, rtcAlarmSettingCommand_t *p_setting);

// アラーム一致によるINT出力を、クリアします
void clearRTCAlarm(rtc_context_t *p_context);

// アラームの有効/無効
void setRTCAlarmEnable(rtc_context_t *p_context, bool flag);
bool getRTCAlarmEnable(rtc_context_t *p_context);

#endif /* twi_slave_rtc_h */
