#include <nordic_common.h>
#include <nrf_log.h>
#include <app_error.h>


#include "senstick_data_models.h"

uint16_t readUInt16AsBigEndian(uint8_t *ptr)
{
    return ((uint16_t)ptr[0] << 8) | ptr[1];
}

int16_t readInt16AsBigEndian(uint8_t *ptr)
{
    return ((int16_t)ptr[0] << 8) | ptr[1];
}

uint16_t readUInt16AsLittleEndian(uint8_t *ptr)
{
    return ptr[0] | ((uint16_t)ptr[1] << 8);
}

int16_t readInt16AsLittleEndian(uint8_t *ptr)
{
    return ptr[0] | ((int16_t)ptr[1] << 8);
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

void uint16ToByteArrayLittleEndian(uint8_t *p_dst, uint16_t src)
{
    p_dst[0] = (0x0ff & (src >> 0));
    p_dst[1] = (0x0ff & (src >> 8));
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
        case AccelerationSensor:
            if(period >= 10 && p_setting->accelerationSamplingPeriod != period) {
                p_setting->accelerationSamplingPeriod = period;
                isChanged = true;
            }
            break;
            
        case GyroSensor:
            if(period >= 10 && p_setting->gyroSamplingPeriod != period) {
                p_setting->gyroSamplingPeriod = period;
                isChanged = true;
            }
            break;
            
        case MagneticFieldSensor:
            if(period >= 10 && p_setting->magneticFieldSamplingPeriod != period) {
                p_setting->magneticFieldSamplingPeriod = period;
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
            
        case HumidityAndTemperatureSensor:
            if(period >= 100 && p_setting->humidityAndTemperatureSamplingPeriod != period) {
                p_setting->humidityAndTemperatureSamplingPeriod = period;
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
// 0    Acceleration sensor config, range.
//          0x00: 2G (default)
//          0x01: 4G
//          0x02: 8G
//          0x03: 16G
// 1    Cyro sensor config, range.
//          0x00 250  dps (default)
//          0x01 500  dps
//          0x02 1000 dps
//          0x03 2000 dps
//
// 符号なし16-bit整数、リトルエンディアン
// 2:3      加速度サンプリングレート   (ミリ秒)
// 4:5      ジャイロサンプリングレート (ミリ秒)
// 6:7      磁気サンプリングレート
// 8:9      湿度温度サンプリングレート
// 10:11    気圧サンプリングレート
// 12:13    輝度サンプリングレート
// 14:15    UVサンプリングレート
uint8_t serializeSensorSetting(uint8_t *p_dst, const sensorSetting_t *p_setting)
{
    p_dst[0] = (uint8_t)p_setting->accelerationRange;
    p_dst[1] = (uint8_t)p_setting->rotationRange;
    
    uint16ToByteArrayLittleEndian(&p_dst[2], p_setting->accelerationSamplingPeriod);
    uint16ToByteArrayLittleEndian(&p_dst[4], p_setting->gyroSamplingPeriod);
    uint16ToByteArrayLittleEndian(&p_dst[6], p_setting->magneticFieldSamplingPeriod);
    uint16ToByteArrayLittleEndian(&p_dst[8], p_setting->humidityAndTemperatureSamplingPeriod);
    uint16ToByteArrayLittleEndian(&p_dst[10], p_setting->airPressureSamplingPeriod);
    uint16ToByteArrayLittleEndian(&p_dst[12], p_setting->brightnessSamplingPeriod);
    uint16ToByteArrayLittleEndian(&p_dst[14], p_setting->ultraVioletSamplingPeriod);
    
    return 16;
}

static uint16_t readPeriod(uint8_t *p_src)
{
    uint16_t value = readUInt16AsLittleEndian(p_src);
    value = MIN(10, (value / 10) * 10); // 10以上、10ミリ秒の倍数に。
    return value;
}

void deserializeSensorSetting(sensorSetting_t *p_setting, uint8_t *p_src)
{
    if((uint8_t)p_src[0] <= (uint8_t)ACCELERATION_RANGE_16G) {
        p_setting->accelerationRange = (AccelerationRange_t)p_src[0];
    }
    if((uint8_t)p_src[1] <= (uint8_t)ROTATION_RANGE_2000DPS) {
        p_setting->rotationRange = (RotationRange_t)p_src[1];
    }
    
    p_setting->accelerationSamplingPeriod       = readPeriod(&p_src[2]);
    p_setting->gyroSamplingPeriod               = readPeriod(&p_src[4]);
    p_setting->magneticFieldSamplingPeriod      = readPeriod(&p_src[6]);
    p_setting->humidityAndTemperatureSamplingPeriod  = readPeriod(&p_src[8]);
    p_setting->airPressureSamplingPeriod         = readPeriod(&p_src[10]);
    p_setting->brightnessSamplingPeriod          = readPeriod(&p_src[12]);
    p_setting->ultraVioletSamplingPeriod         = readPeriod(&p_src[14]);
}

uint8_t serializeSensorData(uint8_t *p_dst, const SensorData_t *p_data)
{
    switch (p_data->type) {
        case AccelerationSensor:
            p_dst[0] = (uint8_t)p_data->type;
            int16ToByteArrayLittleEndian(&p_dst[1], p_data->data.acceleration.x);
            int16ToByteArrayLittleEndian(&p_dst[3], p_data->data.acceleration.y);
            int16ToByteArrayLittleEndian(&p_dst[5], p_data->data.acceleration.z);
            return (1 + 2*3);
            
        case GyroSensor:
            p_dst[0] = (uint8_t)p_data->type;
            int16ToByteArrayLittleEndian(&p_dst[1], p_data->data.rotationRate.x);
            int16ToByteArrayLittleEndian(&p_dst[3], p_data->data.rotationRate.y);
            int16ToByteArrayLittleEndian(&p_dst[5], p_data->data.rotationRate.z);
            return (1 + 2*3);
            
        case MagneticFieldSensor:
            p_dst[0] = (uint8_t)p_data->type;
            int16ToByteArrayLittleEndian(&p_dst[1], p_data->data.magneticField.x);
            int16ToByteArrayLittleEndian(&p_dst[3], p_data->data.magneticField.y);
            int16ToByteArrayLittleEndian(&p_dst[5], p_data->data.magneticField.z);
            return (1 + 2*3);
            
        case BrightnessSensor:
            p_dst[0] = (uint8_t)p_data->type;
            uint16ToByteArrayLittleEndian(&p_dst[1], p_data->data.brightness);
            return (1+ 2);
            
        case UltraVioletSensor:
            p_dst[0] = (uint8_t)p_data->type;
            uint16ToByteArrayLittleEndian(&p_dst[1], p_data->data.ultraViolet);
            return (1+ 2);
            
        case HumidityAndTemperatureSensor:
            p_dst[0] = (uint8_t)p_data->type;
            uint16ToByteArrayLittleEndian(&p_dst[1], p_data->data.humidityAndTemperature.humidity);
            uint16ToByteArrayLittleEndian(&p_dst[3], p_data->data.humidityAndTemperature.temperature);
            return (1+ 2 *2);
            
        case AirPressureSensor:
            p_dst[0] = (uint8_t)p_data->type;
            uint32ToByteArrayBigEndian(&p_dst[1], p_data->data.airPressure);
            return (1+ 4);
        default:
            return 0;
    }
}

// センサー設定をバイナリ配列に展開します。このバイナリ配列はGATTの設定キャラクタリスティクスにそのまま使用できます。 バイト配列は7バイトの領域が確保されていなければなりません。
/*


// バイト配列をセンサー設定情報に展開します。不正な値があった場合は、処理は完了せず、falseを返します。 バイト配列は7バイトの領域が確保されていなければなりません。


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
    serializeRotationRateData(&(p_dst[6]),   &(p_data->rotationRate));
    serializeMagneticFieldData(&(p_dst[12]), &(p_data->magneticField));
}

void serializeBrightnessData(uint8_t *p_dst, const BrightnessData_t *p_data)
{
    uint16ToByteArrayBigEndian(p_dst, *p_data);
}

void serializeHumidityAndTemperatureData(uint8_t *p_dst, const HumidityAndTemperatureData_t *p_data)
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
*/
void debugLogAccerationData(const AccelerationData_t *data)
{
    NRF_LOG_PRINTF_DEBUG("Accs. x:%d, y:%d, z:%d .\n", data->x, data->y, data->z);
}

