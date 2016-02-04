#include "flash_filesystem.h"

#define SECTOR_HEADER_SIZE      (sizeof(sector_header_info_t))
#define MAX_DATA_SIZE_OF_SECTOR (FLASH_SECTOR_SIZE - SECTOR_HEADER_SIZE)

/**
 * Private methods
 */

// セクター先頭にあるセクター情報を読み込みます
void readSectorHeader(flash_sector_list_t *p_context, sector_index_t index, sector_header_info_t *p_sector_header)
{
    flashRawRead(p_context->p_flash_context, FLASH_SECTOR_SIZE * index, p_sector_header, sizeof(sector_header_info_t));
}

// 空きセクターを見つけます
sector_index_t findEmptySector(flash_sector_list_t *p_context, sector_index_t start)
{
    sector_header_info_t header;
    sector_index_t index = start;
    do {
        // ヘッダを読み込みます
        readSectorHeader(p_context, index, &header);
        if(header.next_index == FLASH_SECTOR_INVALID_INDEX) {
            // リストの終端でかつ、データサイズが無効であれば、空きセクタです
            if( header.available_data_size > MAX_DATA_SIZE_OF_SECTOR) {
                return index;
            } else {
                index++;
            }
        } else {
            // リストのアイテムです。次の項目に飛びます。
        }
        
    } while(index < FLASH_SECTOR_MAX_INDEX);
    
    return FLASH_SECTOR_INVALID_INDEX;
}

// セクターを割り当てます。成功すればtrue。次のセクターにアクセス可能状態になります
bool allocateSector(flash_sector_list_t *p_context)
{
    // 次のセクターを割り当てます
    sector_index_t next_sector = p_context->allocator(p_context->p_flash_context, )
    if(next_sector > FLASH_SECTOR_MAX_INDEX) {
        return false;
    }
    // 現在のセクターを閉じます。
    p_context.sector.header.next_index          = next_sector;
    p_context.sector.header.available_data_size = p_context->position;
    // セクター先頭に書き込み
    flashRawWrite(p_context->p_flash_context, FLASH_SECTOR_SIZE * p_context.sector.index, &(p_context.sector.header), sizeof(sector_header_info_t));
    // 次のセクターを開きます
    p_context.sector.header.next_index          = FLASH_SECTOR_INVALID_INDEX;
    p_context.sector.header.available_data_size = 0;
    p_context.sector.index    = next_sector;
    p_context.sector.position = 0;

    return true;
}

// 次のセクターを開きます。成功すればtrue。次のセクターにアクセス可能状態になります
bool getNextSector(flash_sector_list_t *p_context)
{
    sector_index_t next_sector = p_context->sector.header.next_index;
    // 無効なセクターであればここで終了。
    if(next_sector > FLASH_SECTOR_MAX_INDEX) {
        return false;
    }
    // ヘッダを読み込み
    readSectorHeader(p_context, next_sector, &(p_context->sector.header));
    // 次のセクターを開きます
    p_context.sector.index    = next_sector;
    p_context.sector.position = 0;
    
    return true;
}

/**
 * Public methods
 */

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

bool sectorListOpen(flash_sector_list_t *p_list, flash_memory_context_t *p_flash_context, sector_index_t index, sector_allocater_handler_t allocator)
{
    memset(p_list, 0, sizeof(flash_sector_list_t));
    p_list->p_flash_context     = p_flash_context;
    p_flash->allocator          = allocator;
    p_list->first_sector_index  = index;
    
    // リスト末尾を取得する
    
    // 先頭セクターを開く
    readSectorHeader(*p_list, p_list->first_sector_index, &(p_list->sector.header));
    // 新規リストであれば、
}

// データをユニット単位で書き込みます。書き込んだユニット数を返します。
int sectorListWrite(flash_sector_list_t *p_list, uint8_t *p_buffer, const int numOfData, const int sizeofData)
{
    int index = 0;
    do{
        // 書き込み可能なデータのユニット数
        int remaininNumOfData = (MAX_DATA_SIZE_OF_SECTOR - p_list->position) / sizeofData;
        int writeNum = MIN(remaininNumOfData, numOfData);
        // もしもwriteNumが0であれば、次のセクターを割り当てる
        if(writeNum <= 0) {
            bool allocated = allocateSector(p_list);
            if( ! allocated) {
                return index;
            }
            remaininNumOfData = (MAX_DATA_SIZE_OF_SECTOR - p_list->position) / sizeofData;
            writeNum = MIN(remaininNumOfData, numOfData);
        }
        
        // 書き込む
        flashRawWrite(p_list->p_flash_context, SECTOR_HEADER_SIZE + FLASH_SECTOR_SIZE * p_flash->sector.index + p_flash->position, &(p_buffer[index * sizeofData]), writeNum * sizeofData);
        // 次のインデックス
        index             += writeNum;
        p_flash->position += (writeNum * sizeofData);
    } while(index < numOfData);
    
    return index;
}

int sectorListRead(flash_sector_list_t *p_list, uint8_t *p_buffer, const int numOfData, const int sizeofData)
{
    int index = 0;
    do{
        // 読み込み可能なデータのユニット数
        int availableNumOfData = (MAX_DATA_SIZE_OF_SECTOR - p_list->position) / sizeofData;
        int readNum = MIN(availableNumOfData, numOfData);
        // もしもreadNumが0であれば、次のセクターを開く
        if(readNum <= 0) {
            bool issector = getNextSector(p_list);
            if( ! issector) {
                return index;
            }
            availableNumOfData = (MAX_DATA_SIZE_OF_SECTOR - p_list->position) / sizeofData;
            readNum = MIN(availableNumOfData, numOfData);
        }
        if(readNum <= 0) {
            return index;
        }
        
        // 読み込む
        flashRawWrite(p_list->p_flash_context, SECTOR_HEADER_SIZE + FLASH_SECTOR_SIZE * p_flash->sector.index + p_flash->position, &(p_buffer[index * sizeofData]), readNum * sizeofData);
        // 次のインデックス
        index             += readNum;
        p_flash->position += (readNum * sizeofData);
    } while(index < numOfData);
    
    return index;
}

// リストを閉じます。
void sectorListClose(flash_sector_list_t *p_list)
{
    // セクターインデックス番号を確認
    if( p_context->sector.index > FLASH_SECTOR_MAX_INDEX) {
        return;
    }
    // セクター先頭に書き込み
    flashRawWrite(p_context->p_flash_context, FLASH_SECTOR_SIZE * p_context.sector.index, &(p_context.sector.header), sizeof(sector_header_info_t));
    // セクターインデックスを無効値にしておく
    p_context.sector.sector.index = FLASH_SECTOR_INVALID_INDEX;
}

