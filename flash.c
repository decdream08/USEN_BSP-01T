/**********************************************************************
* @file		main.c
* @brief	Contains all macro definitions and function prototypes
* 			support for Self-Write/Erase firmware library
* @version	1.00
* @date		
* @author	ABOV Application Engineer(AE)
*
* Copyright(C) 2020, ABOV Semiconductor
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"
#ifdef FLASH_SELF_WRITE_ERASE
#include "flash.h"
#include "string.h"

#ifdef AD82584F_ENABLE
#include "ad82584f.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h"
#endif

#ifdef MB3021_ENABLE
#include "bt_MB3021.h"
#endif

#ifdef WATCHDOG_TIMER_RESET
#include "A31G21x_hal_wdt.h"
#endif

#define DRIVER_ERROR_PARAMETER				(FALSE)
#define DRIVER_ERROR_OK						(TRUE)
#define FLASH_MEMORY_SIZE					(0x10000)

static uint32_t wdt_value_mirror = 0;

/**
********************************************************************************************************
* @ Name : Flash_Init
* @brief		Initialization for writing or erasing code flash.
* @param[in]	None
* @return       FLASH_STATUS_TYPE variable
*		            0xAA = FLASH_STATUS_INIT
********************************************************************************************************
*/
FLASH_STATUS_TYPE Flash_Init(void) {
    wdt_value_mirror = WDT->CR;
		
    // WDT Disable
	WDT->CR = (0x5A69 << 16)
            |(0x25<<10)
            |(0x1A<<4)
            |0;
#if 0
	// Enable aLL clock sources 
	SCU->CSCR = (0xA507UL<<16)
            |(0x8<<12) // LSE ON
            |(0x8<<8)  // HSE ON
            |(0x8<<4)  // HSI ON
            |(0x8<<0); // LSI ON
    
    // Use HSI Clock as MCLK of SYstem
	SCU->SCCR = (0x570AUL<<16)
            |(0x02);

    // Update System Clock
	SystemCoreClock=32000000;
	SystemPeriClock=32000000;
#endif                    
    // Set Flash Access to 3-wait
	FMC->MR = 0x81;
	FMC->MR = 0x28;
    FMC->CFG = FMCFG_ENTRY;
    
	FMC->MR = 0;
                    
    // Return status
	return (FLASH_STATUS_INIT);
}

/**
********************************************************************************************************
* @Name         Flash_CheckWriteArea
* @brief		Check the write address region that a data is wrote or erased. 
*               If data exists it returns an error status. 
*               Otherwise, it returns a ready state if there is no data.
* @param[in]	Address to write
* @return       FLASH_STATUS_TYPE variable
*		        0xAD        FLASH_STATUS_READY
*		        0xEE        FLASH_STATUS_ERROR
********************************************************************************************************
*/
FLASH_STATUS_TYPE Flash_CheckWriteArea (unsigned long addr) {
    
    addr &= CFMC_FLASH_END_OFFSET;       // Masking addr    
    
    if (*(volatile unsigned long *)addr == 0xFFFFFFFF) {
        return (FLASH_STATUS_READY);
    } else {
        return (FLASH_STATUS_ERROR);
    }
}

/**
********************************************************************************************************
* @Name         Flash_WriteProtection
* @brief		Enable/Disable the write protection and returns the status by checking write protection.
* @param[in]	ENABLE or DISABLE
* @return       FLASH_STATUS_TYPE variable
*		        0xAD        FLASH_STATUS_READY
*               0xAE        FLASH_STATUS_WPROT
*		        0xEE        FLASH_STATUS_ERROR               
********************************************************************************************************
*/
FLASH_STATUS_TYPE Flash_WriteProtection (FunctionalState en)
{    
    // Protection mode entry
	FMC->MR = 0x66; 
	FMC->MR = 0x99;
        
    if (en == ENABLE) {        
        FMC->WPROT = CFMC_FLASH_WPROT_ALL_EN;    // Enable Write Protection
    } else if (en == DISABLE) {
        FMC->WPROT = CFMC_FLASH_WPROT_ALL_DIS;    // Clear write protection
    }

    // Exit from protection mode entry
	FMC->MR = 0x00;
	FMC->MR = 0x00;
    
    // Return Results    
    if (en == ENABLE) {
        if (FMC->WPROT == CFMC_FLASH_WPROT_ALL_EN) {
            return FLASH_STATUS_WPROT;
        }
    } else if (en == DISABLE) {
        if (FMC->WPROT == CFMC_FLASH_WPROT_ALL_DIS) {
            return FLASH_STATUS_READY;
        }       
    }
    
    return FLASH_STATUS_ERROR;
}

/**
********************************************************************************************************
* @Name         Flash_SelfWrite
* @brief		Writes 4 bytes (words) of data to the specified address and returns the status.
* @param[in]	addr        Start address value
* @param[in]	buf         Start address of buffer
* @return       FLASH_STATUS_TYPE variable
*               0xAC        FLASH_STATUS_WRITE
*		        0xAD        FLASH_STATUS_READY
*               0xAE        FLASH_STATUS_WPROT
*		        0xEE        FLASH_STATUS_ERROR
********************************************************************************************************
*/
FLASH_STATUS_TYPE Flash_SelfWrite(unsigned long addr, unsigned char *buf)   
{

    flashStatus = Flash_WriteProtection(DISABLE);      // Clear Write Protection
    if (flashStatus == FLASH_STATUS_READY) {           // Is write protection released?
        
        flashStatus = FLASH_STATUS_WRITE;              // Update the flash status
        
        addr &= CFMC_FLASH_END_OFFSET;                 // Masking addr
        
        if (addr & 0x03) {   	                       // check the word-size address alignment 
            return FLASH_STATUS_ERROR;
        }

        FMC->MR = 0x5A; FMC->MR = 0xA5;                // flash mode entry

        FMC->CR |= (BV_SELFPGM | BV_PGM);              // Enable SELF MODE bit with WRITE MODE bit
                
        *(volatile unsigned long *)(addr) =  *((unsigned long*)buf);  // Word(4) Byte Self-Write to designated address
        
        if (*(volatile unsigned long *)(addr) != *((unsigned long*)buf)) {
            return FLASH_STATUS_ERROR;                 // Verify data value
        }

        FMC->CR &= ~(BV_SELFPGM | BV_PGM);             // Clear SELF MODE bit with WRITE MODE bit
        
        FMC->CR = 0x00000000;
        FMC->MR = 0x00;FMC->MR = 0x00;
                
        // Restore Write Protection and update the flash status
        flashStatus = Flash_WriteProtection(ENABLE);
        if (flashStatus == FLASH_STATUS_WPROT) {
            return FLASH_STATUS_WPROT;
        } else {
            return FLASH_STATUS_ERROR;
        }
        
    } else {
        return FLASH_STATUS_ERROR;
    }
}
/**
********************************************************************************************************
* @Name         Flash_SelfErase
* @brief		Erases 512 bytes (page) of data to the specified address and returns the status.
* @param[in]	page_addr   Start page address to erase
* @param[in]	buf         Start address of buffer
* @return       FLASH_STATUS_TYPE variable
*               0xAB        FLASH_STATUS_ERASE
*		        0xAD        FLASH_STATUS_READY
*               0xAE        FLASH_STATUS_WPROT
*		        0xEE        FLASH_STATUS_ERROR
********************************************************************************************************
*/
FLASH_STATUS_TYPE Flash_SelfErase(unsigned long page_addr)
{
	flashStatus = Flash_WriteProtection(DISABLE);      // Clear Write Protection
    
    if (flashStatus == FLASH_STATUS_READY) {           // Is write protection released?
        flashStatus = FLASH_STATUS_ERASE;              // Update the flash status
        
        page_addr &= CFMC_FLASH_END_OFFSET;            // Masking addr
        
        FMC->MR = 0x5A;FMC->MR = 0xA5;                 // Flash Mode Entry

        FMC->CR |= (BV_SELFPGM | BV_ERS);              // Enable SELF MODE bit with WRITE MODE bit
        
        *(volatile unsigned long *)(page_addr) = 0xFFFFFFFF;     // 512 Bytes Self-Erase to designated address

        FMC->CR &= ~(BV_ERS | BV_SELFPGM);             // Clear SELF MODE bit with WRITE MODE bit
        
        FMC->CR = 0x00000000;
        __NOP();__NOP();__NOP();__NOP();__NOP();
        FMC->MR = 0x00;FMC->MR = 0x00;
        
        // Restore Write Protection and update the flash status
        flashStatus = Flash_WriteProtection(ENABLE);
        if (flashStatus == FLASH_STATUS_WPROT) {
            return FLASH_STATUS_WPROT;
        } else {
            return FLASH_STATUS_ERROR;
        }        
    } else {
        return FLASH_STATUS_ERROR;
    }
}

Bool Flash_Read(uint32_t flash_addr, uint8_t *data, uint32_t len)
{
#ifdef _DBG_FLASH_WRITE_ERASE
	uint32_t i;

	_DBG("\n\rFlash_Read()");
#endif

	if (flash_addr + len > FLASH_MEMORY_SIZE)
		return DRIVER_ERROR_PARAMETER;

	memcpy(data, (void *)flash_addr, len);

#ifdef _DBG_FLASH_WRITE_ERASE
	for(i=0;i<len;i++)
	{
		_DBG("\n\r");
		_DBH(data[i]);
	}
#endif

	return DRIVER_ERROR_OK;
}

void FlashWriteErase(uint8_t *uData, uint8_t uSize)//Execute flash erase and write with data & size
{
    volatile FLASH_STATUS_TYPE TestStatus;                    // Status variable in user code
	uint8_t buffer1[FLASH_SAVE_DATA_LENGTH];      // Buffer data to write  //Just save 12 Byte or 8 Byte memory data to flash
    int i;
	
#ifdef _DBG_FLASH_WRITE_ERASE
    _DBG("\n\rSelf-Write/Erase Test...\r\n");
#endif

	if(uSize > FLASH_SAVE_DATA_LENGTH) //Just Write 12 Byte or 8 Byte
	{
#ifdef _DBG_FLASH_WRITE_ERASE
		_DBG("\n\rWrite Size over than buffer !!! NG !!!");
		_DBH(uSize);
		_DBG(" - ");
		_DBH(FLASH_SAVE_DATA_LENGTH);
#endif
		return;
	}

	for(i=0;i<FLASH_SAVE_DATA_LENGTH;i++)
	{
		buffer1[i] = 0xff; //Init
	}
	
#ifdef _DBG_FLASH_WRITE_ERASE
	_DBG("\n\rWrite Buff Value =");
#endif	
	for(i=0; i<uSize;i++)
	{
		buffer1[i] = uData[i]; //Save Volume Infomration
#ifdef _DBG_FLASH_WRITE_ERASE
		_DBH(buffer1[i]);
#endif
	}

    TestStatus = Flash_Init();  // Flash Init for test

    if (TestStatus == FLASH_STATUS_INIT) 
	{
        // Erase
        for(i = 0 ; i <ABOV_FLASH_PAGE_LENGTH; i+= SELF_ERASE_LENGTH) //Erase 512byte which is 1-Page
		{
            __disable_irq();                     // Disable IRQ
            Flash_SelfErase(FLASH_SAVE_START_ADDR+i);        // Erase page
            __enable_irq();                      // Enable IRQ
#ifdef _DBG_FLASH_WRITE_ERASE
            // _DBG("Erased 0x"); _DBH32(FLASH_SAVE_START_ADDR+i); _DBG(": ");_DBH32(*(volatile uint32_t *)(FLASH_SAVE_START_ADDR+i)); _DBG("\r\n");
#endif
        }

        // Write
        for(i=0 ; i<FLASH_SAVE_DATA_LENGTH; i+=ABOV_FLASH_WRITE_BYTE_LENGTH) //Write 8 byte only - USEN just use volume level with 1 byte
		{
            TestStatus = Flash_CheckWriteArea(FLASH_SAVE_START_ADDR+i); // Check that the address is ready to write?

            if (TestStatus == FLASH_STATUS_READY) 
			{
                __disable_irq();                                           // Disable IRQ
                Flash_SelfWrite(FLASH_SAVE_START_ADDR+i, (unsigned char *)(buffer1+i));    // Write Data
                __enable_irq();                                            // Enable IRQ
#ifdef _DBG_FLASH_WRITE_ERASE               
                //_DBG("\n\rWrote 0x"); _DBH32(FLASH_SAVE_START_ADDR+i); _DBG(": "); _DBH32(*(volatile uint32_t *)(FLASH_SAVE_START_ADDR+i)); _DBG("\r\n");
#endif
            } 
			else if (TestStatus == FLASH_STATUS_ERROR) 
			{
				NOP();
#ifdef _DBG_FLASH_WRITE_ERASE				
                //_DBG("Address 0x"); _DBH32(FLASH_SAVE_START_ADDR+i); _DBG(" was written a data\r\n");
#endif
            }
        }
#ifdef _DBG_FLASH_WRITE_ERASE
	    _DBG("Done!\r\n");
#endif
		WDT->CR = wdt_value_mirror;
	}

#ifdef WATCHDOG_TIMER_RESET
	WDT_Configure();
	WDT_ResetRun();//NVIC_EnableIRQ(WDT_IRQn);
#endif
}

void FlashSaveData(FLASH_SAVE_DATA data_num, uint8_t data) //Now we save FLASH_SAVE_DATA_END byte data only !!!
{
	int8_t i;
	uint8_t databuffers[FLASH_SAVE_DATA_END];

#ifdef _DBG_FLASH_WRITE_ERASE
	_DBG("\n\rFlashSaveData() :");
	_DBG("\n\rdata_num / data =");
	_DBH(data_num);
	_DBG("/");
	_DBH(data);
#endif

	for(i=0; i<FLASH_SAVE_DATA_END; i++)
		databuffers[i] = 0xff;

	Flash_Read(FLASH_SAVE_START_ADDR, databuffers, FLASH_SAVE_DATA_END); //Read Volume Level from Flash and set the value to Amp Device

	//Data init 
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	if(databuffers[FLASH_SAVE_DATA_POWER] == 0xff)
	{
		databuffers[FLASH_SAVE_DATA_POWER] = 0x01; //Power On
	}
	
	if(databuffers[FLASH_SAVE_DATA_MUTE] == 0xff)
	{
#ifdef AD82584F_USE_POWER_DOWN_MUTE
		if(IS_Display_Mute())
			databuffers[FLASH_SAVE_DATA_MUTE] = 0x01;
		else
#endif
			databuffers[FLASH_SAVE_DATA_MUTE] = 0;
	}
#endif
	if(databuffers[FLASH_SAVE_DATA_VOLUME] == 0xff)
	{
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
		databuffers[FLASH_SAVE_DATA_VOLUME] = AD82584F_Amp_Get_Cur_Volume_Level();
#else //TAS5806MD_ENABLE
		databuffers[FLASH_SAVE_DATA_VOLUME] = TAS5806MD_Amp_Get_Cur_Volume_Level();
#endif //AD82584F_ENABLE
#else //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
		databuffers[FLASH_SAVE_DATA_VOLUME] = 10;
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
	}
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
	if(databuffers[FLASH_SAVE_DATA_EQ] == 0xff)
	{
#if defined(MB3021_ENABLE) && defined(FLASH_SELF_WRITE_ERASE_EXTENSION)
		databuffers[FLASH_SAVE_DATA_EQ] = Get_Current_EQ_Mode();
#else
		databuffers[FLASH_SAVE_DATA_EQ] = 0;
#endif
	}
#endif
	if(databuffers[FLASH_SAVE_DATA_PDL_NUM] == 0xff)
	{
		databuffers[FLASH_SAVE_DATA_PDL_NUM] = 0; //1 : PDL Exist / 0 : PDL absence
	}
	
	//Data Update
	switch(data_num)
	{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		case FLASH_SAVE_DATA_POWER: //Power On/Off
		{
			databuffers[FLASH_SAVE_DATA_POWER] = data;
		}
		break;

		case FLASH_SAVE_DATA_MUTE: //Mute On/Off
		{
			databuffers[FLASH_SAVE_DATA_MUTE] = data;
		}
		break;
#endif
		case FLASH_SAVE_DATA_VOLUME: //Volume Level
		{
			databuffers[FLASH_SAVE_DATA_VOLUME] = data;
		}
		break;
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
		case FLASH_SAVE_DATA_EQ: //EQ Mode
		{
			databuffers[FLASH_SAVE_DATA_EQ] = data;
		}
		break;
#endif
		case FLASH_SAVE_DATA_PDL_NUM: //Device Exist in PDL
		{
			databuffers[FLASH_SAVE_DATA_PDL_NUM] = data;
		}
		break;

#if defined(MASTER_SLAVE_GROUPING) && !defined(MASTER_MODE_ONLY)
		case FLASH_SAVE_SLAVE_LAST_CONNECTION: //Check if Slave is last connection(0x01 or 0x02) or first connection(0x00 or 0xff)
		{//Need to save this information even thoug Master mode becasue we erase all data using flash erase function.
			databuffers[FLASH_SAVE_SLAVE_LAST_CONNECTION] = data;
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_SLAVE_LAST_CONNECTION :  Data is ");
			_DBH(data);
#endif
		}
		break;
#endif
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2022-12-23 : To save FLASH_SAVE_GENERAL_MODE_KEEP information to flash
		case FLASH_SAVE_GENERAL_MODE_KEEP: //Check if Master is GIA_PAIRING_MODE(0x01) or DEVICE_NAME_CHEKING_PAIRING MODE(0x00 or 0xff)
		{
			databuffers[FLASH_SAVE_GENERAL_MODE_KEEP] = data;
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_GENERAL_MODE_KEEP :  Data is ");
			_DBH(data);
#endif
		}
		break;
		
#endif
		default:
#ifdef _DBG_FLASH_WRITE_ERASE
			_DBG("\n\rFlashSaveData - Error !!!");
#endif
		break;
	}	

#ifdef _DBG_FLASH_WRITE_ERASE
	_DBG("\n\rWrite Data : ");

	for(i=0; i<(FLASH_SAVE_DATA_END); i++)
	{
		_DBG("/");
		_DBH(databuffers[i]);
	}
#endif

#ifdef MASTER_SLAVE_GROUPING
	FlashWriteErase(databuffers, FLASH_SAVE_DATA_END);
#else
	FlashWriteErase(databuffers, FLASH_SAVE_DATA_END);
#endif
}

#ifdef TWS_MASTER_SLAVE_GROUPING
void FlashSave_SET_DEVICE_ID(uint8_t data_num, uint8_t data) //2022-12-15 //TWS : Save SET_DEVICE_ID to Flash
{
	int8_t i;
	static uint8_t databuffers1[FLASH_SAVE_DATA_END];
	uint8_t readbuffer[FLASH_SAVE_DATA_END];

#ifdef _DBG_FLASH_WRITE_ERASE
	_DBG("\n\rFlashSave_SET_DEVICE_ID() :");
	_DBG("\n\rdata_num / data =");
	_DBH(data_num);
	_DBG("/");
	_DBH(data);
#endif

	if(data_num < FLASH_SAVE_SET_DEVICE_ID_0) //Save SET DEVICE ID Only
		return;
	
	//Data Update
	switch(data_num)
	{
		case FLASH_SAVE_SET_DEVICE_ID_0:
		{
			databuffers1[FLASH_SAVE_SET_DEVICE_ID_0] = data;
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_SET_DEVICE_ID_0 :  Data is ");
			_DBH(data);
#endif
		}
		break;

		case FLASH_SAVE_SET_DEVICE_ID_1:
		{
			databuffers1[FLASH_SAVE_SET_DEVICE_ID_1] = data;
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_SET_DEVICE_ID_1 :  Data is ");
			_DBH(data);
#endif
		}
		break;

		case FLASH_SAVE_SET_DEVICE_ID_2:
		{
			databuffers1[FLASH_SAVE_SET_DEVICE_ID_2] = data;
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_SET_DEVICE_ID_2 :  Data is ");
			_DBH(data);
#endif
		}
		break;

		case FLASH_SAVE_SET_DEVICE_ID_3:
		{
			databuffers1[FLASH_SAVE_SET_DEVICE_ID_3] = data;
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_SET_DEVICE_ID_3 :  Data is ");
			_DBH(data);
#endif
		}
		break;
		
		case FLASH_SAVE_SET_DEVICE_ID_4:
		{
			databuffers1[FLASH_SAVE_SET_DEVICE_ID_4] = data;
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_SET_DEVICE_ID_4 :  Data is ");
			_DBH(data);
#endif
		}
		break;

		case FLASH_SAVE_SET_DEVICE_ID_5:
		{
			databuffers1[FLASH_SAVE_SET_DEVICE_ID_5] = data;
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_SAVE_SET_DEVICE_ID_5 :  Data is ");
			_DBH(data);
#endif
		}
		break;

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2
		case FLASH_TWS_MASTER_SLAVE_ID:
		{
			databuffers1[FLASH_TWS_MASTER_SLAVE_ID] = data; //0x01 : Master / 0x02 : Slave
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rSave FLASH_TWS_MASTER_SLAVE_ID :  Data is ");
			_DBH(data);
#endif
		}

#endif

		default:
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rFlashSave_SET_DEVICE_ID - Error !!!");
#endif
		break;
	}	

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2
	if(data_num == FLASH_TWS_MASTER_SLAVE_ID)
#else
	if(data_num == FLASH_SAVE_SET_DEVICE_ID_5) //When we receive all data, we save SET_DEVICE_ID to flash.
#endif
	{
		Flash_Read(FLASH_SAVE_START_ADDR, readbuffer, FLASH_SAVE_DATA_END); //Read Volume Level from Flash and set the value to Amp Device

		//for(i= 0; i < (FLASH_SAVE_SET_DEVICE_ID_0-1); i++) //Overwrite other flash data excepting SET_DEVICE_ID
		for(i= 0; i < FLASH_SAVE_SET_DEVICE_ID_0; i++) //2023-03-16_1 : Changed max condition because this code erases FLASH_SAVE_SLAVE_LAST_CONNECTION info when user execute TWS connection between Master and Slave.  //Overwrite other flash data excepting SET_DEVICE_ID
		{
#ifdef _DBG_FLASH_WRITE_ERASE		
			//_DBG("\n\rdatabuffers[");_DBD(i);_DBG("] = ");_DBH(databuffers1[i]);
#endif
			databuffers1[i] = readbuffer[i]; 
#ifdef _DBG_FLASH_WRITE_ERASE
			//_DBG("\n\rreadbuffer[");_DBD(i);_DBG("] = ");_DBH(readbuffer[i]);
#endif
		}
		
		FlashWriteErase(databuffers1, FLASH_SAVE_DATA_END);

#ifdef _DBG_FLASH_WRITE_ERASE
		_DBG("\n\rWrite Data : ");

		for(i=0; i<(FLASH_SAVE_DATA_END); i++)
		{
			_DBG("/");
			_DBH(databuffers1[i]);
		}
#endif
	}
}
#endif //TWS_MASTER_SLAVE_GROUPING


void FlashEraseOnly(void)
{
    volatile FLASH_STATUS_TYPE TestStatus;                    // Status variable in user code
    int i;
	
    TestStatus = Flash_Init();  // Flash Init for test

    if (TestStatus == FLASH_STATUS_INIT) 
	{
        // Erase
        for(i = 0 ; i <ABOV_FLASH_PAGE_LENGTH; i+= SELF_ERASE_LENGTH) //Erase 512byte which is 1-Page
		{
            __disable_irq();                     // Disable IRQ
            Flash_SelfErase(FLASH_SAVE_START_ADDR+i);        // Erase page
            __enable_irq();                      // Enable IRQ
#ifdef _DBG_FLASH_WRITE_ERASE            
            // _DBG("Erased 0x"); _DBH32(FLASH_SAVE_START_ADDR+i); _DBG(": ");_DBH32(*(volatile uint32_t *)(FLASH_SAVE_START_ADDR+i)); _DBG("\r\n");
#endif
        }

#ifdef _DBG_FLASH_WRITE_ERASE
	    _DBG("Done!\r\n");
#endif
		WDT->CR = wdt_value_mirror;
	}

#ifdef WATCHDOG_TIMER_RESET
	WDT_Configure();
	WDT_ResetRun();//NVIC_EnableIRQ(WDT_IRQn);
#endif
}

#endif //FLASH_SELF_WRITE_ERASE


