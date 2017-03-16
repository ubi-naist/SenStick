#ifndef gpio_button_monitoring_h
#define gpio_button_monitoring_h

typedef enum {
    BUTTON_RELEASED                = 0,
    BUTTON_PUSH                    = 1,
    BUTTON_PUSH_RELEASED           = 2,
    BUTTON_LONG_PUSH               = 3,
    BUTTON_LONG_PUSH_RELEASED      = 4,
    BUTTON_VERY_LONG_PUSH          = 5,
    BUTTON_VERY_LONG_PUSH_RELEASED = 6,
} ButtonStatus_t;

void initButtonMonitoring(void);

//void enableAwakeByButton(void);

#endif /* gpio_button_monitoring_h */
