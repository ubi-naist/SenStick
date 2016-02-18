#ifndef sensor_data_storage_h
#define sensor_data_storage_h

#include <stdint.h>
#include <stdbool.h>

#include "spi_slave_mx25_flash_memory.h"

#define NUMBER_OF_SENSOR_DEVICE 5

// ヘッダ情報
typedef struct sensor_data_storage_header_s {
    int8_t  data_id;                                      // データID 0-99
    int     starting_position[NUMBER_OF_SENSOR_DEVICE];   // データ開始ポイント, バイト単位
    int     end_position[NUMBER_OF_SENSOR_DEVICE];        // データ終了ポイント, バイト単位
    char    abstract[21];
    
    sensorSetting_t     sensor_setting;
    rtcSettingCommand_t date;
} sensor_data_storage_header_t;

typedef struct sensor_data_storage_s {
    flash_memory_context_t          *p_flash;
    sensor_data_storage_header_t    header;
    
    bool    can_write;
    int     writing_position[NUMBER_OF_SENSOR_DEVICE];      // 現在書き込み位置
    int     reading_position[NUMBER_OF_SENSOR_DEVICE];      // 現在読み込み位置
} sensor_data_storage_t;

// ストレージを開きます。
// 読みだす時には、データID 0-99を指定します。
// 書き込むときには、IDに -1 を指定します。
bool storageOpen(sensor_data_storage_t *p_storage, int8_t data_id, flash_memory_context_t *p_flash, sensorSetting_t *p_sensor_setting, rtcSettingCommand_t *p_date, char *p_abstract);
void storageClose(sensor_data_storage_t *p_storage);

int storageWrite(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, const SensorData_t *p_sensorData);
int storageRead(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, const SensorData_t *p_sensorData);

// データが記録されているデータユニット数を取得します。
int storageGetUnitCount(sensor_data_storage_t *p_storage);

int storageGetSampleCount(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType);
int storageGetSamplePosition(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType);
int storageSeekSamplePosition(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, int sample_count);

int8_t storageGetID(sensor_data_storage_t *p_storage);
void storageGetSensorSetting(sensor_data_storage_t *p_storage, sensorSetting_t *p_sensor_setting);
void storageGetDate(sensor_data_storage_t *p_storage, rtcSettingCommand_t *p_date);
void storageGetAbstract(sensor_data_storage_t *p_storage, char *p_str);

#endif /* sensor_data_storage_h */
