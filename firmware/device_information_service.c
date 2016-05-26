#include <string.h>

#include <ble_dis.h>
#include <app_error.h>
#include <sdk_errors.h>

#include "device_information_service.h"
#include "senstick_device_definition.h"

// デバイスインフォアメーションサービスを初期化
void init_device_information_service(void)
{
    ret_code_t err_code;
    ble_dis_init_t dis_init_obj;
    
    memset(&dis_init_obj, 0, sizeof(dis_init_obj));
    
    // 文字列を設定
    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init_obj.hw_rev_str, HARDWARE_REVISION_STRING);
    ble_srv_ascii_to_utf8(&dis_init_obj.fw_rev_str, FIRMWARE_REVISION_STRING);
    
    // 属性設定
    //    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);
    
    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
}
