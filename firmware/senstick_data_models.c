#include "senstick_data_models.h"
#include "nrf_log.h"
#include "app_error.h"

uint16_t readUInt16AsBigEndian(uint8_t *ptr)
{
    return ((uint16_t)ptr[0] << 8) | (uint16_t)ptr[1];
}

uint16_t readUInt16AsLittleEndian(uint8_t *ptr)
{
    return ((uint16_t)ptr[0] << 0) | ((uint16_t)ptr[1] << 8);
}

uint32_t readUInt32AsLittleEndian(uint8_t *ptr)
{
    return ((uint32_t)ptr[0] << 0) |  ((uint32_t)ptr[1] << 8) | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
}

void int16ToByteArrayBigEndian(uint8_t *p_dst, int16_t src)
{
    p_dst[0] = (0x0ff & (src >> 8));
    p_dst[1] = (0x0ff & (src >> 0));
}

void int16ToByteArrayLittleEndian(uint8_t *p_dst, int16_t src)
{
    p_dst[0] = (0x0ff & (src >> 0));
    p_dst[1] = (0x0ff & (src >> 8));
}

void uint16ToByteArrafyBigEndian(uint8_t *p_dst, uint16_t src)
{
    p_dst[0] = (uint8_t)(0x0ff & (src >> 8));
    p_dst[1] = (uint8_t)(0x0ff & (src >> 0));
}

void uint32ToByteArrayBigEndian(uint8_t *p_dst, uint32_t src)
{
    p_dst[0] = (0x0ff & (src >> 24));
    p_dst[1] = (0x0ff & (src >> 16));
    p_dst[2] = (0x0ff & (src >>  8));
    p_dst[3] = (0x0ff & (src >>  0));
}

bool setSensorSettingPeriod(sensorSetting_t *p_setting, SensorDeviceType_t device_type, int period)
{
    bool isChanged = false;
    period = (period / 10) * 10;
    switch ( device_type) {
        case MotionSensor:
            if(period >= 10 && p_setting->motionSensorSamplingPeriod != period) {
                p_setting->motionSensorSamplingPeriod = period;
                isChanged = true;
            }
            break;
            
        case BrightnessSensor:
            if(period >= 200 && p_setting->brightnessSamplingPeriod != period) {
                p_setting->brightnessSamplingPeriod = period;
                isChanged = true;
            }
            break;
            
        case UltraVioletSensor:
            if(period >= 300 && p_setting->ultraVioletSamplingPeriod != period) {
                p_setting->ultraVioletSamplingPeriod = period;
                isChanged = true;
            }
            break;
            
        case TemperatureAndHumiditySensor:
            if(period >= 100 && p_setting->temperatureAndHumiditySamplingPeriod != period) {
                p_setting->temperatureAndHumiditySamplingPeriod = period;
                isChanged = true;
            }
            break;
            
        case AirPressureSensor:
            if(period >= 100 && p_setting->airPressureSamplingPeriod != period) {
                p_setting->airPressureSamplingPeriod = period;
                isChanged = true;
            }
            break;

        default:
            APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
            break;
    }
    return isChanged;
}

// Byte
// 0    Reserved
// 1    Acceleration sensor config, range.
//          0x00: 2G (default)
//          0x01: 4G
//          0x02: 8G
//          0x03: 16G
// 2    Cyro sensor config, range.
//          0x00 250  dps (default)
//          0x01 500  dps
//          0x02 1000 dps
//          0x03 2000 dps
// 3    Nine axes sensor sampling rate.
//          0x00    0.1 sample/sec
//          0x01    1   sample/sec
//          0x02    10  sample/sec (default)
// 4    THA sensor sampling rage.
//          0x00    0.1 sample/sec (default)
//          0x01    1   sample/sec
//          0x02    10  sample/sec
// 5    HPA sensor sampling rage.
//          0x00    0.1 sample/sec
//          0x01    1   sample/sec
//          0x02    10  sample/sec  (default)
// 6    Illumination sensor sampling rage.
//          0x00    0.1 sample/sec
//          0x01    1   sample/sec (default)
//          0x02    10  sample/sec
// センサー設定をバイナリ配列に展開します。このバイナリ配列はGATTの設定キャラクタリスティクスにそのまま使用できます。 バイト配列は7バイトの領域が確保されていなければなりません。
void serializeSensorSetting(uint8_t *p_dst, const sensorSetting_t *p_setting)
{
    p_dst[0] = 0;
    p_dst[1] = (uint8_t)p_setting->accelerationRange;
    p_dst[2] = (uint8_t)p_setting->rotationRange;
    p_dst[3] = (uint8_t)p_setting->nineAxesSensorSamplingRate;
    p_dst[4] = (uint8_t)p_setting->humiditySensorSamplingRate;
    p_dst[5] = (uint8_t)p_setting->pressureSensorSamplingRate;
    p_dst[6] = (uint8_t)p_setting->illuminationSensorSamplingRate;
}

// バイト配列をセンサー設定情報に展開します。不正な値があった場合は、処理は完了せず、falseを返します。 バイト配列は7バイトの領域が確保されていなければなりません。
bool deserializeSensorSetting(sensorSetting_t *p_setting, uint8_t *p_src )
{
    switch (p_src[1]) {
        case ACCELERATION_RANGE_2G:
        case ACCELERATION_RANGE_4G:
        case ACCELERATION_RANGE_8G:
        case ACCELERATION_RANGE_16G:
            p_setting->accelerationRange = (AccelerationRange_t)p_src[1];
            break;
        default:
            return false;
    }
    
    switch (p_src[2]) {
        case ROTATION_RANGE_250DPS:
        case ROTATION_RANGE_500DPS:
        case ROTATION_RANGE_1000DPS:
        case ROTATION_RANGE_2000DPS:
            p_setting->rotationRange = (RotationRange_t)p_src[2];
            break;
        default:
            return false;
    }

    // レートの項目全てについて、値範囲を確認
    for(int i=3; i<7; i++) {
        // 0x02よりも大きな値は不正値。
        if( p_src[i] > 0x02 ) {
            return false;
        }
    }
    
    p_setting->nineAxesSensorSamplingRate       = (SamplingRate_t)p_src[3];
    p_setting->humiditySensorSamplingRate       = (SamplingRate_t)p_src[4];
    p_setting->pressureSensorSamplingRate       = (SamplingRate_t)p_src[5];
    p_setting->illuminationSensorSamplingRate   = (SamplingRate_t)p_src[6];
    
    return true;
}

void serializeAccelerationData(uint8_t *p_dst, const AccelerationData_t *p_data)
{
    int16ToByteArrayBigEndian(&(p_dst[0]), p_data->x);
    int16ToByteArrayBigEndian(&(p_dst[2]), p_data->y);
    int16ToByteArrayBigEndian(&(p_dst[4]), p_data->z);
}

void serializeRotationRateData(uint8_t *p_dst, const RotationRateData_t *p_data)
{
    int16ToByteArrayBigEndian(&(p_dst[0]), p_data->x);
    int16ToByteArrayBigEndian(&(p_dst[2]), p_data->y);
    int16ToByteArrayBigEndian(&(p_dst[4]), p_data->z);
}

void serializeMagneticFieldData(uint8_t *p_dst, const MagneticFieldData_t *p_data)
{
    int16ToByteArrayBigEndian(&(p_dst[0]), p_data->x);
    int16ToByteArrayBigEndian(&(p_dst[2]), p_data->y);
    int16ToByteArrayBigEndian(&(p_dst[4]), p_data->z);
}


void serializeMotionData(uint8_t *p_dst, const MotionSensorData_t *p_data)
{
    serializeAccelerationData(&(p_dst[0]),   &(p_data->acceleration));
    serializeRotationRateData(&(p_dst[6]),   &(p_data->rotaionRate));
    serializeMagneticFieldData(&(p_dst[12]), &(p_data->magneticField));
}

void serializeBrightnessData(uint8_t *p_dst, const BrightnessData_t *p_data)
{
    uint16ToByteArrayBigEndian(p_dst, *p_data);
}

void serializeTemperatureAndHumidityData(uint8_t *p_dst, const TemperatureAndHumidityData_t *p_data)
{
    uint16ToByteArrayBigEndian(&(p_dst[0]), p_data->temperature);
    uint16ToByteArrayBigEndian(&(p_dst[2]), p_data->humidity);
}

void serializeAirPressureData(uint8_t *p_dst, const AirPressureData_t *p_data)
{
    uint32_t src = *p_data;
    
    p_dst[0] = (uint8_t)(0x0ff & (src >> 16));
    p_dst[1] = (uint8_t)(0x0ff & (src >> 8));
    p_dst[2] = (uint8_t)(0x0ff & (src >> 0));
}

void debugLogAccerationData(const AccelerationData_t *data)
{
    NRF_LOG_PRINTF_DEBUG("Accs. x:%d, y:%d, z:%d .\n", data->x, data->y, data->z);
}

