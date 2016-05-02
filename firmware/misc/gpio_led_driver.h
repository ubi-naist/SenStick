#ifndef gpio_led_driver_h
#define gpio_led_driver_h

#include <app_timer_appsh.h>

typedef struct LEDDriver_s {
    // タイマー
    app_timer_t timer_id_data;
    app_timer_id_t timer_id;

    bool repeat;
    bool is_blinking;
    uint8_t pattern_length;
    uint8_t index;
    uint16_t pattern[5];
} LEDDriver_t;

void LEDDriverInit(LEDDriver_t *p_context);
// 点滅周期(ミリ秒単位)、点滅回数
// 停止するときは、いずれかに0を指定
void setLEDDriverBlinkMode(LEDDriver_t *p_context, uint16_t period, uint16_t blink, bool repeat);

#endif /* gpio_led_driver_h */
