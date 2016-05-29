#include <nordic_common.h>
#include <app_util_platform.h>
#include <nrf_log.h>

#include "senstick_data_model.h"

#include "senstick_control_service.h"
#include "senstick_sensor_controller.h"
#include "metadata_log_controller.h"

#include "twi_slave_rtc.h"
#include "gpio_led_driver.h"
#include "twi_manager.h"
#include "gpio_button_monitoring.h"

#include "bootloader_types.h"
#include "nrf51.h"
#include "bootloader_util.h"
#include "nrf_sdm.h"

#define ABSTRACT_TEXT_LENGTH 20

typedef struct {
    senstick_control_command_t command;
    uint8_t logCount;
    char text[ABSTRACT_TEXT_LENGTH +1];
    uint8_t text_length;
    ButtonStatus_t button_status;
    bool is_disk_full;
    bool is_connected;
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
static void startDFU(void)
{
    uint32_t err_code;
    
    err_code = sd_power_gpregret_set(BOOTLOADER_DFU_START);
    APP_ERROR_CHECK(err_code);
    
    err_code = sd_softdevice_disable();
    APP_ERROR_CHECK(err_code);
    
    err_code = sd_softdevice_vector_table_base_set(NRF_UICR->BOOTLOADERADDR);
    APP_ERROR_CHECK(err_code);
    
    NVIC_ClearPendingIRQ(SWI2_IRQn);
    interrupts_disable();
    bootloader_util_app_start(NRF_UICR->BOOTLOADERADDR);
}

void senstick_setControlCommand(senstick_control_command_t command)
{
    // 不定値を弾きます。
    if(   command != sensorShouldSleep
       && command != sensorShouldWork
       && command != formattingStorage
       && command != enterDeepSleep
       && command != enterDFUmode) {
        NRF_LOG_PRINTF_DEBUG("_setControlCommand, unexpected command: %d.\n", command);
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
        case enterDeepSleep:
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
    CRITICAL_REGION_ENTER();
    getTWIRTCDateTime(p_datetime);
    CRITICAL_REGION_EXIT();
}
void senstick_setCurrentDateTime(ble_date_time_t *p_datetime)
{
    CRITICAL_REGION_ENTER();
    setTWIRTCDateTime(p_datetime);
    CRITICAL_REGION_EXIT();
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
            senstick_setControlCommand(enterDeepSleep);
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
void senstick_setIsConnected(bool value)
{
    context.is_connected = value;
}

