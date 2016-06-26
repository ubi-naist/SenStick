#ifndef log_controller_h
#define log_controller_h

#include <stdint.h>
#include <stdbool.h>

#include "senstick_types.h"
#include "senstick_sensor_base_data.h"

// ログのヘッダ構造
typedef struct {
    uint32_t startAddress; // データ開始位置
    uint32_t size;         // データバイトサイズ
    
    uint8_t              logID;
    samplingDurationType samplingDuration;
    uint16_t             measurementRange;
} log_header_t;

typedef struct {
    uint32_t headerStartAddress; // ヘッダ開始アドレス
    log_header_t header;
    
    // 書き込み可能フラグ。
    bool canWrite;
    // EOP送信終了フラグ
    bool didSendEndOfDataPacket;
    
    uint32_t readPosition;
    uint32_t writePosition;
} log_context_t;

// ログ領域をフォーマットします。
void formatLog(const flash_address_info_t *p_address_info);

//
void createLog(log_context_t *p_context, uint8_t logID, samplingDurationType samplingDuration, uint16_t measurementRange, const flash_address_info_t *p_address_info);

// ログを読み込みモードで開きます。失敗した時はfalseが返ってきます。
void openLog(log_context_t *p_context, uint8_t logID, const flash_address_info_t *p_address_info);

// ログを閉じます。
void closeLog(log_context_t *p_context);

// 読み込み専用で再オープン。ログ構造体をコピーする。
void reOpenLog(log_context_t *p_dst_context, log_context_t *p_src_context);

// 書き込めたサイズを返します。
int writeLog(log_context_t *p_context, uint8_t *p_data, int length);

// 読み込んだサイズを返します。
int readLog(log_context_t *p_context, uint8_t *p_data, int length);

// 読み出し位置をシークします。シーク位置を返します。書き込み位置はseekされません。
int seekLog(log_context_t *p_context, int position);

#endif /* log_controller_h */
