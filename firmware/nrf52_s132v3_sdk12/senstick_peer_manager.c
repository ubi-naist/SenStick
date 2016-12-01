//#include <device_manager.h>
#include <fstorage.h>
#include <fds.h>
#include <peer_manager.h>
#include <ble_conn_state.h>

#include <app_error.h>
#include <sdk_errors.h>

#include "senstick_device_manager.h"

#include "ble_parameters_config.h"

// SDK12.1は、device_managerからpeer managerに切り替えになる。
// ref https://devzone.nordicsemi.com/tutorials/24/ , s132_nrf52_3.0.0_migration_document.pdf

// ピアマネージャのイベント処理ハンドラ
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    switch(p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        case PM_EVT_CONN_SEC_START:
        case PM_EVT_CONN_SEC_SUCCEEDED:
        case PM_EVT_CONN_SEC_FAILED:
        case PM_EVT_CONN_SEC_CONFIG_REQ:
        case PM_EVT_STORAGE_FULL:
        case PM_EVT_ERROR_UNEXPECTED:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_FAILED:
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        case PM_EVT_PEERS_DELETE_FAILED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

// ピアマネージャの初期化
// このメソッドを呼び出す前にfstorageの初期化が完了していること。
void init_device_manager(bool erase_bonds)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;
    
    err_code = pm_init();
    APP_ERROR_CHECK(err_code);
    
    if (erase_bonds)
    {
        err_code = pm_peers_delete();
        APP_ERROR_CHECK(err_code);
    }
    
    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));
    
    // Security parameters to be used for all security procedures.
    sec_param.bond              = SEC_PARAM_BOND;
    sec_param.mitm              = SEC_PARAM_MITM;
    sec_param.lesc              = SEC_PARAM_LESC;
    sec_param.keypress          = SEC_PARAM_KEYPRESS;
    sec_param.io_caps           = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob               = SEC_PARAM_OOB;
    sec_param.min_key_size      = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size      = SEC_PARAM_MAX_KEY_SIZE;
    // ボンディングをしないため、これらのフラグは0に。
    sec_param.kdist_own.enc     = 0;
    sec_param.kdist_own.id      = 0;
    sec_param.kdist_peer.enc    = 0;
    sec_param.kdist_peer.id     = 0;
    
    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);
    
    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}
