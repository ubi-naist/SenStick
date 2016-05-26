#include <string.h>

#include <app_button.h>
#include <app_timer_appsh.h>
#include <sdk_errors.h>
#include <app_error.h>

#include "senstick_ble_definition.h"
#include "senstick_io_definition.h"
#include "senstick_data_model.h"

#include "gpio_button_monitoring.h"

// Delay from a GPIOTE event until a button is reported as pushed.
#define BUTTON_DETECTION_DELAY_MS       50
// 長押し
#define LONG_PUSH_DURATION_MS           2000
// 更に長押し
#define LONG_VERY_PUSH_DURATION_MS      (5000 - LONG_PUSH_DURATION_MS)

// 変数
APP_TIMER_DEF(m_button_timer_id);
static app_button_cfg_t m_buttons[1];
static ButtonStatus_t m_status;

static void setButtonStatus(ButtonStatus_t newStatus)
{
    m_status = newStatus;
    senstick_setButtonStatus(m_status);
}
static void buttonIsReleased(void)
{
    switch(m_status) {
            
        case BUTTON_PUSH:
            setButtonStatus(BUTTON_PUSH_RELEASED);
            setButtonStatus(BUTTON_RELEASED);
            break;
        case BUTTON_LONG_PUSH:
            setButtonStatus(BUTTON_LONG_PUSH_RELEASED);
            setButtonStatus(BUTTON_RELEASED);
            break;
        case BUTTON_VERY_LONG_PUSH:
            setButtonStatus(BUTTON_VERY_LONG_PUSH_RELEASED);
            setButtonStatus(BUTTON_RELEASED);
            break;
        default:
            setButtonStatus(BUTTON_RELEASED);
            break;
    }
}

static void _button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;
    
    if(button_action == 0) {
        // released
        app_timer_stop(m_button_timer_id);
        buttonIsReleased();
    } else {
        // pushed
        setButtonStatus(BUTTON_PUSH);
        // タイマー起動
        err_code = app_timer_start(m_button_timer_id,
                                   APP_TIMER_TICKS(LONG_PUSH_DURATION_MS, APP_TIMER_PRESCALER),
                                   NULL);
        APP_ERROR_CHECK(err_code);
    }
}

static void timer_handler(void *p_arg)
{
    ret_code_t err_code;
    
    switch (m_status) {
        case BUTTON_PUSH: // PUSH -> LONG_PUSH
            setButtonStatus(BUTTON_LONG_PUSH);
            // タイマー起動
            err_code = app_timer_start(m_button_timer_id,
                                       APP_TIMER_TICKS(LONG_VERY_PUSH_DURATION_MS, APP_TIMER_PRESCALER),
                                       NULL);
            APP_ERROR_CHECK(err_code);
            break;
        case BUTTON_LONG_PUSH: // LONG -> VERY_LONG
            setButtonStatus(BUTTON_VERY_LONG_PUSH);
            break;
        default:
            break;
    }
}

void initButtonMonitoring(void)
{
    ret_code_t err_code;

    m_status = BUTTON_RELEASED;
    
    // ボタンのポート設定
    m_buttons[0].pin_no          = PIN_NUMBER_TACT_SWITCH;
    m_buttons[0].active_state    = APP_BUTTON_ACTIVE_LOW;
    m_buttons[0].pull_cfg        = NRF_GPIO_PIN_PULLUP;
    m_buttons[0].button_handler  = _button_event_handler;

    err_code = app_button_init(m_buttons, sizeof(m_buttons) / sizeof(m_buttons[0]), APP_TIMER_TICKS(BUTTON_DETECTION_DELAY_MS,  APP_TIMER_PRESCALER));
    APP_ERROR_CHECK(err_code);

    // SDK8では初期化後に app_button_enable() を呼ばないと、ボタン検出は有効にならない。
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_create(&(m_button_timer_id), APP_TIMER_MODE_SINGLE_SHOT, timer_handler);
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


void enableAwakeByButton(void)
{
    nrf_gpio_cfg_sense_input(PIN_NUMBER_TACT_SWITCH, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
}
