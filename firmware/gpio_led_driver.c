#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <app_timer_appsh.h>
#include <nrf_drv_gpiote.h>
#include <app_error.h>

#include "senstick_ble_definition.h"
#include "senstick_io_definition.h"

#include "gpio_led_driver.h"
#include "senstick_data_model.h"

// 3秒または6秒ごとに次の点灯をする
//      動作         100ミリ秒 1回光る
//      ロギング     100ミリ秒 2回光る
//      ログいっぱい  100ミリ秒 3回光る
// 接続時は1秒、非接続時は3秒

APP_TIMER_DEF(m_led_timer_id);

static int  m_pattern_index;
static int  m_blink_count;
static int  m_blank_period;
static bool m_is_timer_running;

static void setLED(bool blink)
{
    if(blink) {
        nrf_drv_gpiote_out_set(PIN_NUMBER_LED);
    } else {
        nrf_drv_gpiote_out_clear(PIN_NUMBER_LED);
    }
}

static void startBlinking(int count, int blank_period_ms)
{
    m_pattern_index = 0;
    m_blink_count   = count;
    m_blank_period  = blank_period_ms / 100;
}

static void updateNextBlink(void)
{
    ButtonStatus_t button_status = senstick_getButtonStatus();
    if(button_status == BUTTON_RELEASED) {
        bool isConnected = senstick_isConnected();
        int period = isConnected ? 3000 : 6000;
        
        senstick_control_command_t command = senstick_getControlCommand();
        int count = (command == sensorShouldWork) ? 2 : 1;
        
        startBlinking(count, period);
    } else {
        // ボタンが押されているので、何もしない, 適当にブランクタイムを設定しておく
        m_blank_period = 1000;
    }
}

static void led_timer_handler(void *p_arg)
{
    // ブランクタイム
    if(m_blink_count <= 0) {
        m_blank_period--;
        if(m_blank_period <= 0) {
            updateNextBlink();
        }
        return;
    }
    
    // LEDの設定と周期のカウント
    if( (m_pattern_index % 2) == 0) {
        setLED(true);
    } else {
        setLED(false);
        m_blink_count--;
    }
    m_pattern_index++;
}

/**
 * Public methods
 */
void initLEDDriver(void)
{
    ret_code_t err_code;
    
    // gpioteモジュールを初期化する
    if(!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    nrf_drv_gpiote_out_config_t out_config;
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
    out_config.task_pin   = false;
    err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_LED, &out_config);
    APP_ERROR_CHECK(err_code);
    
    // 変数の初期化
    m_pattern_index = 0;
    m_blink_count   = 1;
    m_blank_period  = 1000;

    // タイマーの初期化
    err_code = app_timer_create(&(m_led_timer_id), APP_TIMER_MODE_REPEATED, led_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_led_timer_id,
                               APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                               NULL);
    APP_ERROR_CHECK(err_code);
    
    m_is_timer_running = true;
}

void ledDriver_observeControlCommand(senstick_control_command_t command)
{
    if(command == shouldDeviceSleep) {
        if(m_is_timer_running) {
            // タイマー停止, LEDオフ
            m_is_timer_running = false;
            app_timer_stop(m_led_timer_id);
            setLED(false);
        }
    } else {
        if(!m_is_timer_running) {
            // タイマー停止, LEDオフ
            m_is_timer_running = true;
            app_timer_start(m_led_timer_id,
                            APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                            NULL);
        }
    }
}


void ledDriver_observeButtonStatus(ButtonStatus_t status)
{
    switch(status) {
        case BUTTON_RELEASED:
            break;
        case BUTTON_PUSH:
            startBlinking(1, 10000);
            break;
        case BUTTON_PUSH_RELEASED:
            // RELEASED に落ちるので、ここではなにもしない            
            break;
        case BUTTON_LONG_PUSH:
            startBlinking(2, 10000);
            break;
        case BUTTON_LONG_PUSH_RELEASED:
            // RELEASED に落ちるので、ここではなにもしない
            break;
        case BUTTON_VERY_LONG_PUSH:
            startBlinking(3, 10000);
            break;
        case BUTTON_VERY_LONG_PUSH_RELEASED:
            break;
        default:
            break;
    }
}

