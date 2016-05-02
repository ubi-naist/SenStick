#include "service_util.h"

void setCharacteristicsValue(uint16_t connection_handle, uint16_t value_handle, uint8_t *p_data, uint16_t length)
{
    ret_code_t err_code;
    
    ble_gatts_value_t gatts_value;
    memset(&gatts_value, 0, sizeof(gatts_value));
    gatts_value.p_value = p_data;
    gatts_value.len     = length;
    gatts_value.offset  = 0;
    
    // update database
    err_code = sd_ble_gatts_value_set(connection_handle, value_handle, &gatts_value);
    APP_ERROR_CHECK(err_code);
}

// cccdは16ビット整数、2バイト。
#define BLE_CCCD_VALUE_LEN 2
bool is_indication_enabled(uint16_t connection_handle, uint16_t cccd_handle)
{
    uint32_t err_code;
    uint8_t  cccd_value_buf[BLE_CCCD_VALUE_LEN];
    ble_gatts_value_t gatts_value;
    
    bool result = false;
    
    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));
    
    gatts_value.len     = BLE_CCCD_VALUE_LEN;
    gatts_value.offset  = 0;
    gatts_value.p_value = cccd_value_buf;
    
    err_code = sd_ble_gatts_value_get(connection_handle, cccd_handle, &gatts_value);
    if (err_code == NRF_SUCCESS) {
        result = ble_srv_is_indication_enabled(cccd_value_buf);
    }
    return result;
}

void setCharacteristicsValueAndNotify(uint16_t connection_handle, uint16_t value_handle, uint16_t cccd_handle, uint8_t *p_data, uint16_t length)
{
    setCharacteristicsValue(connection_handle, value_handle, p_data, length);
    if( is_indication_enabled(connection_handle, cccd_handle) ) {
        notifyToClient(connection_handle, value_handle, p_data, length);
    }
}

uint16_t getCharacteristicsValue(uint16_t connection_handle, uint16_t att_handle, uint8_t *p_data, uint16_t buffer_length)
{
    uint32_t err_code;
    ble_gatts_value_t gatts_value;
    
    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));
    
    gatts_value.len     = buffer_length;
    gatts_value.offset  = 0;
    gatts_value.p_value = p_data;
    
    err_code = sd_ble_gatts_value_get(connection_handle, att_handle, &gatts_value);


    if (err_code == NRF_SUCCESS) {
        return gatts_value.len;
    } else {
        return 0;
    }
}

ret_code_t notifyToClient(uint16_t connection_handle, uint16_t value_handle, uint8_t *p_data, uint16_t length)
{
    ble_gatts_hvx_params_t params;
    memset(&params, 0, sizeof(params));
    params.type   = BLE_GATT_HVX_NOTIFICATION;
    params.handle = value_handle;
    params.p_data = p_data;
    params.p_len  = &length;
    params.offset = 0;
    
    return sd_ble_gatts_hvx(connection_handle, &params);
//    if( (err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_INVALID_STATE) && (err_code != BLE_ERROR_NO_TX_BUFFERS) && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING) ) {
//        APP_ERROR_CHECK(err_code);
//    }
}
