/**********************************************************************//**
* @file		A31G21x_hal_spi.c
* @brief	Contains all functions support for SPI firmware library on A31G21x
* @version	1.00
* @date
* @author	ABOV AE1 team
*
* Copyright(C) 2018, ABOV Semiconductor
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


/* Includes ------------------------------------------------------------- */
#include "A31G21x_hal_pcu.h"
#include "A31G21x_hal_spi.h"

/* Public Functions ------------------------------------------------------- */
/*********************************************************************//**
 * @brief		Initializes the SPIx peripheral according to the specified
 *              parameters in the SPI_ConfigStruct.
 * @param[in]	SPIx SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	SPI_ConfigStruct Pointer to a SPI_CFG_Type structure that
 * 				contains the configuration information for the specified
 * 				SPI peripheral.
 * @return 		HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_SPI_Init(SPI_Type *SPIx, SPI_CFG_Type *SPI_ConfigStruct)
{
	uint32_t tmp;

	PORT_ACCESS_EN();
	
	if(SPIx == SPI20) {
		/* SPI0 enable */
		SCU->PER2 &= ~(1<<14);
		SCU->PCER2 &= ~(1<14);
		
		SCU->PER2 |= (1<<14);
		SCU->PCER2 |= (1<<14);
	} 
	else if(SPIx == SPI21) {
		/* SPI1 enable */
		SCU->PER2 &= ~(1<<15);
		SCU->PCER2 &= ~(1<<15);
		
		SCU->PER2 |= (1<<15);
		SCU->PCER2 |= (1<<15);
	} 
		
	PORT_ACCESS_EN();
	
	/* Configure SPI, interrupt is disable, LoopBack mode is disable,
	 * SPI is disable, SS is auto mode.
	 */
	SPIx->EN = 0;
	
	tmp = 0
	|(1<<20)  // TXBC tx buffer clear  
	|(1<<19)  // RXBC rx buffer clear
	|(0<<18)  // TXDIE
	|(0<<17)  // RXDIE
	|(0<<16)  // SSCIE
	|(0<<15)  // TXIE
	|(0<<14)  // RXIE
	|(0<<13)  // SSMOD       0:SS auto, 1:SS is controlled by SSOUT
	|(0<<12)  // SSOUT        0:SS is low, 1:SS is high
	|(0<<11)  // LBE             loop-back mode 
	|(0<<10)  // SSMASK     0:receive data when SS signal is active, 1:receive data at SCLK edge
	|(1<<9)   // SSMO          0:SS output signal is disabled, 1:SS output signal is enabled
	|(0<<8)   // SSPOL         0:SS is active-low, 1:SS is active-high 
	|((SPI_ConfigStruct->Mode & 0x01) << 5)   // MS               0:slave mode, 1:master mode 
	|((SPI_ConfigStruct->DataDir & 0x01) << 4)   // MSBF           0:LSB first, 1:MSB first
	|((SPI_ConfigStruct->CPHA & 0x01) <<3)   // CPHA
	|((SPI_ConfigStruct->CPOL & 0x01) <<2)   // CPOL
	|((SPI_ConfigStruct->Databit & SPI_DS_BITMASK)<<0)   // BITSZ           0:8bit, 1:9bit, 2:16bit, 3:17bit
		;
	SPIx->CR = tmp;
	
	if (SPI_ConfigStruct->Mode == SPI_MASTER_MODE){
		SPIx->BR = (SPI_ConfigStruct->BaudRate & 0xFFFF); // PCLK / (BR + 1) ex) if BR=71, 72Mhz / (71+1)
	}
	else { // SPI_SLAVE_MODE   if using slave, BR has fast clock set and LR has minimum set. 
		SPIx->BR = 2; // PCLK / (BR + 1) ex) if BR=2 and PCLK=72Mhz,  72Mhz / (2 +1)	
	}
	
	SPIx->LR = 0
	|(1<<16)  // SPL >= 1 
	|(1<<8)   // BTL >= 1
	|(1<<0)   // STL >= 1
		;
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		De-initializes the SPIx peripheral registers to their
 *              default reset values.
 * @param[in]	SPIx SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @return 		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_DeInit(SPI_Type* SPIx)
{
	/* Disable SPI operation*/
	SPIx->SR = 0; // register clear 

	if(SPIx == SPI20) {
		/* SPI0 enable */
		SCU->PER2 &= ~(1<<14);
		SCU->PCER2 &= ~(1<<14);
	} 
	else if(SPIx == SPI21) {
		/* SPI1 enable */
		SCU->PER2 &= ~(1<<15);
		SCU->PCER2 &= ~(1<<15);
	} 
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		configure SPI delay length 
 * @param[in]	SPIx	SPI peripheral, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	StartDelay : SPL >= 1 (1~255)
 * @param[in]	BurstDelay : BTL >= 1 (1~255)
 * @param[in]	StopDelay : STL >= 1 (1~255)
 * @return 		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_ConfigDelay(SPI_Type* SPIx, uint8_t StartDelay, uint8_t BurstDelay,uint8_t StopDelay)
{
	if (SPIx->CR & (SPI_MASTER_MODE << 5)){
		SPIx->LR = 0
		|((StopDelay&0xFF)<<16)  // SPL >= 1 (1~255) 
		|((BurstDelay&0xFF)<<8)   // BTL >= 1 (1~255)
		|((StartDelay&0xFF)<<0)   // STL >= 1 (1~255)
			;		
	}
	else {
		SPIx->LR = 0
		|(1<<16)  // SPL >= 1 
		|(1<<8)   // BTL >= 1
		|(1<<0)   // STL >= 1
			;
	}
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		Enable or disable SPI peripheral's operation
 * @param[in]	SPIx	SPI peripheral, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	NewState New State of SPIx peripheral's operation, should be:
 * 					- ENABLE
 * 					- DISABLE
 * @return 		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_Enable(SPI_Type* SPIx, FunctionalState NewState)
{
	if (NewState == ENABLE)
	{
		SPIx->EN = 1;
	}
	else
	{
		SPIx->EN = 0;
	}
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		Enable or disable Loop Back mode function in SPI peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	NewState	New State of Loop Back mode, should be:
 * 					- ENABLE
 * 					- DISABLE
 * @return 		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_ConfigLoopBackMode(SPI_Type* SPIx, FunctionalState NewState)
{
	if (NewState == ENABLE)
	{
		SPIx->CR |= SPI_LBM_EN;
	}
	else
	{
		SPIx->CR &= ~SPI_LBM_EN;
	}
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		control SS Output function in SPI peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	State	State of Slave Output function control, should be:
 * 					- SS_AUTO	: SS is controlled automatically
 * 					- SS_MANUAL	:SS is controlled by SSOUT 12bit in CR
 * @return 		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_ConfigSSOutputMode(SPI_Type* SPIx, uint32_t State)
{
	if (State == SS_AUTO)
	{
		SPIx->CR &= (~SPI_SSMOD_MANUAL);
	}
	else
	{
		SPIx->CR |= SPI_SSMOD_MANUAL;
	}
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		control SS Output in SPI peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	 State	 State of Slave Output , should be:
 * 					- SS_OUT_LO : SS out value is Low
 * 					- SS_OUT_HI  : SS out value is High
 *  use this function after setting as HAL_SPI_ConfigSSOutputMode(SPIx,SS_MANUAL)
 * @return 		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_SetSSOutput(SPI_Type* SPIx, uint32_t State)
{
	if (State == SS_OUT_LO)
	{
		SPIx->CR &= (~SPI_SSOUT_HI);
	}
	else
	{
		SPIx->CR |= SPI_SSOUT_HI;
	}
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		Transmit a single data through SPIx peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	Data	Data to transmit 
 * @return 		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_TransmitData(SPI_Type* SPIx, uint32_t Data)
{
	SPIx->RDR_TDR = (Data);
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		Receive a single data from SPIx peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be
 * 				 	- SP	:SPI0~3 peripheral
 * @return 		Data received 
 **********************************************************************/
uint32_t HAL_SPI_ReceiveData(SPI_Type* SPIx)
{
	return ((uint32_t) (SPIx->RDR_TDR));
}

/**********************************************************************//**
 * @brief		Checks whether the specified SPI status flag is set or not
 * @param[in]	SPIx	SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @return		SPI status flag
 **********************************************************************/
uint32_t HAL_SPI_GetStatus(SPI_Type* SPIx)
{
	return (SPIx->SR);
}

/**********************************************************************//**
 * @brief		Clear specified pending in SPI peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	Type	Interrupt pending to clear
 * @return		None
 **********************************************************************/
HAL_Status_Type HAL_SPI_ClearPending(SPI_Type* SPIx, uint32_t Type)
{
	SPIx->SR = (~Type);
	return HAL_OK;
}

/***********************************************************************//**
 * @brief		Enable or disable specified interrupt type in SPI peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be:
 * 				 	- SP	:SPI0~3 peripheral
 * @param[in]	IntType	Interrupt type in SPI peripheral, should be:
 * 					- SPI_INTCFG_ROR	:Receive Overrun interrupt
 * 					- SPI_INTCFG_RT		:Receive Time out interrupt
 * 					- SPI_INTCFG_RX		:RX FIFO is at least half full interrupt
 * 					- SPI_INTCFG_TX		:TX FIFO is at least half empty interrupt
 * @param[in]	NewState New State of specified interrupt type, should be:
 * 					- ENABLE	:Enable this interrupt type
 * 					- DISABLE	:Disable this interrupt type
 * @return		HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_SPI_ConfigInterrupt(SPI_Type* SPIx, uint32_t IntType, FunctionalState NewState)
{
	if (NewState == ENABLE)
	{
		SPIx->CR |= IntType;
	}
	else
	{
		SPIx->CR &= (~IntType);
	}
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		get control register from SPIx peripheral
 * @param[in]	SPIx	SPI peripheral selected, should be
 * 				 	- SP	:SPI0~3 peripheral
 * 					- SP1	:SPI1 peripheral
 * @return 		Data received 
 **********************************************************************/
uint32_t HAL_SPI_GetControl(SPI_Type* SPIx)
{
	return ((uint32_t) (SPIx->CR));
}


/* --------------------------------- End Of File ------------------------------ */

