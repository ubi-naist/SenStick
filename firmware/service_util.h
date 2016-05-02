#ifndef service_util_h
#define service_util_h

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ble.h>
#include <ble_srv_common.h>
#include <nrf_assert.h>
#include <sdk_errors.h>
#include <app_error.h>

#define GATT_MAX_DATA_LENGTH 20

void setCharacteristicsValue(uint16_t connection_handle, uint16_t handle, uint8_t *p_data, uint16_t length);
bool is_indication_enabled(uint16_t connection_handle, uint16_t cccd_handle);
void setCharacteristicsValueAndNotify(uint16_t connection_handle, uint16_t value_handle, uint16_t cccd_handle, uint8_t *p_data, uint16_t length);

// ハンドラの値を読み出します。有効なデータ長を返します。CCCDの値取得にもつかえます。
uint16_t getCharacteristicsValue(uint16_t connection_handle, uint16_t att_handle, uint8_t *p_data, uint16_t buffer_length);

ret_code_t notifyToClient(uint16_t connection_handle, uint16_t handle, uint8_t *p_data, uint16_t length);

#endif /* service_util_h */
