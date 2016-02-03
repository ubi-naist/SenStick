#include <string.h>

#include <nordic_common.h>

#include <ble.h>
#include <ble_gatts.h>

#include <sdk_errors.h>
#include <app_error.h>

#include "ble_ti_sensortag_service.h"

const ble_uuid128_t ble_sensortag_base_uuid128 = {
    {
        //F000XXXX-0451-4000-B000-000000000000
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xB0,
        0x00, 0x40,
        0x51, 0x04,
        0x00, 0x00, 0x00, 0xF0
    }
};

// GATTレイヤでの最大データ長。ATT MTU23で、20。
#define GATT_MAX_DATA_LENGTH 20

/**
 * Private methods
 */
static void setCharacteristicsValue(const ble_sensortag_service_t *p_context, uint16_t value_handle, uint8_t *p_data, uint16_t data_length)
{
    ret_code_t err_code;

    ble_gatts_value_t gatts_value;
    memset(&gatts_value, 0, sizeof(gatts_value));
    gatts_value.p_value = p_data;
    gatts_value.len     = data_length;
    gatts_value.offset  = 0;

    // update database
    err_code = sd_ble_gatts_value_set(p_context->connection_handle, value_handle, &gatts_value);
    APP_ERROR_CHECK(err_code);
}

static void notifyToClient(const ble_sensortag_service_t *p_context, uint16_t value_handle, uint8_t *data, uint16_t length)
{
    if(data == NULL || length == 0 || p_context->connection_handle == BLE_CONN_HANDLE_INVALID) {
        return;
    }
    
    ble_gatts_hvx_params_t params;
    memset(&params, 0, sizeof(params));
    params.type   = BLE_GATT_HVX_NOTIFICATION;
    params.handle = value_handle;
    params.p_data = data;
    params.p_len  = &length;
    params.offset = 0;
    
    int32_t err_code = sd_ble_gatts_hvx(p_context->connection_handle, &params);
    if( (err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_INVALID_STATE) && (err_code != BLE_ERROR_NO_TX_BUFFERS) && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING) ) {
        APP_ERROR_CHECK(err_code);
    }
}

static int8_t convertAccelerationValue(const sensorSetting_t *p_setting, const int16_t value)
{
    switch(p_setting->accelerationRange) {
        default:
        case ACCELERATION_RANGE_2G:  return (int8_t)(value >> 14);
        case ACCELERATION_RANGE_4G:  return (int8_t)(value >> 13);
        case ACCELERATION_RANGE_8G:  return (int8_t)(value >> 12);
        case ACCELERATION_RANGE_16G: return (int8_t)(value >> 11);
    }
}
static void notifyAcceleromterData(const ble_sensortag_service_t *p_context, const AccelerationData_t *p_acceleration)
{
    int8_t buffer[3];
    // X/Y/Zの順番で、フルスケール+-2G、符号付き1.6固定小数点で1バイトで表現する
    buffer[0] = convertAccelerationValue(&(p_context->sensor_setting), p_acceleration->x);
    buffer[1] = convertAccelerationValue(&(p_context->sensor_setting), p_acceleration->y);
    buffer[2] = convertAccelerationValue(&(p_context->sensor_setting), p_acceleration->z);

    setCharacteristicsValue(p_context, p_context->accelerometer_value_char_handle.value_handle, (uint8_t *)buffer, 3);
    
    if(p_context->is_accelerometer_notifying) {
        notifyToClient(p_context, p_context->accelerometer_value_char_handle.value_handle, (uint8_t *)buffer, 3);
    }
}

static void convertRotationValue(uint8_t *p_dst, const sensorSetting_t *p_setting, const int16_t value)
{
    int32_t v = value * 500 * 8 / 65536;
    switch(p_setting->rotationRange) {
        default:
        case ROTATION_RANGE_250DPS:  v /= 8; break;
        case ROTATION_RANGE_500DPS:  v /= 4; break;
        case ROTATION_RANGE_1000DPS: v /= 2; break;
        case ROTATION_RANGE_2000DPS: v /= 1; break;
    }

    int16ToByteArrayLittleEndian(p_dst, (int16_t)v);
}
static void notifyGyroscopeData(ble_sensortag_service_t *p_context, const RotationRateData_t *p_rotation)
{
    uint8_t buffer[6];
    memset(buffer, 0, sizeof(buffer));
    // マスクに合わせてX/Y/Zを設定
    if( p_context->sensor_setting.is_gyroscope_sampling & 0x01 != 0) {
        convertRotationValue(&(buffer[0]), &(p_context->sensor_setting), p_rotation->x);
    }
    if( p_context->sensor_setting.is_gyroscope_sampling & 0x02 != 0) {
        convertRotationValue(&(buffer[2]), &(p_context->sensor_setting), p_rotation->y);
    }
    if( p_context->sensor_setting.is_gyroscope_sampling & 0x04 != 0) {
        convertRotationValue(&(buffer[4]), &(p_context->sensor_setting), p_rotation->z);
    }

    // 値を設定
    setCharacteristicsValue(p_context, p_context->gyroscope_value_char_handle.value_handle, buffer, 6);
    // 通知
    if(p_context->is_gyroscope_notifying) {
        notifyToClient(p_context, p_context->gyroscope_value_char_handle.value_handle, buffer, 6);
    }
}

static void notifyHumidity(ble_sensortag_service_t *p_context, const TemperatureAndHumidityData_t *p_data)
{
    uint8_t buffer[4];

    uint16ToByteArrayLittleEndian(&(buffer[0]), p_data->temperature);
    uint16ToByteArrayLittleEndian(&(buffer[2]), p_data->humidity);
    
    setCharacteristicsValue(p_context, p_context->humidity_value_char_handle.value_handle, buffer, 4);
    
    if(p_context->is_humidity_notifying) {
        notifyToClient(p_context, p_context->humidity_value_char_handle.value_handle, buffer, 4);
    }
}

static void convertMagnetrometerValue(uint8_t *p_dst, const sensorSetting_t *p_setting, const int16_t value)
{
    // 値はフルスケール、±4800 μT。これをGATTの値のスケール+-1000に直す。
    int16_t v = (int16_t)(((int32_t)value * 48) / 10);
    int16ToByteArrayLittleEndian(p_dst, v);
}
static void notifyMagnetrometer(ble_sensortag_service_t *p_context, const MagneticFieldData_t *p_magneticField)
{
    uint8_t buffer[6];
    
    convertMagnetrometerValue(&(buffer[0]), &(p_context->sensor_setting), p_magneticField->x);
    convertMagnetrometerValue(&(buffer[2]), &(p_context->sensor_setting), p_magneticField->y);
    convertMagnetrometerValue(&(buffer[4]), &(p_context->sensor_setting), p_magneticField->z);
    
    setCharacteristicsValue(p_context, p_context->magnetometer_value_char_handle.value_handle, buffer, 6);
    
    if(p_context->is_magnetrometer_notifying) {
        notifyToClient(p_context, p_context->magnetometer_value_char_handle.value_handle, buffer, 6);
    }
}

static void notifyBarometer(ble_sensortag_service_t *p_context, const AirPressureData_t *p_data)
{
    uint8_t buffer[4];
    // 先頭2バイトは温度、これは0
    uint16ToByteArrayLittleEndian(&(buffer[0]), 0);
    uint16ToByteArrayLittleEndian(&(buffer[2]), *p_data);
    
    setCharacteristicsValue(p_context, p_context->barometer_value_char_handle.value_handle, buffer, 4);
    
    if(p_context->is_barometer_notifying) {
        notifyToClient(p_context, p_context->barometer_value_char_handle.value_handle, buffer, 4);
    }
}

static void onWrite(ble_sensortag_service_t *p_context, ble_evt_t * p_ble_evt)
{

    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // CCCDへの書き込み確認
    if(p_evt_write->len == 2) {
        if(p_evt_write->handle == p_context->accelerometer_value_char_handle.cccd_handle) {
            p_context->is_accelerometer_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        } else if(p_evt_write->handle == p_context->humidity_value_char_handle.cccd_handle) {
            p_context->is_humidity_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        } else if(p_evt_write->handle == p_context->magnetometer_value_char_handle.cccd_handle) {
            p_context->is_magnetrometer_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        } else if(p_evt_write->handle == p_context->barometer_value_char_handle.cccd_handle) {
            p_context->is_barometer_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        } else if(p_evt_write->handle == p_context->gyroscope_value_char_handle.cccd_handle) {
            p_context->is_gyroscope_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        } else if(p_evt_write->handle == p_context->illumination_value_char_handle.cccd_handle) {
            p_context->is_illumination_notifying = ble_srv_is_notification_enabled(p_evt_write->data);
            return;
        }
    }

    bool isSettingChanged = false;
    ret_code_t err_code;
    ble_gatts_value_t gatts_value;

    // Configrationへの設定
    if(p_evt_write->len == 1) {
        // 書き込まれた値を取り出す。
        uint8_t config;
        
        memset(&gatts_value, 0 , sizeof(gatts_value));
        gatts_value.len     = 1;
        gatts_value.offset  = 0;
        gatts_value.p_value = &config;
        err_code = sd_ble_gatts_value_get(p_context->connection_handle, p_evt_write->handle, &gatts_value);
        APP_ERROR_CHECK(err_code);
        
        // configration を設定する。
        if(p_evt_write->handle == p_context->accelerometer_configration_char_handle.value_handle) {
            p_context->sensor_setting.is_accelerometer_sampling = config;
            isSettingChanged = true;
        } else if(p_evt_write->handle == p_context->humidity_configration_char_handle.value_handle) {
            p_context->sensor_setting.is_humidity_sampling = config;
            isSettingChanged = true;
        } else if(p_evt_write->handle == p_context->magnetometer_configration_char_handle.value_handle) {
            p_context->sensor_setting.is_magnetrometer_sampling = config;
            isSettingChanged = true;
        } else if(p_evt_write->handle == p_context->barometer_configration_char_handle.value_handle) {
            p_context->sensor_setting.is_barometer_sampling = config;
            isSettingChanged = true;
        } else if(p_evt_write->handle == p_context->gyroscope_configration_char_handle.value_handle) {
            p_context->sensor_setting.is_gyroscope_sampling = config;
            isSettingChanged = true;
        } else if(p_evt_write->handle == p_context->illumination_configration_char_handle.value_handle) {
            p_context->sensor_setting.is_illumination_sampling = config;
            isSettingChanged = true;
        }

        // サンプリング周期 を設定する。
        uint16_t period = config * 10; // 周期は10ミリ秒単位
        if(p_evt_write->handle == p_context->accelerometer_period_char_handle.value_handle
           || p_evt_write->handle == p_context->magnetometer_period_char_handle.value_handle
           || p_evt_write->handle == p_context->gyroscope_period_char_handle.value_handle ) {
            isSettingChanged = setSensorSettingPeriod(&(p_context->sensor_setting), MotionSensor, period);
        } else if(p_evt_write->handle == p_context->humidity_period_char_handle.value_handle) {
            isSettingChanged = setSensorSettingPeriod(&(p_context->sensor_setting), TemperatureAndHumiditySensor, period);
        } else if(p_evt_write->handle == p_context->barometer_period_char_handle.value_handle) {
            isSettingChanged = setSensorSettingPeriod(&(p_context->sensor_setting), AirPressureSensor, period);
        } else if(p_evt_write->handle == p_context->illumination_period_char_handle.value_handle) {
            isSettingChanged = setSensorSettingPeriod(&(p_context->sensor_setting), BrightnessSensor, period);
        }
        // TBD UVセンサー
    }

    // isSettingChanged が trueであれば、通知する。
    if(isSettingChanged) {
        (p_context->setting_changed_event_handler)(p_context, &(p_context->sensor_setting));
    }
}

static void onDisconnect(ble_sensortag_service_t *p_context, ble_evt_t * p_ble_evt)
{
    // クライアントのコンテキストを消去
    p_context->connection_handle = BLE_CONN_HANDLE_INVALID;
    
    // cccdのフラグをクリア
    p_context->is_temperature_notifying     = false;
    p_context->is_accelerometer_notifying   = false;
    p_context->is_humidity_notifying        = false;
    p_context->is_magnetrometer_notifying   = false;
    p_context->is_barometer_notifying       = false;
    p_context->is_gyroscope_notifying       = false;
    p_context->is_illumination_notifying    = false;
}

static uint32_t addNotifiableCharacteristics(const ble_sensortag_service_t *p_context, uint16_t *p_service_handler, ble_gatts_char_handles_t *p_handle, const uint16_t uuid_sub, bool is_read, bool is_write, const int value_len)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t attribute_md;
    ble_gatts_attr_t    attr_value;
    ble_gatts_attr_md_t cccd_md;
    
    // UUIDを用意
    ble_uuid_t characteristics_uuid;
    characteristics_uuid.type = p_context->uuid_type;
    characteristics_uuid.uuid = uuid_sub;
    
    // クライアント・キャラクタリスティクス・コンフィグレーション・ディスクリプタ メタデータ
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(cccd_md.read_perm));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(cccd_md.write_perm));
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    // ノーティフィケーション・キャラクタリスティクスのメタデータ
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read  = is_read;
    char_md.char_props.write = is_write;
    char_md.char_props.notify   = 1;
    char_md.p_cccd_md           = &cccd_md; // CCCDのメタデータを設定する
    
    memset(&attribute_md, 0, sizeof(attribute_md));
    attribute_md.vloc = BLE_GATTS_VLOC_STACK;  // Attributeの値のメモリ領域は、スタックに管理させる
//    attribute_md.vlen = 1; // variable lengthの場合は設定する
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(attribute_md.read_perm)); // 書き込みおよび読み出しパーミションは、オープン。
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(attribute_md.write_perm));
    
    memset(&attr_value, 0, sizeof(attr_value));
    attr_value.p_uuid    = &characteristics_uuid;
    attr_value.p_attr_md = &attribute_md;
    attr_value.max_len   = value_len;
    
    return sd_ble_gatts_characteristic_add(*p_service_handler, &char_md, &attr_value, p_handle);
}

static uint32_t addCharacteristics(const ble_sensortag_service_t *p_context, uint16_t *p_service_handler, ble_gatts_char_handles_t *p_handle, const uint16_t uuid_sub, bool is_read, bool is_write, const int value_len)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t attribute_md;
    ble_gatts_attr_t    attr_value;
    
    // UUIDを用意
    ble_uuid_t characteristics_uuid;
    characteristics_uuid.type = p_context->uuid_type;
    characteristics_uuid.uuid = uuid_sub;
    
    // 読み書き属性
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read  = is_read;
    char_md.char_props.write = is_write;
    
    memset(&attribute_md, 0, sizeof(attribute_md));
    attribute_md.vloc = BLE_GATTS_VLOC_STACK;  // Attributeの値のメモリ領域は、スタックに管理させる
//    attribute_md.vlen = 1; // variable lengthの場合は設定する
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(attribute_md.read_perm)); // 書き込みおよび読み出しパーミションは、オープン。
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&(attribute_md.write_perm));
    
    memset(&attr_value, 0, sizeof(attr_value));
    attr_value.p_uuid    = &characteristics_uuid;
    attr_value.p_attr_md = &attribute_md;
    attr_value.max_len   = value_len;
    
    return sd_ble_gatts_characteristic_add(*p_service_handler, &char_md, &attr_value, p_handle);
}

static void addAService(ble_sensortag_service_t *p_context,
                        uint16_t *p_service_handler, uint16_t service_uuid,
                        ble_gatts_char_handles_t *p_value_char_handler, uint16_t value_char_uuid, uint8_t value_char_length,
                        ble_gatts_char_handles_t *p_configration_char_handler, uint16_t configration_char_uuid,
                        ble_gatts_char_handles_t *p_period_char_handler, uint16_t period_char_uuid)
{
    ret_code_t err_code;
    
    // サービスを登録
    ble_uuid_t uuid;
    uuid.uuid = service_uuid;
    uuid.type = p_context->uuid_type;
    err_code  = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, p_service_handler);
    APP_ERROR_CHECK(err_code);
    
    // キャラクタリスティクスを登録
    err_code = addNotifiableCharacteristics(p_context, p_service_handler, p_value_char_handler, value_char_uuid, true, false, value_char_length); // read-only
    APP_ERROR_CHECK(err_code);
    
    err_code = addCharacteristics(p_context, p_service_handler, p_configration_char_handler, configration_char_uuid, true, true, 1);
    APP_ERROR_CHECK(err_code);
    
    err_code = addCharacteristics(p_context, p_service_handler, p_period_char_handler, period_char_uuid, true, true, 1);
    APP_ERROR_CHECK(err_code);
}

static void addServices(ble_sensortag_service_t *p_context)
{
    ret_code_t err_code;
    
    addAService(p_context,
                &(p_context->ir_temperature_service_handle),               0xaa00,
                &(p_context->ir_temperature_value_char_handle),            0xaa01, 4,
                &(p_context->ir_temperature_configration_char_handle),     0xaa02,
                &(p_context->ir_temperature_period_char_handle),           0xaa03);

    addAService(p_context,
                &(p_context->accelerometer_service_handle),             0xaa10,
                &(p_context->accelerometer_value_char_handle),          0xaa11, 3,
                &(p_context->accelerometer_configration_char_handle),   0xaa12,
                &(p_context->accelerometer_period_char_handle),         0xaa13);

    addAService(p_context,
                &(p_context->humidity_service_handle),              0xaa20,
                &(p_context->humidity_value_char_handle),           0xaa21, 4,
                &(p_context->humidity_configration_char_handle),    0xaa22,
                &(p_context->humidity_period_char_handle),          0xaa23);
    
    addAService(p_context,
                &(p_context->magnetometer_service_handle),              0xaa30,
                &(p_context->magnetometer_value_char_handle),           0xaa31, 6,
                &(p_context->magnetometer_configration_char_handle),    0xaa32,
                &(p_context->magnetometer_period_char_handle),          0xaa33);
    

    addAService(p_context,
                &(p_context->barometer_service_handle),              0xaa40,
                &(p_context->barometer_value_char_handle),           0xaa41, 4,
                &(p_context->barometer_configration_char_handle),    0xaa42,
                &(p_context->barometer_period_char_handle),          0xaa44);
    // キャリブレーション・キャラクタリスティクスを追加
    err_code = addCharacteristics(p_context, &(p_context->barometer_service_handle), &(p_context->barometer_calibration_char_handle), 0xaa43, true, false, 16);
    APP_ERROR_CHECK(err_code);
    
    addAService(p_context,
                &(p_context->gyroscope_service_handle),              0xaa50,
                &(p_context->gyroscope_value_char_handle),           0xaa51, 6,
                &(p_context->gyroscope_configration_char_handle),    0xaa52,
                &(p_context->gyroscope_period_char_handle),          0xaa53);
    
    addAService(p_context,
                &(p_context->illumination_service_handle),              0xaa80,
                &(p_context->illumination_value_char_handle),           0xaa81, 6,
                &(p_context->illumination_configration_char_handle),    0xaa82,
                &(p_context->illumination_period_char_handle),          0xaa83);
}

/**
 * Public methods
 */

uint32_t bleSensorTagServiceInit(ble_sensortag_service_t *p_context, const sensorSetting_t *p_setting, ble_sensortag_service_event_handler_t setting_changed_event_handler)
{
    if(p_context == NULL || p_setting == NULL || setting_changed_event_handler == NULL) {
        return NRF_ERROR_NULL;
    }
    
    uint32_t   err_code;
    
    // サービス構造体を初期化
    memset(p_context, 0, sizeof(ble_sensortag_service_t));
    
    p_context->setting_changed_event_handler = setting_changed_event_handler;
    p_context->connection_handle = BLE_CONN_HANDLE_INVALID;
    
    // ベースUUIDを登録
    err_code = sd_ble_uuid_vs_add(&ble_sensortag_base_uuid128, &(p_context->uuid_type));
    APP_ERROR_CHECK(err_code);
    
    // サービスを追加
    addServices(p_context);
    
    // 初期値設定
    setSensorTagSetting(p_context, p_setting);
    
    return NRF_SUCCESS;
}

void bleSensorTagServiceOnBLEEvent(ble_sensortag_service_t *p_context, ble_evt_t * p_ble_evt)
{
    if( p_context == NULL || p_ble_evt == NULL) {
        return;
    }
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            p_context->connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            onDisconnect(p_context, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            onWrite(p_context, p_ble_evt);
            break;
        default:
            break;
    }
}

void notifySensorData(ble_sensortag_service_t *p_context, SensorDeviceType_t sensorType, const SensorData_t *p_sensorData)
{
    switch(sensorType) {
        case MotionSensor:
            notifyAcceleromterData(p_context, &(p_sensorData->motionSensorData.acceleration));
            notifyGyroscopeData(p_context, &(p_sensorData->motionSensorData.rotationRate));
            notifyMagnetrometer(p_context, &(p_sensorData->motionSensorData.magneticField));
            break;
        case TemperatureAndHumiditySensor:
            notifyHumidity(p_context, &(p_sensorData->temperatureAndHumidityData));
            break;
        case AirPressureSensor:
            notifyBarometer(p_context, &(p_sensorData->airPressureData));
            break;
        default:
            break;
    }
}

void setSensorTagSetting(ble_sensortag_service_t *p_context, const sensorSetting_t *p_src)
{
    memcpy(&(p_context->sensor_setting), p_src, sizeof(p_context->sensor_setting));

    // BLEのサービスに反映
    uint8_t data;

    /*
    // IR温度計は実装されていない
    data = (uint8_t)p_context->sensor_setting.is_temperature_sampling;
    setCharacteristicsValue(p_context, p_context->temperature_configration_char_handle.value_handle, &data, 1);
    data = (uint8_t)(p_context->sensor_setting.temperatureSamplingPeriod / 10 );
    setCharacteristicsValue(p_context, p_context->temperature_period_char_handle.value_handle, &data, 1);
    */
    
    data = (uint8_t)p_context->sensor_setting.is_accelerometer_sampling;
    setCharacteristicsValue(p_context, p_context->accelerometer_configration_char_handle.value_handle, &data, 1);
    data = (uint8_t)(p_context->sensor_setting.motionSensorSamplingPeriod / 10 );
    setCharacteristicsValue(p_context, p_context->accelerometer_period_char_handle.value_handle, &data, 1);
    
    data = (uint8_t)p_context->sensor_setting.is_humidity_sampling;
    setCharacteristicsValue(p_context, p_context->humidity_configration_char_handle.value_handle, &data, 1);
    data = (uint8_t)(p_context->sensor_setting.temperatureAndHumiditySamplingPeriod / 10 );
    setCharacteristicsValue(p_context, p_context->humidity_period_char_handle.value_handle, &data, 1);

    data = (uint8_t)p_context->sensor_setting.is_magnetrometer_sampling;
    setCharacteristicsValue(p_context, p_context->magnetometer_configration_char_handle.value_handle, &data, 1);
    data = (uint8_t)(p_context->sensor_setting.motionSensorSamplingPeriod / 10 );
    setCharacteristicsValue(p_context, p_context->magnetometer_period_char_handle.value_handle, &data, 1);

    data = (uint8_t)p_context->sensor_setting.is_barometer_sampling;
    setCharacteristicsValue(p_context, p_context->barometer_configration_char_handle.value_handle, &data, 1);
    data = (uint8_t)(p_context->sensor_setting.airPressureSamplingPeriod / 10 );
    setCharacteristicsValue(p_context, p_context->barometer_period_char_handle.value_handle, &data, 1);
    // キャリブレ
    // C1 - C4は符号なし、C5-C8は符号ありの16ビット整数。C3=400 (0x190)、それ以外は0。
    // {0x00, 0x00, 0x00, 0x00, <0x90, 0x01,> 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    uint8_t cnf_data[] = {0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    setCharacteristicsValue(p_context, p_context->barometer_calibration_char_handle.value_handle, cnf_data, sizeof(cnf_data));
    
    data = (uint8_t)p_context->sensor_setting.is_gyroscope_sampling;
    setCharacteristicsValue(p_context, p_context->gyroscope_configration_char_handle.value_handle, &data, 1);
    data = (uint8_t)(p_context->sensor_setting.motionSensorSamplingPeriod / 10 );
    setCharacteristicsValue(p_context, p_context->gyroscope_period_char_handle.value_handle, &data, 1);
    
    data = (uint8_t)p_context->sensor_setting.is_illumination_sampling;
    setCharacteristicsValue(p_context, p_context->illumination_configration_char_handle.value_handle, &data, 1);
    data = (uint8_t)(p_context->sensor_setting.brightnessSamplingPeriod / 10 );
    setCharacteristicsValue(p_context, p_context->illumination_period_char_handle.value_handle, &data, 1);
}

void getSensorTagSetting(ble_sensortag_service_t *p_context, sensorSetting_t *p_dst)
{
    memcpy(p_dst, &(p_context->sensor_setting), sizeof(p_context->sensor_setting));
}

