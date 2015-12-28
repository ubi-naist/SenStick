#ifndef twi_slave_utility_h
#define twi_slave_utility_h

#include <stdint.h>

#include "nrf_drv_twi.h"
#include "senstick_data_models.h"

void writeToTwiSlave(nrf_drv_twi_t *p_twi, uint8_t twi_address, uint8_t target_register, const uint8_t *data, uint8_t data_length);
void readFromTwiSlave(nrf_drv_twi_t *p_twi, uint8_t twi_address, uint8_t target_register, uint8_t *data, uint8_t data_length);

#endif /* twi_slave_utility_h */
