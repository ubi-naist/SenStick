#ifndef senstick_pin_definitions_h
#define senstick_pin_definitions_h

// 配線(役割)
// IO     In/Out      概要
// 割り込み
// P0.20    In      タクトスイッチ接続。スイッチオンでGNDに落とされる。
// P0.08    In      9軸センサーINT
// P0.09    In      RTC INTIR
#define PIN_NUMBER_TACT_SWITCH  20
#define PIN_NUMBER_9AXIS_INT    8
#define PIN_NUMBER_RTC_INT      9

// UART
// P0.23    Out     シリアルTXD。(CP2104のRxDに接続。)
// P0.28    In      シリアルRXD。(CP2104のTxDに接続。)
#define PIN_NUMBER_UART_TXD 23
#define PIN_NUMBER_UART_RXD 28

// P0.22    Out     LED。(正論理、HIGHでLED点灯)
#define PIN_NUMBER_LED      22

// P0.27    In      32kHzクロックが供給される
// P0.05    In      電源電圧。(470k/220k分圧、0.32倍)
#define PIN_NUMBER_32kHz_CLK            27
//#define PIN_NUMBER_SUPPLY_MONITORING    5 //AIN6_2
#define ADC_INPUT_SUPPLY_MONITORING    NRF_ADC_CONFIG_INPUT_6

// TWI
// P0.21            SDA
// P0.24            SCL
// P0.03    Out     TWIの電源
#define PIN_NUMBER_TWI_SDA   21
#define PIN_NUMBER_TWI_SCL   24
#define PIN_NUMBER_TWI_POWER 3

// SPI
// P0.00        /CS
// P0.01        DO
// P0.11        DI
// P0.12        CLK
#define PIN_NUMBER_SPI_nCS  0
#define PIN_NUMBER_SPI_DO   1
#define PIN_NUMBER_SPI_DI   11
#define PIN_NUMBER_SPI_CLK  12

// IOポートのアドレス定義
// I2Cバス上の、スレーブ・アドレス

#endif /* senstick_pin_definitions_h */
