#include <stdint.h>
#include <string.h>

#include "nordic_common.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_drv_config.h"
#include "nrf_adc.h"

#include "app_util_platform.h"
#include "app_error.h"

#include "senstick_definitions.h"
#include "senstick_sensor_manager.h"
#include "senstick_io_definitions.h"
#include "senstick_data_models.h"

/**
 * 型宣言
 */

/**
 * イベントハンドラ
 */
static void gpio_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    switch(pin) {
        case PIN_NUMBER_TACT_SWITCH:
            break;
        default:
            break;
    }
}

/**
* Private メソッド
*/

// IO初期化ルーチン
static void init_gpio(void)
{
    ret_code_t err_code;
    
    nrf_drv_gpiote_out_config_t out_config;
    nrf_drv_gpiote_in_config_t in_config;
    
    // gpioteモジュールを初期化する
    if(!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    
    // LEDピンの設定
    // 引数は init_high。初期値をlowにする。
//    out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
    out_config.task_pin = false;
    err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_LED, &out_config);
    APP_ERROR_CHECK(err_code);
    
    // タクトスイッチの設定
    // P0.20    In      タクトスイッチ接続。スイッチオンでGNDに落とされる。
    // 引数は high accuracy。精度はいらないのでfalseを指定。
//    in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    in_config.is_watcher  = false;
    in_config.hi_accuracy = false;
    in_config.sense       = NRF_GPIOTE_POLARITY_HITOLO;
    in_config.pull        = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(PIN_NUMBER_TACT_SWITCH, &in_config, gpio_event_handler);
    APP_ERROR_CHECK(err_code);

    // TWIのデバイス電源設定
    // 引数は init_high。初期値をhighにする。
//    out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH;
    out_config.task_pin = false;
    err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_TWI_POWER, &out_config);
    APP_ERROR_CHECK(err_code);
    // ドライブストレンクスを"強"に
    nrf_gpio_cfg(PIN_NUMBER_TWI_POWER,
                     NRF_GPIO_PIN_DIR_OUTPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_H0H1,   // 0にハイドライブ、1にハイドライブ
                     NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_set(PIN_NUMBER_TWI_POWER);
    
    // 以下のピンは、今は利用しない
    // P0.08    In      9軸センサーINT
    // P0.09    In      RTC INTIR
    // P0.27    In      32kHzクロックが供給される
}

// ADC初期化関数
static void init_adc(void)
{
    // 10ビット解像度、スケーリング 1/3、リファレンス バンドギャップ電圧()
    nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG_DEFAULT;
    
    // Initialize and configure ADC
    nrf_adc_configure(&nrf_adc_config);
    nrf_adc_input_select(ADC_INPUT_SUPPLY_MONITORING);
}

static void init_twi_slaves(senstick_sensor_manager_t *p_context)
{
    ret_code_t err_code;
    
    // TWIインタフェース TWI1を使用。
    p_context->twi.p_reg        = NRF_TWI1;
    p_context->twi.irq          = TWI1_IRQ;
    p_context->twi.instance_id  = TWI1_INSTANCE_INDEX;

    err_code = nrf_drv_twi_init(&(p_context->twi), NULL, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&(p_context->twi));
    
    // slaveの初期化
    initNineAxesSensor(&(p_context->nine_axes_sensor_context), &(p_context->twi));
    initPressureSensor(&(p_context->pressure_sensor_context),  &(p_context->twi));
    initHumiditySensor(&(p_context->humidity_sensor_context),  &(p_context->twi));
    initUVSensor(&(p_context->uv_sensor_context),  &(p_context->twi));
    initBrightnessSensor(&(p_context->brightness_sensor_context),  &(p_context->twi));
    // 初期化処理完了待ち時間
    nrf_delay_ms(100);
}
/*
static void test_twi_slaves(senstick_sensor_manager_t *p_context)
{
    // 値取得、デバッグ
    while(1) {
        MotionSensorData_t sensor_data;
        getNineAxesData(&(p_context->nine_axes_sensor_context), &sensor_data);
        debugLogAccerationData(&(sensor_data.acceleration));
        
        AirPressureData_t pressure_data;
        getPressureData(&(p_context->pressure_sensor_context), &pressure_data);
        NRF_LOG_PRINTF_DEBUG("Pressure, %d.\n", pressure_data); // 0.01hPa resolution
        
        HumidityData_t humidity_data;
        getHumidityData(&(p_context->humidity_sensor_context), &humidity_data);
        NRF_LOG_PRINTF_DEBUG("Humidity, %d.\n", humidity_data);
        
        UltraVioletData_t uv_data;
        getUVSensorData(&(p_context->uv_sensor_context), &uv_data);
        NRF_LOG_PRINTF_DEBUG("UV, %d.\n", uv_data);
        
        BrightnessData_t brightness_data;
        getBrightnessData(&(p_context->brightness_sensor_context), &brightness_data);
        NRF_LOG_PRINTF_DEBUG("Brightness, %d.\n", brightness_data);
        
        NRF_LOG_PRINTF_DEBUG("\n");
        nrf_delay_ms(500);
    }
}
*/

// サンプリングレートで、通知すべきか否かを判定します
static bool  should_report(int count, SamplingRate_t sampling_rage)
{
    switch (sampling_rage) {
        case SAMPLING_RATE_0_1_Hz:  return ((count % 100) == 0);
        case SAMPLING_RATE_1_Hz:    return ((count % 10 ) == 0);
        case SAMPLING_RATE_10_Hz:   return true;
        default: break;
    }
    return false;
}

static void sensor_timer_handler(void *p_arg)
{
    senstick_sensor_manager_t *p_context = (senstick_sensor_manager_t *)p_arg;
    
    SensorData_t sensor_data;
    
    // 加速度センサー
    if( should_report(p_context->sampling_count, p_context->sensorSetting.nineAxesSensorSamplingRate)) {
        getNineAxesData(&(p_context->nine_axes_sensor_context), &(sensor_data.motionSensorData));
        (p_context->sampling_callback_handler)(MotionSensor, &sensor_data);
    }
    
    // 温度と湿度
    if( should_report(p_context->sampling_count, p_context->sensorSetting.humiditySensorSamplingRate)) {
        getHumidityData(&(p_context->humidity_sensor_context), &(sensor_data.temperatureAndHumidityData.humidity));
        getTemperatureData(&(p_context->humidity_sensor_context), &(sensor_data.temperatureAndHumidityData.temperature));
        (p_context->sampling_callback_handler)(TemperatureAndHumiditySensor, &sensor_data);
    }
    
    // 圧力
    if( should_report(p_context->sampling_count, p_context->sensorSetting.pressureSensorSamplingRate)) {
        getPressureData(&(p_context->pressure_sensor_context), &(sensor_data.airPressureData));
        (p_context->sampling_callback_handler)(AirPressureSensor, &sensor_data);
    }

    // 照度
    if( should_report(p_context->sampling_count, p_context->sensorSetting.illuminationSensorSamplingRate)) {
        getBrightnessData(&(p_context->brightness_sensor_context), &(sensor_data.brightnessData));
        (p_context->sampling_callback_handler)(AirPressureSensor, &sensor_data);
    }

    // 桁あふれしないように、丸めておきます
    p_context->sampling_count ++;
    p_context->sampling_count %= 10000;
}

/**
 * Public関数
 */
void initSensorManager(senstick_sensor_manager_t *p_context, sampling_callback_handler_t samplingCallback)
{
    uint32_t err_code;
    
    // パラメータチェック
    if(samplingCallback == NULL) {
        APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
    }
    
    // コンテキストの初期値設定
    memset(p_context, 0, sizeof(senstick_sensor_manager_t));

    p_context->sampling_callback_handler = samplingCallback;
    

    // SDK10では、SDK8と異なり、タイマー管理のデータ領域の型とID型が分離されたため、ここでID型(データ領域へのポインタ)にポインタを代入する。
    // 構造体のメンバ変数定義では、APP_TIMER_DEFマクロが使えないため、手動でこのコードを書く必要がある。
    p_context->timer_id = &(p_context->timer_id_data);
    err_code = app_timer_create(&(p_context->timer_id), APP_TIMER_MODE_REPEATED, sensor_timer_handler);
    APP_ERROR_CHECK(err_code);
    
    // デフォルト値を設定
    p_context->sensorSetting.accelerationRange = ACCELERATION_RANGE_2G;
    p_context->sensorSetting.rotationRange      = ROTATION_RANGE_250DPS;
    p_context->sensorSetting.nineAxesSensorSamplingRate     = SAMPLING_RATE_10_Hz;
    p_context->sensorSetting.humiditySensorSamplingRate     = SAMPLING_RATE_0_1_Hz;
    p_context->sensorSetting.pressureSensorSamplingRate     = SAMPLING_RATE_10_Hz;
    p_context->sensorSetting.illuminationSensorSamplingRate = SAMPLING_RATE_1_Hz;
    
    // 周辺回路を初期化
    init_gpio();
    init_adc();
    // 周辺デバイスの起動待ち時間。MPU-9250最大100ミリ秒
    nrf_delay_ms(100);
    
    init_twi_slaves(p_context);
//    test_twi_slaves(p_context);

    // センサー設定
    setSensorManagerSetting(p_context, &(p_context->sensorSetting));
}

void setSensorManagerSetting(senstick_sensor_manager_t *p_context, const sensorSetting_t *p_setting)
{
    // 設定情報をコピー
    p_context->sensorSetting = *p_setting;
    
    // レンジを設定
    setNineAxesSensorAccelerationRange(&(p_context->nine_axes_sensor_context), p_setting->accelerationRange);
    setNineAxesSensorRotationRange(&(p_context->nine_axes_sensor_context), p_setting->rotationRange);
}

// サンプリング中?
bool isSensorManagerSampling(senstick_sensor_manager_t *p_context)
{
    return p_context->is_sampling;
}

void sensorManagerStartSampling(senstick_sensor_manager_t *p_context)
{
    if( p_context->is_sampling) {
        return;
    }

    p_context->sampling_count = 0;
    p_context->is_sampling    = true;
    
    // タイマーを開始
    uint32_t err_code = app_timer_start(p_context->timer_id,
                                        APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), // 100ミリ秒
                                        p_context);
    APP_ERROR_CHECK(err_code);
    
    NRF_LOG_PRINTF_DEBUG("sensor_manager: startSampling.\n");
}

void sensorManagerStopSampling(senstick_sensor_manager_t *p_context)
{
    if( ! p_context->is_sampling) {
        return;
    }
    
    app_timer_stop(p_context->timer_id);
    p_context->is_sampling = false;
    
    NRF_LOG_PRINTF_DEBUG("sensor_manager: stopSampling.\n");    
}

uint8_t getBatteryLevel(senstick_sensor_manager_t *p_context)
{
    int32_t voltage;
    int level;
    
    // 電圧を取得, 入力スケール 1/3, BGR 1.2Vがリファレンス、10ビット精度なので、2^10(1024)が3.6V相当。
    voltage = nrf_adc_convert_single(ADC_INPUT_SUPPLY_MONITORING);
    
    // 4.2V - 3.2V 電圧範囲で、残量は電圧にリニアで換算
    // 入力電圧は、input-470k/220k-gnd で抵抗分圧されている。
    voltage *= 3; // 抵抗分圧分、元に戻す. 3.14だけど整数で。
//    int level = (voltage - (int32_t)(3.2 / 3.6 * 1024)) / (int32_t) ((4.0 - 3.2)/3.6  * 1024);
    level = (voltage - 910) * 100 / 228;
    // 値域を100-0にする
    level = MIN(level, 100);
    level = MAX(level, 0);
    
    return (uint8_t)level;
}
