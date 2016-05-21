#ifndef senstick_flash_address_definitions_h
#define senstick_flash_address_definitions_h

// フラッシュのアドレス割当定義
//#define  MX25L25635F_FLASH_SIZE  0x2000000  // 32 MB
//#define  MX25L25635F_SECTOR_SIZE 0x01000    // 4KB
//#define STREAM_HEADER_SECTORS   3

// データ書き込みは、次に書き込むセクターが事前に消去されていると仮定する。
// そのため、データ領域の末尾で、次のセクターは消去されうる。
// したがって、あるデータ領域と次のデータ領域の間には1セクタ空ける。

// フラッシュメモリは、4kBを1セクター(消去単位)とする。
//    int headerStartAddress; // ヘッダ領域スタートアドレス
//    int headerStorageSize;  // ヘッダ領域バイトサイズ
//    int dataStartAddress;   // データ領域スタートアドレス
//    int dataStorageSize;    // データ領域バイトサイズ

// センサーのヘッダのバイトサイズ
// 1KB = 0x0400
// 4KB = 0x1000

// sizeof(log_header_t) = 8バイト, 4kBには、500収まると。最大200くらいで、十分と。
// ヘッダには1セクタを割り当てる。

// メタデータ、7+20 = 27バイト
// センサーログのカウントを最大200としても、1セクター。

// サンプリングレートと測定レンジ=4バイト
// センサーごとのデータ

// 全セクター数は、8000セクター (32MB/4kB)

// セクタは、最初にアクセスした部分は、消去済だとする。

// メタデータ
// ヘッダ       1セクタ
// メタデータ    1セクタ
// 空きセクタ    1セクタ

// 2バイトを1単位として、15単位。(8000 - 3 - 7 ) / 15 = 532.6, 1単位 530セクターとしよう。

// 種類       サンプルのバイトサイズ     想定サンプリング数
// 加速度     3x2= 6バイト            100ミリ秒          3
// ジャイロ   6バイト                 100ミリ秒           3
// 地磁気     6バイト                 100ミリ秒          3
// 照度       2バイト                100ミリ秒          1
// UV        2バイト                100ミリ秒          1
// 湿度       4バイト                100ミリ秒          2
// 気圧       4バイト                100ミリ秒          2

// センサごとのセクター割当
// ヘッダ       1
// データ
// 空きセクタ    1
#define SECTOR_SIZE       0x1000

// メタデータの領域
#define SENSOR_SETTING_STORAGE_START_ADDRESS 0
#define SENSOR_SETTING_STORAGE_SIZE          (1 * SECTOR_SIZE)
#define SENSOR_SETTING_STORAGE_END_ADDRESS   (SENSOR_SETTING_STORAGE_START_ADDRESS + SENSOR_SETTING_STORAGE_SIZE)

#define METADATA_STORAGE_START_ADDRESS (SENSOR_SETTING_STORAGE_END_ADDRESS + SECTOR_SIZE)
#define METADATA_STORAGE_SIZE          (1 * SECTOR_SIZE)
#define METADATA_STORAGE_END_ADDRESS   (METADATA_STORAGE_START_ADDRESS + METADATA_STORAGE_SIZE)

// 2バイトのセンサデータあたりに割りつける、セクター数
#define SENSOR_DATA_SECTOR_UNIT     530

// 書き込みは次のセクタを消去するため、前のデータ領域から1セクタ空ける。
#define ACCELERATION_SENSOR_STORAGE_START_ADDRESS (METADATA_STORAGE_END_ADDRESS + SECTOR_SIZE)
#define ACCELERATION_SENSOR_STORAGE_SIZE          ((1 + SENSOR_DATA_SECTOR_UNIT) * SECTOR_SIZE)
#define ACCELERATION_SENSOR_STORAGE_END_ADDRESS   (ACCELERATION_SENSOR_STORAGE_START_ADDRESS + ACCELERATION_SENSOR_STORAGE_SIZE)

#define HOGE_SENSOR_STORAGE_START_ADDRESS (ACCELERATION_SENSOR_STORAGE_END_ADDRESS + SECTOR_SIZE)
#define HOGE_SENSOR_STORAGE_SIZE          ((1 + SENSOR_DATA_SECTOR_UNIT) * SECTOR_SIZE)
#define HOGE_SENSOR_STORAGE_END_ADDRESS   (HOGE_SENSOR_STORAGE_START_ADDRESS + HOGE_SENSOR_STORAGE_SIZE)


#endif /* senstick_flash_address_definitions_h */