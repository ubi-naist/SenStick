#ifndef flash_stream_h
#define flash_stream_h

#include "spi_slave_mx25_flash_memory.h"

// セクタ管理に使うセクター数
#define STREAM_HEADER_SECTORS   3
// 利用可能領域のバイトサイズ
#define STORAGe_BYTE_SIZE    (MX25L25635F_FLASH_SIZE - MX25L25635F_SECTOR_SIZE * STREAM_HEADER_SECTORS)

#define SECTOR_FLAG_CACHE_SIZE 12
// 構造体の宣言
typedef struct {
    flash_memory_context_t flash_context;
    uint32_t allocated_sector[SECTOR_FLAG_CACHE_SIZE];
    uint8_t ring_index;
} flash_stream_context_t;

// フラッシュメモリへのストリーム書き込みを提供します
void initFlashStream(flash_stream_context_t *p_context, nrf_drv_spi_t *p_spi);
// フォーマット
void formatStream(flash_stream_context_t *p_context);
// フラッシュメモリへの同期書き込みを提供します。もしも新たなセクター割当が必要になった場合は、セクター消去時間(typ. 30ミリ秒)がかかります。
uint32_t writeStream(flash_stream_context_t *p_context, uint32_t address, uint8_t *p_data, uint32_t length);
// フラッシュメモリからの読み込みを提供します。
uint32_t readStream(flash_stream_context_t *p_context, uint32_t address, uint8_t *p_data, uint32_t length);

#endif /* flash_stream_h */
