#ifndef senstick_control_service_h
#define senstick_control_service_h

#include "senstick_types.h"
#include "service_util.h"

#define CONTROL_SERVICE_UUID            0x2000
#define CONTROL_POINT_CHAR_UUID         0x7000
#define AVAILABLE_LOG_COUNT_CHAR_UUID   0x7001
#define CONTROL_RTC_CHAR_UUID           0x7002
#define CONTROL_ABSTRACT_TEXT_CHAR_UUID 0x7003

// 初期化します
uint32_t initSenstickControlService(uint8_t uuid_type);

// observer
void senstickControlService_handleBLEEvent(ble_evt_t * p_ble_evt);
void senstickControlService_observeControlCommand(senstick_control_command_t command);
void senstickControlService_observeCurrentLogCount(uint8_t count);

#endif /* senstick_control_service_h */
