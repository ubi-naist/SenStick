#include <string.h>
#include <stdint.h>
#include <time.h>

#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_assert.h>
#include <app_error.h>
#include <app_timer.h>
#include <sdk_errors.h>
#include <sdk_config.h>

#include "senstick_rtc.h"

#include "value_types.h"
#include "senstick_util.h"
#include "senstick_sensor_base_data.h"
#include "senstick_ble_definition.h"

/**
 * Definitions
 */

// RTCの実行時コンテキスト構造体
typedef struct rtcContext_s {
    time_t current_time;       // 1970年1月1日を基準とした秒数。0であれば、初期設定されていない。
    int    previous_rtc_value; // 前回取得時のRTC1の値
} rtcContext_t;

static rtcContext_t _rtcContext;
APP_TIMER_DEF(m_rtc_timer_id);

/**
 * Private methods
 */

// RTCを読み出し、現在時間を更新
void updateCurrentTime(void)
{
    // current timeが0であれば、初期設定されていないので、何もしない。
    if(_rtcContext.current_time == 0) {
        return;
    }
    
    // RTCを読み出し、前回との差分を計算する。RTC1カウンタ(24ビット)がラウンドアップしているかもしれないので、1<<24を足して剰余を求め、常に正の差分とする。
    int rtc_value = app_timer_cnt_get();
    int duration  = (rtc_value - _rtcContext.previous_rtc_value + (1 << 24)) % (1 << 24);
    
    // 秒数を追加
    _rtcContext.current_time += duration / APP_TIMER_CLOCK_FREQ;
    
    // 読み出しRTCの値を更新。
    _rtcContext.previous_rtc_value = rtc_value;
}

static void rtc_timer_handler(void *p_arg)
{
    updateCurrentTime();
}

void convertBLEDateTimeToCTime(const ble_date_time_t *p_ble_date, struct tm * p_c_time_date)
{
    memset(p_c_time_date, 0, sizeof(struct tm));
    
    p_c_time_date->tm_sec  = p_ble_date->seconds;
    p_c_time_date->tm_min  = p_ble_date->minutes;
    p_c_time_date->tm_hour = p_ble_date->hours;
    p_c_time_date->tm_mday = p_ble_date->day;
    p_c_time_date->tm_mon  = p_ble_date->month;
    p_c_time_date->tm_year = p_ble_date->year - 1900;
    
    // Ignore daylight saving for this conversion.
    p_c_time_date->tm_isdst = 0;
}

void convertCTimeToBLEDateTime(const struct tm * p_c_time_date, ble_date_time_t *p_ble_date)
{
    p_ble_date->seconds = p_c_time_date->tm_sec;
    p_ble_date->minutes = p_c_time_date->tm_min;
    p_ble_date->hours   = p_c_time_date->tm_hour;
    p_ble_date->day     = p_c_time_date->tm_mday;
    p_ble_date->month   = p_c_time_date->tm_mon;
    p_ble_date->year    = p_c_time_date->tm_year + 1900;
}

/**
 * Public methods
 */
void initSenstickRTC(void)
{
    ret_code_t err_code;
    
    // 変数初期化
    memset(&_rtcContext, 0, sizeof(_rtcContext));
    
    // タイマー起動。RTC1は24ビットカウンタなので、ソースクロックが32.768kHzの場合で512秒でカウンタがラウンドする。それを見落とさないよう、その半周期でサンプリングする。
    err_code = app_timer_create(&(m_rtc_timer_id), APP_TIMER_MODE_REPEATED, rtc_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_rtc_timer_id,
                               APP_TIMER_TICKS(256 * 1000, APP_TIMER_PRESCALER),
                               NULL);
    APP_ERROR_CHECK(err_code);
}

void setSenstickRTCDateTime(const ble_date_time_t *p_date)
{
    // ctimeに変換。
    struct tm c_time_date;
    convertBLEDateTimeToCTime(p_date, &c_time_date);
    // 秒数に変換
    _rtcContext.current_time       = mktime(&c_time_date);
    // 現在のRTC1の値を基準とする。
    _rtcContext.previous_rtc_value = app_timer_cnt_get();
    
//    NRF_LOG_PRINTF_DEBUG("\nsetSenstickRTCDateTime() y:%d m:%d h:%d m:%d.", p_date->year, p_date->month, p_date->hours, p_date->minutes);
}

void getSenstickRTCDateTime(ble_date_time_t *p_date)
{
    // RTCを読み出し、現在時間を更新
    updateCurrentTime();
    
    // 秒数をctimeに変換。
    struct tm ctime = *(localtime(&_rtcContext.current_time)); // 構造体のポインタを返すので、そのまま値をコピー。
    // ctimeをble_date_time_t に変換。
    convertCTimeToBLEDateTime(&ctime, p_date);
    
//    NRF_LOG_PRINTF_DEBUG("\ngetSenstickRTCDateTime() y:%d m:%d h:%d m:%d.", p_date->year, p_date->month, p_date->hours, p_date->minutes);
}

// 時刻をデバッグ出力します。
void debugPrintRTCDateTime(const ble_date_time_t *p_date)
{
    NRF_LOG_PRINTF_DEBUG("\nble_date_time: y:%d m:%d d:%d h:%d m:%d.", p_date->year, p_date->month, p_date->day, p_date->hours, p_date->minutes);
}
