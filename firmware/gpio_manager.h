#ifndef gpio_manager_h
#define gpio_manager_h

#include "gpio_button_monitoring.h"
#include "gpio_led_driver.h"

//コールバック
//typedef void (*sampling_callback_handler_t) (SensorDeviceType_t sensorType, const SensorData_t *p_sensorData);

// コンテキストの構造体宣言
typedef struct gpio_manager_s {
    LEDDriver_t led_driver_context;
//    sampling_callback_handler_t sampling_callback_handler;
} gpio_manager_t;

// 初期化関数。最初に必ずこの関数を呼び出すこと。
void initGPIOManager(gpio_manager_t *p_context, button_callback_handler_t button_handler); //,  const sensorSetting_t *p_setting, sampling_callback_handler_t samplingCallback);

// TWIのパワー供給
void setTWIPowerSupply(gpio_manager_t *p_context, bool available);

// バッテリーレベルの取得
// バッテリーレベルを、0 - 100 %の整数で返します。
uint8_t getBatteryLevel(gpio_manager_t *p_context);

// LEDの点滅モードを設定。
// 点滅周期(ミリ秒単位)、点滅回数, 停止するときは、いずれかに0を指定
void setLEDBlink(gpio_manager_t *p_context, uint16_t period, uint16_t blink, bool repeat);

#endif /* gpio_manager_h */
