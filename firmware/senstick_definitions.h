#ifndef senstick_definitions_h
#define senstick_definitions_h

// Senstickの動作振る舞いを決めるグローバルな定数を定義します。

// app_timer モジュールのプリスケーラなどの定義
#define APP_TIMER_PRESCALER             0 // Value of the RTC1 PRESCALER register.
#define APP_TIMER_OP_QUEUE_SIZE         4

/* Scheduler のパラメータ */
#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVT_SIZE, BLE_STACK_HANDLER_SCHED_EVT_SIZE)       /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                10

#endif /* senstick_definitions_h */
