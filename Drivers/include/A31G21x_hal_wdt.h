/**********************************************************************//**
* @file		A31G21x_hal_wdt.h
* @brief	Contains all macro definitions and function prototypes
* 			support for WDT firmware library on A31G21x
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


/* Peripheral group ----------------------------------------------------------- */
#ifndef _WDT_H_
#define _WDT_H_

/* Includes ------------------------------------------------------------------- */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------- BIT DEFINITIONS -------------------------------------- */
/** WDTCR interrupt enable bit */
#define WDTCR_UNFIEN			    ((uint32_t)(1<<2))  // Watch-dog Timer Underflow Interrupt Enable bit	
#define WDTCR_WINMIEN		    ((uint32_t)(1<<3))  // Watch-dog Timer Window Match Interrupt Enable bit
	
#define WDT_INTERRUPT_BITMASK   0x000c

/** WDTSR interrupt enable bit */
#define WDTSR_UNFIFLAG			    ((uint32_t)(1<<0))  // Watch-dog Timer Underflow Interrupt Flag bit	
#define WDTSR_WINMIFLAG		    ((uint32_t)(1<<1))  // Watch-dog Timer Window Match Interrupt Flag bit

#define WDT_STATUS_BITMASK   0x0003

	
	/* Public Types --------------------------------------------------------------- */
/**********************************************************************//**
 * @brief WDT structure definitions
 **********************************************************************/
 /** 
 *@brief WDT structure definitions 
 */
enum {
		WDT_DIV_4=0,
		WDT_DIV_16,
		WDT_DIV_64,
		WDT_DIV_256	
};

/** 
*@brief WDT INTCFG Type  definitions 
*/
typedef enum {
	WDT_INTCFG_UNFIEN = 0,		/*!< UNFIEN Interrupt enable*/
	WDT_INTCFG_WINMIEN,			/*!< WINMIEN Interrupt enable*/
} WDT_INT_Type;

/** 
*@brief WDT config  definitions 
*/
typedef struct Wdt_Config
{
	uint8_t wdtResetEn;			/*!< if ENABLE -> the Reset bit is enabled				*/
	uint16_t wdtClkDiv;		               /*!<wdtClkDiv */
	uint32_t wdtTmrConst;		       /*!< Set Watch-dog Timer Data Register	*/
	uint32_t wdtWTmrConst;		/*!< Set Watch-dog Timer Window Data Register	*/	
} WDT_CFG_Type;

/* Public Functions ----------------------------------------------------------- */
HAL_Status_Type HAL_WDT_Init(WDT_CFG_Type wdtCfg);
HAL_Status_Type HAL_WDT_ReloadTimeCounter(void);
HAL_Status_Type HAL_WDT_Start(FunctionalState ctrl);
uint32_t HAL_WDT_GetCurrentCount(void);
uint32_t HAL_WDT_GetStatus(void);
HAL_Status_Type HAL_WDT_ClearStatus(uint32_t clrbit);
HAL_Status_Type HAL_WDT_ConfigInterrupt(WDT_INT_Type WDTIntCfg, FunctionalState NewState);

#ifdef __cplusplus
}
#endif

#endif /* _WDT_H_ */

/* --------------------------------- End Of File ------------------------------ */
