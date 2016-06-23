#ifndef app_gap_h
#define app_gap_h

#include <ble.h>

void init_app_gap(void);
void app_gap_on_ble_event(ble_evt_t * p_ble_evt);

// GAPのデバイス名を設定します。p_device_nameは0終端指定なくともよいUTF-8の文字列。lengthは文字列の長さ。
void app_gap_set_device_name(uint8_t *p_device_name, uint16_t length);
// GAPのデバイス名を取得します。
uint16_t app_gap_get_device_name(uint8_t *p_device_name, uint16_t length);
#endif /* app_gap_h */
