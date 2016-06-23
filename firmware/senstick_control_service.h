#ifndef senstick_control_service_h
#define senstick_control_service_h

#include "senstick_types.h"
#include "service_util.h"

#define CONTROL_SERVICE_UUID            0x2000
#define CONTROL_POINT_CHAR_UUID         0x7000
#define AVAILABLE_LOG_COUNT_CHAR_UUID   0x7001
#define STORAGE_STATUS_CHAR_UUID        0x7002
#define CONTROL_RTC_CHAR_UUID           0x7003
#define CONTROL_ABSTRACT_TEXT_CHAR_UUID 0x7004
#define DEVICE_NAME_CHAR_UUID           0x7005

// 初期化します
uint32_t initSenstickControlService(uint8_t uuid_type);

// observer
void senstickControlService_handleBLEEvent(ble_evt_t * p_ble_evt);
void senstickControlService_observeControlCommand(senstick_control_command_t command);
void senstickControlService_observeCurrentLogCount(uint8_t count);
void senstickControlService_observeDiskFull(bool flag);
#endif /* senstick_control_service_h */
