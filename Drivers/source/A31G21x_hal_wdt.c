/**********************************************************************//**
* @file		A31G21x_hal_wdt.c
* @brief	Contains all functions support for WDT firmware library
* 			on A31G21x
* @version	1.00
* @date		
* @author      ABOV M team
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
/* Includes ------------------------------------------------------------------- */
#include "A31G21x_hal_wdt.h"

uint32_t _WDTCLK;

/* Public Functions ----------------------------------------------------------- */
/**********************************************************************//**
* @brief 		Initial for Watchdog function
* @param[in]	none
* @return 		 HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_WDT_Init(WDT_CFG_Type wdtCfg)
{
	uint32_t reg_val=0;

	WDT->DR = (wdtCfg.wdtTmrConst & 0x00FFFFFF);
	WDT->WINDR = (wdtCfg.wdtWTmrConst & 0x00FFFFFF);
	
	reg_val = wdtCfg.wdtClkDiv;

	if(wdtCfg.wdtResetEn == ENABLE) {
		reg_val &= ~(0x3f<<10);
	}
	else {
		reg_val |= (0x25<<10);		
	}

	WDT->CR = (0x5A69 << 16) | (0x1a<<4) | reg_val;  //  /w Write Identification Key
	return HAL_OK;
}

/**********************************************************************//**
* @brief 		DeInitialize for Watchdog function
* @param[in]	None
* @return 		HAL Satus
**********************************************************************/
HAL_Status_Type HAL_WDT_DeInit(void)
{
	WDT->CR = 0
	|(0x5A69 << 16) //Write Identification Key
	|(0x25  << 10)    //Disable watch-dog timer reset
	|(0x1A << 4)      //Disable watch-dog timer counter	
	;
	return HAL_OK;
}

/*********************************************************************//**
 * @brief 		Reload WDT counter
 * @param[in]	None
 * @return		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_WDT_ReloadTimeCounter(void)
{
	WDT->CNTR = 0x6a;
	return HAL_OK;
}

/**********************************************************************//**
* @brief 		Enable WDT activity
* @param[in]	FunctionalState ctrl 
*						- DISABLE: wdt enable
*						- ENABLE: wdt disable 
* @return 		 HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_WDT_Start(FunctionalState ctrl)
{
	uint32_t tmp_reg;
	
	tmp_reg = WDT->CR&0xFFFF;
	tmp_reg |= (0x1a<<4); // Disable watch-dog timer counter
	
	if (ctrl == ENABLE){
		tmp_reg &= ~(0x3f<<4); // Enable watch-dog timer counter, 
	}

	 WDT->CR = (0x5A69<<16) | tmp_reg; // Write Identification Key 0x5A69
	 return HAL_OK;
}

/*********************************************************************//**
 * @brief 		Get the current value of WDT
 * @param[in]	None
 * @return		current value of WDT
 *********************************************************************/
uint32_t HAL_WDT_GetCurrentCount(void)
{
	return WDT->CNT;
}

/*********************************************************************//**
 * @brief 		Get the timer status register of WDT
 * @param[in]	None
 * @return		the status register of WDT
 *********************************************************************/
uint32_t HAL_WDT_GetStatus(void)
{
	return WDT->SR;
}

/*********************************************************************
 * @brief 		Clear the timer status register of WDT
 * @param[in]	clear bit
 *						- WDTSR_UNFIFLAG : UNFIFLAG Interrupt flag
 *						- WDTSR_WINMIFLAG : WINMIFLAG Interrupt flag
 * @return		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_WDT_ClearStatus(uint32_t clrbit)
{
	WDT->SR = clrbit;
	return HAL_OK;
}

/********************************************************************//**
 * @brief 		Enable or disable WDT interrupt.
 * @param[in]	WDTIntCfg	Specifies the interrupt flag,
 * 				should be one of the following:
 *						- WDT_INTCFG_UNFIEN : UNFIEN Interrupt enable
 *						- WDT_INTCFG_WINMIEN : WINMIEN Interrupt enable
 * @param[in]	NewState New state of specified WDT interrupt type,
 * 				should be:
 * 					- ENALBE	:Enable this interrupt type.
 * 					- DISALBE	:Disable this interrupt type.
 * @return 		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_WDT_ConfigInterrupt(WDT_INT_Type WDTIntCfg, FunctionalState NewState)
{
	uint32_t reg_val=0;
	uint32_t tmp=0;
	
	reg_val =(WDT->CR &0xFFFF);

	switch(WDTIntCfg){
		case WDT_INTCFG_UNFIEN:
			tmp = WDTCR_UNFIEN;
			break;
		case WDT_INTCFG_WINMIEN:
			tmp = WDTCR_WINMIEN;
			break;	
	}
	
	if (NewState == ENABLE)
	{
		reg_val |= (tmp & WDT_INTERRUPT_BITMASK);
	}
	else
	{
		reg_val &= ((~tmp) & WDT_INTERRUPT_BITMASK);
	}
	
	WDT->CR = (0x5A69<<16) | reg_val; // Write Identification Key 0x5A69	
	return HAL_OK;
}

/* --------------------------------- End Of File ------------------------------ */
