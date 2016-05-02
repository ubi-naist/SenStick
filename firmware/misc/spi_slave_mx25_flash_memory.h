#ifndef spi_slave_mx25_flash_memory_h
#define spi_slave_mx25_flash_memory_h

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_spi.h"
#include "senstick_data_models.h"

#define  MX25L25635F_FLASH_SIZE  0x2000000  // 32 MB
#define  MX25L25635F_SECTOR_SIZE 0x01000    // 4KB

#define FLASH_BYTE_SIZE MX25L25635F_FLASH_SIZE

// 構造体の宣言
typedef struct {
    nrf_drv_spi_t *p_spi;
} flash_memory_context_t;

// 初期化関数。
void initFlashMemory(flash_memory_context_t *p_context, nrf_drv_spi_t *p_spi);

// 指定されたアドレスから256バイト単位(ページ)でメモリに'0'を書き込みます。
// アドレスの最下位8ビットが0ではない場合は、256バイト以下が書き込めます。
void writeFlash(flash_memory_context_t *p_context, uint32_t address, uint8_t *data, uint8_t data_length);
void readFlash(flash_memory_context_t *p_context, uint32_t address, uint8_t *data, uint8_t data_length);
// 4kバイト単位のセクターのデータを消去します
void eraseSector(flash_memory_context_t *p_context, uint32_t address);
// チップ全体を消去します。
// チップ全体の消去には2分ほどかかるため、その間デバイスを止めることはできないから、使わないことに。
//void eraseFlash(flash_memory_context_t *p_context);

// p_bufferからsizeバイトを書き込みます。書き込めたサイズを返します。セクターは事前に消去されているとします。
uint32_t flashRawWrite(flash_memory_context_t *p_memory, uint32_t address, uint8_t *p_buffer, uint32_t size);

// p_bufferからsizeバイトを読み込みます。読み込んだサイズを返します。
uint32_t flashRawRead(flash_memory_context_t *p_memory, uint32_t address, uint8_t *p_buffer, uint32_t size);

#endif
