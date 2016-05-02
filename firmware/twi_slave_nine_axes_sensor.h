#ifndef twi_slave_nine_axes_sensor_h
#define twi_slave_nine_axes_sensor_h

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"

// 16ビット 符号付き数値。フルスケールは設定レンジ値による。2, 4, 8, 16G。
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} AccelerationData_t;

// ジャイロのデータ構造体
// 16ビット 符号付き数値。フルスケールは設定レンジ値による。250, 500, 1000, 2000 DPS。
typedef struct RotationRateData_s {
    int16_t x;
    int16_t y;
    int16_t z;
} RotationRateData_t;

// 磁界のデータ構造体
// 16-bit 符号付き数字 フルスケール f ±4800 μT
typedef struct MagneticFieldData_s {
    int16_t x;
    int16_t y;
    int16_t z;
} MagneticFieldData_t;

// 加速度センサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
typedef enum {
    ACCELERATION_RANGE_2G   = 0x00, // +- 2g
    ACCELERATION_RANGE_4G   = 0x01, // +- 4g
    ACCELERATION_RANGE_8G   = 0x02, // +- 8g
    ACCELERATION_RANGE_16G  = 0x03, // +- 16g
} AccelerationRange_t;

// ジャイロセンサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
typedef enum {
    ROTATION_RANGE_250DPS   = 0x00,
    ROTATION_RANGE_500DPS   = 0x01,
    ROTATION_RANGE_1000DPS  = 0x02,
    ROTATION_RANGE_2000DPS  = 0x03,
} RotationRange_t;

// 初期化関数。センサ使用前に必ずこの関数を呼出ます。初期化に成功すればtrueを返します。
bool initNineAxesSensor(void);

void sleepNineAxesSensor(void);
void awakeNineAxesSensor(void);

void setNineAxesSensorAccelerationRange(AccelerationRange_t range);
void setNineAxesSensorRotationRange(RotationRange_t range);

void getAccelerationData(uint8_t *p_data);
void getRotationRateData(uint8_t *p_data);
void getMagneticFieldData(uint8_t *p_data);

#endif /* twi_slave_nine_axes_sensor_h */
