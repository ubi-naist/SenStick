#include <nrf_assert.h>
#include <nordic_common.h>

#include "log_controller.h"
#include "spi_slave_mx25_flash_memory.h"

#define SECTOR_SIZE MX25L25635F_SECTOR_SIZE

static void readHeader(uint32_t start_address, uint8_t logid, log_header_t *p_header)
{
    readFlash(start_address + sizeof(log_header_t) * logid, (uint8_t *)p_header, sizeof(log_header_t));
}

/**
 * Public methods
 */
void formatLog(const flash_address_info_t *p_address_info)
{
    // 先頭2セクタ(ヘッダ+データの最初のセクタ)をフォーマット
    formatFlash(p_address_info->startAddress, SECTOR_SIZE * 2);
}

void createLog(log_context_t *p_context, uint8_t logID, samplingDurationType samplingDuration, uint16_t measurementRange, const flash_address_info_t *p_address_info)
{
    memset(p_context, 0, sizeof(log_context_t));

    // 書き込み対象のヘッダを読み込み、まだ書き込まれていないこと(logID == 0xff)を確認します。
    log_header_t header;
    readHeader(p_address_info->startAddress, logID, &header);
    ASSERT(header.logID == 0xff);
    
    // コンテキストを設定します。
    p_context->headerStartAddress       = p_address_info->startAddress;
    p_context->header.logID             = logID;
    p_context->header.samplingDuration  = samplingDuration;
    p_context->header.measurementRange  = measurementRange;
    
    // もしもlogIDが > 0 ならば、前のヘッダ情報からスタートアドレスとサイズを設定します
ASSERT(SECTOR_SIZE / sizeof(log_header_t) > 255 ); // ヘッダに1セクタを割り当てているので、ログID0-255のヘッダを収められる容量があることを仮定。
    if(logID == 0) {
        p_context->header.startAddress = p_address_info->startAddress + SECTOR_SIZE;
    } else {
        log_header_t previous_header;
        readHeader(p_address_info->startAddress, logID -1 , &previous_header);
ASSERT(previous_header.logID == (logID -1));
        p_context->header.startAddress = previous_header.startAddress + previous_header.size;
    }

    p_context->header.size = (p_address_info->startAddress + p_address_info->size) - p_context->header.startAddress;
    p_context->canWrite    = true;
}

// ログを開きます。すでに書き込まれたlogIDの場合は、readonlyで開かれます。
void openLog(log_context_t *p_context, uint8_t logID, const flash_address_info_t *p_address_info)
{
    memset(p_context, 0, sizeof(log_context_t));

    // 書き込み対象のヘッダを読み込み、まだ書き込まれていないこと(logID == 0xff)を確認します。
    log_header_t header;
    readHeader(p_address_info->startAddress, logID, &header);
    ASSERT(header.logID == logID);
    
    p_context->headerStartAddress = p_address_info->startAddress;
    p_context->header             = header;
}

// ログを閉じます。
void closeLog(log_context_t *p_context)
{
    if( p_context == NULL || ! p_context->canWrite) {
        return;
    }
    
    // ヘッダを書き込みます
    p_context->header.size = p_context->writePosition;
    writeFlash(p_context->headerStartAddress + sizeof(log_header_t) * p_context->header.logID, (uint8_t *)&(p_context->header), sizeof(log_header_t));
}

void reOpenLog(log_context_t *p_dst_context, log_context_t *p_src_context)
{
    memcpy(p_dst_context, p_src_context, sizeof(log_context_t));
    p_dst_context->header.size = p_dst_context->writePosition;
    p_dst_context->canWrite    = false;
}

// 書き込めたサイズを返します。
int writeLog(log_context_t *p_context, uint8_t *p_data, int length)
{
    ASSERT(p_context != NULL);
    ASSERT(p_context->canWrite);

    // 書き込み領域チェック
    if( (p_context->writePosition + length) > p_context->header.size) {
        return 0;
    }
    
    writeFlash(p_context->header.startAddress + p_context->writePosition, p_data, length);
    p_context->writePosition += length;
    return length;
}

// 読み込んだサイズを返します。
int readLog(log_context_t *p_context, uint8_t *p_data, int length)
{
    ASSERT(p_context != NULL);

    // 読みだし可能かどうかの判定。書き込み中か、読み出しだけかで、振る舞いが異なる。
    if(p_context->canWrite) {
        if( (p_context->readPosition + length) > p_context->writePosition) {
            return 0;
        }
    } else {
        if( (p_context->readPosition + length) > p_context->header.size) {
            return 0;
        }
    }
    readFlash(p_context->header.startAddress + p_context->readPosition, p_data, length);
    p_context->readPosition += length;
    return length;
}

// 読み出し位置をシークします。シーク位置を返します。書き込み位置はseekされません。
int seekLog(log_context_t *p_context, int position)
{
    ASSERT(p_context != NULL);
    
    p_context->readPosition = position;
    return position;
}
