#ifndef twi_slave_rtc_h
#define twi_slave_rtc_h

#include <stdint.h>
#include <stdbool.h>

// RTCとのバス機能を提供します。

// 電源ON時点、タイマー動作、割り込みレベル。initはタイマー一致フラグをクリアしない。明示的に呼び出すべし。
void initRTC(void);

// 時計時刻を設定/取得します。
void setRTCDateTime(const rtcSettingCommand_t *p_setting);
void getRTCDateTime(rtcSettingCommand_t *p_setting);

// アラーム時刻を設定/取得します。
void setRTCAlarmDateTime(const rtcAlarmSettingCommand_t *p_setting);
void getRTCAlarmDateTime(rtcAlarmSettingCommand_t *p_setting);

// アラーム一致によるINT出力を、クリアします
void clearRTCAlarm(void);

// アラームの有効/無効
void setRTCAlarmEnable(bool flag);
bool getRTCAlarmEnable(void);


#endif /* twi_slave_rtc_h */
