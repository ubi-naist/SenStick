#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#include "nrf_gpio.h"

#define LEDS_NUMBER         1

#define LED_PIN_NUMBER_LED  10

#define LED_START      LED_PIN_NUMBER_LED
#define LED_0          LED_PIN_NUMBER_LED
#define LED_STOP       LED_PIN_NUMBER_LED

#define LEDS_ACTIVE_STATE   1
#define LEDS_INV_MASK       LEDS_MASK

#define LEDS_LIST { LED_0 }

#define BSP_LED_0       LED_0
//#define BSP_BOARD_LED_2 LED_1

#define BUTTONS_NUMBER 0
/*
#define BUTTONS_NUMBER 1

#define BUTTON_START   21
#define BUTTON_0       21
#define BUTTON_STOP    21
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_0 }


#define BSP_BUTTON_3    BUTTON_0
*/

// dfu/nrf_dfu.hで BSP_BUTTON_3は、BOOTLOADER_BUTTONに定義されている。
// BOOTLOADER_BUTTONは、main.c でnrf_gpio_cfg_sense_inputの引数、ピン番号として使用される
// Pull-up low-active
#define BSP_BUTTON_3    21

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

/*

// LEDs definitions for PCA10028
#define LEDS_NUMBER    1

#define LED_START      22
#define LED_1          22
#define LED_STOP       22

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_1
#define BSP_LED_2      LED_1
#define BSP_LED_3      LED_1

#define BSP_LED_0_MASK (1<<BSP_LED_0)
#define BSP_LED_1_MASK (1<<BSP_LED_1)
#define BSP_LED_2_MASK (1<<BSP_LED_2)
#define BSP_LED_3_MASK (1<<BSP_LED_3)

#define LEDS_MASK      (BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK | BSP_LED_3_MASK)
// all LEDs are lit when GPIO is low
#define LEDS_INV_MASK  LEDS_MASK

#define BUTTONS_NUMBER 1

#define BUTTON_START   20
#define BUTTON_1       20
#define BUTTON_STOP    20
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_LIST { BUTTON_1 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_1
#define BSP_BUTTON_2   BUTTON_1
#define BSP_BUTTON_3   BUTTON_1

#define BSP_BUTTON_0_MASK (1<<BSP_BUTTON_0)
#define BSP_BUTTON_1_MASK (1<<BSP_BUTTON_1)
#define BSP_BUTTON_2_MASK (1<<BSP_BUTTON_2)
#define BSP_BUTTON_3_MASK (1<<BSP_BUTTON_3)

#define BUTTONS_MASK   0x001E0000

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      NRF_CLOCK_LFCLKSRC_XTAL_250_PPM
*/
#endif // CUSTOM_BOARD_H
