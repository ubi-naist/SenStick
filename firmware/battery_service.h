#ifndef battery_service_h
#define battery_service_h

#include <ble.h>

void init_battery_service(void);
void handle_battery_service_ble_event(ble_evt_t * p_ble_evt);
//void update_battery_service_battery_value(uint8_t battery_level);

uint8_t getBatteryLevel(void);
#endif /* battery_service_h */
