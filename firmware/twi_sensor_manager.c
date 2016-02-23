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
#include "twi_sensor_manager.h"
#include "senstick_io_definitions.h"
#include "senstick_data_models.h"

/**
 * Private メソッド
 */

#define SAMPLING_PERIOD_MS 100

static void init_twi_slaves(sensor_manager_t *p_context)
{
    /*
     if(p_context->sensor_setting.is_gyroscope_sampling != 0 || p_context->sensor_setting.is_accelerometer_sampling || p_context->sensor_setting.is_magnetrometer_sampling ) {
     initNineAxesSensor(&(p_context->nine_axes_sensor_context), &(p_context->twi));
     }
     if(p_context->sensor_setting.is_humidity_sampling || p_context->sensor_setting.is_temperature_sampling ) {
     initHumiditySensor(&(p_context->humidity_sensor_context),  &(p_context->twi));
     }
     if(p_context->sensor_setting.is_barometer_sampling ) {
     initPressureSensor(&(p_context->pressure_sensor_context),  &(p_context->twi));
     }
     if(p_context->sensor_setting.is_illumination_sampling ) {
     initBrightnessSensor(&(p_context->brightness_sensor_context),  &(p_context->twi));
     }
     if(p_context->sensor_setting.is_uv_sampling ) {
     initUVSensor(&(p_context->uv_sensor_context),  &(p_context->twi));
     }*/
    initNineAxesSensor(&(p_context->nine_axes_sensor_context), &(p_context->twi));
    initHumiditySensor(&(p_context->humidity_sensor_context),  &(p_context->twi));
    initPressureSensor(&(p_context->pressure_sensor_context),  &(p_context->twi));
    initBrightnessSensor(&(p_context->brightness_sensor_context),  &(p_context->twi));
    initUVSensor(&(p_context->uv_sensor_context),  &(p_context->twi));
}

static void sensor_timer_handler(void *p_arg)
{
    sensor_manager_t *p_context = (sensor_manager_t *)p_arg;
    SensorData_t sensor_data;
    
    // カウンターをカウントアップ
    for(int i = 0; i < NUM_OF_SENSOR_DEVICES; i++) {
        (p_context->remaining_counter[i]) += SAMPLING_PERIOD_MS;
    }

    // センサーごとに、タイマーを増加、時間になっていたら、通知
    if(p_context->remaining_counter[MotionSensor] >= p_context->sensor_setting.motionSensorSamplingPeriod) {
        p_context->remaining_counter[MotionSensor] = 0;
        if(p_context->sensor_setting.is_gyroscope_sampling != 0 || p_context->sensor_setting.is_accelerometer_sampling || p_context->sensor_setting.is_magnetrometer_sampling ) {
            getNineAxesData(&(p_context->nine_axes_sensor_context), &(sensor_data.data.motionSensor));
            sensor_data.type = MotionSensor;
            (p_context->sampling_callback_handler)(&sensor_data);
        }
    }
    
    if(p_context->remaining_counter[BrightnessSensor] >= p_context->sensor_setting.brightnessSamplingPeriod) {
        p_context->remaining_counter[BrightnessSensor] = 0;
        if(p_context->sensor_setting.is_illumination_sampling) {
            getBrightnessData(&(p_context->brightness_sensor_context), &(sensor_data.data.brightness));
            sensor_data.type = BrightnessSensor;
            (p_context->sampling_callback_handler)(&sensor_data);
        }
    }
    
    if(p_context->remaining_counter[UltraVioletSensor] >= p_context->sensor_setting.ultraVioletSamplingPeriod) {
        p_context->remaining_counter[UltraVioletSensor] = 0;
        if(p_context->sensor_setting.is_uv_sampling) {
            getUVSensorData(&(p_context->uv_sensor_context), &(sensor_data.data.ultraViolet));
            sensor_data.type = UltraVioletSensor;
            (p_context->sampling_callback_handler)(&sensor_data);
        }
    }
    
    if(p_context->remaining_counter[HumidityAndTemperatureSensor] >= p_context->sensor_setting.humidityAndTemperatureSamplingPeriod) {
        p_context->remaining_counter[HumidityAndTemperatureSensor] = 0;
        if(p_context->sensor_setting.is_humidity_sampling) {
            getHumidityData(&(p_context->humidity_sensor_context), &(sensor_data.data.humidityAndTemperature.humidity));
            getTemperatureData(&(p_context->humidity_sensor_context), &(sensor_data.data.humidityAndTemperature.temperature));
            sensor_data.type = HumidityAndTemperatureSensor;
            (p_context->sampling_callback_handler)(&sensor_data);
        }
    }
    
    if(p_context->remaining_counter[AirPressureSensor] >= p_context->sensor_setting.airPressureSamplingPeriod) {
        p_context->remaining_counter[AirPressureSensor] = 0;
        if(p_context->sensor_setting.is_barometer_sampling) {
            getPressureData(&(p_context->pressure_sensor_context), &(sensor_data.data.airPressure));
            sensor_data.type = AirPressureSensor;
            (p_context->sampling_callback_handler)(&sensor_data);
        }
    }
}

/**
 * Public関数
 */
void initSensorManager(sensor_manager_t *p_context, gpio_manager_t *p_gpio_manager_context, const sensorSetting_t *p_setting, sampling_callback_handler_t samplingCallback)
{
    ret_code_t err_code;
    
    // コンテキストの初期値設定
    memset(p_context, 0, sizeof(sensor_manager_t));
    
    // 引数の保存
    p_context->p_gpio_manager_context       = p_gpio_manager_context;
    p_context->sensor_setting                = *p_setting;
    p_context->sampling_callback_handler    = samplingCallback;
    
    // SDK10では、SDK8と異なり、タイマー管理のデータ領域の型とID型が分離されたため、ここでID型(データ領域へのポインタ)にポインタを代入する。
    // 構造体のメンバ変数定義では、APP_TIMER_DEFマクロが使えないため、手動でこのコードを書く必要がある。
    p_context->timer_id = &(p_context->timer_id_data);
    err_code = app_timer_create(&(p_context->timer_id), APP_TIMER_MODE_REPEATED, sensor_timer_handler);
    APP_ERROR_CHECK(err_code);
    
    // TWIインタフェース TWI1を使用。
    p_context->twi.p_reg        = NRF_TWI1;
    p_context->twi.irq          = TWI1_IRQ;
    p_context->twi.instance_id  = TWI1_INSTANCE_INDEX;
    
    err_code = nrf_drv_twi_init(&(p_context->twi), NULL, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&(p_context->twi));
    
    // TWIの電源を上げる
    setTWIPowerSupply(p_context->p_gpio_manager_context,true);
    nrf_delay_ms(100);
    
    // TWIの電源がONになって100ミリ秒経過、センサーを初期化
    init_twi_slaves(p_context);
    
    // 設定を反映
    setSensorManagerSetting(p_context, p_setting);
}

void setSensorManagerSetting(sensor_manager_t *p_context, const sensorSetting_t *p_setting)
{
    // 設定情報をコピー
    p_context->sensor_setting = *p_setting;
    
    // レンジ設定に反映
    setNineAxesSensorAccelerationRange(&(p_context->nine_axes_sensor_context), p_setting->accelerationRange);
    setNineAxesSensorRotationRange(&(p_context->nine_axes_sensor_context), p_setting->rotationRange);
    
    // カウンタを初期化
    memset(p_context->remaining_counter, 0, sizeof(p_context->remaining_counter));
}

// サンプリング中?
bool isSensorManagerSampling(sensor_manager_t *p_context)
{
    return p_context->is_sampling;
}

void sensorManagerStartSampling(sensor_manager_t *p_context)
{
    ret_code_t err_code;
    
    // フラグチェック
    if( p_context->is_sampling) {
        return;
    }
    
    // カウンタを初期化
    memset(p_context->remaining_counter, 0, sizeof(p_context->remaining_counter));
    // サンプリングカウンタを開始
    err_code = app_timer_start(p_context->timer_id,
                               APP_TIMER_TICKS(SAMPLING_PERIOD_MS, APP_TIMER_PRESCALER), 
                               p_context);
    APP_ERROR_CHECK(err_code);
    
    p_context->is_sampling = true;
}

void sensorManagerStopSampling(sensor_manager_t *p_context)
{
    // フラグチェック
    if( ! p_context->is_sampling ) {
        return;
    }
    
    // タイマーを停止
    app_timer_stop(p_context->timer_id);
    p_context->is_sampling = false;
}
