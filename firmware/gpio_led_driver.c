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

static int m_period; // 点灯周期, 2秒または5秒
static int m_pattern_index;
static uint8_t *m_p_pattern;
static uint8_t m_single_blink_pattern[] = {100, 0}; // 100ミリ秒点灯、消灯
static uint8_t m_double_blink_pattern[] = {100, 100, 100, 0}; // 100ミリ秒2回点灯、消灯
static uint8_t m_triple_blink_pattern[] = {100, 100, 100, 100, 100, 0}; // 100ミリ秒3回点灯、消灯
static int m_pres_period;
static uint8_t *m_prev_pattern;

static void setLED(bool blink)
{
    if(blink) {
        nrf_drv_gpiote_out_set(PIN_NUMBER_LED);
    } else {
        nrf_drv_gpiote_out_clear(PIN_NUMBER_LED);
    }
}

static void led_timer_handler(void *p_arg)
{
    ret_code_t err_code;

    if( m_p_pattern == NULL) {
        return;
    }
    
    // 点灯
    setLED((m_pattern_index % 2) == 0);
    
    // 次のタイマーキック
    if(m_p_pattern[m_pattern_index] == 0) {
        err_code = app_timer_start(m_led_timer_id,
                                   APP_TIMER_TICKS(m_period, APP_TIMER_PRESCALER),
                                   NULL);
        APP_ERROR_CHECK(err_code);
        // 終端、次の点灯に
        m_pattern_index = 0;
    } else {
        // 短い点灯
        err_code = app_timer_start(m_led_timer_id,
                                   APP_TIMER_TICKS(m_p_pattern[m_pattern_index], APP_TIMER_PRESCALER),
                                   NULL);
        APP_ERROR_CHECK(err_code);
        // 次の配列要素
        m_pattern_index++;
    }
}

static void stopBlinking(void)
{
    app_timer_stop(m_led_timer_id);
    setLED(false);
}

static void startBlinking(void)
{
    ret_code_t err_code;
    
    setLED(false);
    m_pattern_index = 0;
    err_code = app_timer_start(m_led_timer_id,
                               APP_TIMER_TICKS(m_period, APP_TIMER_PRESCALER),
                               NULL);
    APP_ERROR_CHECK(err_code);
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
    m_period        = 5000;
    m_pattern_index = 0;
    m_p_pattern     = NULL;

    m_pres_period  = 5000;
    m_prev_pattern = NULL;
    
    // タイマーの初期化
    err_code = app_timer_create(&(m_led_timer_id), APP_TIMER_MODE_SINGLE_SHOT, led_timer_handler);
    APP_ERROR_CHECK(err_code);
}

void ledDriver_observeControlCommand(senstick_control_command_t command)
{
    switch(command) {
        case sensorShouldSleep:
            m_pattern_index = 0;
            m_p_pattern = m_single_blink_pattern;
            startBlinking();
            break;
        case sensorShouldWork:
            m_pattern_index = 0;
            m_p_pattern = m_double_blink_pattern;
            startBlinking();
            break;
        case formattingStorage:
            stopBlinking();
            break;
        case enterDeepSleep:
            stopBlinking();
            break;
        case enterDFUmode:
            stopBlinking();
            break;
        default: break;
    }
}

void ledDriver_handleBLEEvent(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            m_period = 2000;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            m_period = 5000;
            break;
        default:
            break;
    }
}

static void pushCurrent(void)
{
    m_pres_period  = m_period;
    m_prev_pattern = m_p_pattern;
}

static void recoverPrev(void)
{
    m_period    = m_pres_period;
    m_p_pattern = m_prev_pattern;
}

void ledDriver_observeButtonStatus(ButtonStatus_t status)
{
    switch(status) {
        case BUTTON_RELEASED:
            recoverPrev();
            startBlinking();
            break;
        case BUTTON_PUSH:
            pushCurrent();
            m_pattern_index = 0;
            m_period    = 10000;
            m_p_pattern = m_single_blink_pattern;
            startBlinking();
            break;
        case BUTTON_PUSH_RELEASED:
            recoverPrev();
            startBlinking();
            break;
        case BUTTON_LONG_PUSH:
            m_pattern_index = 0;
            m_p_pattern = m_double_blink_pattern;
            startBlinking();
            break;
        case BUTTON_LONG_PUSH_RELEASED:
            recoverPrev();
            startBlinking();
            break;
        case BUTTON_VERY_LONG_PUSH:
            m_pattern_index = 0;
            m_p_pattern = m_triple_blink_pattern;
            startBlinking();
            break;
        case BUTTON_VERY_LONG_PUSH_RELEASED:
            recoverPrev();
            startBlinking();
            // 長時間押したらフォーマットに落とします。
            senstick_setControlCommand(formattingStorage);
            break;
        default:
            break;
    }
}
/*
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
*/
