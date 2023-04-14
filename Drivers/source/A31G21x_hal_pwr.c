/**********************************************************************//**
* @file		A31G21x_hal_pwr.c
* @brief	Contains all functions support for Power Control
* 			firmware library on A31G21x
* @version	1.00
* @date		
* @author		ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
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


/* Includes ------------------------------------------------------------ */
#include "A31G21x_hal_scu.h"
#include "A31G21x_hal_pwr.h"

/*********************************************************************//**
 * @brief 		Enter Sleep mode with co-operated instruction by the Cortex-M3.
 * @param[in]	None
 * @return		None
 **********************************************************************/
void HAL_PWR_EnterSleepMode(void)
{
	/* Sleep Mode*/
	SCB->SCR = 0;
	__WFI();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();	
}

/*********************************************************************//**
 * @brief 		Enter Power Down mode with co-operated instruction by the Cortex-M3.
 * @param[in]	None
 * @return		None
 **********************************************************************/
void HAL_PWR_EnterPowerDownMode(void)
{
   /* Deep-Sleep Mode, set SLEEPDEEP bit */
	SCB->SCR = 0x4;
	/* Power Down Mode*/
	__WFI();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
}


/*********************************************************************//**
 * @brief 		Low Voltage Indicator Control
 * @param[in]	None
 * @return		None
 **********************************************************************/
HAL_Status_Type HAL_LVI_Init(LVI_CFG_Type *LVI_ConfigStruct)
{
	uint32_t	tempreg;
	
	tempreg = SCULV->LVICR&0xFF;
	
	tempreg = SCULV->LVICR;
	tempreg = 0
	| ((LVI_ConfigStruct->EnSel)<<7)
	| ((LVI_ConfigStruct->IntSel)<<5)
	| ((LVI_ConfigStruct->LvlSel)<<0)
	;
	SCULV->LVICR = tempreg;
	return HAL_OK;
	
}



/* --------------------------------- End Of File ------------------------------ */
