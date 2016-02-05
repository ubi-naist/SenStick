#ifndef flash_stream_h
#define flash_stream_h

#include <stdint.h>
#include <stdbool.h>

#include "spi_slave_mx25_flash_memory.h"

// ストリームの管理情報
typedef struct sector_header_s {
    uint16_t next_index;
    int size;
} sector_header_t;

// セクターの管理情報
typedef struct {
    uint16_t sector_index;
    uint16_t index;
} stream_position_t;

// ストリームの読み書き管理情報
typedef struct flash_stream_s {
    flash_memory_context_t *p_flash;
    uint8_t stream_id;

    uint16_t first_sector;
    int size;
    
    bool can_write;
    
    stream_position_t write_position;
    stream_position_t read_position;
} flash_stream_t;

// ストリームの最大サイズ
#define MAX_STREAM_ID (MX25L25635F_SECTOR_SIZE / sizeof(sector_hader_t) -1)

bool streamOpen(flash_stream_t *p_stream, uint8_t stream_id, flash_memory_context_t *p_flash);
void streamClose(flash_stream_t *p_stream);

int streamWrite(flash_stream_t *p_stream, uint8_t *p_data, int size);
int streamRead(flash_stream_t *p_stream, uint8_t *p_data, int size);

int streamGetSize(flash_stream_t *p_stream);
int streamMoveReadPosition(int forward);
int streamGetReadPosition(flash_stream_t *p_stream);
uint8_t streamGetID(flash_stream_t *p_stream);

#endif /* flash_stream_h */
