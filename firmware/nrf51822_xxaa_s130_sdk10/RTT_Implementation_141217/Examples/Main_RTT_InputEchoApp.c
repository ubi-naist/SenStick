/*********************************************************************
*          SEGGER MICROCONTROLLER SYSTEME GmbH
*   Solutions for real time microcontroller applications
**********************************************************************
File    : Main_RTT_MenuApp.c
Purpose : Sample application to demonstrate RTT bi-directional functionality
          Target Hardware: Debug J-Link ULTRA/PRO V4
--------- END-OF-HEADER --------------------------------------------*/

#define MAIN_C

#include <stdio.h>

#include "SEGGER_RTT.h"

volatile int _Cnt;
volatile int _Delay;

static char r;

/*********************************************************************
*
*       main
*/
void main(void) {

  SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n");
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  do {
    r = SEGGER_RTT_WaitKey();
    SEGGER_RTT_Write(0, &r, 1);
    r++;
  } while (1);
}

/*************************** End of file ****************************/
