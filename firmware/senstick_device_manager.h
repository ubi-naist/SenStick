#ifndef senstick_device_manager_h
#define senstick_device_manager_h

#include <stdbool.h>

// デバイスマネージャの初期化
// 注意: このメソッドを呼び出す前にpstorageの初期化が完了していること。
void init_device_manager(bool erase_bonds);

#endif /* senstick_device_manager_h */
