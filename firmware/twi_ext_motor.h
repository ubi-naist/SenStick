//
//  twi_ext_motor.h
//  senstick
//
//  Created by AkihiroUehara on 2017/02/23.
//
//

#ifndef twi_ext_motor_h
#define twi_ext_motor_h

#include <stdint.h>
#include <stdbool.h>
#include "senstick_sensor_base_data.h"


// 初期化関数。使用開始前に必ずこの関数を呼出ます。
// 初期化に失敗した場合は、falseが返ります。
// 初期化に失敗した場合に、DRV8830にアクセスする、その他の関数を呼び出したときの振る舞いは保証しません。
bool initDRV8830MotorDriver(void);

// controlレジスタを設定します。
void setDRV8830MotorDriverControlRegister(uint8_t value);

// faultレジスタを読み出します。
uint8_t getDRV8830MotorDriverFaultRegister(void);
// faultレジスタの値をクリアします。
void clearDRV8830MotorDriverFaultRegister(void);

#endif /* twi_ext_motor_h */



