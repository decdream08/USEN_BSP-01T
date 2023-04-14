/**********************************************************************//**
* @file		A31G21x_hal_dmac.c
* @brief	Contains all functions support for DMAC firmware library
* 			on A31G21x
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


/* Peripheral group -------------------------------------------------- */

/* Includes --------------------------------------------------------- */
#include "A31G21x_hal_dmac.h"
#include "A31G21x_hal_scu.h"


/* Public Functions ----------------------------------------------------- */
/*********************************************************************//**
 * @brief 		Initialize DMA controller
 * @param[in] 	None
 * @return 		HAL_Status_Type
 *********************************************************************/
HAL_Status_Type HAL_DMAC_Init(void)
{
	/* to be defined Enable DMA and clock */
	// enabled default on reset
	SCU->PER1&=~SCU_PER1_DMA_Msk; // (1<<4) DMA peri enable 
	SCU->PCER1&=~SCU_PCER1_DMA_Msk; // (1<<4) DMA peri clock enable 
	
	SCU->PER1|=SCU_PER1_DMA_Msk; // (1<<4) DMA peri enable 
	SCU->PCER1|=SCU_PCER1_DMA_Msk; // (1<<4) DMA peri clock enable 
	return HAL_OK;
}

/*********************************************************************//**

* @brief      	Initialize DMAC(Direct Memory Access Controller) peripheral
* @param    	None
* @return     	HAL_Status
*********************************************************************/
HAL_Status_Type HAL_DMAC_DeInit(void)
{
	SCU->PER1&=~SCU_PER1_DMA_Msk; // (1<<4) DMA peri enable 
	SCU->PCER1&=~SCU_PCER1_DMA_Msk; // (1<<4) DMA peri clock enable 
	
	return HAL_OK;
}


/*********************************************************************//**
 * @brief 		Setup DMAC channel peripheral according to the specified
 *              parameters in the GPDMAChannelConfig.
 * @param[in]	DMACx	Pointer to selected DMAC peripheral, should be:
 * 					- DMAC0~3	:DMAC0~3 peripheral
 * @param[in]	DMACChannelConfig Pointer to a DMAC_CH_CFG_Type structure
 * 				that contains the configuration information for the specified
 * 				channel peripheral.
 * @return		HAL_Status_Type
 *********************************************************************/
HAL_Status_Type HAL_DMAC_Setup(DMA_Type *DMACx, DMA_Ch_CFG_Type *DMACChConfig)
{
	
	DMACx->CR = ((DMACChConfig->TransferCnt & 0xfff)<<16) 
		| ((DMACChConfig->PeriSel & 0x0f)<<8)	
		| ((DMACChConfig->TransferWidth & 0x03)<<2)
		| ((DMACChConfig->Dir & 0x01)<<1)	
		;
	DMACx->PAR = DMACChConfig->PeriAddr;	
	DMACx->MAR = DMACChConfig->MemAddr;
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		Enable/Disable DMA channel
 * @param[in]	DMACx	Pointer to selected DMAC peripheral, should be:
 * 					- DMAC0~3	:DMAC0~3 peripheral
 * @param[in]	NewState	New State of this command, should be:
 * 					- ENABLE.
 * 					- DISABLE.
 * @return		HAL_Status_Type
 **********************************************************************/
HAL_Status_Type HAL_DMAC_ChCmd(DMA_Type *DMACx, FunctionalState NewState)
{
	if (NewState == ENABLE) {
		DMACx->SR |= DMAC_SR_DMAEN_Msk;
	} else {
		DMACx->SR &= ~DMAC_SR_DMAEN_Msk;
	}
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		Check if corresponding channel does have an active interrupt
 * 				request or not
 * @param[in]	DMACx	Pointer to selected DMAC peripheral, should be:
 * 					- DMAC0~3	:DMAC0~3 peripheral
 * @return		FlagStatus	status of DMA channel interrupt after masking
 * 				Should be:
 * 					- SET	: all data is transferred 
 * 					- RESET	:data to be transferred is existing 
 **********************************************************************/
FlagStatus HAL_DMAC_GetStatus(DMA_Type *DMACx)
{
	if (DMACx->SR & DMAC_SR_EOT_Msk){	
		return SET;
	}
	else {
		return RESET;
	}
}

/* --------------------------------- End Of File ------------------------------ */

