#ifndef gpio_manager_h
#define gpio_manager_h

//コールバック
//typedef void (*sampling_callback_handler_t) (SensorDeviceType_t sensorType, const SensorData_t *p_sensorData);

// コンテキストの構造体宣言
typedef struct gpio_manager_s {
    int dummy;
    // サンプリング時のコールバック
//    sampling_callback_handler_t sampling_callback_handler;
} gpio_manager_t;

void initGPIOManager(gpio_manager_t *p_context); //,  const sensorSetting_t *p_setting, sampling_callback_handler_t samplingCallback);

void setTWIPowerSupply(gpio_manager_t *p_context, bool available);

// バッテリーレベルの取得
// バッテリーレベルを、0 - 100 %の整数で返します。
uint8_t getBatteryLevel(gpio_manager_t *p_context);

#endif /* gpio_manager_h */
