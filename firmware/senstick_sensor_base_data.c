#include <sdk_errors.h>
#include <app_error.h>

#include "senstick_sensor_base_data.h"

#include "value_types.h"

uint8_t serializesensor_service_setting(uint8_t *p_dst, sensor_service_setting_t *p_src)
{
    p_dst[0] = p_src->command;
    uint16ToByteArrayLittleEndian(&p_dst[1], p_src->samplingDuration);
    uint16ToByteArrayLittleEndian(&p_dst[3], p_src->measurementRange);
    
    return 5;
}

void deserializesensor_service_setting(sensor_service_setting_t *p_dst, uint8_t *p_src)
{
    p_dst->command = (sensor_service_command_t) p_src[0];
    p_dst->samplingDuration = readUInt16AsLittleEndian(&p_src[1]);
    p_dst->measurementRange = readUInt16AsLittleEndian(&p_src[3]);
}

uint8_t serializeSensorServiceLogID(uint8_t *p_dst, sensor_service_logID_t *p_src)
{
    p_dst[0] = p_src->logID;
    uint16ToByteArrayLittleEndian(&p_dst[1], p_src->skipCount);
    uint32ToByteArrayLittleEndian(&p_dst[3], p_src->position);
    return 7;
}

void deserializeSensorServiceLogID(sensor_service_logID_t *p_dst, uint8_t *p_src)
{
    p_dst->logID     = p_src[0];
    p_dst->skipCount = readUInt16AsLittleEndian(&p_src[1]);
    p_dst->position  = readUInt32AsLittleEndian(&p_src[3]);
}

uint8_t serializeSensorMetaData(uint8_t *p_dst, sensor_metadata_t *p_src)
{
    uint16ToByteArrayLittleEndian(&p_dst[0], p_src->samplingDuration);
    uint16ToByteArrayLittleEndian(&p_dst[2], p_src->measurementRange);
    uint32ToByteArrayLittleEndian(&p_dst[4], p_src->sampleCount);
    uint32ToByteArrayLittleEndian(&p_dst[8], p_src->position);
    p_dst[12] = p_src->remainingStorage;
    
    return 13;
}

void deserializeSensorMetaData(sensor_metadata_t *p_dst, uint8_t *p_src)
{
    p_dst->samplingDuration = readUInt16AsLittleEndian(&p_src[0]);
    p_dst->measurementRange = readUInt16AsLittleEndian(&p_src[2]);
    p_dst->sampleCount      = readUInt32AsLittleEndian(&p_src[4]);
    p_dst->position         = readUInt32AsLittleEndian(&p_src[8]);
    p_dst->remainingStorage = p_src[12];
}

