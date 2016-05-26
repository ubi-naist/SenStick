#ifndef gpio_led_driver_h
#define gpio_led_driver_h

#include <ble.h>
#include "senstick_types.h"
#include "gpio_button_monitoring.h"

void initLEDDriver(void);

// observer
void ledDriver_observeControlCommand(senstick_control_command_t command);
void ledDriver_observeButtonStatus(ButtonStatus_t status);

// BLEイベントを受け取ります。
void ledDriver_handleBLEEvent(ble_evt_t * p_ble_evt);
#endif /* gpio_led_driver_h */
