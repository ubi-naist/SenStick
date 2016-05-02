#ifndef senstick_definitions_h
#define senstick_definitions_h

#include <app_util.h>

// Senstickの動作振る舞いを決めるグローバルな定数を定義します。

// app_timer モジュールのプリスケーラなどの定義
#define APP_TIMER_PRESCALER             0 // Value of the RTC1 PRESCALER register.
#define APP_TIMER_OP_QUEUE_SIZE         4

/* Scheduler のパラメータ */
//  Maximum size of events data in the application scheduler queue aligned to 32 bits
//                                            MAX(BLE_STACK_HANDLER_SCHED_EVT_SIZE)) \

#define SCHED_MAX_EVENT_DATA_SIZE   (CEIL_DIV( \
                                        MAX(sizeof(uint32_t), \
                                            BLE_STACK_HANDLER_SCHED_EVT_SIZE) \
                                        , sizeof(uint32_t)) \
                                      * sizeof(uint32_t))

#define SCHED_QUEUE_SIZE                20

#endif /* senstick_definitions_h */
