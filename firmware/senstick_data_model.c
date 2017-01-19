#include <nordic_common.h>
#include <app_util_platform.h>
#include <nrf_log.h>
#include <nrf_sdm.h>

#include "senstick_data_model.h"

#include "senstick_control_service.h"
#include "senstick_sensor_controller.h"
#include "metadata_log_controller.h"

#include "senstick_rtc.h"

#include "gpio_led_driver.h"
#include "twi_manager.h"
#include "gpio_button_monitoring.h"

#ifdef NRF52832
// nRF51, S132, SDK12
#include <nrf_dfu_settings.h>
#include <ble_hci.h>
#else
// nRF51, S110, SDK10
// これらのヘッダファイルに、startDFU()メソッドが、ブートローダにDFUに入らせるための定義がある。
#include "bootloader_types.h"
#include "nrf51.h"
#include "bootloader_util.h"
#include "nrf_sdm.h"
#endif

#define ABSTRACT_TEXT_LENGTH 20

typedef struct {
    senstick_control_command_t command;
    uint8_t logCount;
    char text[ABSTRACT_TEXT_LENGTH +1];
    uint8_t text_length;
    ButtonStatus_t button_status;
    bool is_disk_full;
    bool is_connected;
    
    uint16_t conn_handle;
    bool is_waiting_disconnect_for_dfu;
} senstick_core_data_t;
static senstick_core_data_t context;

// 初期化
void initSenstickDataModel(void)
{
    memset(&context, 0, sizeof(senstick_core_data_t));
    context.command = shouldReset;
}

// コントロールコマンド
senstick_control_command_t senstick_getControlCommand(void)
{
    return context.command;
}

#define MAX_NUMBER_INTERRUPTS  32
#define IRQ_ENABLED            0x01
static void interrupts_disable(void)
{
    uint32_t interrupt_setting_mask;
    uint32_t irq;
    
    // Fetch the current interrupt settings.
    interrupt_setting_mask = NVIC->ISER[0];
    
    // Loop from interrupt 0 for disabling of all interrupts.
    for (irq = 0; irq < MAX_NUMBER_INTERRUPTS; irq++)
    {
        if (interrupt_setting_mask & (IRQ_ENABLED << irq))
        {
            // The interrupt was enabled, hence disable it.
            NVIC_DisableIRQ((IRQn_Type)irq);
        }
    }
}
/*
#ifdef NRF52
void flash_callback(fs_evt_t const * const evt, fs_ret_t result)
{
    if (result == FS_SUCCESS)
    {
        NRF_LOG_INFO("Obtained settings, enter dfu is %d\n", s_dfu_settings.enter_buttonless_dfu);

        // フラグを立てる。
        context.is_waiting_disconnect_for_dfu = true;
        // 切断していればシステムリセットして再起動、または切断処理
        if(context.is_connected == false) {
            NVIC_SystemReset();
        } else {
            sd_ble_gap_disconnect(context.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        }
    }
}
#endif
*/
#ifdef NRF52
#define BOOTLOADER_DFU_START 0xB1
static void startDFU(void)
{
    uint32_t err_code;
    
    // nRF52でもnRF51と同じgpregを使う方式で、DFUフラグをブートローダに伝える。
    err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START); // 0 for GPREGRET, 1 for GPREGRET2.
    APP_ERROR_CHECK(err_code);

    if( context.is_connected == false ) {
        NVIC_SystemReset();
    } else {
        context.is_waiting_disconnect_for_dfu = true;
        sd_ble_gap_disconnect(context.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }    
/*
    err_code = sd_softdevice_disable();
    APP_ERROR_CHECK(err_code);
    
    err_code = sd_softdevice_vector_table_base_set(NRF_UICR->BOOTLOADERADDR);
    APP_ERROR_CHECK(err_code);
    
    NVIC_ClearPendingIRQ(SWI2_IRQn);
    interrupts_disable();
    bootloader_util_app_start(NRF_UICR->BOOTLOADERADDR);
    //#endif
*/
}
#else // nRF51
static void startDFU(void)
{
    uint32_t err_code;
    
    // SDK10のブートローダは、リセットされても内容が保持されるリテンションレジスタを使って、DFU更新をブートローダに伝える。
    // BOOTLOADER_DFU_STARTは、ブートローダの実装側が bootloader_types.h で　0xB1に定義している。
    err_code = sd_power_gpregret_set(BOOTLOADER_DFU_START);
    APP_ERROR_CHECK(err_code);
    
    err_code = sd_softdevice_disable();
    APP_ERROR_CHECK(err_code);
    
    err_code = sd_softdevice_vector_table_base_set(NRF_UICR->BOOTLOADERADDR);
    APP_ERROR_CHECK(err_code);
    
    NVIC_ClearPendingIRQ(SWI2_IRQn);
    interrupts_disable();
    bootloader_util_app_start(NRF_UICR->BOOTLOADERADDR);
    //#endif
}
#endif

void senstick_setControlCommand(senstick_control_command_t command)
{
    // 不定値を弾きます。
    if(   command != sensorShouldSleep
       && command != sensorShouldWork
       && command != formattingStorage
       && command != shouldDeviceSleep
       && command != enterDFUmode) {
//       NRF_LOG_PRINTF_DEBUG("_setControlCommand, unexpected command: %d.\n", command);
        return;
    }
    
    // 同じステートの再呼び出しを禁止します。
    if(context.command == command) {
        return;
    }

    // コマンド実行のアボート。disk fullのときには sensorShouldWork 状態には遷移させません。
    if( command == sensorShouldWork && senstick_isDiskFull() ) {
        return;
    }
    
    // ログ取得できるセンサーがない場合(センサーがハードウェア無効、ログ/リアルタイム動作にない)、動作開始させません。
    uint8_t numOfWillLoggingSensors = senstickSensorControllerGetNumOfLoggingReadySensor();
    if(command == sensorShouldWork && numOfWillLoggingSensors == 0) {
        return;
    }
    
    // コマンドの実行
    // 新旧コマンドを保存
    senstick_control_command_t old_command = context.command;
    senstick_control_command_t new_command = command;

    context.command = command;

    // 新しく作るログのID
    const uint8_t new_log_id = senstick_getCurrentLogCount();
    
    senstickControlService_observeControlCommand(new_command);
    senstickSensorController_observeControlCommand(new_command, new_log_id);
    metaDatalog_observeControlCommand(old_command, new_command, new_log_id);
    ledDriver_observeControlCommand(new_command);

    // 本当はモデルに書くべきではないけど、コントローラの機能をここに直書き。
    switch(command) {
        case sensorShouldSleep:
            break;
        case sensorShouldWork:
            // ログカウントを更新
            senstick_setCurrentLogCount( context.logCount + 1);
            break;
        case formattingStorage:
            // フォーマットの実行, 実際のフォーマット処理は、上記のオブザーバで処理されているはず
            // このモデルのis_header_fullなどの更新をここでする。
            senstick_setDiskFull(false);
            senstick_setCurrentLogCount( 0 );
            // フォーマット状態からの自動復帰
            senstick_setControlCommand(sensorShouldSleep);
            break;
        case shouldDeviceSleep:
            // ボタンで起動するように設定。
            enableAwakeByButton();
            // パワーを落とします。
            twiPowerDown();
            sd_power_system_off();
            break;
        case enterDFUmode:
            startDFU();
            break;
        default: break;
    }
}

// 現在有効なログデータ数, uint8_t
uint8_t senstick_getCurrentLogCount(void)
{
    return context.logCount;
}

void senstick_setCurrentLogCount(uint8_t count)
{
    context.logCount = count;
    
    senstickControlService_observeCurrentLogCount(count);
}

uint8_t senstick_isDiskFull(void)
{
    return context.is_disk_full;
}
void senstick_setDiskFull(bool flag)
{
    context.is_disk_full = flag;
    
    senstickControlService_observeDiskFull(flag);
}

// 現在の時刻
void senstick_getCurrentDateTime(ble_date_time_t *p_datetime)
{
    getSenstickRTCDateTime(p_datetime);
}
void senstick_setCurrentDateTime(ble_date_time_t *p_datetime)
{
    setSenstickRTCDateTime(p_datetime);
}

// 現在のログテキスト概要
uint8_t senstick_getCurrentLogAbstractText(char *str, uint8_t str_length)
{
    uint8_t length = MIN(str_length,  context.text_length);
    strncpy(str, context.text, length);

    return length;
}

void senstick_setCurrentLogAbstractText(char *str, uint8_t length)
{
    context.text_length = MIN(length, ABSTRACT_TEXT_LENGTH);
    strncpy(context.text, str, context.text_length);
}

// ボタン状態
ButtonStatus_t senstick_getButtonStatus(void)
{
    return context.button_status;
}
void senstick_setButtonStatus(ButtonStatus_t status)
{
//    NRF_LOG_PRINTF_DEBUG("senstick_setButtonStatus: %d\n",  status);
    context.button_status = status;
    
    // observer
    ledDriver_observeButtonStatus(status);
    
    // ボタンが押された時の、コントローラ。mainに書くべきだが、mainに書くのもここに書くのも違いなさそうなので。
    switch(status) {
        case BUTTON_RELEASED:
            break;
        case BUTTON_PUSH:
            break;
        case BUTTON_PUSH_RELEASED:
            // 次のログ記録開始
            senstick_setControlCommand(sensorShouldSleep);
            senstick_setControlCommand(sensorShouldWork);
            break;
        case BUTTON_LONG_PUSH: break;
        case BUTTON_LONG_PUSH_RELEASED:
            // 長押しでDeepSleep
            senstick_setControlCommand(shouldDeviceSleep);
            break;            
        case BUTTON_VERY_LONG_PUSH: break;
        case BUTTON_VERY_LONG_PUSH_RELEASED:
            // 長時間押したらフォーマットに落とします。
            senstick_setControlCommand(formattingStorage);
            break;
        default:
            break;
    }
}

bool senstick_isConnected(void)
{
    return context.is_connected;
}
void senstick_setIsConnected(bool value, uint16_t conn_handle)
{
    context.is_connected = value;
    context.conn_handle = conn_handle;
    
    // 切断時にシステムリセットして再起動
    if (context.is_waiting_disconnect_for_dfu && context.is_connected == false)
    {
        NVIC_SystemReset();
    }
}

