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
void test01(flash_memory_context_t *p_flash)
{
    sensor_data_storage_t storage;
    sensorSetting_t setting;
    rtcSettingCommand_t rtc;
    char abstract[21];
    abstract[0] = 0;
    
    // でたらめなシグネチャを書き込む。この状態でストレージを開き、データユニットの数が0であることを確認する。
    int hoge = 0x1234;
    flashRawWrite(p_flash, 0, (uint8_t *)&hoge, sizeof(int));

    // 書き込みで開いてみる
    bool result;
    result = storageOpen(&storage, -1, p_flash, &setting, &rtc, abstract); // 書き込みで開く
    ASSERT(result); // 開けてないとだめ
    ASSERT( storageGetID(&storage) == 0 ); // IDは0のはず
    ASSERT( storageGetSampleCount(&storage, MotionSensor) == 0 );
    // 適当にデータを1つ書いてみる
    SensorData_t data;
    memset(&data.motionSensorData, 0, sizeof(data.motionSensorData));
    storageWrite(&storage,  MotionSensor, &data);
    // 書き込みデータ数は1つ
    ASSERT( storageGetSampleCount(&storage, MotionSensor) == 1 );
    // 1つ読みだしてみる
    ASSERT( storageGetSamplePosition(&storage, MotionSensor) == 0 );
    memset(&data.motionSensorData, 0xff, sizeof(data.motionSensorData));
    storageRead(&storage,  MotionSensor, &data);
    // 全部見るのは辛いから, データの頭と末尾で確認
    ASSERT(data.motionSensorData.acceleration.x == 0 );
    ASSERT(data.motionSensorData.rotationRate.z == 0 );
    // 読み込み位置が移動しているかを確認
    ASSERT(storageGetSamplePosition(&storage, MotionSensor) == 1 );
    // 閉じてみる
    storageClose(&storage);
    
    // データの数は1のはず
    ASSERT( storageGetUnitCount(&storage) == 1 );
    
    // 開いて読んでみる
    memset(&storage, 0, sizeof(storage));
    
    result = storageOpen(&storage, 0, p_flash, &setting, &rtc, abstract);
    ASSERT(result); // 開けてないとだめ
    ASSERT( storageGetID(&storage) == 0 );
    ASSERT( storageGetSampleCount(&storage, MotionSensor) == 1 );
    // 1つ読みだしてみる
    ASSERT( storageGetSamplePosition(&storage, MotionSensor) == 0 );
    memset(&data.motionSensorData, 0xff, sizeof(data.motionSensorData));
    storageRead(&storage,  MotionSensor, &data);
    // 全部見るのは辛いから, データの頭と末尾で確認
    ASSERT(data.motionSensorData.acceleration.x == 0 );
    ASSERT(data.motionSensorData.rotationRate.z == 0 );
    // 読み込み位置が移動しているかを確認
    ASSERT(storageGetSamplePosition(&storage, MotionSensor) == 1 );
    // 閉じてみる
//    storageClose(&storage);

    // もう一回読み出しを繰り返してみる
    // 開いて読んでみる
    memset(&storage, 0, sizeof(storage));
    
    result = storageOpen(&storage, 0, p_flash, &setting, &rtc, abstract);
    ASSERT(result); // 開けてないとだめ
    ASSERT( storageGetID(&storage) == 0 );
    ASSERT( storageGetSampleCount(&storage, MotionSensor) == 1 );
    // 1つ読みだしてみる
    ASSERT( storageGetSamplePosition(&storage, MotionSensor) == 0 );
    memset(&data.motionSensorData, 0xff, sizeof(data.motionSensorData));
    storageRead(&storage,  MotionSensor, &data);
    // 全部見るのは辛いから, データの頭と末尾で確認
    ASSERT(data.motionSensorData.acceleration.x == 0 );
    ASSERT(data.motionSensorData.rotationRate.z == 0 );
    // 読み込み位置が移動しているかを確認
    ASSERT(storageGetSamplePosition(&storage, MotionSensor) == 1 );
    // 閉じてみる
    storageClose(&storage);
}

void do_storage_test(flash_memory_context_t *p_flash)
{
    test01(p_flash);
}
