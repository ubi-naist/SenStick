#ifndef senstick_types_h
#define senstick_types_h

#include <stdint.h>
#include <stdbool.h>

#include <nrf.h>
#include <app_util.h>
#include <ble_date_time.h>

// Senstickのコントロールコマンド
typedef enum {
    sensorShouldSleep = 0x00,
    sensorShouldWork  = 0x01,
    formattingStorage = 0x10,
    enterDeepSleep    = 0x20,
    enterDFUmode      = 0x40
} senstick_control_command_t;

// フラッシュのアドレス/領域情報
typedef struct {
    uint32_t startAddress;
    uint32_t size;
} flash_address_info_t;

#endif /* senstick_types_h */
