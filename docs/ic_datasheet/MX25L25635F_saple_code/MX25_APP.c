/*
 * COPYRIGHT (c) 2010-2012 MACRONIX INTERNATIONAL CO., LTD
 * SPI Flash Low Level Driver (LLD) Sample Code
 *
 * Application program of flash support SPI and QPI interface
 * This sample code provides a reference, not recommand for directing using.
 *
 * $Id: MX25_APP.c,v 1.4 2012/01/03 08:00:06 modelqa Exp $
 */

#include    <stdlib.h>
#include    "MX25_CMD.h"

#define  TRANS_LENGTH  16
#define  RANDOM_SEED   106
#define  FLASH_TARGET_ADDR  0x00000000

#define  Error_inc(x)  x = x + 1;

/* function prototype */
uint8  FlashID_Test( uint8 QPI_Enable );
uint8  FlashReadWrite_Test( uint8 QPI_Enable );

/*
 * Main Program
 */
void main()
{
    /* Setting flash access mode: SPI or QPI */
    uint8 QPI_Enable = TRUE;

    Initial_Spi();
    FlashID_Test( QPI_Enable );    // Simple test : flash ID
    FlashReadWrite_Test( QPI_Enable );   // Simple test : flash read / write
}

/*
 * Simple flash id test
 */
uint8 FlashID_Test( uint8 QPI_Enable )
{
    uint32  flash_id = 0;
    uint16  error_cnt = 0;
    FlashStatus  flash_state = {0};
    ReturnMsg  msg;

    if( QPI_Enable )
        CMD_EQIO( &flash_state );    // Enter QPI mode

    /* Read flash ID */
    if( QPI_Enable ){    // Call QPI command
        msg = CMD_QPIID( &flash_id ,&flash_state );
    }
    else{
        msg = CMD_RDID( &flash_id, &flash_state );
    }

    if( msg != (ReturnMsg)FlashOperationSuccess )  // Check returen message
        return FALSE;

    if( QPI_Enable )
        CMD_RSTQIO( &flash_state );  // Exit QPI mode

    if( flash_id != FlashID )  // Compare to expected value
        Error_inc( error_cnt );

    if( error_cnt != 0 )
        return FALSE;
    else
        return TRUE;

}
/*
 * Simple flash read/write test
 */
uint8 FlashReadWrite_Test( uint8 QPI_Enable )
{
    FlashStatus  flash_state = {0};
    uint32  flash_addr;
    uint32  trans_len = 0;
    uint16  i=0, error_cnt = 0;
    uint16  seed = 0;
    uint8   st_reg = 0;
    uint8   memory_addr[TRANS_LENGTH] = {0};
    uint8   memory_addr_cmp[TRANS_LENGTH] = {0};

    /* Assign initial condition */
    flash_addr = FLASH_TARGET_ADDR;
    trans_len = TRANS_LENGTH;
    seed = RANDOM_SEED;

    if( QPI_Enable )
        CMD_EQIO( &flash_state );    // Enter QPI mode

    /* Prepare data to transfer */
    srand( seed );
    for( i=0; i< (trans_len); i=i+1 ){
        memory_addr[i] = rand()%256;    // generate random byte data
    }

    
    /* Mmarked these code when not using quad IO mode (except QPI mode)
     * Enable Quad IO mode
     * Read status register value */
    CMD_RDSR( &st_reg, &flash_state );
    st_reg = st_reg | FLASH_QE_MASK;    // set QE bit to 1
    CMD_WRSR( st_reg, &flash_state );    // write setting to flash

    /* Check QE value */
    CMD_RDSR( &st_reg, &flash_state );    
    if( (st_reg & FLASH_QE_MASK) != FLASH_QE_MASK )
        Error_inc( error_cnt );
    

    /* Erase 4K sector of flash memory
       Note: It needs to erase dirty sector before program */
    CMD_SE( flash_addr, &flash_state );

    /* Program data to flash */
    if( QPI_Enable == 1)
        CMD_PP( flash_addr, memory_addr, trans_len, &flash_state );
    else{
        //CMD_PP( flash_addr, memory_addr, trans_len, &flash_state );
        CMD_4PP( flash_addr, memory_addr, trans_len, &flash_state );    // need set QE bit
    }

    /* Read flash data to memory buffer */
    if ( QPI_Enable == 1){    
        CMD_4READ( flash_addr, memory_addr_cmp, trans_len, &flash_state );
    }else{
        //Non-QPI mode have different read instruction options:
        CMD_READ( flash_addr, memory_addr_cmp, trans_len, &flash_state );
        //CMD_FASTREAD( flash_addr, memory_addr_cmp, trans_len, &flash_state );
        //CMD_2READ( flash_addr, memory_addr_cmp, trans_len, &flash_state );
        //CMD_4READ( flash_addr, memory_addr_cmp, trans_len, &flash_state );    // need set QE bit
    }

    /* Compare flash data and patten data */
    for( i=0; i < trans_len; i=i+1 ){
        if( memory_addr[i] != memory_addr_cmp[i] )
            Error_inc( error_cnt );
    }

    /* Erase 4K sector of flash memory */
    CMD_SE( flash_addr, &flash_state );

    if( QPI_Enable )
        CMD_RSTQIO( &flash_state );    // Exit QPI mode

    if( error_cnt != 0 )
        return FALSE;
    else
        return TRUE;

}



