#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// sdk_config.h に対する差分で設定をまとめる。
// このファイルはsdk_config.hから読み込まれる。そのために、C/C++で、USE_APP_CONFIG を定義しておくこと。

// IO設定を読み込む。
#include "senstick_io_definition.h"

#define CRC32_ENABLED       1
#define FSTORAGE_ENABLED    1
#define NRF_QUEUE_ENABLED   1

// ====
// BLE Services
// battery serviceを使う。
#define BLE_BAS_ENABLED 1
// device information serviceを使う。
#define BLE_DIS_ENABLED 1

// ===
// SAADC nRF52のSAADCを使用する。
#define SAADC_ENABLED 1

// ===
// SPIを使用する。
#define SPI_ENABLED  1
#define SPI2_ENABLED 1

// <33554432=> 125 kHz
// <67108864=> 250 kHz
// <134217728=> 500 kHz
// <268435456=> 1 MHz
// <536870912=> 2 MHz
// <1073741824=> 4 MHz
// <2147483648=> 8 MHz

#define SPI2_DEFAULT_FREQUENCY 2147483648

// ===
// TWI。
#define TWI_ENABLED 1
// 周波数。26738688=> 100k, 67108864=> 250k, 104857600=> 400k
//#define TWI_DEFAULT_CONFIG_FREQUENCY 104857600
#define TWI_DEFAULT_CONFIG_FREQUENCY 26738688

// TWI0および1を使用する。
#define TWI0_ENABLED 1
#define TWI0_USE_EASY_DMA 0
#define TWI1_ENABLED 1
#define TWI1_USE_EASY_DMA 0
//#define TWI_CONFIG_LOG_ENABLED 1

// UARTは使用しない。
#define UART_ENABLED 0

// ===
// ライブラリ。
#define APP_MAILBOX_ENABLED   1
#define APP_SCHEDULER_ENABLED 1

// ===
// ログ
#define NRF_LOG_ENABLED 1
#define NRF_LOG_DEFERRED 0
#define NRF_LOG_BACKEND_SERIAL_USES_UART 0
#define NRF_LOG_BACKEND_SERIAL_USES_RTT  1
// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug
#define NRF_LOG_DEFAULT_LEVEL 4

#endif // APP_CONFIG_H
