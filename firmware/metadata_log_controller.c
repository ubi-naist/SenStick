#include <string.h>

#include <nrf.h>
#include <app_util.h>
#include <ble_date_time.h>
#include <nordic_common.h>

#include "metadata_log_controller.h"

#include "spi_slave_mx25_flash_memory.h"
#include "senstick_flash_address_definition.h"

#include <nrf_assert.h>
#include <nrf_log.h>

#include "senstick_device_definition.h"
#include "senstick_log_definition.h"

#include "senstick_data_model.h"

// 領域フォーマット済を示すint32のマジックワード, ファームウェアのリビジョンで変化する。
#define MAGIC_WORD (0xab5a ^ FIRMWARE_REVISION)

typedef struct {
    uint8_t is_closed_value; // 0x00 closed, 0xff is not closed
    uint8_t log_id;
    ble_date_time_t date;
    char text[21]; // マジックワード GATTの最大長+1バイト。
} meta_log_content_t;

// メタデータのログを書き込みます。
static uint32_t getTargetAddress(uint8_t logid)
{
    return METADATA_STORAGE_START_ADDRESS + sizeof(uint32_t) + logid * sizeof(meta_log_content_t);
}

static void metaDataLogRead(uint8_t logid, meta_log_content_t *p_content)
{
    const uint32_t target_address = getTargetAddress(logid);
    readFlash(target_address, (uint8_t *)p_content, sizeof(meta_log_content_t));
//    NRF_LOG_PRINTF_DEBUG("metaDataLogRead:hours:%d minutes:%d\n", p_content->date.hours,p_content->date.minutes);
}

static void metaDataLogWriteContext(uint8_t logid, meta_log_content_t *p_content)
{
    meta_log_content_t content;
    const uint32_t target_address = getTargetAddress(logid);
    
    // 書き込みされていないか確認
    readFlash(target_address,(uint8_t *) &content, sizeof(meta_log_content_t));
    ASSERT(content.log_id == 0xff);
    
    // 書き込み
    writeFlash(target_address, (uint8_t *)p_content, sizeof(meta_log_content_t));
//    NRF_LOG_PRINTF_DEBUG("metaDataLogWriteContext:hours:%d minutes:%d\n", p_content->date.hours,p_content->date.minutes);
}

static void metaDataLogWrite(bool is_closed, uint8_t logid, ble_date_time_t *p_date, char *text)
{
    meta_log_content_t content;
    
    memset(&content, 0, sizeof(meta_log_content_t));
    content.is_closed_value = is_closed ? 0x00 : 0xff; // フラッシュはクリアで0xff、書き込みで0。ただし0->1の書き込みはできない。したがってboolをここで変換する。
    content.log_id          = logid;
    memcpy(&(content.date), p_date, sizeof(ble_date_time_t));
    strncpy(content.text, text, sizeof(content.text));
//    NRF_LOG_PRINTF_DEBUG("metaDataLogWrite: sizeof(content.text) %d\n", sizeof(content.text));
    metaDataLogWriteContext(logid, &content);
}

static void closeLog(uint8_t logid)
{
    // 読み込み
    meta_log_content_t content;
    metaDataLogRead(logid, &content);

    content.is_closed_value = 0x00;

    // フラグだけ変えて上書き
    const uint32_t target_address = getTargetAddress(logid);
    writeFlash(target_address, (uint8_t *)&content, sizeof(meta_log_content_t));
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
void metaDataLogGetLogCount(uint8_t *p_count, bool *p_is_header_full)
{
    meta_log_content_t content;
    uint8_t count = 0;
    bool is_header_full = false;
    
    for(uint8_t i=0; i < MAX_NUM_OF_LOG; i++) {
        metaDataLogRead(i, &content);
        // log_id が0xff(フラッシュが初期化されている)ならば、処理終了
        if(content.log_id == 0xff) {
            break;
        }
        // フラグが閉じていないならば、
        if(content.is_closed_value != 0x00) {
            is_header_full = true;
            break;
        }
        count++;
    }
    
    if( count == MAX_NUM_OF_LOG) {
        is_header_full = true;
    }
    
    *p_count = count;
    *p_is_header_full = is_header_full;
}

// ターゲットIDの時刻を返します。もしもターゲットIDが存在しなければ、なにもしません。
void metaDataLogReadDateTime(uint8_t logid, ble_date_time_t *p_date)
{
    memset(p_date, 0, sizeof(ble_date_time_t));
    
    if(logid > senstick_getCurrentLogCount()) {
        return;
    }
    
    meta_log_content_t content;
    metaDataLogRead(logid, &content);
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
    metaDataLogRead(logid, &content);
    strncpy(text, content.text, length);
    return MIN(length, strlen(content.text));
}

void metaDatalog_observeControlCommand(senstick_control_command_t old_command, senstick_control_command_t new_command, uint8_t new_log_id)
{
    ble_date_time_t datetime;
    char txt[21];

    // ログ終了時にclosedフラグを落とす
    if(old_command == sensorShouldWork) {
        closeLog(new_log_id -1); // log id は+1されているので、閉じる対象ログIDは -1 したもの。
        // 記録終了時にログヘッダの最大値を確認する, disk full
        if( new_log_id >= MAX_NUM_OF_LOG ) {
            senstick_setDiskFull(true);
        }
    }
    
    switch(new_command) {
        case sensorShouldSleep:
            break;
        case sensorShouldWork:
            senstick_getCurrentDateTime(&datetime);
            senstick_getCurrentLogAbstractText(txt, sizeof(txt));
            metaDataLogWrite(false, new_log_id, &datetime, txt);
//            NRF_LOG_PRINTF_DEBUG("metaDatalog_observeControlCommand:hour:%d min:%d\n", datetime.hours, datetime.minutes);
            break;
        case formattingStorage:
            metaLogFormatStorage();
            break;
        case shouldDeviceSleep:
            break;
        case enterDFUmode:
            break;
        default:
            break;
    }
}
