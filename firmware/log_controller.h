#ifndef log_controller_h
#define log_controller_h

#include <stdint.h>
#include <stdbool.h>

#define NUM_OF_HEADER_SECTOR 1

// フラッシュのアドレス情報
typedef struct {
    int headerStartAddress; // ヘッダ領域スタートアドレス
    int dataStartAddress;   // データ領域スタートアドレス
    int dataStorageSize;    // データ領域バイトサイズ
} log_address_info_t;

// ログのヘッダ構造
typedef struct {
    int startAddress; // データ開始位置
    int dataSize;     // データサイズ
} log_header_t;

typedef struct {
    uint8_t logID;     // ログID
    
    // フラッシュのアドレス、サイズ情報
    int headerAddress;   // ヘッダが格納されているアドレス
    int dataAddress;     // データ開始アドレス
    int maxDataSize;     // 書き込み可能な最大サイズ
    
    bool canWrite;       // 書き込み可能フラグ。
    
    int readPosition;
    int writePosition;
} log_context_t;

// 有効なログの数(有効なヘッダ数)を取得します。
void logControllerGetLogCount(int magicWord, const log_address_info_t *p_address_info);

// ログを開きます。すでに書き込まれたlogIDの場合は、readonlyで開かれます。
void openLog(log_context_t *p_context, uint8_t logID, int magicWord, const log_address_info_t *p_address_info);

// ログを閉じます。
void closeLog(log_context_t *p_context);

// 書き込めたサイズを返します。
int writeLog(log_context_t *p_context, uint8_t *p_data, int length);

// 読み込んだサイズを返します。
int readLog(log_context_t *p_context, uint8_t *p_data, int length);

// 読み出し位置をシークします。シーク位置を返します。書き込み位置はseekされません。
int seekLogReadPosition(log_context_t *p_context, int position);

#endif /* log_controller_h */
