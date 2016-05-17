#ifndef metadata_log_controller_h
#define metadata_log_controller_h

#include <stdint.h>
#include <nrf.h>
#include <app_util.h>
#include <ble_date_time.h>

#include "senstick_types.h"

void initMetaDataLogController(void);

// メタログ領域をフォーマットします。
void metaLogFormatStorage(void);

// フォーマットされているかを取得します。
bool isMetaLogFormatted(void);

// 有効なログの数を取得します。0はログがないことを示します。
uint8_t metaDataLogGetLogCount(void);

// ターゲットIDの時刻を返します。もしもターゲットIDが存在しなければ、なにもしません。
void metaDataLogReadDateTime(uint8_t logid, ble_date_time_t *p_date);

// ターゲットIDのアブストラクトテキストを取得します。もしもターゲットIDが存在しなければ、"\0"を返します。
// 有効なバイト数を返します。文字列がなくとも終端文字列があるため1バイトです。
uint8_t metaDataLogReadAbstractText(uint8_t logid, char *text, uint8_t length);

void metaDatalog_observeControlCommand(senstick_control_command_t command, uint8_t new_log_id);
#endif /* metadata_log_controller_h */
