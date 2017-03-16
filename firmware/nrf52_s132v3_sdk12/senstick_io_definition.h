#ifndef senstick_io_definition_h
#define senstick_io_definition_h

// 配線(役割)
// IO     In/Out      概要
// 割り込み
// P0.21    In      タクトスイッチ接続。スイッチオンでGNDに落とされる。
// P0.08    In      9軸センサーINT
#define PIN_NUMBER_TACT_SWITCH  21
#define PIN_NUMBER_9AXIS_INT    8

// P0.10    Out     LED。(正論理、HIGHでLED点灯)
#define PIN_NUMBER_LED      10

// P0.05    In      電源電圧。(470k/220k分圧、0.32倍)
#define ADC_INPUT_SUPPLY_MONITORING    NRF_ADC_CONFIG_INPUT_4

// TWI1 - 9軸センサー接続, 高速用TWI
// P0.06            SDA1
// P0.07            SCL1
// P0.09    Out     TWIの電源
#define PIN_NUMBER_TWI1_SDA   6
#define PIN_NUMBER_TWI1_SCL   7
#define PIN_NUMBER_TWI_POWER  9

// TWI2 - その他センサーのTWI
// P0.18            SDA2
// P0.20            SCL2
#define PIN_NUMBER_TWI2_SDA   18
#define PIN_NUMBER_TWI2_SCL   20

// SPI0
// P0.02        /CS
// P0.03        DO
// P0.04        DI
// P0.05        CLK
#define PIN_NUMBER_SPI_nCS  2
#define PIN_NUMBER_SPI_MISO 3
#define PIN_NUMBER_SPI_MOSI 4
#define PIN_NUMBER_SPI_CLK  5

// IOポートのアドレス定義
// I2Cバス上の、スレーブ・アドレス
#define TWI_MPU9250_ADDRESS     0x68 // 01101000 最下位ビットはピンの値で与えられる。この基板はLOW固定。
#define TWI_LPS25HB_ADDRESS     0x5c // 01011100 圧力センサ。最下位ビットはピンの値(SA0)で与えられる。この基板はLOW固定。
#define TWI_SHT20_ADDRESS       0x40 // 01000000 湿度センサ。
#define TWI_VEML6070_ADDRESS    0x38 // UVセンサー。書き込み及び下位バイト読み出しアドレス。
#define TWI_VEML6070_RD_ADDRESS 0x39 // UVセンサー。上位バイト読み出しアドレス。
#define TWI_BH1780GLI_ADDRESS   0x29 // 照度センサー。0010_1001 0x29
#define TWI_AK8963_ADDRESS      0x0c //

#endif /* senstick_io_definition_h */
