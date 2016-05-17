#include <string.h>

#include <nrf.h>
#include <app_util.h>
#include <ble_date_time.h>
#include <nordic_common.h>

#include "metadata_log_controller.h"

#include "spi_slave_mx25_flash_memory.h"
#include "senstick_flash_address_definitions.h"

#include <nrf_assert.h>

#include "senstick_device_definitions.h"

#include "senstick_data_model.h"

// 領域フォーマット済を示すint32のマジックワード, ファームウェアのリビジョンで変化する。
#define MAGIC_WORD (0xab5a ^ FIRMWARE_REVISION)

typedef struct {
    uint8_t log_id;
    ble_date_time_t date;
    char text[21]; // マジックワード GATTの最大長+1バイト。
} meta_log_content_t;

// メタデータのログを書き込みます。
static void metaDataLogWrite(uint8_t log_count, ble_date_time_t *p_date, char *text)
{
    meta_log_content_t content;
    const uint32_t target_address = METADATA_STORAGE_START_ADDRESS + sizeof(uint32_t) + log_count * sizeof(meta_log_content_t);
    
    // 書き込みされていないか確認
    readFlash(target_address,(uint8_t *) &content, sizeof(meta_log_content_t));
    ASSERT(content.log_id == 0xff);
    
    // 書き込み
    memset(&content, 0, sizeof(meta_log_content_t));
    content.log_id = log_count;
    content.date   = *p_date;
    strncpy(content.text, text, sizeof(content.text));
    // 書き込み
    writeFlash(target_address,(uint8_t *) &content, sizeof(meta_log_content_t));
}

/**
 * Public methods
 */
void initMetaDataLogController(void)
{
}

void metaLogFormatStorage(void)
{
    formatFlash(METADATA_STORAGE_START_ADDRESS, METADATA_STORAGE_SIZE);

    // マジックワードを書き込む
    uint32_t format_id = MAGIC_WORD;
    writeFlash(METADATA_STORAGE_START_ADDRESS, (uint8_t *)&format_id, sizeof(uint32_t));
}

bool isMetaLogFormatted(void)
{
    uint32_t i = 0;
    readFlash(METADATA_STORAGE_START_ADDRESS, (uint8_t *)&i, sizeof(uint32_t));
    return (i == MAGIC_WORD);
}

// 有効なログの数を取得します。0はログがないことを示します。
uint8_t metaDataLogGetLogCount(void)
{
    meta_log_content_t content;
    uint8_t count = 0;
    
    for(uint32_t address = METADATA_STORAGE_START_ADDRESS + sizeof(int); address < (METADATA_STORAGE_START_ADDRESS + METADATA_STORAGE_SIZE); address += sizeof(meta_log_content_t)) {
        // 読み込み
        readFlash(address,  (uint8_t *)&content, sizeof(meta_log_content_t));
        // log_id が0xff(フラッシュが初期化されている)ならば、処理終了
        if(content.log_id == 0xff) {
            break;
        }
        // ログのカウントアップ
        count++;
    }
    return count;
}

// ターゲットIDの時刻を返します。もしもターゲットIDが存在しなければ、なにもしません。
void metaDataLogReadDateTime(uint8_t logid, ble_date_time_t *p_date)
{
    if(logid > senstick_getCurrentLogCount()) {
        return;
    }
    
    meta_log_content_t content;
    readFlash(METADATA_STORAGE_START_ADDRESS + sizeof(uint32_t) + logid * sizeof(meta_log_content_t), (uint8_t *)&content, sizeof(meta_log_content_t));
    memcpy(p_date, &(content.date), sizeof(ble_date_time_t));
}

// ターゲットIDのアブストラクトテキストを取得します。読みだしたテキストのバイト列としての長さを返します。
// もしもターゲットIDが存在しなければ、"\0"を返します。
uint8_t metaDataLogReadAbstractText(uint8_t logid, char *text, uint8_t length)
{
    if(logid > senstick_getCurrentLogCount()) {
        text[0] = '\0';
        return 1;
    }
    
    meta_log_content_t content;
    readFlash(METADATA_STORAGE_START_ADDRESS + sizeof(uint32_t) + logid * sizeof(meta_log_content_t), (uint8_t*)&content, sizeof(meta_log_content_t));
    strncpy(text, content.text, length);
    return MIN(length, strlen(content.text));
}

void metaDatalog_observeControlCommand(senstick_control_command_t command, uint8_t new_log_id)
{
    ble_date_time_t datetime;
    char txt[21];
    
    switch(command) {
        case sensorShouldSleep: break;
        case sensorShouldWork:
            senstick_getCurrentDateTime(&datetime);
            senstick_getCurrentLogAbstractText(txt, sizeof(txt));
            metaDataLogWrite(new_log_id, &datetime, txt);
            break;
        case formattingStorage:
            metaLogFormatStorage();
            break;
        case enterDeepSleep: break;
        case enterDFUmode: break;
        default: break;
    }
}
