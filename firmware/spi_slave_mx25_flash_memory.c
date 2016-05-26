#include <string.h>
#include <stdlib.h>

#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_assert.h>
#include <nrf_drv_gpiote.h>
#include <nrf_drv_spi.h>
#include <app_util_platform.h>
#include <app_error.h>

#include "senstick_io_definition.h"
#include "spi_slave_mx25_flash_memory.h"

static nrf_drv_spi_t spi;

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
//    FLASH_CMD_PP   =     0x02,    //PP (page program)
    FLASH_CMD_PP4B =     0x12,    //PP4B (page program with 4 byte address)
    
    //Erase comands
//    FLASH_CMD_BE4B =     0xDC,    //BE4B (Block Erase with 4 byte address)
//    FLASH_CMD_CE   =     0x60,    //CE (Chip Erase) hex code: 60 or C7
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

static void transferToSPISlave(FlashMemoryCommand_t command, uint8_t *p_tx_buffer, uint8_t tx_buffer_length, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    ret_code_t err_code;
    
    // nCSをネゲート
    setChipEnable(true);
    
    // コマンドを書き込み
    uint8_t com = (uint8_t)command;
    err_code = nrf_drv_spi_transfer(&spi, &com, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);
    
    // データを書き込み
    if(tx_buffer_length > 0 || rx_buffer_length > 0) {
        err_code = nrf_drv_spi_transfer(&spi, p_tx_buffer, tx_buffer_length, p_rx_buffer, rx_buffer_length);
        APP_ERROR_CHECK(err_code);
    }
    
    // nCSをアサート
    setChipEnable(false);
    
}

static void writeToSPISlave(FlashMemoryCommand_t command, uint8_t *p_tx_buffer, uint8_t tx_buffer_length)
{
    transferToSPISlave(command, p_tx_buffer, tx_buffer_length, NULL, 0);
}

static void readFromSPISlave(FlashMemoryCommand_t command, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    transferToSPISlave(command, NULL, 0, p_rx_buffer, rx_buffer_length);
}

static void transferSPISlaveWithAddress(FlashMemoryCommand_t command, uint32_t address, uint8_t *p_tx_buffer, uint8_t tx_buffer_length, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    ret_code_t err_code;
    
    // nCSをネゲート
    setChipEnable(true);
    
    // コマンドを書き込み
    uint8_t com = (uint8_t)command;
    err_code = nrf_drv_spi_transfer(&spi, &com, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);
    
    // アドレスを書き込み
    uint8_t buffer[4];
    uint32ToByteArray(buffer, address);
    err_code = nrf_drv_spi_transfer(&spi, buffer, 4, NULL, 0);
    APP_ERROR_CHECK(err_code);
    
    // データを読み出し
    if(tx_buffer_length > 0 || rx_buffer_length > 0) {
        err_code = nrf_drv_spi_transfer(&spi, p_tx_buffer, tx_buffer_length, p_rx_buffer, rx_buffer_length);
        APP_ERROR_CHECK(err_code);
    }
    
    // nCSをアサート
    setChipEnable(false);
}

static void writeToSPISlaveWithAddress(FlashMemoryCommand_t command, uint32_t address, uint8_t *p_tx_buffer, uint8_t tx_buffer_length)
{
    transferSPISlaveWithAddress(command, address, p_tx_buffer, tx_buffer_length, NULL, 0);
}

static void readFromSPISlaveWithAddress(FlashMemoryCommand_t command, uint32_t address, uint8_t *p_rx_buffer, uint8_t rx_buffer_length)
{
    transferSPISlaveWithAddress(command, address, NULL, 0, p_rx_buffer, rx_buffer_length);
}

static void readStatusRegister(uint8_t *p_data)
{
    readFromSPISlave(FLASH_CMD_RDSR, p_data, 1);
}

static void readConfigrationRegister(uint8_t *p_data)
{
    readFromSPISlave(FLASH_CMD_RDCR, p_data, 1);
}
/*
static void readDeviceID(flash_memory_context_t *p_context, uint32_t *p_data)
{
    uint8_t buffer[3];
    
    readFromSPISlave( FLASH_CMD_RDID, buffer, 3);
    
    *p_data = ((uint32_t)buffer[0] << 16) |  ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 0);
}*/

static bool IsFlashBusy(void)
{
    uint8_t status = 0;
    readStatusRegister(&status);
    return ((status & STATUS_REGISTER_WIP) != 0);
}

static void waitFlashReady(void)
{
    const int TIMEOUT_LOOP = 50; // 5 * 50 = 250msec
    
    int cnt = 0;
    while( IsFlashBusy() ) {
        nrf_delay_ms(5);
        cnt++;
        if(cnt > TIMEOUT_LOOP) {
            APP_ERROR_CHECK(NRF_ERROR_TIMEOUT);
            break;
        }
    }
}

static void writeCommandWriteEnable(void)
{
    writeToSPISlave( FLASH_CMD_WREN, NULL, 0);
}
/*
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
    writeToSPISlave( FLASH_CMD_WRSR, buffer, sizeof(buffer));
    
    waitFlashReady(p_context);
}*/

static bool isAddress4ByteMode(void)
{
    uint8_t config;
    readConfigrationRegister(&config);
    return ((config & CONFIG_REGISTER_4BYTE) != 0);
}

// アドレッシングモードを、デフォルトの24ビットから、32ビットモードに設定します
static void enableAddress4ByteMode(void)
{
    // コマンドを書き込み。
    writeToSPISlave( FLASH_CMD_EN4B, NULL, 0);
    
    bool isAddress4BM = isAddress4ByteMode();
    if( ! isAddress4BM ) {
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
}

static void rawWriteFlash(uint32_t address,  uint8_t *data, uint8_t data_length)
{
    // アドレスチェック
    ASSERT(address < MX25L25635F_FLASH_SIZE);
    ASSERT( ! IsFlashBusy() );
    
    // weビットを立てる。
    writeCommandWriteEnable();
    
    // 書き込み。32-bitアドレスモード。
    writeToSPISlaveWithAddress(FLASH_CMD_PP4B, address, data, data_length);
    
    waitFlashReady();
}

static void rawReadFlash(uint32_t address, uint8_t *data, uint8_t data_length)
{
    // アドレスチェック
    ASSERT(address < MX25L25635F_FLASH_SIZE);
    ASSERT( ! IsFlashBusy() );
    
    readFromSPISlaveWithAddress(FLASH_CMD_READ4B, address, data, data_length);
}

/**
 * Public methods
 */

void initFlashMemory(void)
{
    ret_code_t err_code;
    // gpioteモジュールを初期化する
    if(!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    
    nrf_drv_gpiote_out_config_t out_config;
    out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH;
    out_config.task_pin   = false;
    err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_SPI_nCS, &out_config);
    APP_ERROR_CHECK(err_code);
    
    // SPIインタフェース SPI0を使用。
    spi.p_registers  = NRF_SPI0;
    spi.irq          = SPI0_IRQ;
    spi.drv_inst_idx = SPI0_INSTANCE_INDEX;
    spi.use_easy_dma = SPI0_USE_EASY_DMA;
    
    //    nrf_drv_spi_config_t config = NRF_DRV_SPI_DEFAULT_CONFIG(0);
    nrf_drv_spi_config_t config;
    config.sck_pin      = SPI0_CONFIG_SCK_PIN;
    config.mosi_pin     = SPI0_CONFIG_MOSI_PIN;
    config.miso_pin     = SPI0_CONFIG_MISO_PIN;
    config.ss_pin       = NRF_DRV_SPI_PIN_NOT_USED;
    config.irq_priority = SPI0_CONFIG_IRQ_PRIORITY;
    config.orc          = 0xff;
    config.frequency    = NRF_DRV_SPI_FREQ_4M;
    //    config.frequency    = NRF_DRV_SPI_FREQ_250K;
    config.mode         = NRF_DRV_SPI_MODE_0;
    config.bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    
    err_code = nrf_drv_spi_init(&spi, &config, NULL); // blocking mode
    APP_ERROR_CHECK(err_code);
    
    // アドレス4バイトモードに設定
    enableAddress4ByteMode();
}

void writeFlash(uint32_t address, uint8_t *p_buffer, uint8_t size)
{
    // 末尾がフラッシュの領域を超える場合は、書き込み失敗
    ASSERT((address + size) < FLASH_BYTE_SIZE);

//    NRF_LOG_PRINTF_DEBUG("writeFlash:0x%04x, %d\n", address, size);
    
    waitFlashReady();
    
    uint32_t index = 0;
    uint32_t write_address = address;
    do {
        // 1ページは256バイト。
        // 255バイトまでなので、そのサイズで書き込み可能サイズを求める
        int remainingSize  = (size - index);
        int page_size      = 256 - (write_address % 256);
        uint8_t write_size = (uint8_t) MIN(255, MIN(page_size, remainingSize));
        // 書き込む
        rawWriteFlash(write_address, &(p_buffer[index]), write_size);
        // 書き込み位置を更新、全て書き終わるまで繰り返す
        index += write_size;
        write_address += write_size;
        // 次に書き込む位置を示すwrite_addressが次のセクション先頭アドレスのときは、そのセクターを消去する。
        if((write_address % MX25L25635F_SECTOR_SIZE) == 0) {
            erase4kSector(write_address);
        }
    } while (index < size);
}

void readFlash(uint32_t address, uint8_t *p_buffer, uint8_t size)
{
    // 末尾がフラッシュの領域を超える場合は、読み出し失敗
    ASSERT((address + size) < FLASH_BYTE_SIZE);
    
//    NRF_LOG_PRINTF_DEBUG("readFlash:0x%04x, %d\n", address, size);
    
    // サイズが0なら終了
    if(size == 0) {
        return ;
    }
    
    waitFlashReady();
    
    uint32_t index = 0;
    uint32_t read_address = address;
    do {
        // 256バイト単位で読みだすので、読み出し可能サイズを求める
        int remainingSize  = (size - index);
        int page_size      = 256 - (read_address % 256);
        uint8_t read_size  = (uint8_t) MIN(255, MIN(page_size, remainingSize));
        if(read_size <= 0) {
//            return index;
            return;
        }
        // 先頭セクターはファイル情報に使うので、1セクター分アドレスをずらす
        rawReadFlash(read_address, &(p_buffer[index]), (uint8_t)read_size);
        // 位置などを変更する
        index         += read_size;
        read_address  += read_size;
    } while (index < size);
}

// 4kバイト単位のセクターのデータを消去します
void erase4kSector(uint32_t address)
{
    // アドレスチェック
    ASSERT(address < MX25L25635F_FLASH_SIZE);
    
//    NRF_LOG_PRINTF_DEBUG("erase4kSector:0x%04x\n",address);
    
    ASSERT( ! IsFlashBusy() );
    
    // weビットを立てる。
    writeCommandWriteEnable();
    
    uint8_t buffer[4];
    uint32ToByteArray(buffer, address);
    writeToSPISlave( FLASH_CMD_SE4B, buffer, sizeof(buffer));
    
    waitFlashReady();
}

void formatFlash(uint32_t address, int size)
{
    ASSERT((address % MX25L25635F_SECTOR_SIZE) == 0);
    ASSERT((size    % MX25L25635F_SECTOR_SIZE) == 0);
    
    const int count = size / MX25L25635F_SECTOR_SIZE;
    for(int i=0; i < count; i++) {
        erase4kSector(address);
        address += MX25L25635F_SECTOR_SIZE;
    }
}
