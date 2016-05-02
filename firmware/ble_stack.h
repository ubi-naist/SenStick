#ifndef ble_stack_h
#define ble_stack_h

#include <ble.h>
#include <softdevice_handler.h>

void init_ble_stack(sys_evt_handler_t systemHandler, ble_evt_handler_t bleHandler);

#endif /* ble_stack_h */
