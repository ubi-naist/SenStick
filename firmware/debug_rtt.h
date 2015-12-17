#ifndef _DEBUG_RTT_H_
#define _DEBUG_RTT_H_

#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#include <stdio.h>

/**
 * @defgroup rtt_logger SEGGER Real time terminal logger
 * @{
 * @brief Enables debug logs over SEGGER Real time terminal.
 */
#ifdef ENABLE_RRT_DEBUG_LOG
extern char __dgbstr[128];
#define RTT_PRINTF(...) \
do { snprintf(__dgbstr, sizeof(__dgbstr) -1, __VA_ARGS__); SEGGER_RTT_WriteString(0, __dgbstr); } while(0);

#define LOG RTT_PRINTF

#define LOG_ASSERT(expr) if(expr) { } else { LOG("\nAssertion failure: line:%d file:%s.", (uint16_t)__LINE__, (uint8_t *)__FILE__); }

#else // ENABLE_RRT_DEBUG_LOG
#define RTT_PRINTF(...)
#define LOG(...)
#define LOG_ASSERT(expr)
#endif // ENABLE_RRT_DEBUG_LOG

#endif // _DEBUG_RTT_H_
