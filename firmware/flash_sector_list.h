#ifndef flash_sector_list_h
#define flash_sector_list_h

#include <stdint.h>
#include <stdbool.h>

#include "spi_slave_mx25_flash_memory.h"

#define FLASH_BYTE_SIZE         MX25L25635F_FLASH_SIZE  // 32 MB
#define FLASH_SECTOR_SIZE       MX25L25635F_SECTOR_SIZE // 4KB
#define FLASH_SECTOR_MAX_INDEX  (FLASH_BYTE_SIZE / FLASH_SECTOR_SIZE - 1 )

#define FLASH_SECTOR_INVALID_INDEX      0xffff

typedef uint16_t sector_index_t;

// セクターの情報
typedef struct sector_header_info_s {
    sector_index_t  next_index;
    uint16_t        available_data_size;
} sector_header_info_t;

typedef struct sector_info_s {
    sector_index_t  index;
    sector_header_info_t header;
} sector_info_t;

// セクターのリスト管理構造体
typedef struct flash_sector_list_s {
    flash_memory_context_t      *p_flash_context;

    // 先頭インデックス
    sector_index_t  first_sector_index;
    // 現在のインデックス
    sector_info_t   sector;
    int             position;
} flash_sector_list_t;

// p_bufferからsizeバイトを書き込みます。書き込めたサイズを返します。セクターは事前に消去されているとします。
uint32_t flashRawWrite(flash_memory_context_t *p_memory, uint32_t address, const uint8_t *p_buffer, const uint32_t size);
// p_bufferからsizeバイトを読み込みます。読み込んだサイズを返します。
uint32_t flashRawRead(flash_memory_context_t *p_memory, uint32_t address, uint8_t *p_buffer, const uint32_t size);

// セクターのリスト構造を開きます
bool sectorListOpen(flash_sector_list_t *p_list, flash_memory_context_t *p_flash_context, sector_index_t index);
// データをユニット単位で書き込みます。書き込んだユニット数を返します。
int sectorListWrite(flash_sector_list_t *p_list, uint8_t *p_buffer, const int numOfData, const int sizeofData);
// データをユニット単位で読み込みます。読み込んだユニット数を返します。
int sectorListRead(flash_sector_list_t *p_list, uint8_t *p_buffer, const int numOfData, const int sizeofData);
// リストを閉じます。
void sectorListClose(flash_sector_list_t *p_list);

#endif /* flash_sector_list_h */
