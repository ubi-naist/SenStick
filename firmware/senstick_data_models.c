#include "senstick_data_models.h"
#include "nrf_log.h"

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

void debugLogAccerationData(const AccelerationData_t *data)
{
    NRF_LOG_PRINTF_DEBUG("Accs. x:%d, y:%d, z:%d .\n", data->x, data->y, data->z);
}

