#include "flash_stream.h"

#define FLASH_SECTOR_SIZE           MX25L25635F_SECTOR_SIZE             // 4KB
#define INVALID_SECTOR_INDEX        0xffff

/**
 * Private methods
 */
// セクター先頭にあるセクター情報を読み込みます
void readSectorHeader(flash_memory_context_t *p_flash_context, uint16_t index, sector_header_t *p_header)
{
    flashRawRead(p_flash_context, index * FLASH_SECTOR_SIZE, p_header, , sizeof(sector_header_s));
}
// セクター先頭にあるセクター情報を読み込みます
void writeSectorHeader(flash_memory_context_t *p_flash_context, uint16_t index, sector_header_t *p_header)
{
    flashRawWrite(p_flash_context, index * FLASH_SECTOR_SIZE, p_header, , sizeof(sector_header_s));
}

// セクターを割り当てます
sector_index_t allocateSector()
{}

/**
 * Public methods
 */
bool streamOpen(flash_stream_t *p_stream, uint8_t stream_id, flash_memory_context_t *p_flash)
{
    if(stream_id > MAX_STREAM_ID) {
        return false;
    }

    // 初期値設定
    p_stream->p_flash   = p_flash;
    p_stream->stream_id = stream_id;
    // ヘッダ情報を読みだし
    sector_header_t header;
    flashRawRead(p_flash_context, p_stream->stream_id * sizeof(sector_header_s), &header, sizeof(sector_header_t));
    p_stream->first_sector  = header.next_index;
    p_stream->size          = header.size;
    p_stream->can_write     = (first_sector == INVALID_SECTOR_INDEX);
    
    // 最初のセクションを割当
    if(first_sector == INVALID_SECTOR_INDEX) {
        first_sector = allocateSector();
    }
    p_stream->write_position =
    // 読みだし、書き出し位置を設定
    if( p_stream->first_sector.next_index == INVALID_SECTOR_INDEX) {}
    stream_position_t write_position;
    stream_position_t read_position;
}













#define FLASH_BYTE_SIZE             MX25L25635F_FLASH_SIZE              // 32 MB

#define FLASH_SECTOR_MAX_INDEX      (FLASH_BYTE_SIZE / FLASH_SECTOR_SIZE - 1 )

#define SECTOR_HEADER_SIZE      (sizeof(sector_header_info_t))
#define MAX_DATA_SIZE_OF_SECTOR (FLASH_SECTOR_SIZE - SECTOR_HEADER_SIZE)

/**
 * Private methods
 */

// セクター先頭にあるセクター情報を読み込みます
void readSectorHeader(flash_memory_context_t *p_flash_context, sector_index_t index, sector_header_info_t *p_sector_header)
{
    flashRawRead(p_context->p_flash_context, FLASH_SECTOR_SIZE * index, p_sector_header, sizeof(sector_header_info_t));
}
// セクター先頭にあるセクター情報を読み込みます
void writeSectorHeader(flash_memory_context_t *p_flash_context, sector_index_t index, sector_header_info_t *p_sector_header)
{
    flashRawWrite(p_context->p_flash_context, FLASH_SECTOR_SIZE * index, p_sector_header, sizeof(sector_header_info_t));
}

/**
 * Public methods
 */
int sectorWriteData(flash_memory_context_t *p_memory, sector_index_t sector_index, int position, const uint8_t *p_buffer, const int size)
{
    
}
void sectorWriteDataSize(flash_memory_context_t *p_memory, sector_index_t sector_index, int size);
int sectorReadData(flash_memory_context_t *p_memory, sector_index_t sector_index, int position, const uint8_t *p_buffer, const int size);

/*
// 空きセクターを見つけます
sector_index_t findEmptySector(flash_stream_t *p_context, sector_index_t start)
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
bool allocateSector(flash_stream_t *p_context)
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
bool getNextSector(flash_stream_t *p_context)
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
*/
/**
 * Public methods
 */

bool sectorOpen(flash_memory_context_t *p_flash_context, sector_info_t *p_sector_info, sector_index_t index)
{
    if(index > FLASH_SECTOR_MAX_INDEX) {
        return false;
    }

    // 初期化
    memset(p_sector_info, 0xff, sizeof(sector_info_t));
    p_sector_info->index    = index;
    p_sector_info->position = 0;
    
    // ヘッダ情報を読みだす
    readSectorHeader(p_flash_context, index, &(p_sector_info->header));

    return true;
}

void sectorClose(flash_memory_context_t *p_flash_context, sector_info_t *p_sector_info)
{
    if(p_sector_info->available_data_size < (FLASH_SECTOR_SIZE - sizeof(sector_header_info_t))) {
        return;
    }
    // ヘッダを変更する
    p_sector_info->header.available_data_size   = p_sector_info->position;
    p_sector_info->header.next_index            = FLASH_SECTOR_INVALID_INDEX;
    writeSectorHeader(p_flash_context, index, &(p_sector_info->header));
}

bool sectorSetPosition(flash_memory_context_t *p_flash_context, sector_info_t *p_sector_info, int position)
{
    if(position > p_sector_info)
}

int  sectorGetSize(flash_memory_context_t *p_flash_context, sector_info_t *p_sector_info)
{
}
int  sectorWrite(flash_memory_context_t *p_flash_context, sector_info_t *p_sector_info, const uint8_t *p_buffer, const int size);
int  sectorRead(flash_memory_context_t *p_flash_context, sector_info_t *p_sector_info, uint8_t *p_buffer, const int size);





































bool sectorListOpen(flash_stream_t *p_list, flash_memory_context_t *p_flash_context, sector_index_t index, sector_allocater_handler_t allocator)
{
    memset(p_list, 0, sizeof(flash_stream_t));
    p_list->p_flash_context     = p_flash_context;
    p_flash->allocator          = allocator;
    p_list->first_sector_index  = index;
    
    // リスト末尾を取得する
    
    // 先頭セクターを開く
    readSectorHeader(*p_list, p_list->first_sector_index, &(p_list->sector.header));
    // 新規リストであれば、
}

// データをユニット単位で書き込みます。書き込んだユニット数を返します。
int sectorListWrite(flash_stream_t *p_list, uint8_t *p_buffer, const int numOfData, const int sizeofData)
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

int sectorListRead(flash_stream_t *p_list, uint8_t *p_buffer, const int numOfData, const int sizeofData)
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
void sectorListClose(flash_stream_t *p_list)
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

