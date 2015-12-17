#ifndef ble_connection_parameter_manager_h
#define ble_connection_parameter_manager_h
#include <stdint.h>
#include "ble.h"
#include "ble_connection_parameter_manager_config.h"



// 概要:
// コネクションインターバルの制御クラスです。
// コネクションパラメータは connection_parameter_manager_config.h で定義します。

// 消費電力低減のためには、コネクションインターバルをなるべく長くする必要があります。しかしコネクションインターバルが長くなると、ANCSのイベントおよびダイレクトモードでの表示更新が遅くなります。
// レイテンシを下げるときには FIRST_MODE に、FIRST_MODE 要求が一定時間なければ、自動的にノーマルモード(ANCS使用時)に落とします。

// 内部で使うリソース:
// なし

// 依存性:
// なし

// コネクションパラメータモジュールの設定値
// 初回のコネクションインターバル更新リクエストを脱すまでの時間。20秒。
#define FIRST_CONN_PARAMS_UPDATE_DELAY_MILLISEC (20 * 1000)
// 2回目以降のコネクションインターバル更新リクエストを出す時間。 Bluetooth仕様から30秒が推奨値。
#define NEXT_CONN_PARAMS_UPDATE_DELAY_MILLISEC (30 * 1000)
// コネクションパラメータ更新が拒否されたときの、リトライ回数。この回数を超えた時は、failする。
#define MAX_CONN_PARAMS_UPDATE_COUNT    0

// 接続モードの更新周期。
// タイマーはこの周期ごとに コネクションインターバルを、1段階低いものに変更します。
#define MODE_FAST_TO_DEFAULT_MILLISEC     ( 5 * 60 * 1000)
//#define MODE_DEFAULT_TO_LOWPOWER_MILLISEC ( 1 * 60 * 1000)

typedef enum {
    CONNECTION_PARAMETER_UNACCEPTED = 0,
    //    LOW_POWER_CONNECTION_MODE       = 1,
    DEFAULT_CONNECTION_MODE         = 2,
    FAST_CONNECTION_MODE            = 3,
} connectionParameterManagerMode;

// 初期化関数
// 使用前にかならずこの関数を呼び出すこと。
void initConnectionParameterManager(void);

// アプリケーションのBLEスタックイベントを処理する関数。
//BLEスタックからのイベントを、必ずこのメソッドに渡すこと。
void connectionParameterManagerOnBleEvent(ble_evt_t * p_ble_evt);

// コネクションインターバルの高速化要求。このメソッドは、何度連続してよびだしても、かまいません。
//void requestConnectionMode(connectionParameterManagerMode mode);
void requestFastConnectionMode(void);

// 現在のモードを読み出します。
connectionParameterManagerMode getCurrentMode(void);

// コネクションパラメータ構造体の値設定のヘルパーメソッド。
void setGAPConnectionParameter(ble_gap_conn_params_t *p_gap_conn_params, connectionParameterManagerMode mode);


#endif /* ble_connection_parameter_manager_h */

