#include <nordic_common.h>

#include "senstick_data_model.h"

#include "senstick_control_service.h"
#include "senstick_sensor_controller.h"

#define ABSTRACT_TEXT_LENGTH 20

typedef struct {
    senstick_control_command_t command;
    uint8_t logCount;
    uint8_t text[ABSTRACT_TEXT_LENGTH +1];
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
    context.command = command;
    
    senstickControlService_observeControlCommand(command);
    senstickSensorController_observeControlCommand(command);
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
    // TBD
}
void senstick_setCurrentDateTime(ble_date_time_t *p_datetime)
{
    // TBD
}

// 現在のログテキスト概要
uint8_t senstick_getCurrentLogAbstractText(uint8_t *str, uint8_t str_length)
{
    uint8_t length = MIN(str_length,  context.text_length);
    strncpy((char *)str, (char *)context.text, length);

    return length;
}

void senstick_setCurrentLogAbstractText(uint8_t *str, uint8_t length)
{
    context.text_length = MIN(length, ABSTRACT_TEXT_LENGTH);
    strncpy((char *)context.text, (char *)str, context.text_length);
}
