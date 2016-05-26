#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_assert.h>
#include <app_error.h>
#include <sdk_errors.h>

#include "value_types.h"
#include "twi_manager.h"
#include "twi_slave_rtc.h"

#include "senstick_sensor_base_data.h"
#include "senstick_io_definition.h"

/**
 * Definitions
 */

// RTCのデータ構造
typedef struct rtcSettingCommand_s {
    // RTCの数値はすべてBCD(Binary-coded decimal)。16進表記したときの各桁が0-9の値を表す。
    uint8_t second;
    uint8_t minute;
    uint8_t hour;       // BCD 24時間表記。
    uint8_t dayOfWeek;  // 曜日(1が日曜日 7が土曜日)
    uint8_t day;
    uint8_t month;
    uint8_t year;       // BCD 西暦の下2桁
} rtcSettingCommand_t;

// RTC_ALARM_SETTING_COMMAND
/*
typedef struct rtcAlarmSettingCommand_s {
    // RTCの数値はすべてBCD
    uint8_t minute;
    uint8_t hour;
    uint8_t dayOfWeekBitFields;  // 曜日(1が日曜日 7が土曜日)
} rtcAlarmSettingCommand_t;
*/

// RTC内部レジスタのタイプ。4ビットの値は、RTC内部のレジスタアドレスに対応する
typedef enum {
    RTCSecondCountRegister  = 0x00,
    RTCMinuteCountRegister  = 0x01,
    RTCHourCountRegister    = 0x02,
    RTCDayOfWeekCountRegister   = 0x03,
    RTCDayCountRegister     = 0x04,
    RTCMonthCountRegister   = 0x05,
    RTCYearCountRegister    = 0x06,
    // 時刻誤差補正カウンタ (0x07) は使用しない
    // AlarmW は、1つあるいは複数の曜日と時間分を指定した、アラート出力を表す。
    RTCAlarm_DayOfWeek_MinuteRegister       = 0x08,
    RTCAlarm_DayOfWeek_HourRegister         = 0x09,
    RTCAlarm_DayOfWeek_DayOfWeekRegister    = 0x0a,
    
    // AlarmD は、時分だけをしていたアラーム出力。
    RTCAlarm_Day_MinuteRegister       = 0x0b,
    RTCAlarm_Day_HourRegister         = 0x0c,
    
    RTCUserRAMRegister  = 0x0d,
    RTCControlRegister1 = 0x0e,
    RTCControlRegister2 = 0x0f,
} RTCRegister_t;

/**
 * Private methods
 */

static uint8_t convertToBCD(uint8_t value)
{
    return ((value / 10) << 4) | (value % 10);
}

static void convertBLEDateTimeToRTCSetting(rtcSettingCommand_t *p_setting, ble_date_time_t *p_date)
{
    p_setting->second   = convertToBCD(p_date->seconds);
    p_setting->minute   = convertToBCD(p_date->minutes);
    p_setting->hour     = convertToBCD(p_date->hours);
    p_setting->dayOfWeek= 1;
    p_setting->day      = convertToBCD(p_date->day);
    p_setting->month    = convertToBCD(p_date->month);
    p_setting->year     = convertToBCD((uint8_t)(p_date->year % 100));
}

static uint8_t convertFromBCD(uint8_t value)
{
    return ((value & 0xf0) >> 4) * 10 + (value & 0x0f);
}

static void convertRTCSettingToBLEDateTime(ble_date_time_t *p_date, rtcSettingCommand_t *p_setting)
{
    p_date->seconds  = convertFromBCD(p_setting->second);
    p_date->minutes  = convertFromBCD(p_setting->minute);
    p_date->hours    = convertFromBCD(p_setting->hour);
    p_date->day      = convertFromBCD(p_setting->day);
    p_date->month    = convertFromBCD(p_setting->month);
    p_date->year     = (uint16_t)convertFromBCD(p_setting->year) + 2000;
}

// RTCに書き込みます。TWI_RTC_ADDRESSは megane_io_definitions.h で定義されているI2Cバス上のRTCのアドレスです。
static void writeToRTC(RTCRegister_t target_register, const uint8_t *data, uint32_t data_length)
{
    uint8_t reg = (((uint8_t)target_register) << 4) | 0x00; // 4ビットの内部アドレス + 4ビットの転送フォーマット(書き込みは0x00しかない)
    writeToTwiSlave(TWI_RTC_ADDRESS, reg, data, data_length);
    nrf_delay_us(31); // R2221L/T 仕様書 p.29 ストップコンディションから次のスタートコンディションまで、31us以上の時間を空ける。
}

static void readFromRTC( RTCRegister_t target_register, uint8_t *data, uint32_t data_length)
{
    uint8_t reg = (((uint8_t)target_register) << 4) | 0x00; // 4ビットの内部アドレス + 4ビットの転送フォーマット(0x00)
    readFromTwiSlave(TWI_RTC_ADDRESS, reg, data, data_length);
    nrf_delay_us(31); // R2221L/T 仕様書 p.29 ストップコンディションから次のスタートコンディションまで、31us以上の時間を空ける。
}

/**
 * Public methods
 */

void initRTC(void)
{
}

void setTWIRTCDateTime(ble_date_time_t *p_date)
{
    rtcSettingCommand_t setting;

    convertBLEDateTimeToRTCSetting(&setting, p_date);
    
    // 値はBCDなので注意すること
//    NRF_LOG_PRINTF_DEBUG("setRTCDateTime y:%0x m:%0x d:%0x dow:%0x h:%0x m:%0x\n", setting.year, setting.month, setting.day, setting.dayOfWeek, setting.hour, setting.minute);
    
    // チップ仕様書 p.29 時刻の設定は、1回のスタートからストップコンディションの間に、0.5秒以内に完了させる制約がある。
    // 時刻を設定する。レジスタのアドレスが連続しているので、値を1度に書き込む。
    // レジスタのアドレス並びは以下のとおり:
    /*
     RTCSecondCountRegister  = 0x00,
     RTCMinuteCountRegister  = 0x01,
     RTCHourCountRegister    = 0x02,
     RTCDayOfWeekCountRegister   = 0x03,
     RTCDayCountRegister     = 0x04,
     RTCMonthCountRegister   = 0x05,
     RTCYearCountRegister    = 0x06,
     */
    
    // 構造体は次の通り。
    // dayOfWeek は、曜日(1が日曜日 7が土曜日)なので、気をつけること。
    /*
     typedef struct {
     // RTCの数値はすべてBCD
     uint8_t second;
     uint8_t minute;
     uint8_t hour;       // BCD 24時間表記。
     uint8_t dayOfWeek;  // 曜日(1が日曜日 7が土曜日)
     uint8_t day;
     uint8_t month;
     uint8_t year;       // BCD 西暦の下2桁
     } rtcSettingCommand_t;
     */
    uint8_t data[] = {
        setting.second,
        setting.minute,
        setting.hour,
        (setting.dayOfWeek -1),
        setting.day,
        setting.month,
        setting.year };
    writeToRTC(RTCSecondCountRegister, data, sizeof(data));
    
//    p_context->is_calender_available = true;
}

void getTWIRTCDateTime(ble_date_time_t *p_date)
{
    rtcSettingCommand_t setting;
    
    // カレンダーが初期化されていない。
//    if( ! p_context->is_calender_available) {
//        return false;
//    }
    
//    NRF_LOG_PRINTF_DEBUG("\ngetRTCDateTime()");
    
    // レジスタのアドレス並びは以下のとおり:
//     RTCSecondCountRegister      = 0x00,
//     RTCMinuteCountRegister      = 0x01,
//     RTCHourCountRegister        = 0x02,
//     RTCDayOfWeekCountRegister   = 0x03,
//     RTCDayCountRegister     = 0x04,
//     RTCMonthCountRegister   = 0x05,
//     RTCYearCountRegister    = 0x06,

    uint8_t data[7];
    readFromRTC(RTCSecondCountRegister, data, sizeof(data));
    
    // 構造体は次の通り。
    // dayOfWeek は、曜日(1が日曜日 7が土曜日)なので、気をつけること。
//     typedef struct {
//     // RTCの数値はすべてBCD
//     uint8_t second;
//     uint8_t minute;
//     uint8_t hour;       // BCD 24時間表記。
//     uint8_t dayOfWeek;  // 曜日(1が日曜日 7が土曜日)
//     uint8_t day;
//     uint8_t month;
//     uint8_t year;       // BCD 西暦の下2桁
//     } rtcSettingCommand_t;

    setting.second    = data[0];
    setting.minute    = data[1];
    setting.hour      = data[2];
    setting.dayOfWeek = data[3] +1; // RTCは週曜日始まりを0として記録するので、+1する。
    setting.day       = data[4];
    setting.month     = data[5];
    setting.year      = data[6];
    
    convertRTCSettingToBLEDateTime(p_date, &setting);
    
//    return true;
}
/*
void setRTCAlarmDateTime( const rtcAlarmSettingCommand_t *p_setting)
{
    NRF_LOG_PRINTF_DEBUG("\nsetRTCAlarmDateTime() h:%0x m:%0x dow_bits:0x%0x", p_setting.hour, p_setting.minute, p_setting.dayOfWeekBitFields);
    
    // アラームの現状のフラグを習得。
    bool isAlarm = getRTCAlarmEnable(p_context);
    
    // アラームを停止
    if(isAlarm) {
        setRTCAlarmEnable(p_context, false);
    }
    
    // アラーム出力フラグをクリア
    clearRTCAlarm(p_context);
    
    // アラーム時刻を設定する。レジスタのアドレスが連続しているので、3つの値を1度に書き込む。
    // rtcAlarmSettingCommand_t の dayOfWeek は、曜日(1が日曜日 7が土曜日)なので、気をつけること。
    uint8_t alarm_w_data[] = {
        p_setting.minute,
        p_setting.hour,
        p_setting.dayOfWeekBitFields };
//
//     RTCAlarm_DayOfWeek_MinuteRegister       = 0x08,
//     RTCAlarm_DayOfWeek_HourRegister         = 0x09,
//     RTCAlarm_DayOfWeek_DayOfWeekRegister    = 0x0a,

    writeToRTC( RTCAlarm_DayOfWeek_MinuteRegister, alarm_w_data, sizeof(alarm_w_data));
    
    // アラームのフラグを再度設定
    if(isAlarm) {
        setRTCAlarmEnable(p_context, true);
    }
}
*/
/*
void getRTCAlarmDateTime( rtcAlarmSettingCommand_t *p_setting)
{
    NRF_LOG_PRINTF_DEBUG("\ngetRTCAlarmDateTime()");
    // レジスタのアドレス並びは以下のとおり:

//     RTCAlarm_DayOfWeek_MinuteRegister       = 0x08,
//     RTCAlarm_DayOfWeek_HourRegister         = 0x09,
//     RTCAlarm_DayOfWeek_DayOfWeekRegister    = 0x0a,

    uint8_t data[3];
    readFromRTC( RTCAlarm_DayOfWeek_MinuteRegister, data, sizeof(data));
    p_setting.minute               = data[0];
    p_setting.hour                 = data[1];
    p_setting.dayOfWeekBitFields   = data[2]; // アラームのdayOfWeekはビットフラグなので、時刻設定のように+1のような加工は必要はない。
}
*/
/*
void clearRTCAlarm(rtc_context_t *p_context)
{
    NRF_LOG_PRINTF_DEBUG("\nclearRTCAlarm()");
    
    // RTCControlRegister2
    // D7: Scratch
    // D6: VDET     0   電源監視結果ビット 0を書き込むことで電源監視を再開する
    // D5: XSTP     0   発振停止検出フラグ 0を書き込むことで発振状態に戻る
    // D4: PON      0   パワーオンリセットビット 0を書き込むことで通常動作になる
    // D3: /CLEN1   0   32kHzクロック出力ビット 0を書き込むことで32kHzクロックが出力される
    // D2: CTFG     0   定周期割込フラグ    0
    // D1: WAFG     0   Alarm_W フラグビット アラーム一致になると1になる。0を書き込むと、一致信号がクリアされる。
    // D0: DAFG     0   Alarm_D フラグビット アラーム一致になると1になる。0を書き込むと、一致信号がクリアされる。
    
    const uint8_t data[] = {0x00};
    writeToRTC( RTCControlRegister2, data, sizeof(data));
}

void setRTCAlarmEnable( bool flag)
{
    NRF_LOG_PRINTF_DEBUG("\nsetRTCAlarmEnable() flag:%d", flag);
    
    uint8_t data;
    if(flag) {
        // RTCControlRegister1
        // D7: WALE     1   Alarm_W 一致動作
        // D6: DALE     0   Alarm_D 一致動作無効
        // D5: 12 /24   1   24時間制度
        // D4: CLEN2    0   32kHzクロック出力有効
        // D3: TEST     0   通常動作モード
        // D2: CT2      0   CT2:0 INTRに一定周期の波形を出力するモード、OFF
        // D1: CT1      0
        // D0: CT0      0
        data = 0xa0;
    } else {
        // RTCControlRegister1
        // D7: WALE     0   Alarm_W 一致動作無効
        // D6: DALE     0   Alarm_D 一致動作無効
        // D5: 12 /24   1   24時間制度
        // D4: CLEN2    0   32kHzクロック出力有効
        // D3: TEST     0   通常動作モード
        // D2: CT2      0   CT2:0 INTRに一定周期の波形を出力するモード、OFF
        // D1: CT1      0
        // D0: CT0      0
        data = 0x20;
    }
    writeToRTC( RTCControlRegister1, &data, 1);
    // hoge もう1つのフラグ?
}

bool getRTCAlarmEnable(rtc_context_t *p_context)
{
    // コントロールレジスタ1を読み出します。
    uint8_t data = 0;
    readFromRTC( RTCControlRegister1, &data, 1);
    // RTCControlRegister1
    // D7: WALE     1   Alarm_W 一致動作
    // D6: DALE     0   Alarm_D 一致動作無効
    // D5: 12 /24   1   24時間制度
    // D4: CLEN2    0   32kHzクロック出力有効
    // D3: TEST     0   通常動作モード
    // D2: CT2      0   CT2:0 INTRに一定周期の波形を出力するモード、OFF
    // D1: CT1      0
    // D0: CT0      0
    bool result = ((data & 0x80) != 0);
    
    NRF_LOG_PRINTF_DEBUG("\ngetRTCAlarmEnable() %d", result);
    return result;
}

*/
