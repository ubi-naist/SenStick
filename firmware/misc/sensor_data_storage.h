#ifndef sensor_data_storage_h
#define sensor_data_storage_h

#include <stdint.h>
#include <stdbool.h>

#include <sdk_common.h>
#include <nrf.h>
#include <app_util.h>
#include <ble_date_time.h>

#include "flash_stream.h"

// データユニットの数
#define NUMBER_OF_DATA_UNIT         100

// センサデバイスの数
#define NUMBER_OF_SENSOR_DEVICE     7

// ヘッダ情報
typedef struct sensor_data_storage_header_s {
    uint8_t data_id;                                      // データID 0-99
    int     starting_position[NUMBER_OF_SENSOR_DEVICE];   // データ開始ポイント, バイト単位
    int     end_position[NUMBER_OF_SENSOR_DEVICE];        // データ終了ポイント, バイト単位
    char    abstract[21];
    
    sensorSetting_t     sensor_setting;
    ble_date_time_t date;
} sensor_data_storage_header_t;

typedef struct {
    flash_stream_context_t      *p_stream;
    sensor_data_storage_header_t header;
    
    bool    can_write;
    int     writing_position[NUMBER_OF_SENSOR_DEVICE];      // 現在書き込み位置
    int     reading_position[NUMBER_OF_SENSOR_DEVICE];      // 現在読み込み位置
} sensor_data_storage_t;
                   
// ストレージを開きます。
// 読みだす時には、データID 0-99を指定します。
// 書き込むときには、IDに -1 を指定します。
bool storageOpen(sensor_data_storage_t *p_storage, uint8_t data_id, flash_stream_context_t *p_stream, sensorSetting_t *p_sensor_setting, ble_date_time_t *p_date, char *p_abstract);
void storageClose(sensor_data_storage_t *p_storage);

int storageWrite(sensor_data_storage_t *p_storage, const SensorData_t *p_sensorData);
int storageRead(sensor_data_storage_t *p_storage, sensor_device_t_t sensorType, SensorData_t *p_sensorData);

// データが記録されているデータユニット数を取得します。
int storageGetUnitCount(flash_stream_context_t *p_stream);
// データユニット数と残容量を0-100%で返します。加速度、角速度、磁場、照度、UV、気圧、湿度温度、の順番。
void storageGetRemainingCapacity(flash_stream_context_t *p_stream, uint8_t *p_num_of_unit, uint8_t *p_data);

int storageGetSampleCount(sensor_data_storage_t *p_storage, sensor_device_t_t sensorType);
int storageGetSamplePosition(sensor_data_storage_t *p_storage, sensor_device_t_t sensorType);
int storageSeekSamplePosition(sensor_data_storage_t *p_storage, sensor_device_t_t sensorType, int sample_count);

uint8_t storageGetID(sensor_data_storage_t *p_storage);
void storageGetSensorSetting(sensor_data_storage_t *p_storage, sensorSetting_t *p_sensor_setting);
void storageGetDate(sensor_data_storage_t *p_storage, ble_date_time_t *p_date);
void storageGetAbstract(sensor_data_storage_t *p_storage, char *p_str);

#endif /* sensor_data_storage_h */
