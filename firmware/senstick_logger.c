#include <string.h>
#include <stdlib.h>

#include "nordic_common.h"
#include "app_error.h"

#include "senstick_logger.h"

#define MAGIC_WORD  0xc0ffeede
#define SECTOR_SIZE (4 * 1024)

// 先頭セクターの構造
//  4バイト    32ビットのマジックワード
//  4バイト    有効なバイトサイズ

/**
 * Private methods
 */
// 符号なし32ビット整数をバイト配列に展開します。ビッグエンディアン。
static void uint32ToByteArray(uint8_t *p_dst, uint32_t src)
{
    // アドレスは4バイト。MSB-first
    p_dst[0] = (uint8_t)(0x0ff & (src >> 24));
    p_dst[1] = (uint8_t)(0x0ff & (src >> 16));
    p_dst[2] = (uint8_t)(0x0ff & (src >>  8));
    p_dst[3] = (uint8_t)(0x0ff & (src >>  0));
}

static uint32_t byteArrayToUint32(uint8_t *p_dst)
{
    return
      ((uint32_t)p_dst[0] << 24)
    | ((uint32_t)p_dst[1] << 16)
    | ((uint32_t)p_dst[2] <<  8)
    | ((uint32_t)p_dst[3] <<  0);
}

//マジックワードがあるかどうかを読み出します
static bool isMagicWord(senstick_logger_t *p_context)
{
    uint8_t buffer[4];

    readFlash(p_context->p_memory, 0, buffer, 4);
    uint32_t value = byteArrayToUint32(buffer);
    
    return (value == MAGIC_WORD);
}

//フラッシュに、サイズ情報を書き込みます
static void writeSize(senstick_logger_t *p_context)
{
    // 先頭セクタを消去
    eraseSector(p_context->p_memory, 0);

    // 書き込みデータを用意
    uint8_t buffer[8];
    uint32ToByteArray(&(buffer[0]), MAGIC_WORD);
    uint32ToByteArray(&(buffer[4]), p_context->size);

    // マジックワードとサイズを、書き込み
    writeFlash(p_context->p_memory, 0, buffer, 8);
}

//フラッシュから、サイズ情報を読み込みます
static uint32_t readSize(senstick_logger_t *p_context)
{
    // マジックワードがなければ、無効です
    if( ! isMagicWord(p_context)) {
        return 0;
    }
    
    uint8_t buffer[4];

    readFlash(p_context->p_memory, 0x04, buffer, 4);
    uint32_t size = byteArrayToUint32(buffer);
    
    return size;
}

/**
 * Public methods
 */
void loggerOpen(senstick_logger_t *p_context, flash_memory_context_t *p_memory, bool is_writing_mode)
{
    memset(p_context, 0, sizeof(senstick_logger_t));

    p_context->p_memory = p_memory;
    p_context->is_writing_mode = is_writing_mode;
    
    // もしも書き込みモードならば先頭セクターを消去する
    if(is_writing_mode) {
        eraseSector(p_context->p_memory, 0);
    } else {
        // 読み込みモードならば、サイズを読み込む
        p_context->size = readSize(p_context);
    }
}

// ロガーを閉じます。ロギングを完了するために、このメソッドを最後に必ず呼び出すこと。
void loggerClose(senstick_logger_t *p_context)
{
    if(p_context->is_writing_mode) {
        writeSize(p_context);
    }
}
// p_bufferからsizeバイトを書き込みます。書き込めたサイズを返します。
uint32_t logger_write(senstick_logger_t *p_context, uint8_t *p_buffer, uint32_t size)
{
    // 書き込みモードか?
    if( ! p_context->is_writing_mode || size == 0) {
        return 0;
    }
    
    uint32_t index = 0;
    do {
        // 書き込み開始位置が4kBセクターであれば、書き込む前に消去する。
        // 256バイト単位で書き込むから、かならずセクター先頭に書き込み位置が来る。
        if((p_context->write_position % SECTOR_SIZE) == 0) {
            // 先頭セクターはファイル情報に使うので、1セクター分アドレスをずらす
            eraseSector(p_context->p_memory, p_context->write_position +SECTOR_SIZE);
        }
        
        // 256バイト単位で書き込むので、書き込み可能サイズを求める
        int page_size      = 256 - (p_context->write_position % 256);
        uint8_t write_size = (uint8_t) MIN(255, MIN(page_size, size));
        // 先頭セクターはファイル情報に使うので、1セクター分アドレスをずらす
        writeFlash(p_context->p_memory, p_context->write_position +SECTOR_SIZE, &(p_buffer[index]), write_size);
        // 書き込み位置などを変更する
        index += write_size;
        p_context->write_position += write_size;
        p_context->size           += write_size;
    } while (index < size);
    
    return index;
}

// p_bufferからsizeバイトを読み込みます。読み込んだサイズを返します。
uint32_t loggerRead(senstick_logger_t *p_context, uint8_t *p_buffer, uint32_t size)
{
    if(size == 0) {
        return 0;
    }
    
    uint32_t index = 0;
    do {
        // 読みだし可能サイズ
        int remainingSize = (p_context->size - p_context->read_position);
        // 256バイト単位で読みだすので、読み出し可能サイズを求める
        int page_size      = 256 - (p_context->read_position % 256);
        uint8_t read_size  = (uint8_t) MIN(255, MIN( MIN(size, page_size), remainingSize));
        if(readSize <= 0) {
            return index;
        }
        // 先頭セクターはファイル情報に使うので、1セクター分アドレスをずらす
        readFlash(p_context->p_memory, p_context->read_position +SECTOR_SIZE, &(p_buffer[index]), (uint8_t)read_size);
        // 位置などを変更する
        index += read_size;
        p_context->read_position += read_size;
    } while (index < size);
    
    return index;
}

// 先頭からの位置を返します。
uint32_t loggerSize(senstick_logger_t *p_context)
{
    return p_context->size;
}

// 先頭からpositonバイトに移動します。まだ書き込みをしていない領域を超えてpositionを指定した場合、その飛び越えた領域の値は不定値になります。
uint32_t loggerSeek(senstick_logger_t *p_context, uint32_t position)
{
    uint32_t pos = MIN(p_context->size, position);
    
    p_context->write_position = pos;
    p_context->read_position  = pos;
    
    return pos;
}

void testLogger(flash_memory_context_t *p_context)
{
    senstick_logger_t logger_context;
    uint8_t data[11];
    uint8_t rd_buffer[11];

    // ロガーを開きます。
    loggerOpen(&logger_context, p_context, true);
    
    // 逐次書き込み
    //    const uint32_t max_address = MX25L25635F_FLASH_SIZE;
    const uint32_t max_address = 10 * 1024; // 簡易に10kBでテスト
//    srand(1);
    int rand = 0;
    for(uint32_t address = 0x00000000; address < max_address; address += sizeof(data)) {
        // ランダムデータを書き込む
        for(int i=0; i < sizeof(data); i++) {
//            data[i] = (uint8_t) rand();
            data[i] = rand++ % 256;

        }
        // 書き込み
        logger_write(&logger_context, data, sizeof(data));
    }
    loggerClose(&logger_context);

    // ロガーを開きます。
    loggerOpen(&logger_context, p_context, false);

    // 読み込み、比較
//    srand(1);
    rand = 0;
    for(uint32_t address = 0x00000000; address < max_address; address += sizeof(data)) {
        loggerRead(&logger_context, rd_buffer, sizeof(rd_buffer));
        for(int i=0; i < sizeof(data); i++) {
//            if( rd_buffer[i] != (uint8_t)rand() ) {
            if( rd_buffer[i] != (rand++ % 256) ) {
                APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
            }
        }
    }
    loggerClose(&logger_context);
    
}
