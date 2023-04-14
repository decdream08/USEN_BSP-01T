/**********************************************************************//**
* @brief	Contains all functions support for WT firmware library
* 			on A31G21x
* @version	1.00
* @date		
* @authorABOV M team
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
#include "A31G21x_hal_wt.h"


uint32_t _WTCLK;

/* Public Functions ----------------------------------------------------------- */
/**********************************************************************//**
* @brief 		Initial for Watch Timer function
* @param[in]	WT_CFG_Type 
* @return 		 HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_WT_Init(WT_CFG_Type wtCfg)
{
	SCU->PER1 &= ~(1UL<<31); // WT 31 bit
	SCU->PCER1 &= ~(1UL<<31);	

	SCU->PER1 |= (1UL<<31);
	SCU->PCER1 |= (1UL<<31);	
	

	WT->CR = 0
		|((wtCfg.wtClkDiv&0x03)<<4) // WTINTV[1:0]
		|WTCR_WTCLR
		;
	WT->DR = (wtCfg.wtTmrConst & 0xFFF);
	return HAL_OK;
}

/**********************************************************************//**
* @brief 		deinit for Watch Timer function
* @param[in]	None
* @return 		 HAL Satus
**********************************************************************/
HAL_Status_Type HAL_WT_DeInit(void)
{
	WT->CR = 0;
	return HAL_OK;
}

/**********************************************************************//**
* @brief 		Enable WT activity
* @param[in]	FunctionalState ctrl 
*						- DISABLE: wt enable
*						- ENABLE: wt disable 
* @return 		 HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_WT_Start(FunctionalState ctrl)
{
	uint32_t tmp_reg;
	
	tmp_reg = WT->CR&0xFF;
	tmp_reg &= ~(0x1<<7); // Disable watch-dog timer counter
	
	if (ctrl == ENABLE){
		tmp_reg |= (0x1<<7); // Enable watch timer counter
	}
	
	tmp_reg |= WTCR_WTCLR; // clear the counter and divider

	WT->CR = tmp_reg; 
	return HAL_OK;
}

/*********************************************************************//**
 * @brief 		Get the current value of WT
 * @param[in]	None
 * @return		current value of WT
 *********************************************************************/
uint32_t HAL_WT_GetCurrentCount(void)
{
	return (WT->CNT & 0xFFF);
}

/*********************************************************************//**
 * @brief 		Get the timer status register of WT
 * @param[in]	None
 * @return		the status register of WT
 *********************************************************************/
uint32_t HAL_WT_GetStatus(void)
{
	return (WT->CR & WT_STATUS_BITMASK);
}

/*********************************************************************//**
 * @brief 		Clear the timer status register of WT
 * @param[in]	None
 * @return		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_WT_ClearStatus(void)
{
	WT->CR |= WTCR_WTIFLAG; //(1<<1) : clear for WTIFLAG
	return HAL_OK;

}

/********************************************************************//**
 * @brief 		Enable or disable WT interrupt.
 * @param[in]	NewState New state of specified WT interrupt type,
 * 				should be:
 * 					- ENALBE	:Enable this interrupt type.
 * 					- DISALBE	:Disable this interrupt type.
 * @return 		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_WT_ConfigInterrupt(FunctionalState NewState)
{
	uint32_t reg_val=0;
	
	reg_val =(WT->CR &0xFF);
	
	if (NewState == ENABLE) {
		reg_val |= WTCR_WTIEN; // WTIEN bit
	}
	else	{
		reg_val &= ~WTCR_WTIEN;
	}
	
	WT->CR = reg_val | WTCR_WTIFLAG; //(1<<1) : clear for WTIFLAG
	return HAL_OK;

}

/* --------------------------------- End Of File ------------------------------ */
