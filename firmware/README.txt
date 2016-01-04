/*
 * COPYRIGHT (c) 2010-2012 MACRONIX INTERNATIONAL CO., LTD
 * SPI Flash Low Level Driver (LLD) Sample Codes
 *
 * This README file introduces low level driver of MX25-series.
 *
 * Filename   : README.txt
 * Issued Date: August 28, 2012
 *
 * Any questions or suggestions, please send mails to:
 *
 *     flash_model@mxic.com.tw
 */

 * Notice: All source files are saved as UNIX text format.

This README file describes the functions of MXIC MX25-series low level driver
(LLD) sample codes. It consists of several sections as follows:

1. Overview
2. File structure
3. Functions
4. Porting
5. Return messages

1. Overview
---------------------------------
The MX25-series LLD sample codes are useful for programmers to development
flash drivers or software applications of MX25-series SPI flash products. The
included source codes are ANSI C compliant. Any ANSI C compliant complier can
be compiled with these codes. The MX25-series LLD sample codes have been
verified under hardware and software co-simulation platform with standard 8051
IP core.

All the contents of LLD sample code are for reference only. It might contain
some unknown problems. MACRONIX will not be liable for any claims or damages
due to the use of LLD sample codes.


2. File Structure
---------------------------------
2.1 File content
  The following files will be available after extracting the zipped file
  (or other compression format):

  MX25XXXX\
    |- README.txt
    |- MX25_CMD.c
    |- MX25_CMD.h
    |- MX25_DEF.h
    |- MX25_APP.c

  These files are key elements of LLD and function in different ways. Here are
  the details of these files:

  MX25_CMD.c (.h): Flash command sequences of flash devices. The number
                   of commands depend on the type of devices. All the
                   commands are programmed according to the datasheet of
                   flash products.
  MX25_DEF.h: Definitions of flash devices and program parameters.
  MX25_APP.c: Sample codes of applications for references. This file is not
              a major one of LLD.

  The naming rule of MX25-series LLD sample codes is as follows:

  MX25_XXX.x:
       --- -
        |  |--> c or h. Source codes or header file.
        |
        |-----> CMD, DEF or APP.
                File names represent for different functions.
       
2.2 File dependency
  MX25_APP.c
    |
    |- <stdlib.h> (*Note1)
    |- MX25_CMD.h <-> MX25_CMD.c
         |
         |- <8051.h> (*Note2)
         |- MX25_DEF.h

  *Note1:
   This header file is for MX25_APP.c to generate random data.
  *Note2:
   This header file is optional. It needs to be included if the system
   uses 8051 micro-controller and connected flash device via GPIO port.


3. Function descriptions
--------------------------------
The MX25-series SPI flash contains special I/O transfer modes besides
traditional SPI mode. These modes are QPI and PARALLEL mode (*Note3). LLD
source codes are different between these modes even if the same flash command.
Types of function of LLD are introduced at section 3.1 and command mapping at
section 3.2.

3.1 Types of Function: There are three types of function for LLD as follows:
    a. Basic function:
       Include CS_Low(), CS_High(), InsertDummyCycle(), SendByte() and
       GetByte(). Basic functions are highly hardware dependent. You
       usually need to modify the functions for different hardwares
       applications. Please see section 4 for details.

    b. Command function:
       Command function is the major part of LLD. It is used to describe
       the command sequences of flash commands defined in the datasheet.
       Normally, the sequences from LLD and datasheet are fully matched.
       However, please follow the datasheet if any discrepancies.
       Naming rules of command function are as follows:

           ReturnMsg CMD_XXX(...);

       Where XXX is a command name like RDID, READ, and so on.
       ReturnMsg report the execution result of the command. Section 5 describes
       more about the return messages.

    c. Utility function:
       Those commands which are neither basic nor command functions belong to
       utility functions. Utility function provides some unique functionality
       to make LLD work smoothly.

3.2 Command mapping
    As mentioned at section 3.1.b, the command functions are implementation of
    command sequences defined in the datasheet. In this section, the mapping
    of command sequence and command function is introduced. Here takes the RDID
    command as a example.

    a. RDID command sequence:
    CS# goes low-> send RDID instruction code-> 24-bits ID data out on SO->
    to end RDID operation can drive CS# to high at any time during data out.

    b. RDID command function:
    ReturnMsg CMD_RDID( uint32 *Identification )
    {
        uint32 temp;
        uint8  gDataBuffer[3];

        // CS# goes low
        CS_Low();

        // Send RDID instruction code
        SendByte( FLASH_CMD_RDID, SIO );

        // 24-bits ID data out on SO
        gDataBuffer[0] = GetByte( SIO );
        gDataBuffer[1] = GetByte( SIO );
        gDataBuffer[2] = GetByte( SIO );

        // CS# to high to end RDID operation
        CS_High();

        // Store ID data and return
        temp =  gDataBuffer[0];
        temp =  (temp << 8) | gDataBuffer[1];
        *Identification =  (temp << 8) | gDataBuffer[2];

        return FlashOperationSuccess;
    }
    In this example, the procedures of command sequence and command function
    are matched. The other sequences and functions are similar to this example.
    Feel free to verify the relationship between different commands.

*Note3:
SPI : Flash devices support Serial Peripheral Interface (SPI) mode only.
QPI : Flash devices support both Serial Peripheral Interface (SPI) mode
      and Quad Peripheral Interface (QPI) mode.
SPAR: Flash devices support both Serial Peripheral Interface (SPI) mode
      and serial flash's parallel (SPAR) mode.
      (This parallel mode is not equivalent to the one of MX29 parallel
       flash series.)

4. Porting
--------------------------------
The MX25-series LLD sample codes are verified by 8051 IP core. And it uses
8051's GPIO ports to connect flash device. With different target hardware
platforms, it is necessary to modify the LLD source codes. In this section,
porting notices are introduced. And it is divided into data types, definitions
of options and function modification for your references.

4.1 Data types
    For the ability of port LLD to different target system, the length of data
    type can be modified in the MX25_DEF.h header file. Please verify
    these data types before applying.

    Basic data type:
    typedef    unsigned long     uint32;    /* 32 bit wide */
    typedef    unsigned int      uint16;    /* 16 bit wide */
    typedef    unsigned char     uint8;     /* 8 bit wide */
    typedef    unsigned char     BOOL;      /* 1 bit wide */

4.2 Definition options
    There are several definition options available based on the target system.

    System option:
    #define    MCU8051             /* Use 8051 as micro-controller */
    #define    GPIO_SPI            /* Use GPIO port to connect flash */
    #define    MX25XXX             /* Flash part name */
    #define    NON_SYNCHRONOUS_IO  /* MCU is not busy waiting */

    MCU option:
    #define    CLK_PERIOD          /* MCU clock speed */
    #define    Min_Cycle_Per_Inst  /* Cycle count per instruction */
    #define    One_Loop_Inst       /* Instruction count per loop */

4.3 Function modifications
    As mentioned at section 3.1, basic functions are highly dependent on the
    hardwares. And some utility functions work the same. These functions
    should be modified based on different hardware architectures. They are
    displayed as follows:

       void CS_Low();
       void CS_High();
       void InsertDummyCycle( uint8 dummy_cycle);
       void SendByte( uint8 byte_value, uint8 transfer_type );
       uint8 GetByte( uint8 transfer_type );
       void Wait_Flash_WarmUp();
       void Initial_Spi();

    a. Chip select control
       CS_Low() function is simply to clear chip select pin. CS_High() is to
       set the chip select pin with same method.

       void CS_Low()
       {
       #ifdef GPIO_SPI
           CSn = 0;    /* Clear GPIO port connect to chip select */
       #else
           /* Insert your code here to make chip select go low.
              Related information can be found in the target
              hardware datasheet. */
       #endif
       }

    b. Insert dummy clock cycle
       void InsertDummyCycle( uint8 dummy_cycle )
       {
       #ifdef GPIO_SPI
           uint8 i;
           for( i=0; i < dummy_cycle; i=i+1 )
           {
               SCLK = 0;
               SCLK = 1;
           }
       #else
          /* Insert your code here to generate dummy clock cycle.
             The dummy cycle count should be equal to dummy_cycle parameter. */
       #endif
       }

    c. Send/get one byte data
       There are seven I/O transfer modes in current MX25-series flash. The
       supported I/O modes have been defined in MX25_DEF.h file. Use GPIO
       port to get and send data is default. If your target system has flash
       controllers, read the related specifications for more information. Take
       SendByte() as an example. GetByte() is similar to SendByte() function.

       void SendByte( uint8 byte_value, uint8 transfer_type )
       {
           switch( transfer_type )
           {
       #ifdef SIO
           case SIO:
               /* Insert your code here for single I/O transfer. */
               break;
       #endif
       #ifdef DIO
           case DIO:
               /* Insert your code here for dual I/O transfer.
                  Dual I/O port should send data synchronously. */
               break;
       #endif
       #ifdef QIO
           case QIO:
               /* Insert your code here for quad I/O transfer.
                  Quad I/O port should send data synchronous. */
               break;
       #endif
       #ifdef PIO
           case PIO:
               /* Insert your code here for parallel transfer.
                  Eight I/O port should send data synchronous. */
               break;
       #endif
       #ifdef DTSIO
           case DTSIO:
               /* Insert your code here for DT single I/O transfer.
                  Single I/O port should send data synchronously at
                  positive edge and negative edge of clock. */
               break;
       #endif
       #ifdef DTDIO
           case DTDIO:
               /* Insert your code here for DT dual I/O transfer.
                  Dual I/O port should send data synchronously
                  at positive edge and negative edge of clock. */
               break;
       #endif
       #ifdef DTQIO
           case DTQIO:
               /* Insert your code here for DT quad I/O transfer.
                  Quad I/O port should send data synchronously at
                  positive edge and negative edge of clock. */
               break;
       #endif
           default:
               break;
           }
       }

    d. Initial function
       These functions initiate hardware state and wait for flash to warm-up.
       You may insert some initial codes.

       void Initial_Spi()
       {
       #ifdef GPIO_SPI
           WPn = 1;        // Disable write protected
           SI = 0;         // Flash data input
           SCLK = 1;       // Flash clock input
           CSn = 1;        // Set chip select
       #endif

       /* Insert your initial source code here */

       Wait_Flash_WarmUp();  /* Wait for flash to warm-up until fully
                                accessible. Can be replaced with timer
                                function */
       }

5. Return messages
--------------------------------
The command functions return a message for operation result. These messages are
helpful for programmers to debug at development stage.

    ReturnMsg CMD_XXX( ... );

    XXX:
    Flash command name. Like RDID, READ, RDSR, and so on.

    ReturnMsg:
    -FlashOperationSuccess
     Flash command execute successful.

    -FlashWriteRegFailed
     Write security register failed or enable block protected mode failed.

    -FlashTimeOut
     The time period of write register, program or erase data exceed expected
     time period.

    -FlashIsBusy
     The flash device is busy. Waiting WIP bit clear before next write command.

    -FlashCmdSpiOnly
     This flash command is supported in SPI mode only.

    -FlashCmdQpiOnly
     This flash command is supported in QPI mode only.

    -FlashQuadNotEnable
     This flash command needs to set to QE bit before executing.

    -FlashAddressInvalid
     The input address exceeds flash's address space.

