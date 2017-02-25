//
//  twi_ext_services.h
//  senstick
//
//  Created by AkihiroUehara on 2017/02/24.
//
//

#ifndef twi_ext_services_h
#define twi_ext_services_h

#include "service_util.h"
#include "twi_ext_motor.h"

/**
 * BLEのサービスおよびキャラクタリスティクスのUUID定義。
 */
#define TWI_EXT_MOTOR_SERVICE_UUID             0x2206
#define TWI_EXT_MOTOR_CONTROL_CHAR_UUID        0x7600
#define TWI_EXT_MOTOR_FAULT_STATUS_CHAR_UUID   0x7600

// コンテキスト構造体。
typedef struct twi_ext_services_s {
    uint16_t connection_handle;
    
    bool is_morter_available;
    uint16_t twi_ext_motor_service_handle;
    ble_gatts_char_handles_t twi_ext_motor_control_char_handle;
    ble_gatts_char_handles_t twi_ext_motor_fault_status_char_handle;
    
} twi_ext_services_t;


// 初期化します
ret_code_t initTwiExtService(uint8_t uuid_type);

// BLEイベントを受け取ります。
void twiExtServices_handleBLEEvent(ble_evt_t * p_ble_evt);

#endif /* twi_ext_services_h */
