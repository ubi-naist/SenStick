/*
 * COPYRIGHT (c) 2010-2012 MACRONIX INTERNATIONAL CO., LTD
 * SPI Flash Low Level Driver (LLD) Sample Code
 *
 * SPI and QPI interface command hex code, type definition and
 * function prototype.
 *
 * $Id: MX25_CMD.h,v 1.18 2012/08/16 05:05:34 mxclldb1 Exp $
 */
#ifndef    __MX25_CMD_H__
#define    __MX25_CMD_H__

#include    "MX25_DEF.h"

/*** MX25 series command hex code definition ***/
//ID comands
#define    FLASH_CMD_RDID      0x9F    //RDID (Read Identification)
#define    FLASH_CMD_RES       0xAB    //RES (Read Electronic ID)
#define    FLASH_CMD_REMS      0x90    //REMS (Read Electronic & Device ID)
#define    FLASH_CMD_QPIID     0xAF    //QPIID (QPI ID Read)

//Register comands
#define    FLASH_CMD_WRSR      0x01    //WRSR (Write Status Register)
#define    FLASH_CMD_RDSR      0x05    //RDSR (Read Status Register)
#define    FLASH_CMD_WRSCUR    0x2F    //WRSCUR (Write Security Register)
#define    FLASH_CMD_RDSCUR    0x2B    //RDSCUR (Read Security Register)
#define    FLASH_CMD_RDCR      0x15    //RDCR (Read Configuration Register)
#define    FLASH_CMD_RDEAR     0xC8    //RDEAR (Read Extended Address Register)
#define    FLASH_CMD_WREAR     0xC5    //WREAR (Write Extended Address Register)
#define    FLASH_CMD_RDFBR     0x16    //RDFBR (read fast boot Register)
#define    FLASH_CMD_WRFBR     0x17    //WRFBR (write fast boot Register)
#define    FLASH_CMD_ESFBR     0x18    //ESFBR (erase fast boot Register)
#define    FLASH_CMD_WRLR      0x2C    //WRLR (write lock Register)
#define    FLASH_CMD_RDLR      0x2D    //RDLR (read lock Register)
#define    FLASH_CMD_RDSPB     0xE2    //RDSPB (read SPB status)
#define    FLASH_CMD_WRSPB     0xE3    //WRSPB (write SPB bit)
#define    FLASH_CMD_ESSPB     0xE4    //ESSPB (erase all SPB status)
#define    FLASH_CMD_SPBLK     0xA6    //SPBLK (SPB lock set)
#define    FLASH_CMD_RDSPBLK   0xA7    //SPBLK (read SPB lock register)
#define    FLASH_CMD_WRPASS    0x28    //WRPASS (write password Register)
#define    FLASH_CMD_RDPASS    0x27    //RDPASS (read password Register)
#define    FLASH_CMD_PASSULK   0x29    //RDPASS (password unlock)
#define    FLASH_CMD_RDDPB     0xE0    //RDDPB (read DPB register)
#define    FLASH_CMD_WRDPB     0xE1    //WRDPB (write DPB register)


//READ comands
#define    FLASH_CMD_READ        0x03    //READ (1 x I/O)
#define    FLASH_CMD_2READ       0xBB    //2READ (2 x I/O)
#define    FLASH_CMD_4READ       0xEB    //4READ (4 x I/O)
#define    FLASH_CMD_4READ_TOP   0xEA    //4READ (4 x I/O read from top 128Mb)
#define    FLASH_CMD_FASTREAD    0x0B    //FAST READ (Fast read data).
#define    FLASH_CMD_DREAD       0x3B    //DREAD (1In/2 Out fast read)
#define    FLASH_CMD_QREAD       0x6B    //QREAD (1In/4 Out fast read)
#define    FLASH_CMD_RDSFDP      0x5A    //RDSFDP (Read SFDP)
#define    FLASH_CMD_READ4B      0x13    //READ4B (1 x I/O with 4 byte address)
#define    FLASH_CMD_FASTREAD4B  0x0C    //FASTREAD4B (1 x I/O with 4 byte address)
#define    FLASH_CMD_2READ4B     0xBC    //2READ4B (2 x I/O with 4 byte address)
#define    FLASH_CMD_4READ4B     0xEC    //4READ4B (4 x I/O with 4 byte address)
#define    FLASH_CMD_DREAD4B     0x3C    //DREAD4B (1In/2 Out fast read with 4 byte addr)
#define    FLASH_CMD_QREAD4B     0x6C    //QREAD4B (1In/4 Out fast read with 4 byte addr)

//Program comands
#define    FLASH_CMD_WREN     0x06    //WREN (Write Enable)
#define    FLASH_CMD_WRDI     0x04    //WRDI (Write Disable)
#define    FLASH_CMD_PP       0x02    //PP (page program)
#define    FLASH_CMD_4PP      0x38    //4PP (Quad page program)
#define    FLASH_CMD_PP4B     0x12    //PP4B (page program with 4 byte address)
#define    FLASH_CMD_4PP4B    0x3E    //4PP4B (Quad page program with 4 byte address)

//Erase comands
#define    FLASH_CMD_SE       0x20    //SE (Sector Erase)
#define    FLASH_CMD_BE32K    0x52    //BE32K (Block Erase 32kb)
#define    FLASH_CMD_BE       0xD8    //BE (Block Erase)
#define    FLASH_CMD_BE4B     0xDC    //BE4B (Block Erase with 4 byte address)
#define    FLASH_CMD_CE       0x60    //CE (Chip Erase) hex code: 60 or C7
#define    FLASH_CMD_SE4B     0x21    //SE (Sector Erase with 4 byte addr)
#define    FLASH_CMD_BE32K4B  0x5C    //BE32K4B (Block Erase 32kb with 4 byte addr)

//Mode setting comands
#define    FLASH_CMD_DP       0xB9    //DP (Deep Power Down)
#define    FLASH_CMD_RDP      0xAB    //RDP (Release form Deep Power Down)
#define    FLASH_CMD_ENSO     0xB1    //ENSO (Enter Secured OTP)
#define    FLASH_CMD_EXSO     0xC1    //EXSO  (Exit Secured OTP)
#define    FLASH_CMD_EQIO     0x35    //EQIO (Enable Quad I/O)
#define    FLASH_CMD_WPSEL    0x68    //WPSEL (Enable block protect mode)
#ifdef SBL_CMD_0x77
#define    FLASH_CMD_SBL      0x77    //SBL (Set Burst Length), new: 0x77
#else
#define    FLASH_CMD_SBL      0xC0    //SBL (Set Burst Length), old: 0xC0
#endif
#define    FLASH_CMD_EN4B     0xB7    //EN4B( Enter 4-byte Mode )
#define    FLASH_CMD_EX4B     0xE9    //EX4B( Exit 4-byte Mode )

//Reset comands
#define    FLASH_CMD_RSTEN     0x66    //RSTEN (Reset Enable)
#define    FLASH_CMD_RST       0x99    //RST (Reset Memory)
#define    FLASH_CMD_RSTQIO    0xF5    //RSTQIO (Reset Quad I/O)

//Security comands
#define    FLASH_CMD_GBLK       0x7E    //GBLK (Gang Block Lock)
#define    FLASH_CMD_GBULK      0x98    //GBULK (Gang Block Unlock)

//Suspend/Resume comands
#ifdef PGM_ERS_0xB0
#define    FLASH_CMD_PGM_ERS_S    0xB0    //PGM/ERS Suspend (Suspends Program/Erase) old: 0xB0
#define    FLASH_CMD_PGM_ERS_R    0x30    //PGM/ERS Erase (Resumes Program/Erase) old: 0x30
#else
#define    FLASH_CMD_PGM_ERS_S    0x75    //PGM/ERS Suspend (Suspends Program/Erase) old: 0xB0
#define    FLASH_CMD_PGM_ERS_R    0x7A    //PGM/ERS Erase (Resumes Program/Erase) old: 0x30
#endif

#define    FLASH_CMD_NOP          0x00    //NOP (No Operation)

// Return Message
typedef enum {
    FlashOperationSuccess,
    FlashWriteRegFailed,
    FlashTimeOut,
    FlashIsBusy,
    FlashCmdSpiOnly,
    FlashCmdQpiOnly,
    FlashQuadNotEnable,
    FlashAddressInvalid,
    FlashCmd3ByteOnly,
}ReturnMsg;

// Flash status structure define
struct sFlashStatus{
    /* Mode Register:
     * Bit  Description
     * -------------------------
     *  7   RYBY enable
     *  6   Reserved
     *  5   Reserved
     *  4   Reserved
     *  3   Reserved
     *  2   Reserved
     *  1   Parallel mode enable
     *  0   QPI mode enable
    */
    uint8    ModeReg;
    BOOL     ArrangeOpt;
};

typedef struct sFlashStatus FlashStatus;

/* Basic functions */
void CS_High();
void CS_Low();
void InsertDummyCycle( uint8 dummy_cycle);
void SendByte( uint8 byte_value, uint8 transfer_type );
uint8 GetByte( uint8 transfer_type );

/* Utility functions */
void Wait_Flash_WarmUp();
void Initial_Spi();
BOOL WaitFlashReady( uint32  ExpectTime, FlashStatus *fsptr );
BOOL WaitRYBYReady( uint32 ExpectTime );
BOOL IsFlashBusy( FlashStatus *fsptr );
BOOL IsFlashQPI( FlashStatus *fsptr );
BOOL IsFlashQIO( FlashStatus *fsptr );
BOOL IsFlash4Byte( FlashStatus *fsptr );
void SendFlashAddr( uint32 flash_address, uint8 io_mode, BOOL addr_4byte_mode );
uint8 GetDummyCycle( uint32 default_cycle, FlashStatus *fsptr );

/* Flash commands */
ReturnMsg CMD_RDID( uint32 *Identification, FlashStatus *fsptr );
ReturnMsg CMD_RES( uint8 *ElectricIdentification, FlashStatus *fsptr );
ReturnMsg CMD_REMS( uint16 *REMS_Identification, FlashStatus *fsptr );
ReturnMsg CMD_QPIID( uint32 *Identification, FlashStatus *fsptr );

ReturnMsg CMD_RDSR( uint8 *StatusReg, FlashStatus *fsptr );
#ifdef SUPPORT_WRSR_CR
   ReturnMsg CMD_WRSR( uint16 UpdateValue, FlashStatus *fsptr );
#else
   ReturnMsg CMD_WRSR( uint8 UpdateValue, FlashStatus *fsptr );
#endif
ReturnMsg CMD_RDSCUR( uint8 *SecurityReg, FlashStatus *fsptr );
ReturnMsg CMD_WRSCUR( FlashStatus *fsptr );
ReturnMsg CMD_RDCR( uint8 *ConfigReg, FlashStatus *fsptr );
ReturnMsg CMD_RDEAR( uint8 *ExtReg, FlashStatus *fsptr );
ReturnMsg CMD_WREAR( uint8 UpdateValue, FlashStatus *fsptr );
ReturnMsg CMD_RDFBR( uint32 *FbReg, FlashStatus *fsptr );
ReturnMsg CMD_WRFBR( uint32 UpdateValue, FlashStatus *fsptr );
ReturnMsg CMD_ESFBR( FlashStatus *fsptr );
ReturnMsg CMD_WRLR( uint16 UpdateValue, FlashStatus *fsptr );
ReturnMsg CMD_RDLR( uint16 *LockReg, FlashStatus *fsptr );
ReturnMsg CMD_RDSPB( uint32 flash_address, uint8 *SPBReg, FlashStatus *fsptr );
ReturnMsg CMD_WRSPB( uint32 flash_address, FlashStatus *fsptr );
ReturnMsg CMD_ESSPB( FlashStatus *fsptr );
ReturnMsg CMD_SPBLK( FlashStatus *fsptr );
ReturnMsg CMD_RDSPBLK( uint8 *SPBLKReg, FlashStatus *fsptr );
ReturnMsg CMD_WRPASS( uint8 *UpdateValue, FlashStatus *fsptr );
ReturnMsg CMD_RDPASS( uint8 *PassReg, FlashStatus *fsptr );
ReturnMsg CMD_PASSULK( uint8 *PassValue, FlashStatus *fsptr );
ReturnMsg CMD_RDDPB( uint32 flash_address, uint8 *DPBReg, FlashStatus *fsptr );
ReturnMsg CMD_WRDPB( uint32 flash_address, uint8 UpdateValue, FlashStatus *fsptr );

ReturnMsg CMD_READ( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_2READ( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_4READ( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_4READ_TOP( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_DREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_QREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_FASTREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_RDSFDP( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_READ4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_FASTREAD4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_2READ4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_4READ4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_DREAD4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_QREAD4B( uint32 flash_address, uint8 *target_address, uint32 byte_length, FlashStatus *fsptr );

ReturnMsg CMD_WREN( FlashStatus *fsptr );
ReturnMsg CMD_WRDI( FlashStatus *fsptr );
ReturnMsg CMD_PP( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_4PP( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_PP4B( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr );
ReturnMsg CMD_4PP4B( uint32 flash_address, uint8 *source_address, uint32 byte_length, FlashStatus *fsptr );

ReturnMsg CMD_SE( uint32 flash_address, FlashStatus *fsptr );
ReturnMsg CMD_BE32K( uint32 flash_address, FlashStatus *fsptr );
ReturnMsg CMD_BE( uint32 flash_address, FlashStatus *fsptr );
ReturnMsg CMD_BE4B( uint32 flash_address, FlashStatus *fsptr );
ReturnMsg CMD_CE( FlashStatus *fsptr );
ReturnMsg CMD_SE4B( uint32 flash_address, FlashStatus *fsptr );
ReturnMsg CMD_BE32K4B( uint32 flash_address, FlashStatus *fsptr );

ReturnMsg CMD_DP( FlashStatus *fsptr );
ReturnMsg CMD_RDP( FlashStatus *fsptr );
ReturnMsg CMD_ENSO( FlashStatus *fsptr );
ReturnMsg CMD_EXSO( FlashStatus *fsptr );
ReturnMsg CMD_EQIO( FlashStatus *fsptr );
ReturnMsg CMD_WPSEL( FlashStatus *fsptr );
ReturnMsg CMD_SBL( uint8 burstconfig, FlashStatus *fsptr );
ReturnMsg CMD_EN4B( FlashStatus *fsptr );
ReturnMsg CMD_EX4B( FlashStatus *fsptr );

ReturnMsg CMD_RSTEN( FlashStatus *fsptr );
ReturnMsg CMD_RST( FlashStatus *fsptr );
ReturnMsg CMD_RSTQIO( FlashStatus *fsptr );

ReturnMsg CMD_GBLK( FlashStatus *fsptr );
ReturnMsg CMD_GBULK( FlashStatus *fsptr );

ReturnMsg CMD_PGM_ERS_S( FlashStatus *fsptr );
ReturnMsg CMD_PGM_ERS_R( FlashStatus *fsptr );
ReturnMsg CMD_NOP( FlashStatus *fsptr );

#endif    /* __MX25_CMD_H__ */
