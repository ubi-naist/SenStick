#include <device_manager.h>

#include <app_error.h>
#include <sdk_errors.h>

#include "senstick_device_manager.h"

#include "ble_parameters_config.h"

static dm_application_instance_t m_app_handle;

// デバイスマネージャのイベント処理ハンドラ
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle, dm_event_t const  * p_event, ret_code_t event_result)
{
    APP_ERROR_CHECK(event_result);
    /*
     #ifdef BLE_DFU_APP_SUPPORT
     if (p_event->event_id == DM_EVT_LINK_SECURED)
     {
     app_context_load(p_handle);
     }
     #endif // BLE_DFU_APP_SUPPORT
     */
    return NRF_SUCCESS;
}


// デバイスマネージャの初期化
// このメソッドを呼び出す前にpstorageの初期化が完了していること。
void init_device_manager(bool erase_bonds)
{
    ret_code_t             err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;
    
    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);
    
    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));
    
    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
    
    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}

