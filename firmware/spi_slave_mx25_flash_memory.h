#ifndef spi_slave_mx25_flash_memory_h
#define spi_slave_mx25_flash_memory_h

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_spi.h"
#include "senstick_data_models.h"

// 構造体の宣言
typedef struct {
    nrf_drv_spi_t *p_spi;
} flash_memory_context_t;

// 初期化関数。
void initFlashMemory(flash_memory_context_t *p_context, nrf_drv_spi_t *p_spi);

// 指定されたアドレスから256バイト単位(ページ)でメモリに'0'を書き込みます。
// アドレスの最下位8ビットが0ではない場合は、256バイト以下が書き込めます。
void writePage(flash_memory_context_t *p_context, uint32_t address, uint8_t *data, uint8_t data_length);
void readFlash(flash_memory_context_t *p_context, uint32_t address, uint8_t *data, uint8_t data_length);

void testFlashMemory(flash_memory_context_t *p_context);


#endif
