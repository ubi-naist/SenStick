#include "senstick_data_models.h"

uint16_t readUInt16AsBigEndian(uint8_t *ptr)
{
    return ((uint16_t)ptr[0] << 8) | (uint16_t)ptr[1];
}

uint16_t readUInt16AsLittleEndian(uint8_t *ptr)
{
    return ((uint16_t)ptr[0] << 0) | ((uint16_t)ptr[1] << 8);
}

// 加速度の16ビット整数をGに変換します。1G=16384。
static float accelerationFloatValue(int16_t value)
{
    return (float)value / 16384f;
}
void debugLogAccerationData(const AccelerationData_t *data)
{
    NRF_LOG_PRINTF_DEBUG("%Accs. x:%1.2f, y:%1.2f, z:%1.2f.\n",
                         accelerationFloatValue(data->x),
                         accelerationFloatValue(data->y),
                         accelerationFloatValue(data->z)
                         );
}

