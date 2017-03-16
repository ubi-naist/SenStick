/*
 * COPYRIGHT (c) 2010-2012 MACRONIX INTERNATIONAL CO., LTD
 * SPI Flash Low Level Driver (LLD) Sample Code
 *
 * SPI and QPI interface command set
 *
 * $Id: MX25_CMD.c,v 1.18 2012/08/27 08:03:08 mxclldb1 Exp $
 */

#include "MX25_CMD.h"

/*
 --Common functions
 */

/*
 * Function:       Wait_Flash_WarmUp
 * Arguments:      None.
 * Description:    Wait some time until flash read / write enable.
 * Return Message: None.
 */
void Wait_Flash_WarmUp()
{
    uint32 time_cnt = FlashFullAccessTime;
    while( time_cnt > 0 )
    {
        time_cnt--;
    }
}

/*
 * Function:       Initial_Spi
 * Arguments:      None
 * Description:    Initial spi flash state and wait flash warm-up
 *                 (enable read / write).
 * Return Message: None
 */
void Initial_Spi()
{

#ifdef GPIO_SPI
    WPn = 1;        // Write potected initial high
    SI = 0;         // Flash input data
    SCLK = 1;       // Flash input clock
    CSn = 1;        // Chip Select
#endif

    // Wait flash warm-up
    Wait_Flash_WarmUp();
}

/*
 * Function:       CS_Low, CS_High
 * Arguments:      None.
 * Description:    Chip select go low / high.
 * Return Message: None.
 */
void CS_Low()
{
#ifdef GPIO_SPI
    CSn = 0;
#else
    //--- insert your chip select code here. ---//
#endif
}

void CS_High()
{
#ifdef GPIO_SPI
    CSn = 1;
#else
    //--- insert your chip select code here. ---//
#endif
}

/*
 * Function:       InsertDummyCycle
 * Arguments:      dummy_cycle, number of dummy clock cycle
 * Description:    Insert dummy cycle of SCLK
 * Return Message: None.
 */
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
    //--- insert your code here. ---//
#endif
}

/*
 * Function:       SendByte
 * Arguments:      byte_value, data transfer to flash
 *                 transfer_type, select different type of I/O mode.
 *                 Seven mode:
 *                 SIO, single IO
 *                 DIO, dual IO
 *                 QIO, quad IO
 *                 PIO, parallel
 *                 DTSIO, double transfer rate SIO
 *                 DTDIO, double transfer rate DIO
 *                 DTQIO, double transfer rate QIO
 * Description:    Send one byte data to flash
 * Return Message: None.
 */
void SendByte( uint8 byte_value, uint8 transfer_type )
{
#ifdef GPIO_SPI
    uint16 i;
    uint8 cycle_cnt;

    switch( transfer_type )
    {
#ifdef SIO
    case SIO: // single I/O
        cycle_cnt = 8;
        for( i= 0; i < cycle_cnt; i++ )
        {
            SCLK = 0;
            if ( (byte_value & IO_MASK) == 0x80 ){
                SI = 1;
            }
            else{
                SI = 0;
            }
            byte_value = byte_value << 1;
            SCLK = 1;
        }
        break;
#endif
#ifdef DIO
    case DIO: // dual I/O
        cycle_cnt = 4;
        for( i= 0; i < cycle_cnt; i++ )
        {
            SCLK = 0;
            P1 = (( byte_value & 0xc0 ) >> 2) | (P1 & 0xcf);
            byte_value = byte_value << 2;
            SCLK = 1;
        }
        break;
#endif
#ifdef QIO
    case QIO: // quad I/O
        cycle_cnt = 2;
        for( i= 0; i < cycle_cnt; i++ )
        {
            P1 = (byte_value & 0xf0);  // CS# and SCLK must be zero at this phase
            byte_value = byte_value << 4;
            SCLK = 1;
        }
        break;
#endif
#ifdef PIO
    case PIO: //  Parallel I/O
        SCLK = 0;
           PO7 = ( (byte_value & IO_MASK)  == IO_MASK )?1:0;
        PO6 = ( (byte_value & 0x40)  == 0x40 )?1:0;
        P3 = (byte_value & 0x3f ) | (P3 & 0xc0);
        SCLK = 1;
        break;
#endif
#ifdef DTSIO
    case DTSIO: //  Double transfer rate single I/O
        cycle_cnt = 4;
         for( i= 0; i < cycle_cnt; i++ )
        {
             SCLK = 0;
            SI = ( (byte_value & IO_MASK) == IO_MASK)?1:0;
            byte_value = byte_value << 1;

            SCLK = 1;
            SI = ( (byte_value & IO_MASK) == IO_MASK)?1:0;
            byte_value = byte_value << 1;
        }
        break;
#endif
#ifdef DTDIO
    case DTDIO: //  Double transfer rate dual I/O
        cycle_cnt = 2;
        for( i= 0; i < cycle_cnt; i++ )
        {
            SCLK = 0;
            P1 = (( byte_value & 0xc0 ) >> 2) | (P1 & 0xcf);
            byte_value = byte_value << 2;

            SCLK = 1;
            P1 = (( byte_value & 0xc0 ) >> 2) | (P1 & 0xcf);
            byte_value = byte_value << 2;
        }
        break;
#endif
#ifdef DTQIO
    case DTQIO: //  Double transfer rate quad I/O
        SCLK = 0;
        P1 = (byte_value & 0xf0);

        SCLK = 1;
        byte_value = byte_value << 4;

        P1 = (byte_value & 0xf0);  // CS# and SCLK must be zero at this phase
        break;
#endif
    default:
        break;
    }
#else
    switch( transfer_type )
    {
#ifdef SIO
    case SIO: // Single I/O
        //--- insert your code here for single IO transfer. ---//
        break;
#endif
#ifdef DIO
    case DIO: // Dual I/O
        //--- insert your code here for dual IO transfer. ---//
        break;
#endif
#ifdef QIO
    case QIO: // Quad I/O
        //--- insert your code here for quad IO transfer. ---//
        break;
#endif
#ifdef PIO
    case PIO: // Parallel I/O
        //--- insert your code here for parallel IO transfer. ---//
        break;
#endif
#ifdef DTSIO
    case DTSIO: // Double transfer rate Single I/O
        //--- insert your code here for DT single IO transfer. ---//
        break;
#endif
#ifdef DTDIO
    case DTDIO: // Double transfer rate Dual I/O
        //--- insert your code here for DT dual IO transfer. ---//
        break;
#endif
#ifdef DTQIO
    case DTQIO: // Double transfer rate Quad I/O
        //--- insert your code here for DT quad IO transfer. ---//
        break;
#endif
    default:
        break;
    }
#endif  /* End of GPIO_SPI */
}
/*
 * Function:       GetByte
 * Arguments:      byte_value, data receive from flash
 *                 transfer_type, select different type of I/O mode.
 *                 Seven mode:
 *                 SIO, single IO
 *                 DIO, dual IO
 *                 QIO, quad IO
 *                 PIO, parallel IO
 *                 DTSIO, double transfer rate SIO
 *                 DTDIO, double transfer rate DIO
 *                 DTQIO, double transfer rate QIO
 * Description:    Get one byte data to flash
 * Return Message: 8 bit data
 */
uint8 GetByte( uint8 transfer_type )
{
#ifdef GPIO_SPI
    uint16 i;
    uint8 cycle_cnt;
    uint8 data_buf;

    data_buf = 0;
    cycle_cnt = 8 >> transfer_type;

    switch( transfer_type )
    {
#ifdef SIO
    case SIO: // single I/O
        // Set VIP 8051 GPIO as input ( need pull to high )
        SO = 1;
        // End VIP 8051 GPIO

        for( i= 0; i < cycle_cnt; i++ )
        {
            SCLK = 0;
            if ( SO == 1 ){
                data_buf = (data_buf | (0x80 >> i));
            }
            SCLK = 1;
        }
        break;
#endif
#ifdef DIO
    case DIO: // dual I/O
        // Set VIP 8051 GPIO as input ( need pull to high )
        SIO0 = 1;
        SIO1 = 1;
        // End VIP 8051 GPIO

        for( i= 0; i < cycle_cnt; i++ )
        {
            SCLK = 0;
            data_buf = data_buf << 2;
            data_buf = ( data_buf | ((P1 & 0x30) >> 4 ) );
            SCLK = 1;
        }
        break;
#endif
#ifdef QIO
    case QIO: // quad I/O
        // Set VIP 8051 GPIO as input ( need pull to high )
        SIO0 = 1;
        SIO1 = 1;
        SIO2 = 1;
        SIO3 = 1;
        // End VIP 8051 GPIO

        SCLK = 0;
        data_buf = P1 & 0xf0;
        SCLK = 1;

        SCLK = 0;
        data_buf = ((P1 & 0xf0)>> 4)| data_buf;
        SCLK = 1;
        break;
#endif
#ifdef PIO
    case PIO: //  Parallel I/O
        // Set VIP 8051 GPIO as input ( need pull to high )
        PO7 = 1;
        PO6 = 1;
        PO5 = 1;
        PO4 = 1;
        PO3 = 1;
        PO2 = 1;
        PO1 = 1;
        PO0 = 1;
        // End VIP 8051 GPIO

        SCLK = 0;
        data_buf = (( P1 & 0x20 )<< 2) | ((P1 & 0x02 ) << 5) | (P3 & 0x3f );
        SCLK = 1;
        break;
#endif
#ifdef DTSIO
    case DTSIO: //  Double transfer rate Single I/O
        // Set VIP 8051 GPIO as input ( need pull to high )
        SO = 1;
        // End VIP 8051 GPIO

        cycle_cnt = 4;
        for( i= 0; i < cycle_cnt; i++ )
        {
            SCLK = 0;
            if ( SO == 1 ){

                data_buf = (data_buf | ( 0x80 >> (i*2) ));
            }
            SCLK = 1;
            if ( SO == 1 ){
                data_buf = (data_buf | (0x80 >> ((i*2) + 1) ));
            }
        }
        break;
#endif
#ifdef DTDIO
    case DTDIO: //  Double transfer rate Dual I/O
        // Set VIP 8051 GPIO as input ( need pull to high )
        SIO0 = 1;
        SIO1 = 1;
        // End VIP 8051 GPIO

        cycle_cnt = 2;
        for( i= 0; i < cycle_cnt; i++ )
        {
            SCLK = 0;
            data_buf = data_buf << 2;
            data_buf = ( data_buf  | ( (P1 & 0x30) >> 4 ) );

            SCLK = 1;
            data_buf = data_buf << 2;
            data_buf = ( data_buf  | ( (P1 & 0x30) >> 4 ) );
        }
        break;
#endif
#ifdef DTQIO
    case DTQIO: //  DTR qual I/O
        // Set VIP 8051 GPIO as input ( need pull to high )
        SIO0 = 1;
        SIO1 = 1;
        SIO2 = 1;
        SIO3 = 1;
        // End VIP 8051 GPIO

        SCLK = 0;
        data_buf = P1 & 0xf0;
        SCLK = 1;
        data_buf = ( (P1 & 0xf0) >> 4 )| data_buf;
        break;
#endif
    default:
        break;

    }
#else
    switch( transfer_type )
    {
#ifdef SIO
    case SIO: // Single I/O
        //--- insert your code here for single IO receive. ---//
        break;
#endif
#ifdef DIO
    case DIO: // Dual I/O
        //--- insert your code here for dual IO receive. ---//
        break;
#endif
#ifdef QIO
    case QIO: // Quad I/O
        //--- insert your code here for qual IO receive. ---//
        break;
#endif
#ifdef PIO
    case PIO: // Parallel I/O
        //--- insert your code here for parallel IO receive. ---//
        break;
#endif
#ifdef DTSIO
    case DTSIO: // Double transfer rate Single I/O
        //--- insert your code here for DT single IO receive. ---//
        break;
#endif
#ifdef DTDIO
    case DTDIO: // Double transfer rate Dual I/O
        //--- insert your code here for DT dual IO receive. ---//
        break;
#endif
#ifdef DTQIO
    case DTQIO: // Double transfer rate Qual I/O
        //--- insert your code here for DT quad IO receive. ---//
#endif
    default:
        break;
    }
#endif  /* End of GPIO_SPI */
    return data_buf;
}

/*
 * Function:       WaitFlashReady
 * Arguments:      ExpectTime, expected time-out value of flash operations.
 *                 No use at non-synchronous IO mode.
 *                 fsptr, pointer of flash status structure
 *                 No use at non-synchronous IO mode.
 * Description:    Synchronous IO:
 *                 If flash is ready return TRUE.
 *                 If flash is time-out return FALSE.
 *                 Non-synchronous IO:
 *                 Always return TRUE
 * Return Message: TRUE, FALSE
 */
BOOL WaitFlashReady( uint32  ExpectTime, FlashStatus *fsptr )
{
#ifndef NON_SYNCHRONOUS_IO
    uint32 temp = 0;
    while( IsFlashBusy( fsptr ) )
    {
        if( temp > ExpectTime )
        {
            return FALSE;
        }
        temp = temp + 1;
    }
       return TRUE;

#else
    return TRUE;
#endif
}

/*
 * Function:       WaitRYBYReady
 * Arguments:      ExpectTime, expected time-out value of flash operations.
 *                 No use at non-synchronous IO mode.
 * Description:    Synchronous IO:
 *                 If flash is ready return TRUE.
 *                 If flash is time-out return FALSE.
 *                 Non-synchronous IO:
 *                 Always return TRUE
 * Return Message: TRUE, FALSE
 */
BOOL WaitRYBYReady( uint32 ExpectTime )
{
#ifndef NON_SYNCHRONOUS_IO
    uint32 temp = 0;
#ifdef GPIO_SPI
    while( SO == 0 )
#else
    // Insert your code for waiting RYBY (SO) pin ready
#endif
    {
        if( temp > ExpectTime )
        {
            return FALSE;
        }
        temp = temp + 1;
    }
    return TRUE;

#else
    return TRUE;
#endif
}

/*
 * Function:       IsFlashBusy
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    Check status register WIP bit.
 *                 If  WIP bit = 1: return TRUE ( Busy )
 *                             = 0: return FALSE ( Ready ).
 * Return Message: TRUE, FALSE
 */
BOOL IsFlashBusy( FlashStatus *fsptr )
{
    uint8  gDataBuffer;

    CMD_RDSR( &gDataBuffer, fsptr  );
    if( (gDataBuffer & FLASH_WIP_MASK)  == FLASH_WIP_MASK )
        return TRUE;
    else
        return FALSE;
}
/*
 * Function:       IsFlashQPI
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    Return flash current state: SPI or QPI
 * Return Message: QPI enable or not ( TRUE / FALSE )
 */
BOOL IsFlashQPI( FlashStatus *fsptr )
{
    if( (fsptr->ModeReg&0x01) == 0x01 )
        return TRUE;
    else
        return FALSE;
}

/*
 * Function:       IsFlashQIO
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    If flash QE bit = 1: return TRUE
 *                                 = 0: return FALSE.
 * Return Message: TRUE, FALSE
 */
BOOL IsFlashQIO( FlashStatus *fsptr )
{
    uint8  gDataBuffer;
    CMD_RDSR( &gDataBuffer, fsptr );
    if( (gDataBuffer & FLASH_QE_MASK) == FLASH_QE_MASK )
        return TRUE;
    else
        return FALSE;
}

/*
 * Function:       IsFlash4Byte
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    Check flash address is 3-byte or 4-byte.
 *                 If flash 4BYTE bit = 1: return TRUE
 *                                    = 0: return FALSE.
 * Return Message: TRUE, FALSE
 */
BOOL IsFlash4Byte( FlashStatus *fsptr )
{
#ifdef FLASH_CMD_RDCR
    uint8  gDataBuffer;
    CMD_RDCR( &gDataBuffer, fsptr );
    if( (gDataBuffer & FLASH_4BYTE_CF_MASK) == FLASH_4BYTE_CF_MASK )
        return TRUE;
    else
        return FALSE;
#elif FLASH_CMD_RDSCUR
    uint8  gDataBuffer;
    CMD_RDSCUR( &gDataBuffer, fsptr );
    if( (gDataBuffer & FLASH_4BYTE_MASK) == FLASH_4BYTE_MASK )
        return TRUE;
    else
        return FALSE;
#else
    return FALSE;
#endif
}

/*
 * Function:       SendFlashAddr
 * Arguments:      flash_address, 32 bit flash memory address
 *                 io_mode, I/O mode to transfer address
 *                 addr_4byte_mode,
 * Description:    Send flash address with 3-byte or 4-byte mode.
 * Return Message: None
 */
void SendFlashAddr( uint32 flash_address, uint8 io_mode, BOOL addr_4byte_mode )
{
    /* Check flash is 3-byte or 4-byte mode.
       4-byte mode: Send 4-byte address (A31-A0)
       3-byte mode: Send 3-byte address (A23-A0) */
    if( addr_4byte_mode == TRUE ){
        SendByte( (flash_address >> 24), io_mode ); // A31-A24
    }
    /* A23-A0 */
    SendByte( (flash_address >> 16), io_mode );
    SendByte( (flash_address >> 8), io_mode );
    SendByte( (flash_address), io_mode );
}

/*
 * Function:       GetDummyCycle
 * Arguments:      default_cycle, default dummy cycle
 *                 fsptr, pointer of flash status structure
 * Description:    Get dummy cycle for different condition
 *                 default_cycle: Byte3 | Byte2 | Byte1 | Byte0
 *                      DC 1 bit:   x       1       x       0
 *                      DC 2 bit:   11      10      01      00 
 *                 Note: the variable dummy cycle only support
                         in some product. 
 * Return Message: Dummy cycle value
 */
uint8 GetDummyCycle( uint32 default_cycle, FlashStatus *fsptr )
{
#ifdef FLASH_CMD_RDCR
    uint8 gDataBuffer;
    uint8 dummy_cycle = default_cycle;
    CMD_RDCR( &gDataBuffer, fsptr );
    #if SUPPORT_CR_DC == 1
        // product support 1-bit dummy cycle configuration
        if( (gDataBuffer & FLASH_DC_MASK) == FLASH_DC_MASK )
            dummy_cycle = default_cycle >> 16;
        else
            dummy_cycle = default_cycle;
    #elif SUPPORT_CR_DC_2bit == 1
        // product support 2-bit dummy cycle configuration
        switch( gDataBuffer & FLASH_DC_2BIT_MASK ){
            case 0x00:
                dummy_cycle = default_cycle;
            break;
            case 0x40:
                dummy_cycle = default_cycle >> 8;
            break;
            case 0x80:
                dummy_cycle = default_cycle >> 16;
            break;
            case 0xC0:
                dummy_cycle = default_cycle >> 24;
            break;
        }
    #else
         // configuration register not support dummy configuration
         dummy_cycle = default_cycle;
    #endif
    return dummy_cycle;
#else
    // default case: return default dummy cycle
    return default_cycle; 
#endif
}


/*
 * ID Command
 */

/*
 * Function:       CMD_RDID
 * Arguments:      Identification, 32 bit buffer to store id
 *                 fsptr, pointer of flash status structure
 * Description:    The RDID instruction is to read the manufacturer ID
 *                 of 1-byte and followed by Device ID of 2-byte.
 * Return Message: FlashCmdSpiOnly, FlashOperationSuccess
 */
ReturnMsg CMD_RDID( uint32 *Identification, FlashStatus *fsptr )
{
    uint32 temp;
    uint8  gDataBuffer[3];

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )    return    FlashCmdSpiOnly;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_RDID, SIO );

    // Get manufacturer identification, device identification
    gDataBuffer[0] = GetByte( SIO );
    gDataBuffer[1] = GetByte( SIO );
    gDataBuffer[2] = GetByte( SIO );

    // Chip select go high to end a command
    CS_High();

    // Store identification
    temp =  gDataBuffer[0];
    temp =  (temp << 8 )  | gDataBuffer[1];
    *Identification =  (temp << 8)  | gDataBuffer[2];

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_RES
 * Arguments:      ElectricIdentification, 8 bit buffer to store electric id
 *                 fsptr, pointer of flash status structure
 * Description:    The RES instruction is to read the Device
 *                 electric identification of 1-byte.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RES( uint8 *ElectricIdentification, FlashStatus *fsptr )
{
    uint8    io_mode;
    uint8    dummy_cycle;

    // Check current state: SPI or QPI
    #ifdef RES_SPI_ONLY
        if( IsFlashQPI( fsptr ) != FALSE ) return FlashCmdSpiOnly;
        io_mode = SIO;
        dummy_cycle = 24;
    #else 
        if( IsFlashQPI( fsptr ) ){
            io_mode = QIO;
            dummy_cycle = 10;
        }
        else{
            io_mode = SIO;
            dummy_cycle = 24;
        }
    #endif

    // Chip select go low to start a flash command
    CS_Low();

    // Send flash command and insert dummy cycle ( if need )
    SendByte( FLASH_CMD_RES, io_mode );
    InsertDummyCycle( dummy_cycle );

    // Get electric identification
    *ElectricIdentification = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_REMS
 * Arguments:      REMS_Identification, 16 bit buffer to store id
 *                 fsptr, pointer of flash status structure
 * Description:    The REMS instruction is to read the Device
 *                 manufacturer ID and electric ID of 1-byte.
 * Return Message: FlashCmdSpiOnly, FlashOperationSuccess
 */
ReturnMsg CMD_REMS( uint16 *REMS_Identification, FlashStatus *fsptr )
{
    uint8  gDataBuffer[2];

    // Check current state
    if( IsFlashQPI( fsptr ) )    return    FlashCmdSpiOnly;

    // Chip select go low to start a flash command
    CS_Low();

    // Send flash command and insert dummy cycle ( if need )
    // ArrangeOpt = 0x00 will output the manufacturer's ID first
    //              = 0x01 will output electric ID first
    // Default is 0x00
    SendByte( FLASH_CMD_REMS, SIO );
    InsertDummyCycle( 16 );
    SendByte( fsptr->ArrangeOpt, SIO );

    // Get ID
    gDataBuffer[0] = GetByte( SIO );
    gDataBuffer[1] = GetByte( SIO );

    // Store identification informaion
    *REMS_Identification = (gDataBuffer[0] << 8) | gDataBuffer[1];

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_QPIID
 * Arguments:      Identification, 32 bit buffer to store id
 *                 fsptr, pointer of flash status structure
 * Description:    The QPIID instruction is to read the manufacturer ID
 *                 of 1-byte and followed by Device ID of 2-byte in QPI interface.
 * Return Message: FlashCmdQpiOnly, FlashOperationSuccess
 */
ReturnMsg CMD_QPIID( uint32 *Identification, FlashStatus *fsptr )
{
    uint32 temp;
    uint8  gDataBuffer[3];

    // Check current state
    if( !IsFlashQPI( fsptr ) )    return    FlashCmdQpiOnly;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_QPIID, QIO );

    // Get manufacturer identification
    gDataBuffer[0] = GetByte( QIO );
    gDataBuffer[1] = GetByte( QIO );
    gDataBuffer[2] = GetByte( QIO );

    // Chip select go high to end a command
    CS_High();

    // Store manufacturer identification
    temp =  gDataBuffer[0];
    temp =  (temp << 8 )  | gDataBuffer[1];
    *Identification =  (temp << 8)  | gDataBuffer[2];


    return FlashOperationSuccess;
}

/*
 * Register Command
 */
/*
 * Function:       CMD_RDSR
 * Arguments:      StatusReg, 8 bit buffer to store status register value
 *                 fsptr, pointer of flash status structure
 * Description:    The RDSR instruction is for reading Status
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDSR( uint8 *StatusReg, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  gDataBuffer;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_RDSR, io_mode );
    gDataBuffer = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    *StatusReg = gDataBuffer;

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WRSR
 * Arguments:      UpdateValue, 8/16 bit status register value to updata
 *                 fsptr, pointer of flash status structure
 * Description:    The WRSR instruction is for changing the values of
 *                 Status Register Bits
 * Return Message: FlashIsBusy, FlashTimeOut, FlashOperationSuccess
 */
#ifdef SUPPORT_WRSR_CR
ReturnMsg CMD_WRSR( uint16 UpdateValue, FlashStatus *fsptr )
#else
ReturnMsg CMD_WRSR( uint8 UpdateValue, FlashStatus *fsptr )
#endif

{
    uint8  io_mode;
    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Send command and update value
    SendByte( FLASH_CMD_WRSR, io_mode );
    SendByte( UpdateValue, io_mode );
#ifdef SUPPORT_WRSR_CR
    SendByte( UpdateValue >> 8, io_mode );    // write configuration register
#endif


    // Chip select go high to end a flash command
    CS_High();


    if( WaitFlashReady( WriteStatusRegCycleTime, fsptr  ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;

}

/*
 * Function:       CMD_RDSCUR
 * Arguments:      SecurityReg, 8 bit buffer to store security register value
 *                 fsptr, pointer of flash status structure
 * Description:    The RDSCUR instruction is for reading the value of
 *                 Security Register bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDSCUR( uint8 *SecurityReg, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  gDataBuffer;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    //Send command
    SendByte( FLASH_CMD_RDSCUR, io_mode );
    gDataBuffer = GetByte( io_mode );

    CS_High();

    *SecurityReg = gDataBuffer;

    return FlashOperationSuccess;

}

/*
 * Function:       CMD_WRSCUR
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The WRSCUR instruction is for changing the values of
 *                 Security Register Bits.
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashWriteRegFailed,
 *                 FlashTimeOut
 */
ReturnMsg CMD_WRSCUR( FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  gDataBuffer;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write WRSCUR command
    SendByte( FLASH_CMD_WRSCUR, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( WriteStatusRegCycleTime, fsptr  ) ){

        CMD_RDSCUR( &gDataBuffer, fsptr );

        // Check security register LDSO bit
        if( (gDataBuffer & FLASH_LDSO_MASK) == FLASH_LDSO_MASK )
            return FlashOperationSuccess;
        else
            return FlashWriteRegFailed;
    }
    else
        return FlashTimeOut;

}

/*
 * Function:       CMD_RDCR
 * Arguments:      StatusReg, 8 bit buffer to store status register value
 *                 fsptr, pointer of flash status structure
 * Description:    The RDCR instruction is for reading Configuration
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDCR( uint8 *ConfigReg, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  gDataBuffer;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_RDCR, io_mode );
    gDataBuffer = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    *ConfigReg = gDataBuffer;

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_RDEAR
 * Arguments:      ExtReg, 8 bit buffer to store extend address register value
 *                 fsptr, pointer of flash status structure
 * Description:    The RDEAR instruction is for reading Extended Address 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDEAR( uint8 *ExtReg, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  gDataBuffer;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_RDEAR, io_mode );
    gDataBuffer = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    *ExtReg = gDataBuffer;

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WREAR
 * Arguments:      UpdateValue, 8 bit data of new extend address register value
 *                 fsptr, pointer of flash status structure
 * Description:    The WREAR instruction is for writing Extended Address 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_WREAR( uint8 UpdateValue, FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Send command and update value
    SendByte( FLASH_CMD_WREAR, io_mode );
    SendByte( UpdateValue, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( WriteExtRegCycleTime, fsptr  ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_RDFBR
 * Arguments:      FbReg, 32 bit buffer to store fast boot register value
 *                 fsptr, pointer of flash status structure
 * Description:    The RDFBR instruction is for reading fast boot 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDFBR( uint32 *FbReg, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint32 temp;
    uint8  gDataBuffer[4];

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_RDFBR, io_mode );
#ifdef FASTBOOT_SEQ_LSB
    gDataBuffer[3] = GetByte( io_mode );
    gDataBuffer[2] = GetByte( io_mode );
    gDataBuffer[1] = GetByte( io_mode );
    gDataBuffer[0] = GetByte( io_mode );
#else
    gDataBuffer[0] = GetByte( io_mode );
    gDataBuffer[1] = GetByte( io_mode );
    gDataBuffer[2] = GetByte( io_mode );
    gDataBuffer[3] = GetByte( io_mode );

#endif
    // Chip select go high to end a flash command
    CS_High();

    temp =  gDataBuffer[0];
    temp =  (temp << 8 )  | gDataBuffer[1];
    temp =  (temp << 8 )  | gDataBuffer[2];
    *FbReg =  (temp << 8)  | gDataBuffer[3];

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WRFBR
 * Arguments:      UpdateValue, 32 bit data of fast boot register value
 *                 fsptr, pointer of flash status structure
 * Description:    The WRFBR instruction is for writing fast boot 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_WRFBR( uint32 UpdateValue, FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Send command and update value
    SendByte( FLASH_CMD_WRFBR, io_mode );
#ifdef FASTBOOT_SEQ_LSB
    SendByte( (UpdateValue), io_mode );
    SendByte( (UpdateValue >> 8), io_mode );
    SendByte( (UpdateValue >> 16), io_mode );
    SendByte( UpdateValue >> 24 , io_mode );
#else
    SendByte( (UpdateValue >> 24), io_mode );
    SendByte( (UpdateValue >> 16), io_mode );
    SendByte( (UpdateValue >> 8), io_mode );
    SendByte( UpdateValue , io_mode );
#endif

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( WriteExtRegCycleTime, fsptr  ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_ESFBR
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The ESFBR instruction is for erase fast boot register
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_ESFBR( FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write command
    SendByte( FLASH_CMD_ESFBR, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    //if( WaitFlashReady( WriteExtRegCycleTime, fsptr  ) )
    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Function:       CMD_WRLR
 * Arguments:      UpdateValue, 16 bit data of lock register value
 *                 fsptr, pointer of flash status structure
 * Description:    The WRLR instruction is for writing lock 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_WRLR( uint16 UpdateValue, FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Send command and update value
    SendByte( FLASH_CMD_WRLR, io_mode );
    SendByte( UpdateValue , io_mode );
    SendByte( (UpdateValue >> 8), io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_RDLR
 * Arguments:      LockReg, 16 bit buffer to store lock register value
 *                 fsptr, pointer of flash status structure
 * Description:    The RDLR instruction is for reading lock 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDLR( uint16 *LockReg, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint16 temp;
    uint8  gDataBuffer[2];

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_RDLR, io_mode );
    gDataBuffer[0] = GetByte( io_mode );
    gDataBuffer[1] = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    temp =  gDataBuffer[0];
    //*LockReg =  (temp << 8)  | gDataBuffer[1];
    *LockReg =  (gDataBuffer[1] << 8) | temp;

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_RDSPB
 * Arguments:      flash_address, 32 bit flash memory address
 *                 SPBReg, 8 bit buffer to store SPB status
 *                 fsptr, pointer of flash status structure
 * Description:    The RDSPB instruction is for reading SPB status.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_RDSPB( uint32 flash_address, uint8 *SPBReg, FlashStatus *fsptr )
{
    uint8 temp;
    uint8  io_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;


    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_RDSPB, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE );
    temp = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    *SPBReg = temp;

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WRSPB
 * Arguments:      flash_address, 32 bit flash memory address
 *                 fsptr, pointer of flash status structure
 * Description:    The WRSPB instruction is for writing SPB value.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_WRSPB( uint32 flash_address, FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;


    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_WRSPB, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;

}

/*
 * Function:       CMD_ESSPB
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The ESSPB instruction is for erase all SPB value.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_ESSPB( FlashStatus *fsptr )
{
    uint8 io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;


    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_ESSPB, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;

}

/*
 * Function:       CMD_SPBLK
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The SPBLK instruction is set SPB lock.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_SPBLK( FlashStatus *fsptr )
{
    uint8 io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_SPBLK, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;

}

/*
 * Function:       CMD_RDSPBLK
 * Arguments:      SPBLKReg, 8 bit buffer to store SPB lock bit
 *                 fsptr, pointer of flash status structure
 * Description:    The RDSPBLK instruction is for reading SPB lock bit.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_RDSPBLK( uint8 *SPBLKReg, FlashStatus *fsptr )
{
    uint8 temp;
    uint8 io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;


    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_RDSPBLK, io_mode );
    temp = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    *SPBLKReg = temp;

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WRPASS
 * Arguments:      UpdateValue, 64 bit data of password register value
 *                 fsptr, pointer of flash status structure
 * Description:    The WRPASS instruction is for writing password 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_WRPASS( uint8 *UpdateValue, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8 i;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Send command and update value
    SendByte( FLASH_CMD_WRPASS, io_mode );
    for( i=0; i < 8; i=i+1 )
       SendByte( UpdateValue[i], io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_RDPASS
 * Arguments:      PassReg, 64 bit buffer to store password register value
 *                 fsptr, pointer of flash status structure
 * Description:    The RDPASS instruction is for reading password 
 *                 Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDPASS( uint8 *PassReg, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  i;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send command
    SendByte( FLASH_CMD_RDPASS, io_mode );
    for( i=0; i < 8; i=i+1 )
       PassReg[i] = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_PASSULK
 * Arguments:      PassValue, 64 bit data of password value
 *                 fsptr, pointer of flash status structure
 * Description:    The PASSULK instruction is for password unlock
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_PASSULK( uint8 *PassValue, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  i;


    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Send command and update value
    SendByte( FLASH_CMD_PASSULK, io_mode );
    for( i=0; i < 8; i=i+1 ){
       SendByte( PassValue[i], io_mode );
       // For fast MCU, user need wait 2us for every byte
    }

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Function:       CMD_RDDPB
 * Arguments:      flash_address, 32 bit flash memory address
 *                 DPBReg, 8 bit buffer to store DPB status
 *                 fsptr, pointer of flash status structure
 * Description:    The RDDPB instruction is for reading DPB status.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_RDDPB( uint32 flash_address, uint8 *DPBReg, FlashStatus *fsptr )
{
    uint8 temp;
    uint8 io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;


    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;


    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_RDDPB, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE );
    temp = GetByte( io_mode );

    // Chip select go high to end a flash command
    CS_High();

    *DPBReg = temp;

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WRDPB
 * Arguments:      flash_address, 32 bit flash memory address
                   UpdateValue, 8 bit DPB register vlaue
 *                 fsptr, pointer of flash status structure
 * Description:    The WRDPB instruction is for writing DPB value.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_WRDPB( uint32 flash_address, uint8 UpdateValue, FlashStatus *fsptr )
{
    uint8 io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_WRDPB, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE );
    SendByte( UpdateValue , io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;

}


/*
 * Read Command
 */
/*
 * Function:       CMD_READ
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The READ instruction is for reading data out.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_READ( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  addr_4byte_mode;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE ) return FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ command and address
    SendByte( FLASH_CMD_READ, SIO );
    SendFlashAddr( flash_address, SIO, addr_4byte_mode );

    // Set a loop to read data into buffer
    for( index=0; index < byte_length; index++ )
    {
        // Read data one byte at a time
        *(target_address + index ) = GetByte( SIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_2READ
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The 2READ instruction enable double throughput of Serial
 *                 Flash in read mode
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_2READ( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  addr_4byte_mode;
    uint8  dc;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE )    return    FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // get dummy cycle number
    dc = GetDummyCycle( DUMMY_CONF_2READ, fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write 2-I/O Read command and address
    SendByte( FLASH_CMD_2READ, SIO );
    SendFlashAddr( flash_address, DIO, addr_4byte_mode );
    InsertDummyCycle( dc );     // Wait dummy cycles

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( DIO );
    }
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_4READ
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The 4READ instruction enable quad throughput of
 *                 Serial Flash in read mode.
 * Return Message: FlashAddressInvalid, FlashQuadNotEnable, FlashOperationSuccess
 */
ReturnMsg CMD_4READ( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index=0;
    uint8  io_mode;
    uint8  addr_4byte_mode;
    uint8  dc;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) == TRUE )
        io_mode = QIO;
    else{
        io_mode = SIO;
        // Check QE bit
        if( IsFlashQIO( fsptr ) != TRUE )  return FlashQuadNotEnable;
    }

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // get dummy cycle number
    dc = GetDummyCycle( DUMMY_CONF_4READ, fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write 4-I/O Read Array command
    SendByte( FLASH_CMD_4READ, io_mode );
    SendFlashAddr( flash_address, QIO, addr_4byte_mode );
    InsertDummyCycle ( dc );                // Wait dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index=index + 1 )
    {
        *(target_address + index ) = GetByte( QIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_4READ_TOP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The 4READ_TOP instruction enable quad throughput of
 *                 Serial Flash in read mode. And read from Top 128Mb.
 * Return Message: FlashAddressInvalid, FlashQuadNotEnable, FlashOperationSuccess
 */
ReturnMsg CMD_4READ_TOP( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index=0;
    uint8  io_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check 4-byte mode
    if( IsFlash4Byte( fsptr ) == TRUE ) return FlashCmd3ByteOnly;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) == TRUE )
        io_mode = QIO;
    else{
        io_mode = SIO;
        // Check QE bit
        if( IsFlashQIO( fsptr ) != TRUE )  return FlashQuadNotEnable;
    }

    // Chip select go low to start a flash command
    CS_Low();

    // Write 4-I/O Read Array command
    SendByte( FLASH_CMD_4READ_TOP, io_mode );
    SendFlashAddr( flash_address, QIO, FALSE );  // Only support 3 byte address
    InsertDummyCycle ( 6 );                // Wait 6 dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index=index + 1 )
    {
        *(target_address + index ) = GetByte( QIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_DREAD
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The DREAD instruction enable double throughput of Serial
 *                 Flash in read mode
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_DREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  addr_4byte_mode;
    uint8  dc;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE )    return    FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // get dummy cycle number
    dc = GetDummyCycle( DUMMY_CONF_DREAD, fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write DRead command and address
    SendByte( FLASH_CMD_DREAD, SIO );
    SendFlashAddr( flash_address, SIO, addr_4byte_mode );
    InsertDummyCycle( dc );    // Wait dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( DIO );
    }
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_DREAD4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The DREAD4B instruction enable double throughput of Serial
 *                 Flash in read mode with 4 byte addr
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_DREAD4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE )    return    FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Chip select go low to start a flash command
    CS_Low();

    // Write DRead command and address
    SendByte( FLASH_CMD_DREAD4B, SIO );
    SendFlashAddr( flash_address, SIO, TRUE );
    InsertDummyCycle( 8 );                    // Wait dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( DIO );
    }
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_QREAD
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The QREAD instruction enable quad throughput of
 *                 Serial Flash in read mode.
 * Return Message: FlashAddressInvalid, FlashQuadNotEnable, FlashOperationSuccess
 */
ReturnMsg CMD_QREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  addr_4byte_mode;
    uint8  dc;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE )    return    FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check QE bit
    if( IsFlashQIO( fsptr ) != TRUE )  return FlashQuadNotEnable;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // get dummy cycle number
    dc = GetDummyCycle( DUMMY_CONF_QREAD, fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write QRead command and address
    SendByte( FLASH_CMD_QREAD, SIO );
    SendFlashAddr( flash_address, SIO, addr_4byte_mode );
    InsertDummyCycle( dc );                    // Wait dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( QIO );
    }
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_QREAD4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The QREAD4B instruction enable quad throughput of
 *                 Serial Flash in read mode.
 * Return Message: FlashAddressInvalid, FlashQuadNotEnable, FlashOperationSuccess
 */
ReturnMsg CMD_QREAD4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE )    return    FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check QE bit
    if( IsFlashQIO( fsptr ) != TRUE )  return FlashQuadNotEnable;

    // Chip select go low to start a flash command
    CS_Low();

    // Write QRead command and address
    SendByte( FLASH_CMD_QREAD4B, SIO );
    SendFlashAddr( flash_address, SIO, TRUE );
    InsertDummyCycle( 8 );                    // Wait dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( QIO );
    }
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_FASTREAD
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The FASTREAD instruction is for quickly reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_FASTREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  io_mode;
    uint8  dc;
    uint8  addr_4byte_mode;

    // Check current state: SPI or QPI
    #ifdef FASTREAD_SPI_ONLY
        if( IsFlashQPI( fsptr ) != FALSE ) return FlashCmdSpiOnly;
        io_mode = SIO;
        dc = GetDummyCycle( DUMMY_CONF_FASTREAD, fsptr );
    #else 
        if( IsFlashQPI( fsptr ) ){
            io_mode = QIO;
            dc = 4;
        }
        else{
            io_mode = SIO;
            dc = 8;
        }
    #endif

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write Fast Read command, address and dummy cycle
    SendByte( FLASH_CMD_FASTREAD, io_mode );
    SendFlashAddr( flash_address, io_mode, addr_4byte_mode );
    InsertDummyCycle ( dc );            // Wait dummy cycle

    // set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( io_mode );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}



/*
 * Function:       CMD_READ4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The READ4B instruction is for reading data out with 4 byte address.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_READ4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE ) return FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ4B command and 4 byte address
    SendByte( FLASH_CMD_READ4B, SIO );
    SendFlashAddr( flash_address, SIO, TRUE );

    // Set a loop to read data into buffer
    for( index=0; index < byte_length; index++ )
    {
        // Read data one byte at a time
        *(target_address + index ) = GetByte( SIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_FASTREAD4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The FASTREAD4B instruction is for quickly reading data out with 4 byte address.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_FASTREAD4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;

    // Check current state
    if( IsFlashQPI( fsptr ) )    return    FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Chip select go low to start a flash command
    CS_Low();

    // Write Fast Read command, address and dummy cycle
    SendByte( FLASH_CMD_FASTREAD4B, SIO );
    SendFlashAddr( flash_address, SIO, TRUE );
    InsertDummyCycle ( 8 );            // Wait dummy cycle

    // set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( SIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_2READ4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The 2READ4B instruction enable double throughput of Serial
 *                 Flash in read mode with 4 byte address.
 * Return Message: FlashCmdSpiOnly, FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_2READ4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;

    // Check current state
    if( IsFlashQPI( fsptr ) != FALSE )    return    FlashCmdSpiOnly;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Chip select go low to start a flash command
    CS_Low();

    // Write 2-I/O Read command and 4 byte address
    SendByte( FLASH_CMD_2READ4B, SIO );
    SendFlashAddr( flash_address, DIO, TRUE);
    InsertDummyCycle( 4 );                    // Wait 4 dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index ) = GetByte( DIO );
    }
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_4READ4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 *                 fsptr, pointer of flash status structure
 * Description:    The 4READ4B instruction enable quad throughput of
 *                 Serial Flash in read mode with 4 byte address.
 * Return Message: FlashAddressInvalid, FlashQuadNotEnable, FlashOperationSuccess
 */
ReturnMsg CMD_4READ4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index=0;
    uint8  io_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) == TRUE )
        io_mode = QIO;
    else{
        io_mode = SIO;
        // Check QE bit
        if( IsFlashQIO( fsptr ) != TRUE )  return FlashQuadNotEnable;
    }

    // Chip select go low to start a flash command
    CS_Low();

    // Write 4-I/O Read Array command
    SendByte( FLASH_CMD_4READ4B, io_mode );
    SendFlashAddr( flash_address, QIO, TRUE );
    InsertDummyCycle ( 6 );                // Wait 6 dummy cycle

    // Set a loop to read data into flash's buffer
    for( index=0; index < byte_length; index=index + 1 )
    {
        *(target_address + index ) = GetByte( QIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_RDSFDP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    RDSFDP can retrieve the operating characteristics, structure
 *                 and vendor-specified information such as identifying information,
 *                 memory size, operating voltages and timinginformation of device
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_RDSFDP( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  io_mode;
    uint8  dummy_cycle;
    uint8  addr_4byte_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check 3-byte or 4-byte mode
    if( IsFlash4Byte( fsptr ) )
        addr_4byte_mode = TRUE;  // 4-byte mode
    else
        addr_4byte_mode = FALSE; // 3-byte mode

    if( IsFlashQPI( fsptr ) ){
        io_mode = QIO;
        dummy_cycle = 4;
    }
    else{
        io_mode = SIO;
        dummy_cycle = 8;
    }

    // Chip select go low to start a flash command
    CS_Low();

    // Write Read SFDP command
    SendByte( FLASH_CMD_RDSFDP, io_mode );
    SendFlashAddr( flash_address, io_mode, addr_4byte_mode );
    InsertDummyCycle ( dummy_cycle );        // Insert dummy cycle

    // Set a loop to read data into data buffer
    for( index=0; index < byte_length; index++ )
    {
        *(target_address + index) = GetByte( io_mode );
    }

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Program Command
 */

/*
 * Function:       CMD_WREN
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The WREN instruction is for setting rite Enable Latch
 *                 (WEL) bit.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_WREN(  FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write Enable command = 0x06, Setting Write Enable Latch Bit
    SendByte( FLASH_CMD_WREN, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WRDI
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The WRDI instruction is to reset
 *                 Write Enable Latch (WEL) bit.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_WRDI( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write Disable command = 0x04, resets Write Enable Latch Bit
    SendByte( FLASH_CMD_WRDI, io_mode );

    CS_High();

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_PP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to programm
 *                 byte_length, byte length of data to programm
 *                 fsptr, pointer of flash status structure
 * Description:    The PP instruction is for programming
 *                 the memory to be "0".
 *                 The device only accept the last 256 byte ( one page ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_PP( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  io_mode;
    uint8  addr_4byte_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write Page Program command
    SendByte( FLASH_CMD_PP, io_mode );
    SendFlashAddr( flash_address, io_mode, addr_4byte_mode );

    // Set a loop to down load whole page data into flash's buffer
    // Note: only last 256 byte will be programmed
    for( index=0; index < byte_length; index++ )
    {
        SendByte( *(source_address + index ), io_mode );
    }

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_4PP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to programm
 *                 byte_length, byte length of data to programm
 *                 fsptr, pointer of flash status structure
 * Description:    The 4PP instruction is for programming the memory to be "0".
 *                 The device only accept the last 256 byte ( one page ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 * Return Message: FlashCmdSpiOnly, FlashQuadNotEnable,FlashAddressInvalid
 *                 FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg CMD_4PP( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr )
{

    uint32 index;
    uint8  addr_4byte_mode;

    // Check current state
    if( IsFlashQPI( fsptr ) )    return    FlashCmdSpiOnly;

    // Check QE bit
    if( !IsFlashQIO( fsptr ) ) return FlashQuadNotEnable;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write 4-I/O Page Program command
    SendByte( FLASH_CMD_4PP, SIO );
    SendFlashAddr( flash_address, QIO, addr_4byte_mode );

    // Send source data to flash.
    for( index=0; index < byte_length; index++ )
    {
        SendByte( *(source_address + index ), QIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Function:       CMD_PP4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to programm
 *                 byte_length, byte length of data to programm
 *                 fsptr, pointer of flash status structure
 * Description:    The PP4B instruction is for programming
 *                 the memory to be "0" with 4 byte address.
 *                 The device only accept the last 256 byte ( one page ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_PP4B( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;
    uint8  io_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write Page Program command
    SendByte( FLASH_CMD_PP4B, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE);

    // Set a loop to down load whole page data into flash's buffer
    // Note: only last 256 byte will be programmed
    for( index=0; index < byte_length; index++ )
    {
        SendByte( *(source_address + index ), io_mode );
    }

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_4PP4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to programm
 *                 byte_length, byte length of data to programm
 *                 fsptr, pointer of flash status structure
 * Description:    The 4PP4B instruction is for programming the memory to be "0" with 4 byte address.
 *                 The device only accept the last 256 byte ( one page ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 * Return Message: FlashCmdSpiOnly, FlashQuadNotEnable,FlashAddressInvalid
 *                 FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg CMD_4PP4B( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr )
{
    uint32 index;

    // Check current state
    if( IsFlashQPI( fsptr ) )    return    FlashCmdSpiOnly;

    // Check QE bit
    if( !IsFlashQIO( fsptr ) ) return FlashQuadNotEnable;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write 4-I/O Page Program command
    SendByte( FLASH_CMD_4PP4B, SIO );
    SendFlashAddr( flash_address, QIO, TRUE);

    // Send source data to flash.
    for( index=0; index < byte_length; index++ )
    {
        SendByte( *(source_address + index ), QIO );
    }

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( PageProgramCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Erase Command
 */

/*
 * Function:       CMD_SE
 * Arguments:      flash_address, 32 bit flash memory address
 *                 fsptr, pointer of flash status structure
 * Description:    The SE instruction is for erasing the data
 *                 of the chosen sector (4KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_SE( uint32 flash_address, FlashStatus *fsptr )
{
    uint8 io_mode;
    uint8  addr_4byte_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    //Write Sector Erase command = 0x20;
    SendByte( FLASH_CMD_SE, io_mode );
    SendFlashAddr( flash_address, io_mode, addr_4byte_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( SectorEraseCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_SE4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 fsptr, pointer of flash status structure
 * Description:    The SE4B instruction is for erasing the data
 *                 of the chosen sector (4KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_SE4B( uint32 flash_address, FlashStatus *fsptr )
{
    uint8 io_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    //Write Sector Erase command = 0x20;
    SendByte( FLASH_CMD_SE4B, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( SectorEraseCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_BE32K
 * Arguments:      flash_address, 32 bit flash memory address
 *                 fsptr, pointer of flash status structure
 * Description:    The BE32K instruction is for erasing the data
 *                 of the chosen sector (32KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_BE32K( uint32 flash_address, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  addr_4byte_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    //Write Block Erase32KB command;
    SendByte( FLASH_CMD_BE32K, io_mode );
    SendFlashAddr( flash_address, io_mode, addr_4byte_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( BlockErase32KCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_BE32K4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 fsptr, pointer of flash status structure
 * Description:    The BE32K4B instruction is for erasing the data
 *                 of the chosen sector (32KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_BE32K4B( uint32 flash_address, FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    //Write Block Erase32KB command;
    SendByte( FLASH_CMD_BE32K4B, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( BlockErase32KCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_BE
 * Arguments:      flash_address, 32 bit flash memory address
 *                 fsptr, pointer of flash status structure
 * Description:    The BE instruction is for erasing the data
 *                 of the chosen sector (64KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_BE( uint32 flash_address, FlashStatus *fsptr )
{
    uint8  io_mode;
    uint8  addr_4byte_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Check 3-byte or 4-byte mode
    addr_4byte_mode = IsFlash4Byte( fsptr );

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    //Write Block Erase command = 0xD8;
    SendByte( FLASH_CMD_BE, io_mode );
    SendFlashAddr( flash_address, io_mode, addr_4byte_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( BlockEraseCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_BE4B
 * Arguments:      flash_address, 32 bit flash memory address
 *                 fsptr, pointer of flash status structure
 * Description:    The BE4B instruction is for erasing the data
 *                 of the chosen sector (64KB) to be "1" with 4 byte address.
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_BE4B( uint32 flash_address, FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    //Write Block Erase command = 0xD8;
    SendByte( FLASH_CMD_BE4B, io_mode );
    SendFlashAddr( flash_address, io_mode, TRUE );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( BlockEraseCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_CE
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The CE instruction is for erasing the data
 *                 of the whole chip to be "1".
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg CMD_CE( FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    //Write Chip Erase command = 0x60;
    SendByte( FLASH_CMD_CE, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( ChipEraseCycleTime, fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Mode setting Command
 */

/*
 * Function:       CMD_DP
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The DP instruction is for setting the
 *                 device on the minimizing the power consumption.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_DP( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Deep Power Down Mode command
    SendByte( FLASH_CMD_DP, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_RDP
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The RDP instruction is putting the device in the
 *                 Stand-by Power mode.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDP( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Deep Power Down Mode command
    SendByte( FLASH_CMD_RDP, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_ENSO
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The ENSO instruction is for entering the secured OTP mode.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_ENSO( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write ENSO command
    SendByte( FLASH_CMD_ENSO, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_EXSO
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The EXSO instruction is for exiting the secured OTP mode.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_EXSO( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write EXSO command = 0xC1
    SendByte( FLASH_CMD_EXSO, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_EQIO
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    Enable QPI mode
 * Return Message: FlashCmdSpiOnly, FlashOperationSuccess
 */
ReturnMsg CMD_EQIO( FlashStatus *fsptr )
{
    // Check current state
    if( IsFlashQPI( fsptr ) )    return    FlashCmdSpiOnly;

    // Chip select go low to start a flash command
    CS_Low();

    // Write EQIO command = 0x35
    SendByte( FLASH_CMD_EQIO, SIO );

    // Chip select go high to end a flash command
    CS_High();

    // Set QPI enable flag
    fsptr->ModeReg = ( fsptr->ModeReg|0x01 );

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_WPSEL
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    When the system accepts and executes WPSEL instruction,
 *                 the bit 7 in security register (WPSEL) will be set.
 * Return Message: FlashOperationSuccess, FlashWriteRegFailed, FlashTimeOut
 */
ReturnMsg CMD_WPSEL( FlashStatus *fsptr )
{
    uint8 io_mode;
    uint8  gDataBuffer;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Send WPSEL command
    SendByte( FLASH_CMD_WPSEL, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( WriteStatusRegCycleTime, fsptr  ) ){

        CMD_RDSCUR( &gDataBuffer, fsptr);

        // Chech WPSEL bit
        if( (gDataBuffer & FLASH_WPSEL_MASK) == FLASH_WPSEL_MASK )
            return FlashOperationSuccess;
        else
            return FlashWriteRegFailed;
    }
    else
        return FlashTimeOut;

}

/*
 * Function:       CMD_SBL
 * Arguments:      burstconfig, burst length configuration
 *                 fsptr, pointer of flash status structure
 * Description:    To set the Burst length
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_SBL( uint8 burstconfig, FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send SBL command and config data
    SendByte( FLASH_CMD_SBL, io_mode );
    SendByte( burstconfig, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_EN4B
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The EN4B instruction is for entering 4-byte mode.
 * Return Message: FlashOperationSuccess, FlashWriteRegFailed
 */
ReturnMsg CMD_EN4B( FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write command
    SendByte( FLASH_CMD_EN4B, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( IsFlash4Byte( fsptr ) )
        return FlashOperationSuccess;
    else
        return FlashWriteRegFailed;
}

/*
 * Function:       CMD_EX4B
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The EX4B instruction is for exiting 4-byte mode.
 * Return Message: FlashOperationSuccess, FlashWriteRegFailed
 */
ReturnMsg CMD_EX4B( FlashStatus *fsptr )
{
    uint8  io_mode;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write command
    SendByte( FLASH_CMD_EX4B, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( IsFlash4Byte( fsptr ) )
        return FlashWriteRegFailed;
    else
        return FlashOperationSuccess;

}

/*
 * Reset setting Command
 */

/*
 * Function:       CMD_RSTEN
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    Enable RST command
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RSTEN( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write RSTEN command
    SendByte( FLASH_CMD_RSTEN, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_RST
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The RST instruction is used as a system (software) reset that
 *                 puts the device in normal operating Ready mode.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RST( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write RST command = 0x99
    SendByte(  FLASH_CMD_RST, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    // Reset current state
    fsptr->ArrangeOpt = FALSE;
    fsptr->ModeReg = 0x00;

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_RSTQIO
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The RSTQIO instruction resets the device
 *                 from QPI mode to 1-bit SPI protocol operation.
 * Return Message: FlashCmdQpiOnly, FlashOperationSuccess
 */
ReturnMsg CMD_RSTQIO( FlashStatus *fsptr )
{
    // Check current state
    if( !IsFlashQPI( fsptr ) )    return FlashCmdQpiOnly;

    // Chip select go low to start a flash command
    CS_Low();

    // 4-I/O write RSTQIO command = 0xF5
    SendByte(  FLASH_CMD_RSTQIO, QIO );

    // Chip select go high to end a flash command
    CS_High();

    // Reset QPI enable flag
    fsptr->ModeReg = ( fsptr->ModeReg&0xfe );

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_GBLK
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The GBLK instruction is for enablethe lock protection
 *                 block of the whole chip.
 *                 This instruction is only effective after WPSEL was executed.
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg CMD_GBLK( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write GBLK command
    SendByte( FLASH_CMD_GBLK, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( WriteStatusRegCycleTime, fsptr  ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Function:       CMD_GBULK
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The GBULK instruction is for disable the lock protection
 *                 block of the whole chip.
 *                 This instruction is only effective after WPSEL was executed.
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg CMD_GBULK( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check flash is busy or not
    if( IsFlashBusy( fsptr ) )    return FlashIsBusy;

    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Setting Write Enable Latch bit
    CMD_WREN( fsptr );

    // Chip select go low to start a flash command
    CS_Low();

    // Write GBULK command = 0x98;
    SendByte( FLASH_CMD_GBULK, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    if( WaitFlashReady( WriteStatusRegCycleTime, fsptr  ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Suspend/Resume Command
 */

/*
 * Function:       CMD_PGM_ERS_S
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The PGM_ERS_S suspend Sector-Erase, Block-Erase or
 *                 Page-Program operations and conduct other operations.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_PGM_ERS_S( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send program/erase suspend command
    SendByte( FLASH_CMD_PGM_ERS_S, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_PGM_ERS_R
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The PGM_ERS_R resume Sector-Erase, Block-Erase or
 *                 Page-Program operations.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_PGM_ERS_R( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Send resume command
    SendByte( FLASH_CMD_PGM_ERS_R, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_NOP
 * Arguments:      fsptr, pointer of flash status structure
 * Description:    The NOP instruction is null operation of flash.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_NOP( FlashStatus *fsptr )
{
    uint8 io_mode;
    // Check current state: SPI or QPI
    if( IsFlashQPI( fsptr ) )
        io_mode = QIO;
    else
        io_mode = SIO;

    // Chip select go low to start a flash command
    CS_Low();

    // Write NOP command = 0x00
    SendByte( FLASH_CMD_NOP, io_mode );

    // Chip select go high to end a flash command
    CS_High();

    return FlashOperationSuccess;
}


