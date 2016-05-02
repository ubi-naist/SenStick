#ifndef app_gap_h
#define app_gap_h

#include <ble.h>

void init_app_gap(void);
void app_gap_on_ble_event(ble_evt_t * p_ble_evt);

#endif /* app_gap_h */
