#ifndef senstick_data_model_h
#define senstick_data_model_h

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <nrf.h>
#include <app_util.h>
#include <ble_date_time.h>

#include "senstick_types.h"

// 初期化
void initSenstickDataModel(void);

// コントロールコマンド
senstick_control_command_t senstick_getControlCommand(void);
void senstick_setControlCommand(senstick_control_command_t command);

// 現在有効なログデータ数, uint8_t
uint8_t senstick_getCurrentLogCount(void);
void senstick_setCurrentLogCount(uint8_t count);

// 現在の時刻
void senstick_getCurrentDateTime(ble_date_time_t *p_datetime);
void senstick_setCurrentDateTime(ble_date_time_t *p_datetime);

// 現在のログテキスト概要
uint8_t senstick_getCurrentLogAbstractText(char *str, uint8_t length);
void senstick_setCurrentLogAbstractText(char *str, uint8_t length);

#endif /* senstick_data_model_h */
