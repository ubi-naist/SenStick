#ifndef gpio_led_driver_h
#define gpio_led_driver_h

#include <ble.h>
#include "senstick_types.h"

void initLEDDriver(void);

// observer
void ledDriver_observeControlCommand(senstick_control_command_t command);
// BLEイベントを受け取ります。
void ledDriver_handleBLEEvent(ble_evt_t * p_ble_evt);
#endif /* gpio_led_driver_h */
