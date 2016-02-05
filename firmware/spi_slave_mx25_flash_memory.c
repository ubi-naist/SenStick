#include "spi_slave_mx25_flash_memory.h"

#include <string.h>
#include <stdlib.h>

#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"

#include "senstick_io_definitions.h"

//#ifdef MX25L25635F
//#define    FlashID          0xc22019
//#define    ElectronicID     0x18
//#define    RESID0           0xc218
//#define    RESID1           0x18c2
//#define    FlashSize        0x2000000      // 32 MB
//#define    CE_period        156250200      // 300s, = tCE /  ( CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
//#define    tW               40000000       // 40ms
//#define    tDP              10000          // 10us
//#define    tBP              30000          // 30us
//#define    tPP              3000000        // 3ms
//#define    tSE              200000000      // 200ms
//#define    tBE32            1000000000     // 1s
//#define    tBE              2000000000     // 2s
//#define    tPUW             10000000       // 10ms
//#define    tWSR             1000000        // 1ms
//#define    tWREAW           40             // 40ns
//#endif

typedef enum {
    // ID commands
    FLASH_CMD_RDID =     0x9F,    //RDID (Read Identification)
    //Register comands
    FLASH_CMD_WRSR =     0x01,    //WRSR (Write Status Register)
    FLASH_CMD_RDSR =     0x05,    //RDSR (Read Status Register)
    FLASH_CMD_RDCR =     0x15,    //RDCR (Read Configuration Register)
    
    //READ comands
    FLASH_CMD_READ4B =    0x13,    //READ4B (1 x I/O with 4 byte address)
    
    //Program comands
    FLASH_CMD_WREN =     0x06,    //WREN (Write Enable)
    FLASH_CMD_WRDI =     0x04,    //WRDI (Write Disable)
    FLASH_CMD_PP   =     0x02,    //PP (page program)
    FLASH_CMD_PP4B =     0x12,    //PP4B (page program with 4 byte address)
    
    //Erase comands
    FLASH_CMD_BE4B =     0xDC,    //BE4B (Block Erase with 4 byte address)
    FLASH_CMD_CE   =     0x60,    //CE (Chip Erase) hex code: 60 or C7
    FLASH_CMD_SE4B =     0x21,    //SE (Sector Erase with 4 byte addr)
    
    //Mode setting comands

    FLASH_CMD_EN4B =    0xB7,    //EN4B( Enter 4-byte Mode )
    FLASH_CMD_EX4B =    0xE9,    //EX4B( Exit 4-byte Mode )
    
    //Reset comands
    FLASH_CMD_RSTEN =    0x66,    //RSTEN (Reset Enable)
    FLASH_CMD_RST   =    0x99,    //RST (Reset Memory)
    
    FLASH_CMD_NOP   =    0x00,    //NOP (No Operation)
    
} FlashMemoryCommand_t;

// Status Register
// bit7 SRWD (status register write protect)
// bit6 QE (Quad Enable)
// bit5:2 BP3:0 (level of protected block)
// bit1 WEL (write enable latch)
// bit0 WIP (write in progress bit)

#define STATUS_REGISTER_SRWD    0x80
#define STATUS_REGISTER_WEL     0x02
#define STATUS_REGISTER_WIP     0x01

//Configration Register
//bit7 DC1 (Dummy cycle 1)
//bit6 DC0 (Dummy cycle 0)
//bit5 4 BYTE
//bit4 Reserved
//bit3 TB (top/bottom selected)
//bit2:0 ODS 2:0 (output driver strength)

#define CONFIG_REGISTER_4BYTE 0x20

/**
 * Private methods
 */

// nCSを設定してチップを選択します。isEnabledがtrueならばチップが選択されます。
// 物理的な電圧や負論理かいなかなどは、このメソッドが吸収します。
static void setChipEnable(bool isEnabled)
{
    if(isEnabled) {
        nrf_drv_gpiote_out_clear(PIN_NUMBER_SPI_nCS);
    } else {
        nrf_drv_gpiote_out_set(PIN_NUMBER_SPI_nCS);
    }
}

// 符号なし32ビット整数をバイト配列に展開します。ビッグエンディアン。
static void uint32ToByteArray(uint8_t *p_dst, uint32_t src)
{
    // アドレスは4バイト。MSB-first
    p_dst[0] = (uint8_t)(0x0ff & (src >> 24));
    p_dst[1] = (uint8_t)(0x0ff & (src >> 16));
    p_dst[2] = (uint8_t)(0x0ff & (src >>  8));
    p_dst[3] = (uint8_t)(0x0ff & (src >>  0));
}

static void transferToSPISlave(nrf_drv_spi_t *p_spi, FlashMemoryCommand_t command, uint8_t *p_tx_buffer, uint8_t tx_buffer_length, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    ret_code_t err_code;
    
    // nCSをネゲート
    setChipEnable(true);
    
    // コマンドを書き込み
    uint8_t com = (uint8_t)command;
    err_code = nrf_drv_spi_transfer(p_spi, &com, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);
    
    // データを書き込み
    if(tx_buffer_length > 0 || rx_buffer_length > 0) {
        err_code = nrf_drv_spi_transfer(p_spi, p_tx_buffer, tx_buffer_length, p_rx_buffer, rx_buffer_length);
        APP_ERROR_CHECK(err_code);
    }
    
    // nCSをアサート
    setChipEnable(false);
    
}

static void writeToSPISlave(nrf_drv_spi_t *p_spi, FlashMemoryCommand_t command, uint8_t *p_tx_buffer, uint8_t tx_buffer_length)
{
    transferToSPISlave(p_spi, command, p_tx_buffer, tx_buffer_length, NULL, 0);
}

static void readFromSPISlave(nrf_drv_spi_t *p_spi, FlashMemoryCommand_t command, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    transferToSPISlave(p_spi, command, NULL, 0, p_rx_buffer, rx_buffer_length);
}

static void transferSPISlaveWithAddress(nrf_drv_spi_t *p_spi, FlashMemoryCommand_t command, uint32_t address, uint8_t *p_tx_buffer, uint8_t tx_buffer_length, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    ret_code_t err_code;
    
    // nCSをネゲート
    setChipEnable(true);
    
    // コマンドを書き込み
    uint8_t com = (uint8_t)command;
    err_code = nrf_drv_spi_transfer(p_spi, &com, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);
    
    // アドレスを書き込み
    uint8_t buffer[4];
    uint32ToByteArray(buffer, address);
    err_code = nrf_drv_spi_transfer(p_spi, buffer, 4, NULL, 0);
    APP_ERROR_CHECK(err_code);
    
    // データを読み出し
    if(tx_buffer_length > 0 || rx_buffer_length > 0) {
        err_code = nrf_drv_spi_transfer(p_spi, p_tx_buffer, tx_buffer_length, p_rx_buffer, rx_buffer_length);
        APP_ERROR_CHECK(err_code);
    }
    
    // nCSをアサート
    setChipEnable(false);
}

static void writeToSPISlaveWithAddress(nrf_drv_spi_t *p_spi, FlashMemoryCommand_t command, uint32_t address, uint8_t *p_tx_buffer, uint8_t tx_buffer_length)
{
    transferSPISlaveWithAddress(p_spi, command, address, p_tx_buffer, tx_buffer_length, NULL, 0);
}

static void readFromSPISlaveWithAddress(nrf_drv_spi_t *p_spi, FlashMemoryCommand_t command, uint32_t address, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    transferSPISlaveWithAddress(p_spi, command, address, NULL, 0, p_rx_buffer, rx_buffer_length);
}

static void readStatusRegister(flash_memory_context_t *p_context, uint8_t *p_data)
{
    readFromSPISlave(p_context->p_spi, FLASH_CMD_RDSR, p_data, 1);
}

static void readConfigrationRegister(flash_memory_context_t *p_context, uint8_t *p_data)
{
    readFromSPISlave(p_context->p_spi, FLASH_CMD_RDCR, p_data, 1);
}

static void readDeviceID(flash_memory_context_t *p_context, uint32_t *p_data)
{
    uint8_t buffer[3];
    
    readFromSPISlave(p_context->p_spi, FLASH_CMD_RDID, buffer, 3);
    
    *p_data = ((uint32_t)buffer[0] << 16) |  ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 0);
}

static bool IsFlashBusy(flash_memory_context_t *p_context)
{
    uint8_t status = 0;
    readStatusRegister(p_context, & status);
    return ((status & STATUS_REGISTER_WIP) != 0);
}

static void waitFlashReady(flash_memory_context_t *p_context)
{
    const int TIMEOUT_LOOP = 100;
    
    int cnt = 0;
    while( IsFlashBusy(p_context) ) {
        nrf_delay_ms(1);
        cnt++;
        if(cnt > TIMEOUT_LOOP) {
            APP_ERROR_CHECK(NRF_ERROR_TIMEOUT);
            break;
        }
    }
}

static void writeCommandWriteEnable(flash_memory_context_t *p_context)
{
    writeToSPISlave(p_context->p_spi, FLASH_CMD_WREN, NULL, 0);
}

static void writeStatusConfigrationRegister(flash_memory_context_t *p_context, uint8_t status, uint8_t config)
{
    // Write in progressフラグをチェック
    if( IsFlashBusy(p_context) ) {
        APP_ERROR_CHECK(NRF_ERROR_BUSY);
    }
    
    // Setting Write Enable Latch bit
    writeCommandWriteEnable(p_context);

    uint8_t buffer[2];
    buffer[0] = status;
    buffer[1] = config;
    writeToSPISlave(p_context->p_spi, FLASH_CMD_WRSR, buffer, sizeof(buffer));
    
    waitFlashReady(p_context);
}

static bool isAddress4ByteMode(flash_memory_context_t *p_context)
{
    uint8_t config;
    readConfigrationRegister(p_context, &config);
    return ((config & CONFIG_REGISTER_4BYTE) != 0);
}

// アドレッシングモードを、デフォルトの24ビットから、32ビットモードに設定します
static void enableAddress4ByteMode(flash_memory_context_t *p_context)
{
    // コマンドを書き込み。
    writeToSPISlave(p_context->p_spi, FLASH_CMD_EN4B, NULL, 0);
    
    bool isAddress4BM = isAddress4ByteMode(p_context);
    if( ! isAddress4BM ) {
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
}

/**
 * Public methods
 */

void initFlashMemory(flash_memory_context_t *p_context, nrf_drv_spi_t *p_spi)
{
    memset(p_context, 0, sizeof(flash_memory_context_t));
    p_context->p_spi = p_spi;
    
    // アドレス4バイトモードに設定
    enableAddress4ByteMode(p_context);
}

// 4kバイト単位のセクターのデータを消去します
void eraseSector(flash_memory_context_t *p_context, uint32_t address)
{
    // アドレスチェック
    if( address > MX25L25635F_FLASH_SIZE) {
        APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
    }
    
    // Write in progressフラグをチェック
    if( IsFlashBusy(p_context) ) {
        APP_ERROR_CHECK(NRF_ERROR_BUSY);
    }
    
    // weビットを立てる。
    writeCommandWriteEnable(p_context);
    
    uint8_t buffer[4];
    uint32ToByteArray(buffer, address);
    writeToSPISlave(p_context->p_spi, FLASH_CMD_SE4B, buffer, sizeof(buffer));
    
    waitFlashReady(p_context);
}

void writeFlash(flash_memory_context_t *p_context, uint32_t address, uint8_t *data, uint8_t data_length)
{
    // アドレスチェック
    if( address > MX25L25635F_FLASH_SIZE) {
        APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
    }
    
    // Write in progressフラグをチェック
    if( IsFlashBusy(p_context) ) {
        APP_ERROR_CHECK(NRF_ERROR_BUSY);
    }
    
    // weビットを立てる。
    writeCommandWriteEnable(p_context);
    
    // 書き込み。32-bitアドレスモード。
    writeToSPISlaveWithAddress(p_context->p_spi, FLASH_CMD_PP, address, data, data_length);
    
    waitFlashReady(p_context);
}

void readFlash(flash_memory_context_t *p_context, uint32_t address, uint8_t *data, uint8_t data_length)
{
    // アドレスチェック
    if( address > MX25L25635F_FLASH_SIZE) {
        APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
    }
    
    // Write in progressフラグをチェック
    if( IsFlashBusy(p_context) ) {
        APP_ERROR_CHECK(NRF_ERROR_BUSY);
    }

    readFromSPISlaveWithAddress(p_context->p_spi, FLASH_CMD_READ4B, address, data, data_length);
}

uint32_t flashRawWrite(flash_memory_context_t *p_memory, uint32_t address, const uint8_t *p_buffer, const uint32_t size)
{
    // 末尾がフラッシュの領域を超える場合は、書き込み失敗
    if( (address + sizeof) >= FLASH_BYTE_SIZE) {
        return 0;
    }
    
    uint32_t index = 0;
    uint32_t write_position = address;
    do {
        // 256バイト単位で書き込むので、書き込み可能サイズを求める
        int page_size      = 256 - (write_position % 256);
        uint8_t write_size = (uint8_t) MIN(255, MIN(page_size, size));
        // 書き込む
        writeFlash(p_memory, write_position, &(p_buffer[index]), write_size);
        // 書き込み位置を更新、全て書き終わるまで繰り返す
        index += write_size;
        write_position += write_size;
    } while (index < size);
    
    return index;
}

uint32_t flashRawRead(flash_memory_context_t *p_memory, uint32_t address, uint8_t *p_buffer, const uint32_t size)
{
    // 末尾がフラッシュの領域を超える場合は、読み出し失敗
    if( (address + sizeof) >= FLASH_BYTE_SIZE) {
        return 0;
    }
    
    if(size == 0) {
        return 0;
    }
    
    uint32_t index = 0;
    uint32_t read_position = address;
    do {
        // 読みだし可能サイズ
        int remainingSize = (size - read_position);
        // 256バイト単位で読みだすので、読み出し可能サイズを求める
        int page_size      = 256 - (read_position % 256);
        uint8_t read_size  = (uint8_t) MIN(255, MIN( MIN(size, page_size), remainingSize));
        if(readSize <= 0) {
            return index;
        }
        // 先頭セクターはファイル情報に使うので、1セクター分アドレスをずらす
        readFlash(p_memory, read_position, &(p_buffer[index]), (uint8_t)read_size);
        // 位置などを変更する
        index           += read_size;
        read_position   += read_size;
    } while (index < size);
    
    return index;
}

