#ifndef senstick_meta_data_service_h
#define senstick_meta_data_service_h

#include "service_util.h"

#define METADATA_SERVICE_UUID       0x2001
#define TARGET_LOG_ID_CHAR_UUID     0x7010
#define TARGET_DATETIME_CHAR_UUID   0x7011
#define TARGET_ABSTRACT_CHAR_UUID   0x7012

// 初期化します
ret_code_t initSenstickMetaDataService(uint8_t uuid_type);

// BLEイベントを受け取ります。
void senstickMetaDataService_handleBLEEvent(ble_evt_t * p_ble_evt);

#endif /* senstick_meta_data_service_h */
