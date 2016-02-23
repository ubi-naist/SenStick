#include <string.h>

#include <app_button.h>
#include <app_timer_appsh.h>
#include <sdk_errors.h>
#include <app_error.h>

#include "senstick_definitions.h"
#include "senstick_io_definitions.h"

#include "gpio_button_monitoring.h"

// Delay from a GPIOTE event until a button is reported as pushed.
#define BUTTON_DETECTION_DELAY_MS       50
// 長押し
#define LONG_PUSH_DURATION_MS           3000
// 更に長押し
#define LONG_VERY_PUSH_DURATION_MS      (6000 - LONG_PUSH_DURATION_MS)

// ボタンアクセスを提供します。
typedef struct {
    app_timer_t timer_id_data;
    app_timer_id_t timer_id;
    
    app_button_cfg_t buttons[1];
    
    button_callback_handler_t handler;
    ButtonStatus_t status;
} ButtonMonitoring_t;

static ButtonMonitoring_t context;

static void changeButtonStatus(ButtonStatus_t newStatus)
{
    ButtonStatus_t prev = context.status;
    context.status = newStatus;
    
    (context.handler)(context.status, prev);
}

static void _button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;
    
    if(button_action == 0) {
        // released
        app_timer_stop(context.timer_id);
        changeButtonStatus(BUTTON_RELEASED);
    } else {
        // pushed
        changeButtonStatus(BUTTON_PUSHED);
        // タイマー起動
        err_code = app_timer_start(context.timer_id,
                                   APP_TIMER_TICKS(LONG_PUSH_DURATION_MS, APP_TIMER_PRESCALER),
                                   NULL);
        APP_ERROR_CHECK(err_code);
    }
}

static void timer_handler(void *p_arg)
{
    ret_code_t err_code;
    
    switch (context.status) {
        case BUTTON_PUSHED: // PUSH -> LONG_PUSH
            changeButtonStatus(BUTTON_LONG_PUSH);
            // タイマー起動
            err_code = app_timer_start(context.timer_id,
                                       APP_TIMER_TICKS(LONG_VERY_PUSH_DURATION_MS, APP_TIMER_PRESCALER),
                                       NULL);
            APP_ERROR_CHECK(err_code);
            break;
        case BUTTON_LONG_PUSH: // LONG -> VERY_LONG
            changeButtonStatus(BUTTON_VERY_LONG_PUSH);
            break;
        default:
            break;
    }
}

void buttonMonitoringInit(button_callback_handler_t handler)
{
    ret_code_t err_code;
    
    context.handler = handler;

    // ボタンのポート設定
    context.buttons[0].pin_no          = PIN_NUMBER_TACT_SWITCH;
    context.buttons[0].active_state    = APP_BUTTON_ACTIVE_LOW;
    context.buttons[0].pull_cfg        = NRF_GPIO_PIN_PULLUP;
    context.buttons[0].button_handler  = _button_event_handler;

    err_code = app_button_init(context.buttons, sizeof(context.buttons) / sizeof(context.buttons[0]), APP_TIMER_TICKS(BUTTON_DETECTION_DELAY_MS,  APP_TIMER_PRESCALER));
    APP_ERROR_CHECK(err_code);

    // SDK8では初期化後に app_button_enable() を呼ばないと、ボタン検出は有効にならない。
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
    
    // SDK10では、SDK8と異なり、タイマー管理のデータ領域の型とID型が分離されたため、ここでID型(データ領域へのポインタ)にポインタを代入する。
    // 構造体のメンバ変数定義では、APP_TIMER_DEFマクロが使えないため、手動でこのコードを書く必要がある。
    context.timer_id = &(context.timer_id_data);
    err_code = app_timer_create(&(context.timer_id), APP_TIMER_MODE_SINGLE_SHOT, timer_handler);
    APP_ERROR_CHECK(err_code);
    
    // SDK8ではピン情報を保存する内部配列のインデックスに変更されている。
    // バグではないかと、2014年に指摘されているが、その後ドキュメントが配列インデックスと変更されているため、この状態が公式だと思われる。
    // https://devzone.nordicsemi.com/question/13701/bug-in-app_button_is_pushed/
    /*
    err_code = app_button_is_pushed(0, &isPushed); // ピン番号ではなく、ボタン情報の配列インデックスで指定する。
    APP_ERROR_CHECK(err_code);
    err_code = app_button_is_pushed(1, &isAlert);
    APP_ERROR_CHECK(err_code);
     */
}
