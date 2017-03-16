#ifndef spi_slave_mx25_flash_memory_h
#define spi_slave_mx25_flash_memory_h

#include <stdint.h>
#include <stdbool.h>

#include "senstick_data_model.h"

#define  MX25L25635F_FLASH_SIZE  0x2000000  // 32 MB
#define  MX25L25635F_SECTOR_SIZE 0x01000    // 4KB

#define FLASH_BYTE_SIZE MX25L25635F_FLASH_SIZE

// 初期化関数。
void initFlashMemory(void);

bool isFlashBusy(void);

void writeFlash(uint32_t address, uint8_t *data, uint8_t data_length);
void readFlash(uint32_t address,  uint8_t *data, uint8_t data_length);

// 4kバイト単位のセクターのデータを消去します
void erase4kSector(uint32_t address);
// 4kバイト単位のセクター単位で、データを消去します
void formatFlash(uint32_t address, int size);

void flashMemoryEnterDeepPowerDown(void);
void flashMemoryReleasePowerDown(void);
#endif
