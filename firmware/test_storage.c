#include <string.h>
#include <stdint.h>

#include "sensor_data_storage.h"

#include "test_storage.h"

#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_assert.h"
#include "sdk_errors.h"

// 基本的なテスト
void test01(flash_stream_context_t *p_stream)
{
    sensor_data_storage_t storage;    
    sensorSetting_t setting;
    ble_date_time_t date;
    char abstract[21];
    abstract[0] = 0;
    
    // でたらめなシグネチャを書き込む。この状態でストレージを開き、データユニットの数が0であることを確認する。
//    int hoge = 0x1234;
//    flashRawWrite(&(p_stream->flash_context), 0, (uint8_t *)&hoge, sizeof(int));
    
    formatStream(p_stream);
    
    // 書き込みで開いてみる
    bool result;
    result = storageOpen(&storage, 0xff, p_stream, &setting, &date, abstract); // 書き込みで開く
    ASSERT(result); // 開けてないとだめ
    ASSERT( storageGetID(&storage) == 0 ); // IDは0のはず
    ASSERT( storageGetSampleCount(&storage, AccelerationSensor) == 0 );
    // 適当にデータを1つ書いてみる
    SensorData_t data;
    memset(&data.data.acceleration, 0, sizeof(data.data.acceleration));
    data.type = AccelerationSensor;
    data.data.acceleration.x = 1;
    data.data.acceleration.z = 3;
    storageWrite(&storage, &data);
    // 書き込みデータ数は1つ
    ASSERT( storageGetSampleCount(&storage, AccelerationSensor) == 1 );
    // 1つ読みだしてみる
    ASSERT( storageGetSamplePosition(&storage, AccelerationSensor) == 0 );
    memset(&data.data.acceleration, 0xff, sizeof(data.data.acceleration));
    storageRead(&storage, AccelerationSensor, &data);
    // 全部見るのは辛いから, データの頭と末尾で確認
    ASSERT(data.data.acceleration.x == 1 );
    ASSERT(data.data.acceleration.z == 3 );
    // 読み込み位置が移動しているかを確認
    ASSERT(storageGetSamplePosition(&storage, AccelerationSensor) == 1 );
    // 閉じてみる
    storageClose(&storage);
    
    // データの数は1のはず
    ASSERT( storageGetUnitCount(storage.p_stream) == 1 );
    
    // 開いて読んでみる
    memset(&storage, 0, sizeof(storage));
    
    result = storageOpen(&storage, 0, p_stream, &setting, &date, abstract);
    ASSERT(result); // 開けてないとだめ
    ASSERT( storageGetID(&storage) == 0 );
    ASSERT( storageGetSampleCount(&storage, AccelerationSensor) == 1 );
    // 1つ読みだしてみる
    ASSERT( storageGetSamplePosition(&storage, AccelerationSensor) == 0 );
    memset(&data.data.acceleration, 0xff, sizeof(data.data.acceleration));
    storageRead(&storage,  AccelerationSensor, &data);
    // 全部見るのは辛いから, データの頭と末尾で確認
    ASSERT(data.data.acceleration.x == 1 );
    ASSERT(data.data.acceleration.z == 3 );
    // 読み込み位置が移動しているかを確認
    ASSERT(storageGetSamplePosition(&storage, AccelerationSensor) == 1 );
    // 閉じてみる
//    storageClose(&storage);

    // もう一回読み出しを繰り返してみる
    // 開いて読んでみる
    memset(&storage, 0, sizeof(storage));
    
    result = storageOpen(&storage, 0, p_stream, &setting, &date, abstract);
    ASSERT(result); // 開けてないとだめ
    ASSERT( storageGetID(&storage) == 0 );
    ASSERT( storageGetSampleCount(&storage, AccelerationSensor) == 1 );
    // 1つ読みだしてみる
    ASSERT( storageGetSamplePosition(&storage, AccelerationSensor) == 0 );
    memset(&data.data.acceleration, 0xff, sizeof(data.data.acceleration));
    storageRead(&storage,  AccelerationSensor, &data);
    // 全部見るのは辛いから, データの頭と末尾で確認
    ASSERT(data.data.acceleration.x == 1 );
    ASSERT(data.data.acceleration.z == 3 );
    // 読み込み位置が移動しているかを確認
    ASSERT(storageGetSamplePosition(&storage, AccelerationSensor) == 1 );
    // 閉じてみる
    storageClose(&storage);
}

void do_storage_test(flash_stream_context_t *p_stream)
{
    test01(p_stream);
}
