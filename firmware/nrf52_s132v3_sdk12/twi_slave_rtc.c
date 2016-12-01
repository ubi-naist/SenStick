#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_assert.h>
#include <app_error.h>
#include <sdk_errors.h>

#include "value_types.h"
#include "twi_manager.h"
#include "twi_slave_rtc.h"

#include "senstick_sensor_base_data.h"
#include "senstick_io_definition.h"

/**
 * Definitions
 */

/**
 * Public methods
 */

void initRTC(void)
{
}

void setTWIRTCDateTime(ble_date_time_t *p_date)
{
}

void getTWIRTCDateTime(ble_date_time_t *p_date)
{
    memset(p_date, 0, sizeof(ble_date_time_t));
}

