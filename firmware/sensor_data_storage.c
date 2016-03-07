#include <string.h>
#include "nrf_assert.h"
#include "sensor_data_storage.h"

// ヘッダのサイズ
#define HEADER_AREA_SIZE            (sizeof(sensor_data_storage_header_t) * NUMBER_OF_DATA_UNIT)
// センサデータすべての合計が30バイト。1バイト当たりの割当容量
#define DATA_STARTING_POSITION      (HEADER_AREA_SIZE)
#define MAX_SAMPLING_COUNT          ((STORAGe_BYTE_SIZE - DATA_STARTING_POSITION) / 30)

// センサ種別ごとのデータ書き込み開始位置
const int SENSOR_DATA_STARTING_POSITION[NUMBER_OF_SENSOR_DEVICE +1] = {
    (DATA_STARTING_POSITION),                                       // Acceleration = 0
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * 6),              // Gyro
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * 12),             // MagneticField
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * 18),             // BrightnessSensor
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2)),        // UltraVioletSensor
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2 +2)),     // HumidityAndTemperatureSensor
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2 +2 +4)),  // AirPressureSensor
    
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2 +2 +4 +4))// 終端
};

// ヘッダ情報を読みだす
static bool readHeader(flash_stream_context_t *p_stream, uint8_t data_id, sensor_data_storage_header_t *p_header)
{
    uint32_t address = data_id * sizeof(sensor_data_storage_header_t);
    int length = readStream(p_stream, address, (uint8_t *)p_header, sizeof(sensor_data_storage_header_t));
    return (length == sizeof(sensor_data_storage_header_t));
}

// ヘッダ情報を書き込む
static void writeHeader(flash_stream_context_t *p_stream, sensor_data_storage_header_t *p_header)
{
    uint32_t address = p_header->data_id * sizeof(sensor_data_storage_header_t);
    writeStream(p_stream, address, (uint8_t *)p_header, sizeof(sensor_data_storage_header_t));
}

static int sizeOfSensorData(SensorDeviceType_t type)
{
    switch (type) {
        case AccelerationSensor:            return sizeof(AccelerationData_t);
        case GyroSensor:                    return sizeof(RotationRateData_t);
        case MagneticFieldSensor:           return sizeof(MagneticFieldData_t);
        case BrightnessSensor:              return sizeof(BrightnessData_t);
        case UltraVioletSensor:             return sizeof(UltraVioletData_t);
        case HumidityAndTemperatureSensor:  return sizeof(HumidityAndTemperatureData_t);
        case AirPressureSensor:             return sizeof(AirPressureData_t);
        default: return 0;
    }
}

bool storageOpen(sensor_data_storage_t *p_storage, uint8_t data_id, flash_stream_context_t *p_stream, sensorSetting_t *p_sensor_setting, ble_date_time_t *p_date, char *p_abstract)
{
    p_storage->p_stream         = p_stream;
    p_storage->header.data_id   = 0xff;

    // もしもdata_idが 0xff であれば書き込み。最後尾のIDを取得する。
    if(data_id == 0xff) {
        data_id = storageGetUnitCount(p_stream);
    }
    // パラメータの値チェック
    if(data_id >= NUMBER_OF_DATA_UNIT) {
        return false;
    }

    // ヘッダを読み込む
    readHeader(p_stream, data_id, &(p_storage->header));
    // まだ書き込まれていない領域であれば、データIDが0xff
    p_storage->can_write = (p_storage->header.data_id == 0xff);
    // 書き込み開始位置、初期値設定
    if(p_storage->can_write) {
        // IDを設定
        p_storage->header.data_id = data_id;
        // アブストラクトを書き込み
        strncpy(p_storage->header.abstract, p_abstract, 20);
        
        if(data_id == 0) {
            memcpy(p_storage->header.starting_position, SENSOR_DATA_STARTING_POSITION, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
        } else {
            // もしも前のブロックがあるなら、そのデータを読み込む
            sensor_data_storage_header_t prev_header;
            readHeader(p_stream, data_id -1, &prev_header);
            memcpy(p_storage->header.starting_position, prev_header.end_position, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
        }
        // 終了位位置は開始位置と同じ
        memcpy(p_storage->header.end_position, p_storage->header.starting_position, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
        // 書き込みなのでメタデータをコピーする
        p_storage->header.sensor_setting = *p_sensor_setting;
        p_storage->header.date = *p_date;
    } else {
        // 書き込み不可能なので、ヘッダのstarting/ending positiionは有効なデータ範囲を示している。設定する必要はない。
    }
    // 書き込み/読み込み位置を設定する
    memcpy(p_storage->writing_position, p_storage->header.starting_position, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
    memcpy(p_storage->reading_position, p_storage->header.starting_position, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
    
    return true;
}

void storageClose(sensor_data_storage_t *p_storage)
{
    // 読み込みのみなら、終了
    if( ! p_storage->can_write) {
        return;
    }
    //書き込み位置をヘッダにコピー
    memcpy(p_storage->header.end_position, p_storage->writing_position, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
    //ヘッダを書き込む
    writeHeader(p_storage->p_stream, &(p_storage->header));
    //書き込み可能フラグを落とす
    p_storage->can_write = false;
}

int storageWrite(sensor_data_storage_t *p_storage, const SensorData_t *p_sensorData)
{
    ASSERT(p_storage->can_write);
    ASSERT(p_sensorData->type < SensorDeviceNone);
    
    // アドレス, サイズを取得
    int address = p_storage->writing_position[p_sensorData->type];
    int size = sizeOfSensorData(p_sensorData->type);
    // 最大書き込みサンプル数をチェック
    int sample_count = (address - SENSOR_DATA_STARTING_POSITION[p_sensorData->type]) / size;
    if( sample_count > (MAX_SAMPLING_COUNT -1)) {
        return 0;
    }
    
    // 書き込む
    writeStream(p_storage->p_stream, address, (uint8_t *)p_sensorData, size);
    // 次のアドレスを保存
    p_storage->writing_position[p_sensorData->type] += size;
    // TBD フラッシュ末尾?
    return size;
}

int storageRead(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, SensorData_t *p_sensorData)
{
    ASSERT(sensorType < SensorDeviceNone);
    
    // タイプを書き込み
    p_sensorData->type = sensorType;

    // アドレス, サイズを取得
    int address = p_storage->reading_position[sensorType];
    int size = sizeOfSensorData(sensorType);
    // 最大サンプル数をチェック
    int sample_count = (address - SENSOR_DATA_STARTING_POSITION[sensorType]) / size;
    if( sample_count > (MAX_SAMPLING_COUNT -1)) {
        return 0;
    }
    
    // 読み込む
    readStream(p_storage->p_stream, address, (uint8_t *)p_sensorData, size);
    // 次のアドレスを保存
    p_storage->reading_position[sensorType] += size;

    return size;
}

int storageGetUnitCount(flash_stream_context_t *p_stream)
{
    // フォーマットされていたら、ヘッダを読む
    sensor_data_storage_header_t header;
    for(int i = 0; i < NUMBER_OF_DATA_UNIT; i++ ) {
        int length = readHeader(p_stream, i, &header);
        if(length == 0 || header.data_id == 0xff) {
            return i;
        }
    }
    return NUMBER_OF_DATA_UNIT;
}

void storageGetRemainingCapacity(flash_stream_context_t *p_stream, uint8_t *p_num_of_unit, uint8_t *p_data)
{
    // 初期値
    *p_num_of_unit = NUMBER_OF_DATA_UNIT;
    memset(p_data, 100, NUMBER_OF_SENSOR_DEVICE);

    // フォーマットされていたら、ヘッダを読む
    sensor_data_storage_header_t header;
    int unit = -1;
    for(int i = 0; i < NUMBER_OF_DATA_UNIT; i++ ) {
        int length = readHeader(p_stream, i, &header);
        if(length == 0 || header.data_id == 0xff) {
            break;
        }
        unit = i;
    }
    // 何も記録されていない
    if(unit < 0 ) {
        return;
    }
    // 値を設定
    *p_num_of_unit = NUMBER_OF_DATA_UNIT - unit -1;
    for(int i=0; i < NUMBER_OF_SENSOR_DEVICE; i++) {
        int remaining_byte_capacity = (SENSOR_DATA_STARTING_POSITION[i +1] - header.end_position[i]);
        int remaining_count = remaining_byte_capacity / sizeOfSensorData((SensorDeviceType_t)i);
        int remaining_capacity_percent = (100 * remaining_count) / MAX_SAMPLING_COUNT;
        remaining_capacity_percent = MIN(100, remaining_capacity_percent);
        p_data[i] = (uint8_t) remaining_capacity_percent;
    }
}

void storageGetSetting(sensor_data_storage_t *p_storage, sensorSetting_t *p_setting)
{
    *p_setting = p_storage->header.sensor_setting;
}

void storageGetDateTime(sensor_data_storage_t *p_storage, ble_date_time_t *p_date)
{
    *p_date = p_storage->header.date;
}

uint8_t storageGetID(sensor_data_storage_t *p_storage)
{
    return p_storage->header.data_id;
}

void storageGetAbstract(sensor_data_storage_t *p_storage, char *p_str)
{
    strncpy(p_str, p_storage->header.abstract, 20);
}

int storageGetSampleCount(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType)
{
    int size = sizeOfSensorData(sensorType);
    int start_address = p_storage->header.starting_position[sensorType];
    int end_address   = p_storage->can_write ? p_storage->writing_position[sensorType] : p_storage->header.end_position[sensorType];

    return (end_address - start_address) / size;
}

int storageGetSamplePosition(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType)
{
    int size = sizeOfSensorData(sensorType);
    int start_address = p_storage->header.starting_position[sensorType];
    int end_address   = p_storage->reading_position[sensorType];
    
    return (end_address - start_address) / size;
}

int storageSeekSamplePosition(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, int sample_count)
{
    int size = sizeOfSensorData(sensorType);
    int start_address = p_storage->header.starting_position[sensorType];
    // TBD末尾データの位置チェック必要ない?
    p_storage->reading_position[sensorType] = start_address + sample_count * size;
    
    return  p_storage->reading_position[sensorType];
}


