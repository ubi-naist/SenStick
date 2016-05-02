#ifndef twi_manager_h
#define twi_manager_h

#include <stdint.h>
#include <stdbool.h>

void initTWIManager(void);

// TWIバスアクセス
bool writeToTwiSlave(uint8_t twi_address, uint8_t target_register, const uint8_t *data, uint8_t length);
bool readFromTwiSlave(uint8_t twi_address, uint8_t target_register, uint8_t *data, uint8_t length);

// TWIバスの電源On/Off
//void IOManagerSetTWIPower(bool awake);

#endif /* twi_manager_h */
