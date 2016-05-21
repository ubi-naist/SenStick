#include "senstick_types.h"

bool isValidSenstickControlCommand(uint8_t value)
{
    /*
     sensorShouldSleep = 0x00,
     sensorShouldWork  = 0x01,
     formattingStorage = 0x10,
     enterDeepSleep    = 0x20,
     enterDFUmode      = 0x40*/
    switch(value)
    {
        case 0x00:
        case 0x01:
        case 0x10:
        case 0x20:
        case 0x40:
            return true;
        default:
            return false;
    }
}