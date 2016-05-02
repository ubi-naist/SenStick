#ifndef value_types_h
#define value_types_h

#include <stdint.h>
#include <stdbool.h>

// 数値とバイト列との変換

uint16_t readUInt16AsBigEndian(uint8_t *ptr);
int16_t readInt16AsBigEndian(uint8_t *ptr);
uint16_t readUInt16AsLittleEndian(uint8_t *ptr);
int16_t readInt16AsLittleEndian(uint8_t *ptr);
uint32_t readUInt32AsLittleEndian(uint8_t *ptr);

void int16ToByteArrayBigEndian(uint8_t *p_dst, int16_t src);
void int16ToByteArrayLittleEndian(uint8_t *p_dst, int16_t src);

void uint16ToByteArrayBigEndian(uint8_t *p_dst, uint16_t src);
void uint16ToByteArrayLittleEndian(uint8_t *p_dst, uint16_t src);

void uint32ToByteArrayBigEndian(uint8_t *p_dst, uint32_t src);
void uint32ToByteArrayLittleEndian(uint8_t *p_dst, uint32_t src);

#endif /* value_types_h */
