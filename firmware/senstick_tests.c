#include "senstick_tests.h"

#include <string.h>
#include <stdlib.h>

#include "app_error.h"

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

void testFlashMemory(flash_memory_context_t *p_context)
{
    // DeviceIDを確認。0x00c22019 のはず;
    /*
    const uint32_t expected_id = 0x00c22019;
    uint32_t device_id = 0;
    readDeviceID(p_context, &device_id);
    if(device_id != expected_id) {
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
    */
    
    uint8_t data[128];
    uint8_t rd_buffer[128];
    
    
    // 128B/256Bの境界確認
    // 4kバイトのセクターをクリアする
    eraseSector(p_context, MX25L25635F_FLASH_SIZE - 256);
    eraseSector(p_context, MX25L25635F_FLASH_SIZE /2 - 256);
    // ランダムデータを用意。
    srand(9);
    for(int i=0; i < sizeof(data); i++) {
        data[i] = (uint8_t) rand();
    }
    // 読み書き
    writeFlash(p_context, MX25L25635F_FLASH_SIZE - 256, data, sizeof(data));
    readFlash(p_context, MX25L25635F_FLASH_SIZE - 256, rd_buffer, sizeof(rd_buffer));
    if( memcmp(data, rd_buffer, sizeof(data)) != 0) {
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
    // 128MB境界に誤って書き込んでいないか?
    readFlash(p_context, MX25L25635F_FLASH_SIZE /2 - 256, rd_buffer, sizeof(rd_buffer));
    if( memcmp(data, rd_buffer, sizeof(data)) == 0) {
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
    
    // 逐次書き込み
    //    const uint32_t max_address = MX25L25635F_FLASH_SIZE;
    const uint32_t max_address = 10 * 1024; // 簡易に10kBでテスト
    srand(1);
    for(uint32_t address = 0x00000000; address < max_address; address += sizeof(data)) {
        // 4kバイトのセクターをクリアする
        if( address % 0x01000 == 0) {
            eraseSector(p_context, address);
        }
        
        // ランダムデータを書き込む
        for(int i=0; i < sizeof(data); i++) {
            data[i] = (uint8_t) rand();
        }
        writeFlash(p_context, address, data, sizeof(data));
    }
    
    // 読み込み、比較
    srand(1);
    for(uint32_t address = 0x00000000; address < max_address; address += sizeof(data)) {
        readFlash(p_context, address, rd_buffer, sizeof(rd_buffer));
        
        for(int i=0; i < sizeof(data); i++) {
            if( rd_buffer[i] != (uint8_t)rand() ) {
                APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
            }
        }
    }
}

