#include <nordic_common.h>
#include <app_util_platform.h>

#include "senstick_data_model.h"

#include "senstick_control_service.h"
#include "senstick_sensor_controller.h"
#include "metadata_log_controller.h"

#include "twi_slave_rtc.h"
#include "gpio_led_driver.h"

#define ABSTRACT_TEXT_LENGTH 20

typedef struct {
    senstick_control_command_t command;
    uint8_t logCount;
    char text[ABSTRACT_TEXT_LENGTH +1];
    uint8_t text_length;
} senstick_core_data_t;
static senstick_core_data_t context;

// 初期化
void initSenstickDataModel(void)
{
    memset(&context, 0, sizeof(senstick_core_data_t));
}


// コントロールコマンド
senstick_control_command_t senstick_getControlCommand(void)
{
    return context.command;
}

void senstick_setControlCommand(senstick_control_command_t command)
{
    // 2重スタートを禁止します。mainでの初期化のために、センサースリープコマンドだけは重複受付を許します。
    if(command != sensorShouldSleep && context.command == command) {
        return;
    }

    context.command = command;

    // 新しく作るログのID
    const uint8_t new_log_id = context.logCount;
    
    senstickControlService_observeControlCommand(command);
    senstickSensorController_observeControlCommand(command, new_log_id);
    metaDatalog_observeControlCommand(command, new_log_id);
    ledDriver_observeControlCommand(command);

    switch(command) {
        case sensorShouldSleep:
            break;
        case sensorShouldWork:
            // ログカウントを更新
            senstick_setCurrentLogCount( context.logCount + 1);
            break;
        case formattingStorage:
            senstick_setCurrentLogCount( 0 );
            // フォーマット状態からの自動復帰
            senstick_setControlCommand(sensorShouldSleep);
            break;
        case enterDeepSleep:
            break;
        case enterDFUmode:
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
