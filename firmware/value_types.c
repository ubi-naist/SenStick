#include "value_types.h"

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

void uint32ToByteArrayLittleEndian(uint8_t *p_dst, uint32_t src)
{
    p_dst[0] = (0x0ff & (src >> 0));
    p_dst[1] = (0x0ff & (src >> 8));
    p_dst[2] = (0x0ff & (src >> 16));
    p_dst[3] = (0x0ff & (src >> 24));
}
