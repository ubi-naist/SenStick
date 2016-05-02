#ifndef gpio_button_monitoring_h
#define gpio_button_monitoring_h

typedef enum {
    BUTTON_RELEASED         = 0,
    BUTTON_PUSHED           = 1,
    BUTTON_LONG_PUSH        = 2,
    BUTTON_VERY_LONG_PUSH   = 3,
} ButtonStatus_t;

typedef void (*button_callback_handler_t) (ButtonStatus_t status, ButtonStatus_t previous_status);

void buttonMonitoringInit(button_callback_handler_t handler);

#endif /* gpio_button_monitoring_h */
