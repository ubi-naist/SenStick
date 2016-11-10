//
//  senstick_util.h
//  senstick
//
//  Created by AkihiroUehara on 2016/10/18.
//
//

#ifndef senstick_util_h
#define senstick_util_h

#include <nrf_log.h>

// SDK12をSDK10に合わせるための、マクロ定義
#ifndef NRF_LOG_PRINTF
#define NRF_LOG_PRINTF(...)             NRF_LOG_INTERNAL_DEBUG( __VA_ARGS__)
#endif

#ifndef NRF_LOG_PRINTF_DEBUG
#define NRF_LOG_PRINTF_DEBUG(...)       NRF_LOG_INTERNAL_DEBUG( __VA_ARGS__)
#endif

#endif /* senstick_util_h */
