#include <string.h>
#include "sensor_data_storage.h"

#define STREAM_SIGNATURE            0xc0ffeea5

// ヘッダの開始バイト位置
#define HEADER_STARTING_POSITION    (sizeof(uint32_t))
// ヘッダのサイズ
#define HEADER_AREA_SIZE            (sizeof(sensor_data_storage_header_t) * 100)
// センサデータすべての合計が30バイト。1バイト当たりの割当容量
#define DATA_STARTING_POSITION      (HEADER_STARTING_POSITION + HEADER_AREA_SIZE)
#define MAX_SAMPLING_COUNT          ((MX25L25635F_FLASH_SIZE - DATA_STARTING_POSITION) / 30)
// センサ種別ごとのデータ書き込み開始位置
const int SENSOR_DATA_STARTING_POSITION[NUMBER_OF_SENSOR_DEVICE +1] = {
    (DATA_STARTING_POSITION),                                       // MotionSensor                    = 0,
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * 18),             // BrightnessSensor                = 1,
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2)),        // UltraVioletSensor               = 2,
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2 +2)),     // HumidityAndTemperatureSensor    = 3,
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2 +2 +4)),  // AirPressureSensor               = 4
    
    (DATA_STARTING_POSITION + MAX_SAMPLING_COUNT * (18 +2 +2 +4 +4))// 終端
};

// ヘッダ情報を読みだす
static void readHeader(flash_memory_context_t *p_flash, int8_t data_id, sensor_data_storage_header_t *p_header)
{
    flashRawRead(p_flash,HEADER_STARTING_POSITION + data_id * sizeof(sensor_data_storage_header_t), (uint8_t *)p_header, sizeof(sensor_data_storage_header_t));
}

// ヘッダ情報を書き込む
static void writeHeader(flash_memory_context_t *p_flash, sensor_data_storage_header_t *p_header)
{
    flashRawWrite(p_flash, HEADER_STARTING_POSITION + p_header->data_id * sizeof(sensor_data_storage_header_t), (uint8_t *)p_header, sizeof(sensor_data_storage_header_t));
}

static int sizeOfSensorData(SensorDeviceType_t type)
{
    switch (type) {
        case MotionSensor:                  return sizeof(MotionSensorData_t);
        case BrightnessSensor:              return sizeof(BrightnessData_t);
        case UltraVioletSensor:             return sizeof(UltraVioletData_t);
        case HumidityAndTemperatureSensor:  return sizeof(HumidityAndTemperatureData_t);
        case AirPressureSensor:             return sizeof(AirPressureData_t);
        default: return 0;
    }
}

bool storageOpen(sensor_data_storage_t *p_storage, int8_t data_id, flash_memory_context_t *p_flash, sensorSetting_t *p_sensor_setting, rtcSettingCommand_t *p_date, char *p_abstract)
{
    p_storage->p_flash = p_flash;
    
    // フォーマット確認、必要な場合はフォーマット
    uint32_t signature;
    flashRawRead(p_flash, 0, (uint8_t *)&signature, sizeof(uint32_t));
    if(signature != STREAM_SIGNATURE) {
        eraseFlash(p_flash);
        flashRawWrite(p_flash, 0, (uint8_t *)&signature, sizeof(uint32_t));
    }

    // もしもdata_idが-1であれば書き込み。最後尾のIDを取得する。
    if(data_id == -1) {
        data_id = storageGetUnitCount(p_storage);
    }
    // パラメータの値チェック
    if(data_id < 0 || data_id >= 100) {
        return false;
    }

    // ヘッダを読み込む
    readHeader(p_flash, data_id, &(p_storage->header));
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
            readHeader(p_flash, data_id -1, &prev_header);
            memcpy(p_storage->header.starting_position, prev_header.end_position, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
        }
        // 終了位位置は開始位置と同じ
        memcpy(p_storage->header.end_position, p_storage->header.starting_position, sizeof(int) * NUMBER_OF_SENSOR_DEVICE);
        // 書き込みなのでメタデータをコピーする
        memcpy(&(p_storage->header.sensor_setting), p_sensor_setting, sizeof(sensorSetting_t));
        memcpy(&(p_storage->header.date), p_date, sizeof(rtcSettingCommand_t));
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
    writeHeader(p_storage->p_flash, &(p_storage->header));
}

int storageWrite(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, const SensorData_t *p_sensorData)
{
    // アドレス, サイズを取得
    int address = p_storage->writing_position[sensorType];
    int size = sizeOfSensorData(sensorType);
    // 最大書き込みサンプル数をチェック
    int sample_count = (address - SENSOR_DATA_STARTING_POSITION[sensorType]) / size;
    if( sample_count > (MAX_SAMPLING_COUNT -1)) {
        return 0;
    }
    
    // 書き込む
    flashRawWrite(p_storage->p_flash, address, (uint8_t *)p_sensorData, size);
    // 次のアドレスを保存
    p_storage->writing_position[sensorType] += size;
    // TBD フラッシュ末尾?
    return size;
}

int storageRead(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, const SensorData_t *p_sensorData)
{
    // アドレス, サイズを取得
    int address = p_storage->reading_position[sensorType];
    int size = sizeOfSensorData(sensorType);
    // 最大サンプル数をチェック
    int sample_count = (address - SENSOR_DATA_STARTING_POSITION[sensorType]) / size;
    if( sample_count > (MAX_SAMPLING_COUNT -1)) {
        return 0;
    }
    
    // 読み込む
    flashRawRead(p_storage->p_flash, address, (uint8_t *)p_sensorData, size);
    // 次のアドレスを保存
    p_storage->reading_position[sensorType] += size;

    return size;
}

int storageGetUnitCount(sensor_data_storage_t *p_storage)
{
    // フォーマットを確認する
    uint32_t signature;
    flashRawRead(p_storage->p_flash, 0, (uint8_t *)&signature, sizeof(uint32_t));
    if(signature != STREAM_SIGNATURE) {
        return 0;
    }
    
    // フォーマットされていたら、ヘッダを読む
    sensor_data_storage_header_t header;
    for(int8_t i = 0; i < 100; i++ ) {
        readHeader(p_storage->p_flash, i, &header);
        if(header.data_id == 0xff) {
            return i;
        }
    }
    return 100;
}

void storageGetSetting(sensor_data_storage_t *p_storage, sensorSetting_t *p_setting)
{
    memcpy(p_setting,&(p_storage->header.sensor_setting), sizeof(sensorSetting_t));
}

void storageGetDateTime(sensor_data_storage_t *p_storage, rtcSettingCommand_t *p_date)
{
    memcpy(p_date, &(p_storage->header.date), sizeof(rtcSettingCommand_t));
}

int8_t storageGetID(sensor_data_storage_t *p_storage)
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
    int start_address = p_storage->header.starting_position[sensorType];
    int end_address   = p_storage->reading_position[sensorType];
    
    return (end_address - start_address);
}

int storageSeekSamplePosition(sensor_data_storage_t *p_storage, SensorDeviceType_t sensorType, int sample_count)
{
    int size = sizeOfSensorData(sensorType);
    int start_address = p_storage->header.starting_position[sensorType];
    // TBD末尾データの位置チェック必要ない?
    p_storage->reading_position[sensorType] = start_address + sample_count * size;
    
    return  p_storage->reading_position[sensorType];
}


