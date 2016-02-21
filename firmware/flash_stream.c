#include <string.h>
#include "flash_stream.h"

#define STREAM_SIGNATURE    0xc0ffeea5
#define SIGNATURE_SIZE      (sizeof(uint32_t))

// データエリアの開始位置
#define DATA_START_POSITION (MX25L25635F_SECTOR_SIZE * STREAM_HEADER_SECTORS)

/*
 * Private methods
 */
static bool isCache(flash_stream_context_t *p_context, uint32_t address)
{
    // 正規化
    address = (address / MX25L25635F_SECTOR_SIZE) * MX25L25635F_SECTOR_SIZE;
    for(int i =0; i < SECTOR_FLAG_CACHE_SIZE; i++) {
        if( p_context->allocated_sector[i] == address ) {
            return true;
        }
    }
    return false;
}

// 該当セクターはすでに割当されているか(キャッシュ)
static void setCache(flash_stream_context_t *p_context, uint32_t address)
{
    // 正規化
    address = (address / MX25L25635F_SECTOR_SIZE) * MX25L25635F_SECTOR_SIZE;
    // キャッシュに保存
    p_context->allocated_sector[p_context->ring_index] = address;
    p_context->ring_index = (p_context->ring_index + 1) % SECTOR_FLAG_CACHE_SIZE;
}

// セクタ割当フラグを設定する
static void setAllocationFlag(flash_stream_context_t *p_context, uint32_t address)
{
    uint32_t sector_address     = address / MX25L25635F_SECTOR_SIZE;
    uint32_t flag_address       = (sector_address / 8 ) + SIGNATURE_SIZE;
    uint8_t  flag_bit_position  = sector_address % 8;
    
    // 読みだす
    uint8_t data;
    readFlash(&(p_context->flash_context), flag_address, &data, 1);
    // ビットを0に落とす
    data = data & ~(0x01 << flag_bit_position);
    // 書き戻す
    writeFlash(&(p_context->flash_context), flag_address, &data, 1);
    // キャッシュに格納
    setCache(p_context, address);
}

// セクタ割当フラグを読みだす。割り当てられていたらtrueを返す。
static bool readAllocationFlag(flash_stream_context_t *p_context, uint32_t address)
{
    // キャッシュチェック
    if( isCache(p_context, address) ) {
        return true;
    }
    
    // フラッシュから読み出し
    uint32_t sector_address     = address / MX25L25635F_SECTOR_SIZE;
    uint32_t flag_address       = (sector_address / 8 ) + SIGNATURE_SIZE;
    uint8_t  flag_bit_position  = sector_address % 8;
    
    // 読みだす
    uint8_t data;
    readFlash(&(p_context->flash_context), flag_address, &data, 1);
    
    // ビット判定
    bool isAllocated = ((data & (0x01 << flag_bit_position)) == 0);
    // キャッシュ保存
    if( isAllocated ){
        setCache(p_context, address);
    }
    
    return isAllocated;
}

// セクターを消去、割り当てフラグを設定する
static void allocateSector(flash_stream_context_t *p_context, uint32_t address)
{
    uint32_t sector_address = (address / MX25L25635F_SECTOR_SIZE) * MX25L25635F_SECTOR_SIZE;
    eraseSector(&(p_context->flash_context), sector_address);
    
    setAllocationFlag(p_context, address);
}

/*
 * Public methods
 */

// フラッシュメモリへのストリーム書き込みを提供します
void initFlashStream(flash_stream_context_t *p_context, nrf_drv_spi_t *p_spi)
{
    memset(p_context, 0, sizeof(flash_stream_context_t));
    
    initFlashMemory(&(p_context->flash_context), p_spi);
    
    // フォーマット確認、必要な場合はフォーマット
    uint32_t signature;
    flashRawRead(&(p_context->flash_context), 0, (uint8_t *)&signature, sizeof(uint32_t));
    if(signature != STREAM_SIGNATURE) {
        formatStream(p_context);
    }
}

void formatStream(flash_stream_context_t *p_context)
{
    // 先頭3セクターを消去する。
    for(int i=0; i < STREAM_HEADER_SECTORS; i++) {
        eraseSector(&(p_context->flash_context), i * MX25L25635F_SECTOR_SIZE);
    }
    // シグネチャを書き込む。
    uint32_t signature = STREAM_SIGNATURE;
    flashRawWrite(&(p_context->flash_context), 0, (uint8_t *)&signature, sizeof(uint32_t));
    // キャッシュをクリアする
    memset(p_context->allocated_sector, 0, sizeof(uint32_t) * SECTOR_FLAG_CACHE_SIZE);
    p_context->ring_index = 0;
}

// フラッシュメモリへの同期書き込みを提供します。もしも新たなセクター割当が必要になった場合は、セクター消去時間(typ. 30ミリ秒)がかかります。
uint32_t writeStream(flash_stream_context_t *p_context, uint32_t address, uint8_t *p_data, uint32_t length)
{
    // 先頭セクターはフラグに使用している。ターゲットアドレスをもとめる
    address += DATA_START_POSITION;
    
    // セクターの割当フラグをチェック, 割当されていなければ割当処理
    bool isAllocated = readAllocationFlag(p_context, address);
    if( ! isAllocated ) {
        allocateSector(p_context, address);
    }
    // 書き込み
    return flashRawWrite(&(p_context->flash_context), address, p_data, length);
}

// フラッシュメモリからの読み込みを提供します。
uint32_t readStream(flash_stream_context_t *p_context, uint32_t address, uint8_t *p_data, uint32_t length)
{
    // 先頭セクターはフラグに使用している。ターゲットアドレスをもとめる
    address += DATA_START_POSITION;
    
    // セクターの割当フラグをチェック, データがセクターをまたぐ可能性があるので、末尾領域まで確認。
    bool isAllocated0 = readAllocationFlag(p_context, address);
    bool isAllocated1 = readAllocationFlag(p_context, address + length - 1);
    if( ! (isAllocated0 && isAllocated1) ) {
        return 0;
    }

    return flashRawRead(&(p_context->flash_context), address, p_data, length);
}
