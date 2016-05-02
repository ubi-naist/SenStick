#include <nrf.h>
#include <app_util.h>
#include <ble_date_time.h>

#include "metadata_log_controller.h"

/**
 * Public methods
 */
void initMetaDataLogController(void)
{}

// 有効なログの数を取得します。0はログがないことを示します。
uint8_t metaDataLogGetLogCount(void)
{
    return 0;
}

// メタデータのログを書き込みます。
void metaDataLogWrite(ble_date_time_t *p_date, uint8_t *p_buffer, uint8_t length)
{}

// ターゲットIDの時刻を返します。もしもターゲットIDが存在しなければ、なにもしません。
void metaDataLogReadDateTime(uint8_t logid, ble_date_time_t *p_date)
{}

// ターゲットIDのアブストラクトテキストを取得します。読みだしたテキストの長さを返します。もしもターゲットIDが存在しなければ、"\0"を返します。
uint8_t metaDataLogReadAbstractText(uint8_t logid, uint8_t *p_buffer, uint8_t buffer_length)
{
    return 0;
}
