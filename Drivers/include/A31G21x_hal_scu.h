/**********************************************************************//**
* @file		A31G21x_hal_scu.h
* @brief	Contains all macro definitions and function prototypes
* 			support for SCU firmware library on A31G21x
* @version	1.00
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
************************************************************************
* ABOV Disclaimer
*
*IMPORTANT NOTICE ? PLEASE READ CAREFULLY
*ABOV Semiconductor ("ABOV") reserves the right to make changes, corrections, enhancements, 
*modifications, and improvements to ABOV products and/or to this document at any time without notice. 
*ABOV does not give warranties as to the accuracy or completeness of the information included herein.
*Purchasers should obtain the latest relevant information of ABOV products before placing orders. 
*Purchasers are entirely responsible for the choice, selection, and use of ABOV products and 
*ABOV assumes no liability for application assistance or the design of purchasers¡¯ products. No license, 
*express or implied, to any intellectual property rights is granted by ABOV herein. 
*ABOV disclaims all express and implied warranties and shall not be responsible or
*liable for any injuries or damages related to use of ABOV products in such unauthorized applications. 
*ABOV and the ABOV logo are trademarks of ABOV.
*All other product or service names are the property of their respective owners. 
*Information in this document supersedes and replaces the information previously
*supplied in any former versions of this document.
*2020 ABOV Semiconductor  All rights reserved
*
**********************************************************************/


#ifndef _SCU_H_
#define _SCU_H_

/* Includes ------------------------------------------------------------- */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"



/* Private macros -------------------------------------------------------- */
/* External sub crystal oscillator control */
#define EXSOSC_STOP               0x0				
#define EXSOSC_EN                    0x8				
#define EXSOSC_EN_DIV2            0x9		
#define EXSOSC_EN_DIV4            0xa

/* External sub crystal oscillator stable timeout */
#define SOSC_STARTUP_TIMEOUT 0xFFFFFFFFUL

	/* low speed oscillator control */
#define LSIOSC_STOP               0x0				
#define LSIOSC_EN                    0x8				
#define LSIOSC_EN_DIV2            0x9		
#define LSIOSC_EN_DIV4            0xa
	
	/* Internal high speed oscillator control */
#define HSIOSC_STOP               0x0				
#define HSIOSC_EN                    0x8				
#define HSIOSC_EN_DIV2            0x9		
#define HSIOSC_EN_DIV4            0xa					
#define HSIOSC_EN_DIV8            0xb		
#define HSIOSC_EN_DIV16          0xc
#define HSIOSC_EN_DIV32          0xd

/* External crystal oscillator control */
#define EXOSC_STOP               0x0				
#define EXOSC_EN                    0x8				
#define EXOSC_EN_DIV2            0x9		
#define EXOSC_EN_DIV4            0xa

/* External crystal oscillator stable timeout */
#define MOSC_STARTUP_TIMEOUT 0xFFFFFFFFUL


/* PLL stable timeout */
#define PLL_STARTUP_TIMEOUT  8 /* Time out start up, in 1ms */   

/* PLLCON[BYPASS] */
#define PLLCON_BYPASS_FIN 0
#define PLLCON_BYPASS_PLL 1

// PLL freq = (FIN or FIN/2) * M / N
/* PLLCON[PREDIV] */
#define PLLCON_FIN_DIV_1 0
#define PLLCON_FIN_DIV_2 1

/* PLLCON[FBCTRL] */
#define PLLCON_M_6 0
#define PLLCON_M_8 1
#define PLLCON_M_10 2
#define PLLCON_M_12 3
#define PLLCON_M_16 4
#define PLLCON_M_18 5
#define PLLCON_M_20 6
#define PLLCON_M_26 7
#define PLLCON_M_32 8
#define PLLCON_M_36 9
#define PLLCON_M_40 10
#define PLLCON_M_64 11

/* PLLCON[POSTDIV] */
#define PLLCON_N_1 0
#define PLLCON_N_2 1
#define PLLCON_N_3 2
#define PLLCON_N_4 3
#define PLLCON_N_6 4
#define PLLCON_N_8 5
#define PLLCON_N_16 7


/* system clock control setting */
#define SC_LSIOSC	0x0
#define SC_SOSC	   0x1
#define SC_HSIOSC          0x2
#define SC_HSIOSCPLL     0x3 
#define SC_EXOSC          0x6
#define SC_EXOSCPLL     0x7


#define SC_FIN_HSI   0x0
#define SC_FIN_HSE   0x1

/**
 * @brief  SCU CLOCK TYPE  definition
 */
enum {
	SYSTICK_TYPE=0,
	TIMER10_TYPE,
	TIMER20_TYPE,
	TIMER30_TYPE,	
	WDT_TYPE,
	WT_TYPE,
	PD0_TYPE,
	PD1_TYPE,
	LED_TYPE
};

/**
 * @brief  SCU CLOCK LSI,SOSC,MCLK,HSI, MOSC,PLL  definition
 */
enum {
	CLKSRC_LSI=0,
	CLKSRC_SOSC=3,	
	CLKSRC_MCLK=4,
	CLKSRC_HSI=5,	
	CLKSRC_MOSC=6,
	CLKSRC_PLL=7	
};

/**
 * @brief  SCU LVD  Level  definition
 */
enum {
	LVD_LEVEL_1_65 = 0,
	LVD_LEVEL_2_5,
	LVD_LEVEL_3_6,
	LVD_LEVEL_4_2
};

//----------------- SCU Reset Source definition -----------------
#define RST_PINRST     (1UL<<6)
#define RST_CPURST     (1UL<<5)
#define RST_SWRST     (1UL<<4)
#define RST_WDTRST     (1UL<<3)
#define RST_MCKFRST        (1UL<<2)
#define RST_MOFRST        (1UL<<1)
#define RST_LVDRST        (1UL<<0)


#define WAKEUP_GPIOF	 (1UL<<13)
#define WAKEUP_GPIOE	 (1UL<<12)
#define WAKEUP_GPIOD     (1UL<<11)
#define WAKEUP_GPIOC     (1UL<<10)
#define WAKEUP_GPIOB     (1UL<<9)
#define WAKEUP_GPIOA     (1UL<<8)
#define WAKEUP_FRT        (1UL<<2)
#define WAKEUP_WDT        (1UL<<1)
#define WAKEUP_LVD        (1UL<<0)


//----------------- T1N SCU Clock Constant Definition -----------------
#define TIMER1nCLK_MCCR1                   0
#define TIMER1nCLK_PCLK                   1

//----------------- T20 SCU Clock Constant Definition -----------------
#define TIMER20CLK_MCCR2                  0
#define TIMER20CLK_PCLK                   1

//----------------- T30 SCU Clock Constant Definition -----------------
#define TIMER30CLK_MCCR2                   0
#define TIMER30CLK_PCLK                     1

//----------------- LED SCU Clock Constant Definition -----------------
#define LEDCLK_MCCR5                   0
#define LEDCLK_PCLK                     1

//----------------- USB SCU Clock Constant Definition -----------------
#define USBCLK_MCCR6                   0
#define USBCLK_PCLK                     1

//---------------- LCD Driver Clock Constant Definition ---------------
#define LCDCLK_MCCR5                   0
#define LCDCLK_SOSC                     1
#define LCDCLK_WDTRC                  2

//--------------- Watch Timer Clock Constant Definition ---------------
#define WTCLK_MCCR3                   0
#define WTCLK_SOSC                     1
#define WTCLK_WDTRC                   2

//------------- Watch-Dog Timer Clock Constant Definition -------------
#define WDTCLK_WDTRC                  0
#define WDTCLK_MCCR3                   1


/* Public Functions ----------------------------------------------------------- */
void HAL_SCU_LSE_ClockConfig(uint32_t esosccon);
Status HAL_SCU_LSE_ClockMonitoring(void);

void HAL_SCU_LSI_ClockConfig(uint32_t ringosc); // in ring osc set 
void HAL_SCU_HSI_ClockConfig(uint32_t hsiosc); // hsi osc set

void HAL_SCU_HSE_ClockConfig(uint32_t eosccon);	 // exosc set
Status HAL_SCU_HSE_ClockMonitoring(void); // check for MOSC stable 


void HAL_SCU_SystemClockChange(uint32_t sysclkcon); // set system clock ==> final MCLK
void HAL_SCU_SystemClockFinClock(uint32_t sysclkcon);
void HAL_SCU_ClockOutput(uint8_t divval, FunctionalState endis);
Status HAL_SCU_PLL_ClockConfig(FunctionalState pllsetstate, uint8_t selbypass, uint8_t selfin,  uint8_t PREDIV, uint8_t POSTDIV1, uint8_t POSTDIV2, uint8_t OUTDIV);

void HAL_SCU_Timer1n_ClockConfig(uint32_t t1nclk);
void HAL_SCU_Timer20_ClockConfig(uint32_t t20clk);
void HAL_SCU_Timer30_ClockConfig(uint32_t t30clk);
void HAL_SCU_LED_ClockConfig(uint32_t ledclk);
void HAL_SCU_WT_ClockConfig(uint32_t wtclk);
void HAL_SCU_WDT_ClockConfig(uint32_t wdtclk);

void HAL_SCU_MiscClockConfig(uint8_t mccrnum, uint8_t type, uint8_t clksrc, uint8_t clkdiv);

uint32_t HAL_SCU_LVI_GetLviFlag(void);
void HAL_SCU_LVI_ClrLviFlag(void);
	
void HAL_SCU_WakeUpSRCCmd(uint32_t WakeUpSrc, FunctionalState NewState);
uint32_t HAL_SCU_GetWakeUpSRCStatus(void);
void HAL_SCU_ClearWakeUpSRC(uint32_t WakeUpStatus);

void HAL_SCU_LVRCmd(FunctionalState NewState);
	
uint32_t HAL_SCU_GetNMIStatus(void);
void HAL_SCU_SetNMI(uint32_t nmisrc);

void HAL_SCU_SetResetSrc(uint32_t RstSrc, FunctionalState NewState);
uint32_t HAL_SCU_GetResetSrc(void);
void HAL_SCU_ClearResetStatus(uint32_t rststatus);

#ifdef __cplusplus
}
#endif

#endif /* end _SCU_H_ */

/* --------------------------------- End Of File ------------------------------ */
