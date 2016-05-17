#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <nordic_common.h>
#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_assert.h>
#include <app_error.h>

#include "spi_slave_mx25_flash_memory.h"
#include "test_storage.h"
#include "log_controller.h"

static flash_address_info_t address_info = { 0x00000, 0x1000 * 12 };

// 基本的なテスト
void test01()
{
    log_context_t log_context;
    
    // でたらめなシグネチャを書き込む。この状態でストレージを開き、データユニットの数が0であることを確認する。
//    int hoge = 0x1234;
//    flashRawWrite(&(p_stream->flash_context), 0, (uint8_t *)&hoge, sizeof(int));
    
    formatLog(&address_info);
    
    // 書き込みで開いてみる
    bool result;
    createLog(&log_context, 0x00, 0, 0, &address_info);
    // 適当にデータを1つ書いてみる
    uint32_t data = 0x1234;
    writeLog(&log_context, (uint8_t *)&data, sizeof(uint32_t));
    // 1つ読みだしてみる
    uint32_t read_data = 0;
    readLog(&log_context, (uint8_t *)&read_data, sizeof(uint32_t));
    ASSERT(read_data == 0x1234);

/*
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
 */
}

/*
 static void test_twi_slaves(sensor_manager_t *p_context)
 {
 // 値取得、デバッグ
 while(1) {
 MotionSensorData_t sensor_data;
 getNineAxesData(&(p_context->nine_axes_sensor_context), &sensor_data);
 debugLogAccerationData(&(sensor_data.acceleration));
 
 AirPressureData_t pressure_data;
 getPressureData(&(p_context->pressure_sensor_context), &pressure_data);
 NRF_LOG_PRINTF_DEBUG("Pressure, %d.\n", pressure_data); // 0.01hPa resolution
 
 HumidityData_t humidity_data;
 getHumidityData(&(p_context->humidity_sensor_context), &humidity_data);
 NRF_LOG_PRINTF_DEBUG("Humidity, %d.\n", humidity_data);
 
 UltraVioletData_t uv_data;
 getUVSensorData(&(p_context->uv_sensor_context), &uv_data);
 NRF_LOG_PRINTF_DEBUG("UV, %d.\n", uv_data);
 
 BrightnessData_t brightness_data;
 getBrightnessData(&(p_context->brightness_sensor_context), &brightness_data);
 NRF_LOG_PRINTF_DEBUG("Brightness, %d.\n", brightness_data);
 
 NRF_LOG_PRINTF_DEBUG("\n");
 nrf_delay_ms(500);
 }
 }
 */

void testFlashMemory(void)
{
    // DeviceIDを確認。0x00c22019 のはず;
    /*
     const uint32_t expected_id = 0x00c22019;
     uint32_t device_id = 0;
     readDeviceID( &device_id);
     if(device_id != expected_id) {
     APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
     }
     */
    
    uint8_t data[128];
    uint8_t rd_buffer[128];
    
    // 128B/256Bの境界確認
    // 4kバイトのセクターをクリアする
    erase4kSector( MX25L25635F_FLASH_SIZE - 256);
    erase4kSector( MX25L25635F_FLASH_SIZE /2 - 256);
    // ランダムデータを用意。
    srand(9);
    for(int i=0; i < sizeof(data); i++) {
        data[i] = (uint8_t) rand();
    }
    // 読み書き
    writeFlash( MX25L25635F_FLASH_SIZE - 256, data, sizeof(data));
    readFlash( MX25L25635F_FLASH_SIZE - 256, rd_buffer, sizeof(rd_buffer));
    if( memcmp(data, rd_buffer, sizeof(data)) != 0) {
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
    // 128MB境界に誤って書き込んでいないか?
    readFlash( MX25L25635F_FLASH_SIZE /2 - 256, rd_buffer, sizeof(rd_buffer));
    if( memcmp(data, rd_buffer, sizeof(data)) == 0) {
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
    
    // 逐次書き込み
    //    const uint32_t max_address = MX25L25635F_FLASH_SIZE;
//    const uint32_t max_address = 10 * 1024; // 簡易に10kBでテスト
    const uint32_t max_address = 1 * 1024 * 1024; //1MB
    srand(1);
    for(uint32_t address = 0x00000000; address < max_address; address += sizeof(data)) {
        // 4kバイトのセクターをクリアする
        if( address % 0x01000 == 0) {
            erase4kSector( address);
        }
        
        // ランダムデータを書き込む
        for(int i=0; i < sizeof(data); i++) {
            data[i] = (uint8_t) rand();
        }
        writeFlash( address, data, sizeof(data));
    }
    
    // 読み込み、比較
    srand(1);
    for(uint32_t address = 0x00000000; address < max_address; address += sizeof(data)) {
        readFlash( address, rd_buffer, sizeof(rd_buffer));
        
        for(int i=0; i < sizeof(data); i++) {
            if( rd_buffer[i] != (uint8_t)rand() ) {
                APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
            }
        }
    }
}


void do_storage_test()
{
//    test01(p_stream);
    testFlashMemory();
}
