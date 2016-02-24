#include <string.h>

#include <nrf_drv_gpiote.h>
#include <app_error.h>

#include "senstick_io_definitions.h"
#include "senstick_definitions.h"

#include "gpio_led_driver.h"

static void setLED(bool blink)
{
    if(blink) {
        nrf_drv_gpiote_out_set(PIN_NUMBER_LED);
    } else {
        nrf_drv_gpiote_out_clear(PIN_NUMBER_LED);
    }

}

static void timer_handler(void *p_arg)
{
    ret_code_t err_code;
    LEDDriver_t *p_context = (LEDDriver_t *)p_arg;
  
    // インデックスをすすめる。
    // もし雲繰り返しではないならば、LEDを消して終了する。
    (p_context->index)++;
    if(p_context->index >= p_context->pattern_length) {
        p_context->index %= p_context->pattern_length;
        if( ! p_context->repeat) {
            setLED(false);
            return;
        }
    }
    
    // indexが奇数の場合はLEDを点灯。
    setLED((p_context->index % 2) != 0);
    
    // 次回の呼び出しタイマーセット
    err_code = app_timer_start(p_context->timer_id,
                               APP_TIMER_TICKS(p_context->pattern[p_context->index], APP_TIMER_PRESCALER),
                               p_arg);
    APP_ERROR_CHECK(err_code);
    
}

void LEDDriverInit(LEDDriver_t *p_context)
{
    ret_code_t err_code;
    
    memset(p_context, 0, sizeof(LEDDriver_t));

    // 初期パターンを設定
    p_context->pattern[0] = 100; // 最初の黒期間
    p_context->pattern[1] = 100; // 1回めの点灯
    p_context->pattern[2] = 100;
    p_context->pattern[3] = 100; // 2回めの点灯
    p_context->pattern[4] = 100;
    
    // SDK10では、SDK8と異なり、タイマー管理のデータ領域の型とID型が分離されたため、ここでID型(データ領域へのポインタ)にポインタを代入する。
    // 構造体のメンバ変数定義では、APP_TIMER_DEFマクロが使えないため、手動でこのコードを書く必要がある。
    p_context->timer_id = &(p_context->timer_id_data);
    err_code = app_timer_create(&(p_context->timer_id), APP_TIMER_MODE_SINGLE_SHOT, timer_handler);
    APP_ERROR_CHECK(err_code);
}

void setLEDDriverBlinkMode(LEDDriver_t *p_context, uint16_t period, uint16_t mode, bool repeat)
{
    ret_code_t err_code;
    
    // まず止める。
    if(p_context->is_blinking) {
        p_context->is_blinking = false;
        setLED(false);
        app_timer_stop(p_context->timer_id);
    }
    
    // 停止すべきであれば、ここで終了
    if(period == 0 || mode == 0) {
        return;
    }
    
    // パラメータ設定
    p_context->index = 0;
    p_context->pattern[0] = period;
    switch (mode) {
        case 1:
            p_context->pattern_length = 2;
            break;
        case 2:
            p_context->pattern_length = 4;
            break;
        default:
            break;
    }

    // 点灯
    p_context->repeat = repeat;
    p_context->is_blinking = true;
    p_context->index = 0;
    err_code = app_timer_start(p_context->timer_id,
                               APP_TIMER_TICKS(p_context->pattern[0], APP_TIMER_PRESCALER),
                               p_context);
    APP_ERROR_CHECK(err_code);
}

