#ifndef ble_connection_parameter_manager_config_h
#define ble_connection_parameter_manager_config_h

#include "megane_app_timer_configration.h"

// コネクションパラメータモジュールの設定値
// 値の設定制約。
// FASTの領域とDEFAULTのの領域は、重なる部分があってはなりません。またFASTのの領域はDEFAULTのの領域よりも値が小さい範囲になければなりません。
// FASTとDEFAULTの時間幅は、接している必要はありません。

// Apple Bluetooth design guideline
// https://developer.apple.com/hardwaredrivers/BluetoothDesignGuidelines.pdf

//Slave latencyは、スレーブがセントラルに送るべきデータが無いときは返信を省略することで省電力化するオプションです。
// 雰囲気メガネはANCSで通知を受け取る、セントラルからペリフェラルに何かを送る用途のデバイスです。ですから、slave latencyでの省電力の効果は、ないです。

// ダイレクトモードなどの高速通信時のパラメータ
#define FAST_MIN_CONN_INTERVAL_MILLISEC      (20)
#define FAST_MAX_CONN_INTERVAL_MILLISEC      (50)
#define FAST_SLAVE_LATENCY                   0
#define FAST_CONN_SUP_TIMEOUT_MILISEC        (4 * 1000)

// 通常時のパラメータ
#define DEFAULT_MIN_CONN_INTERVAL_MILLISEC      (200)
#define DEFAULT_MAX_CONN_INTERVAL_MILLISEC      (600)
#define DEFAULT_SLAVE_LATENCY                   0
#define DEFAULT_CONN_SUP_TIMEOUT_MILISEC        (6 * 1000)


#endif /* ble_connection_parameter_manager_config_h */
