#ifndef senstick_logger_h
#define senstick_logger_h

#include <stdint.h>
#include <stdbool.h>

#include "spi_slave_mx25_flash_memory.h"

// 構造体の宣言
typedef struct {
    flash_memory_context_t *p_memory;
    // データのバイトサイズ
    uint32_t size;
    // 読み出しポイント
    uint32_t write_position;
    uint32_t read_position;
    bool is_writing_mode;
} senstick_logger_t;

// ロガーを開きます。
void logger_open(senstick_logger_t *p_context, flash_memory_context_t *p_memory, bool is_writing_mode);
// ロガーを閉じます。ロギングを完了するために、このメソッドを最後に必ず呼び出すこと。
void logger_close(senstick_logger_t *p_context);
// p_bufferからsizeバイトを書き込みます。書き込めたサイズを返します。
uint32_t logger_write(senstick_logger_t *p_context, uint8_t *p_buffer, uint32_t size);
// p_bufferからsizeバイトを読み込みます。読み込んだサイズを返します。
uint32_t logger_read(senstick_logger_t *p_context, uint8_t *p_buffer, uint32_t size);
// 先頭からの位置を返します。
uint32_t logger_size(senstick_logger_t *p_context);
// 先頭からpositonバイトに移動します。まだ書き込みをしていない領域を超えてpositionを指定した場合、その飛び越えた領域の値は不定値になります。
uint32_t logger_seek(senstick_logger_t *p_context, uint32_t position);

#endif /* senstick_logger_h */
