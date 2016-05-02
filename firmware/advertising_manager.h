#ifndef advertising_h
#define advertising_h

#include <ble.h>
#include <ble_advertising.h>

void init_advertising_manager(ble_uuid_t *p_uuid);
void startAdvertising(void);

#endif /* advertising_h */
