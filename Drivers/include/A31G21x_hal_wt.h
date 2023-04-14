/**********************************************************************//**
* @file		A31G21x_hal_wt.h
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
#ifndef _WT_H_
#define _WT_H_

/* Includes ------------------------------------------------------------------- */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------- BIT DEFINITIONS -------------------------------------- */
/** WTCR interrupt enable bit */
#define WTCR_WTIEN			    ((uint32_t)(1<<3))  // Watch Timer Interrupt Enable bit	
#define WT_INTERRUPT_BITMASK   0x0008

/** WTCR interrupt status flag bit */
#define WTCR_WTIFLAG			    ((uint32_t)(1<<1))  // Watch Timer Interrupt Flag bit	
#define WT_STATUS_BITMASK   0x0002

#define WTCR_WTCLR 			    ((uint32_t)(1<<0))  // Watch Timer counter and divider clear bit	

	
	/* Public Types --------------------------------------------------------------- */
/**********************************************************************//**
 * @brief WDT structure definitions
 **********************************************************************/
/**
 * @brief  watch timer clock divider selection enumerated definition
*/
enum {
		WT_DIV_2_7=0, /*!< WT Clock / (2 ^ 7) */
		WT_DIV_2_13,   /*!< WT Clock / (2 ^ 13) */
		WT_DIV_2_14, /*!< WT Clock / (2 ^ 14) */
		WT_DIV_2_14_MUL_WTDR	  /*!< WT Clock / ((2 ^ 14) * (WTDR + 1)) */
};

/**
 * @brief  watch timer configuration structure definition
 */
typedef struct Wt_Config
{
	uint32_t wtClkDiv;		       /* !< wtClkDiv */
	uint32_t wtTmrConst;		/*!< Set Watch Timer Data Register	*/
} WT_CFG_Type;

/* Public Functions ----------------------------------------------------------- */
HAL_Status_Type HAL_WT_Init(WT_CFG_Type wtCfg);
HAL_Status_Type HAL_WT_DeInit(void);
HAL_Status_Type HAL_WT_Start(FunctionalState ctrl);
uint32_t HAL_WT_GetCurrentCount(void);
uint32_t HAL_WT_GetStatus(void);
HAL_Status_Type HAL_WT_ClearStatus(void);
HAL_Status_Type HAL_WT_ConfigInterrupt(FunctionalState NewState);

#ifdef __cplusplus
}
#endif

#endif /* _WT_H_ */

/* --------------------------------- End Of File ------------------------------ */
