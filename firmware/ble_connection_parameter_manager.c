//
//  ble_connection_parameter_manager.c
//  senstick
//
//  Created by AkihiroUehara on 2015/12/17.
//
//

#include "ble_connection_parameter_manager.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app_timer.h"
#include "ble_conn_params.h"
#include "app_error.h"

#include "connection_parameter_manager.h"
#include "connection_parameter_manager_config.h"

#include "debug_rtt.h"

/*
 * instance variables
 */
static app_timer_id_t app_timer_id;
static uint16_t m_connection_interval_millisec;
static bool m_is_updating;
static connectionParameterManagerMode m_current_mode;
static connectionParameterManagerMode m_updating_mode;

/**
 * Private methods
 */

// モードを引き下げるタイマーを起動します。
static void startTimer()
{
    uint32_t ticks = 0;
    switch (m_current_mode) {
        case CONNECTION_PARAMETER_UNACCEPTED:
            break;
            //        case LOW_POWER_CONNECTION_MODE:
            //            break;
        case DEFAULT_CONNECTION_MODE:
            //            ticks = APP_TIMER_TICKS(MODE_DEFAULT_TO_LOWPOWER_MILLISEC, APP_TIMER_PRESCALER);
            break;
        case FAST_CONNECTION_MODE:
            ticks = APP_TIMER_TICKS(MODE_FAST_TO_DEFAULT_MILLISEC, APP_TIMER_PRESCALER);
            break;
        default:
            break;
    }
    
    if(ticks != 0) {
        app_timer_stop(app_timer_id);
        uint32_t err_code = app_timer_start(app_timer_id,
                                            ticks,
                                            NULL);
        APP_ERROR_CHECK(err_code);
    }
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    // コネクションパラメータモジュールで、ネゴシエーション失敗時のdisconnectをfalseにしている。
    // パラメータ更新失敗で、切断処理をする必要はない。
    m_is_updating  = false;
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED) {
        m_current_mode = m_updating_mode;
    }
    
    // 更新タイマーを再スタートしておく
    startTimer();
    
    LOG("\non_conn_parameters_evt(), isSucceeded:%d", (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED));
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    LOG("\nconn_params_error_handler(), error_code:0x%0x.", nrf_error);
    APP_ERROR_HANDLER(nrf_error);
}

static void setGAPConnectionParameter_withparams(ble_gap_conn_params_t *p_gap_conn_params,uint16_t minConnectionIntervalAppMilliSec, uint16_t  maxConnectionIntervalAppMilliSec, uint16_t slaveLatency, uint16_t superVisionTimeOutMilliSec )
{
    memset(p_gap_conn_params, 0, sizeof(ble_gap_conn_params_t));
    
    p_gap_conn_params->min_conn_interval = MSEC_TO_UNITS(minConnectionIntervalAppMilliSec, UNIT_1_25_MS);
    p_gap_conn_params->max_conn_interval = MSEC_TO_UNITS(maxConnectionIntervalAppMilliSec, UNIT_1_25_MS);
    p_gap_conn_params->slave_latency     = slaveLatency;
    p_gap_conn_params->conn_sup_timeout  = MSEC_TO_UNITS(superVisionTimeOutMilliSec, UNIT_10_MS);
    
    LOG("\nsetGAPConnectionParameter_withparams(), con_min %d, con_max %d, late %d, sup %d.",
        p_gap_conn_params->min_conn_interval,
        p_gap_conn_params->max_conn_interval,
        p_gap_conn_params->slave_latency,
        p_gap_conn_params->conn_sup_timeout
        );
}

static void updateConnectionInterval(connectionParameterManagerMode mode)
{
    // モードが同じであれば、更新する必要はない。更新タイマーを再スタートしておく
    if(m_current_mode == mode) {
        startTimer();
        return;
    }
    
    // 指定モードで更新中であれば、更新する必要はない。
    if(m_is_updating && m_updating_mode == mode) {
        return;
    }
    
    LOG("\nupdateConnectionInterval(), mode:%d.", mode);
    
    // 更新モードを保存、フラグを立てる。
    m_updating_mode = mode;
    m_is_updating   = true;
    
    // コネクションパラメータの設定と更新
    ble_gap_conn_params_t gap_conn_params;
    setGAPConnectionParameter(&gap_conn_params, mode);
    
    uint32_t err_code = ble_conn_params_change_conn_params(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void _app_timer_handler(void *p_context)
{
    // 接続モードを1段階下げる
    switch (m_current_mode) {
            //        case LOW_POWER_CONNECTION_MODE:
            //            break;
        case DEFAULT_CONNECTION_MODE:
            //            updateConnectionInterval(LOW_POWER_CONNECTION_MODE);
            break;
        case FAST_CONNECTION_MODE:
            updateConnectionInterval(DEFAULT_CONNECTION_MODE);
            break;
        default:
            break;
    }
    LOG("\n_app_timer_handler(), changing a connection mode to default mode, current mode:%d. inerval:%d", m_current_mode, m_connection_interval_millisec);
}

/**
 * Public methods
 */
void initConnectionParameterManager()
{
    // インスタンス変数の初期化
    m_is_updating       = false;
    m_updating_mode     = FAST_CONNECTION_MODE;
    m_current_mode      = CONNECTION_PARAMETER_UNACCEPTED;
    
    // コネクションパラメータを設定する。
    ble_gap_conn_params_t gap_conn_params;
    setGAPConnectionParameter(&gap_conn_params, FAST_CONNECTION_MODE);
    
    // コネクションパラメータモジュールを初期化する。
    ble_conn_params_init_t cp_init;
    memset(&cp_init, 0, sizeof(cp_init));
    
    cp_init.p_conn_params                  = &gap_conn_params;
    cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(FIRST_CONN_PARAMS_UPDATE_DELAY_MILLISEC, APP_TIMER_PRESCALER);
    cp_init.next_conn_params_update_delay  = APP_TIMER_TICKS(NEXT_CONN_PARAMS_UPDATE_DELAY_MILLISEC,  APP_TIMER_PRESCALER);
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;      // 書き込まれた時に接続パラメータ更新を開始する、CCCDハンドラ。
    cp_init.disconnect_on_fail             = false;                        // 接続失敗時に切断するかのフラグ。
    // イベントハンドラ
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    // GAPでもprefferred connectionを設定しているが、その設定はここで上書きされる。
    // Softwaredevice のprefferred connection parametersが この関数内部で設定される。
    uint32_t err_code;
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
    
    // 定期的なパラメータ更新のためのタイマーを作成
    err_code = app_timer_create(&app_timer_id, APP_TIMER_MODE_SINGLE_SHOT, _app_timer_handler );
    APP_ERROR_CHECK(err_code);
}

void connectionParameterManagerOnBleEvent(ble_evt_t * p_ble_evt) {
    uint32_t err_code;
    
    // NORDICのライブラリの接続マネージャのイベントハンドラを呼び出す。
    ble_conn_params_on_ble_evt(p_ble_evt);
    
    // 独自部分の処理。
    switch(p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            m_current_mode = FAST_CONNECTION_MODE; // 接続時点では、FASTモードにあるはず。
            m_connection_interval_millisec = (uint16_t)(((uint32_t)p_ble_evt->evt.gap_evt.params.connected.conn_params.max_conn_interval * 125) / 100);
            LOG("\ncntMgr(),connected, int:%d msec, mode:%d", m_connection_interval_millisec, m_current_mode);
            // コネクションインターバルを低レベルに落とすタイマーを立ち上げておく。
            startTimer();
            break;
            
        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            m_connection_interval_millisec = (uint16_t)(((uint32_t)p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params.max_conn_interval * 125) / 100);
            LOG("\ncntMgr(),update, int:%d msec, mode:%d", m_connection_interval_millisec, m_current_mode);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            m_is_updating  = false;
            m_current_mode = FAST_CONNECTION_MODE; // 初期値にしておく。
            
            err_code = app_timer_stop(app_timer_id);
            APP_ERROR_CHECK(err_code);
            break;
            
        default:
            break;
    }
}

//void requestConnectionMode(connectionParameterManagerMode mode)
void requestFastConnectionMode()
{
    // 小さいモードには強制的に落とす必要はない。
    if( m_current_mode >= FAST_CONNECTION_MODE || m_is_updating) {
        return;
    }
    updateConnectionInterval(FAST_CONNECTION_MODE);
}

connectionParameterManagerMode getCurrentMode(void)
{
    return m_current_mode;
}

void setGAPConnectionParameter(ble_gap_conn_params_t *p_gap_conn_params, connectionParameterManagerMode mode)
{
    // コネクションパラメータを設定する。
    switch (mode) {
        default:
        case DEFAULT_CONNECTION_MODE:
            setGAPConnectionParameter_withparams(p_gap_conn_params, DEFAULT_MIN_CONN_INTERVAL_MILLISEC, DEFAULT_MAX_CONN_INTERVAL_MILLISEC, DEFAULT_SLAVE_LATENCY, DEFAULT_CONN_SUP_TIMEOUT_MILISEC);
            break;
        case FAST_CONNECTION_MODE:
            setGAPConnectionParameter_withparams(p_gap_conn_params, FAST_MIN_CONN_INTERVAL_MILLISEC, FAST_MAX_CONN_INTERVAL_MILLISEC, FAST_SLAVE_LATENCY, FAST_CONN_SUP_TIMEOUT_MILISEC);
            break;
            /*
             case LOW_POWER_CONNECTION_MODE:
             setGAPConnectionParameter_withparams(p_gap_conn_params, LOWPOWER_MIN_CONN_INTERVAL_MILLISEC, LOWPOWER_MAX_CONN_INTERVAL_MILLISEC, LOWPOWER_SLAVE_LATENCY, LOWPOWER_CONN_SUP_TIMEOUT_MILISEC);
             break;*/
    }
}
