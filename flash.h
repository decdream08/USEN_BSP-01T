#ifndef _SELFWRITEERASE_H_
#define _SELFWRITEERASE_H_

/*CODE FLASH ------------------------------------------------------------------------*/

/* Definition of device (must be set to use library) */
#define A31G21x_Series
//#define A31G31x_Series

#ifdef A31G21x_Series
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"

#define A31G213             // Specifies Target Device

#ifdef A31G212
#define CFMC_FLASH_START_OFFSET       (0x00000000)
#define CFMC_FLASH_END_OFFSET         (0x7FFF)        // 32KB
#define CFMC_FLASH_WPROT_ALL_EN       (0x0000FFFF)    // Enable Write Protection (each 2KB segments per bit)
#define CFMC_FLASH_WPROT_ALL_DIS      (0x00000000)    // Enable Write Protection (each 2KB segments per bit)
#endif
#ifdef A31G213
#define CFMC_FLASH_START_OFFSET       (0x00000000)
#define CFMC_FLASH_END_OFFSET         (0xFFFF)        // 64KB
#define CFMC_FLASH_WPROT_ALL_EN       (0xFFFFFFFF)    // Enable Write Protection (each 2KB segments per bit)
#define CFMC_FLASH_WPROT_ALL_DIS      (0x00000000)    // Enable Write Protection (each 2KB segments per bit)
#endif
#endif /* A31G21x_Series */

#ifdef A31G31x_Series
#include "A31G31x.h"
#include "aa_types.h"

#define A31G314             // Specifies Target Device

#ifdef A31G313
#define CFMC_FLASH_START_OFFSET       (0x00000000)
#define CFMC_FLASH_END_OFFSET         (0x0000FFFF)    // 64KB
#define CFMC_FLASH_WPROT_ALL_EN       (0x0000000F)    // Enable Write Protection (each 16KB segments per bit)
#define CFMC_FLASH_WPROT_ALL_DIS      (0x00000000)    // Enable Write Protection (each 16KB segments per bit)
#endif 

#ifdef A31G314
#define CFMC_FLASH_START_OFFSET       (0x00000000)
#define CFMC_FLASH_END_OFFSET         (0x0001FFFF)    // 128KB
#define CFMC_FLASH_WPROT_ALL_EN       (0x000000FF)    // Enable Write Protection (each 16KB segments per bit)
#define CFMC_FLASH_WPROT_ALL_DIS      (0x00000000)    // Enable Write Protection (each 16KB segments per bit)
#endif 

#ifdef A31G316
#define CFMC_FLASH_START_OFFSET       (0x00000000)
#define CFMC_FLASH_END_OFFSET         (0x0003FFFF)    // 256KB
#define CFMC_FLASH_WPROT_ALL_EN       (0x0000FFFF)    // Enable Write Protection (each 16KB segments per bit)
#define CFMC_FLASH_WPROT_ALL_DIS      (0x00000000)    // Enable Write Protection (each 16KB segments per bit)
#endif

#endif /* A31G31x_Series */


#define SELF_ERASE_LENGTH         (512)
#if (SELF_ERASE_LENGTH != 512 )
#error "Please check SELF_ERASE_LENGTH is align with 512 bytes"
#endif

#define SELF_WRITE_LENGTH          (4)
#if (SELF_WRITE_LENGTH != 4 )
#error "Please check SELF_WRITE_LENGTH length is align with 4 bytes"
#endif

#ifdef FLASH_SELF_WRITE_ERASE
//Flash Size 0xFFFF (64KB)
#define ABOV_FLASH_PAGE_LENGTH    					(512)//(256)//(2048)
#define ABOV_FLASH_WRITE_BYTE_LENGTH				(4) //Can write with just Word size

/* The test area should not be the user code execution area of code flash. */
#define FLASH_SAVE_START_ADDR           			(0x0000FE00UL) //(0x0000F800UL)(0x0000F800UL)//Size : byte (0x00004000UL)
#ifdef TWS_MASTER_SLAVE_GROUPING
#ifndef FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ //2023-01-17 : To save EQ Mode
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2
#define FLASH_SAVE_DATA_LENGTH						(20) //Just save 20 Byte memory data to flash
#else
#define FLASH_SAVE_DATA_LENGTH						(16) //Just save 16 Byte memory data to flash
#endif
#else
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2
#define FLASH_SAVE_DATA_LENGTH						(16) //Just save 16 Byte memory data to flash
#else
#define FLASH_SAVE_DATA_LENGTH						(12) //Just save 12 Byte memory data to flash
#endif
#endif
#else //TWS_MASTER_SLAVE_GROUPING
#define FLASH_SAVE_DATA_LENGTH						(8) //Just save 8 Byte memory data to flash
#endif //TWS_MASTER_SLAVE_GROUPING
#endif //FLASH_SELF_WRITE_ERASE

/* CODE FLASH - COMMON REGISTERS ----------------------------------------------------*/
#define FMCFG_ENTRY         0x78580500
#define FMCFG_EXIT          0x78580500

#define BV_LOCKSEL          (1<<24)
#define BV_SELFPGM          (1<<23)
#define BV_TIMEREN          (1<<20)
#define BV_CFGRDEN          (1<<15)
#define BV_IFEN             (1<<12)
#define BV_BBLOCK           (1<<8)

#define BV_MAS              (1<<7)
#define BV_4KEN             (1<<6)
#define BV_1KEN             (1<<5)
#define BV_PMOD             (1<<4)
#define BV_WADCK            (1<<3)
#define BV_PGM              (1<<2)
#define BV_ERS              (1<<1)
#define BV_HVEN             (1<<0)

#define FMMR_BUSY               (FMC->BUSY&0x01)
#define FMCR_LOCKSEL_SET        (FMC->CR|= BV_LOCKSEL)
#define FMCR_LOCKSEL_CLR        (FMC->CR&=~BV_LOCKSEL)
#define FMCR_SELFPGM_SET        (FMC->CR|= BV_SELFPGM)
#define FMCR_SELFPGM_CLR        (FMC->CR&=~BV_SELFPGM)
#define FMCR_TIMEREN_SET        (FMC->CR|= BV_TIMEREN)
#define FMCR_TIMEREN_CLR        (FMC->CR&=~BV_TIMEREN)
#define FMCR_CFGRDEN_SET        (FMC->CR|= BV_CFGRDEN)
#define FMCR_CFGRDEN_CLR        (FMC->CR&=~BV_CFGRDEN)
#define FMCR_IFEN_SET           (FMC->CR|= BV_IFEN)
#define FMCR_IFEN_CLR           (FMC->CR&=~BV_IFEN)
#define FMCR_BBLOCK_SET         (FMC->CR|= BV_BBLOCK)
#define FMCR_BBLOCK_CLR         (FMC->CR&=~BV_BBLOCK)
#define FMCR_MAS_SET            (FMC->CR|= BV_MAS)
#define FMCR_MAS_CLR            (FMC->CR&=~BV_MAS)
#define FMCR_4KEN_SET           (FMC->CR|= BV_4KEN)
#define FMCR_4KEN_CLR           (FMC->CR&=~BV_4KEN)
#define FMCR_1KEN_SET           (FMC->CR|= BV_1KEN)
#define FMCR_1KEN_CLR           (FMC->CR&=~BV_1KEN)
#define FMCR_PMOD_SET           (FMC->CR|= BV_PMOD)
#define FMCR_PMOD_CLR           (FMC->CR&=~BV_PMOD)
#define FMCR_WADCK_SET          (FMC->CR|= BV_WADCK)
#define FMCR_WADCK_CLR          (FMC->CR&=~BV_WADCK)
#define FMCR_PGM_SET            (FMC->CR|= BV_PGM)
#define FMCR_PGM_CLR            (FMC->CR&=~BV_PGM)
#define FMCR_ERS_SET            (FMC->CR|= BV_ERS)
#define FMCR_ERS_CLR            (FMC->CR&=~BV_ERS)
#define FMCR_HVEN_SET           (FMC->CR|= BV_HVEN)
#define FMCR_HVEN_CLR           (FMC->CR&=~BV_HVEN)

/* Flash status typedef enumeration */
typedef enum {
    FLASH_STATUS_INIT   = 0xAA,     // Initialization status
    FLASH_STATUS_ERASE  = 0xAB,     // Erasing status
    FLASH_STATUS_WRITE  = 0xAC,     // Writing status
    FLASH_STATUS_READY  = 0xAD,     // Ready to erase or write to the code flash.
    FLASH_STATUS_WPROT  = 0xAE,     // Write protection is enabled.
    FLASH_STATUS_ERROR  = 0xEE      // Error
} FLASH_STATUS_TYPE;

typedef enum {
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	FLASH_SAVE_DATA_POWER,
	FLASH_SAVE_DATA_MUTE,
#endif
	FLASH_SAVE_DATA_VOLUME,
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
	FLASH_SAVE_DATA_EQ,
#endif
	FLASH_SAVE_DATA_PDL_NUM,
#if defined(MASTER_SLAVE_GROUPING) && !defined(MASTER_MODE_ONLY)
	FLASH_SAVE_SLAVE_LAST_CONNECTION,
#endif
#ifdef TWS_MASTER_SLAVE_GROUPING
	FLASH_SAVE_SET_DEVICE_ID_0,
	FLASH_SAVE_SET_DEVICE_ID_1,
	FLASH_SAVE_SET_DEVICE_ID_2,
	FLASH_SAVE_SET_DEVICE_ID_3,
	FLASH_SAVE_SET_DEVICE_ID_4,
	FLASH_SAVE_SET_DEVICE_ID_5,
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2 : When Slave is changed to Master under TWS mode, we don't connect the changed Master to Original Master.
	FLASH_TWS_MASTER_SLAVE_ID, //0x01 : Master / 0x02 : Slave
#endif
#endif
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2022-12-23
	FLASH_SAVE_GENERAL_MODE_KEEP,
#endif
	FLASH_SAVE_DATA_END
} FLASH_SAVE_DATA;

/* this global variable use only in SelfWriteErase.c */
volatile static FLASH_STATUS_TYPE flashStatus;

/* Prototypes of code flash self-write/erase */
FLASH_STATUS_TYPE Flash_Init (void);
FLASH_STATUS_TYPE Flash_WriteProtection (FunctionalState en);
FLASH_STATUS_TYPE Flash_CheckWriteArea (unsigned long addr);
FLASH_STATUS_TYPE Flash_SelfWrite(unsigned long addr, unsigned char *buf);
FLASH_STATUS_TYPE Flash_SelfErase (unsigned long page_addr);
FLASH_STATUS_TYPE Flash_DeInit (void);

Bool Flash_Read(uint32_t flash_addr, uint8_t *data, uint32_t len);
void FlashWriteErase(uint8_t *uData, uint8_t uSize);

void FlashSaveData(FLASH_SAVE_DATA data_num, uint8_t data); //Now we save byte data only !!!
#ifdef TWS_MASTER_SLAVE_GROUPING
void FlashSave_SET_DEVICE_ID(uint8_t data_num, uint8_t data); //Now we save FLASH_SAVE_DATA_END byte data only !!
#endif
void FlashEraseOnly(void);

#endif

