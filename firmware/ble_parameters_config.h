#ifndef ble_parameters_config_h
#define ble_parameters_config_h

// コネクションパラメータモジュールの設定値
// アドバタイジング:
//  1. 30秒間  インターバル 20ミリ秒
//  2. 150秒間 インターバル 1秒
//  3. 停止 (スリープ)

#define ADV_FAST_INTERVAL_0625UNIT      (32)   // アドバタイジング・インターバルは、0.625ミリ秒単位で指定する。20 / 0.625 = 32。
#define ADV_FAST_TIMEOUT_SEC            (30)

#define ADV_SLOW_INTERVAL_0625UNIT      (1600) // 1000ミリ秒 / 0.625ミリ秒 = 1600
#define ADV_SLOW_TIMEOUT_SEC            (150)

// コネクション・インターバル
// min:20ミリ秒 max:50ミリ秒
// super vision timeout 4000ミリ秒
// slave latency 0

#define DEFAULT_MIN_CONN_INTERVAL_MILLISEC      (20)
#define DEFAULT_MAX_CONN_INTERVAL_MILLISEC      (50)
#define DEFAULT_SLAVE_LATENCY                   0
#define DEFAULT_CONN_SUP_TIMEOUT_MILISEC        (4 * 1000)

#endif /* ble_parameters_config_h */
