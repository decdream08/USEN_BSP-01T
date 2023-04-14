/**********************************************************************//**
* @file		A31G21x_hal_usart1n.h
* @brief	Contains all functions support for USART firmware library on A31G21x
* @version	1.00
* @date:    
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor. All rights reserved.
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


#include "A31G21x_hal_usart1n.h"
#include "A31G21x_hal_scu.h"

uint32_t UsartBaseClock; 

void HAL_USART_set_divisors(USART_Type *USARTx, uint32_t baudrate);


/********************************************************************//**
 * @brief		Initializes the USART1x peripheral according to the specified
 *               parameters in the USART_ConfigStruct.
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @param[in]	USART_ConfigStruct Pointer to a USART_CFG_Type structure
 *              that contains the configuration information for the
 *              specified USART peripheral.
 * @return 		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_USART_Init(USART_Type *USARTx, USART_CFG_Type *USART_ConfigStruct)
{
	uint32_t tmp ;

	if(USARTx == USART10)
	{
		SCU->PER2&=~(1<<8);
		SCU->PCER2&=~(1<<8);		
		/* Set up peripheral clock for USART10 module */
		SCU->PER2|=(1<<8);
		SCU->PCER2|=(1<<8);	
	}
	else if(USARTx == USART11)
	{
		SCU->PER2&=~(1<<9);
		SCU->PCER2&=~(1<<9);		
		/* Set up peripheral clock for USART11 module */
		SCU->PER2|=(1<<9);
		SCU->PCER2|=(1<<9);
	}
	else 
	{
	
	}


	USARTx->CR2 = (1<<9);     // EN     
	
    if(USART_ConfigStruct->Mode == USART_UART_MODE){
		tmp = 0
			| ((USART_ConfigStruct->Mode & 0x3) << 14)
			| ((USART_ConfigStruct->Parity & 0x3) << 12)
			| ((USART_ConfigStruct->Databits & 0x7) << 9)
			| (1 << 1)  //Tx Enable
			| (1 << 0)  //Rx Enable
			; 
		USARTx->CR1 = tmp;

		tmp = 0
			|(1<<9)  // EN
			| ((USART_ConfigStruct->Stopbits & 0x1)<<2)
			;
		USARTx->CR2 = tmp;
			
        UsartBaseClock=SystemCoreClock >> 4;  //     UsartBaseClock = SystemCoreClock / 16
		HAL_USART_set_divisors(USARTx, USART_ConfigStruct->Baud_rate);
   
    }
	else if(USART_ConfigStruct->Mode == USART_SYNC_MODE){
		tmp = 0
			| ((USART_ConfigStruct->Mode & 0x3) << 14)
			| ((USART_ConfigStruct->Parity & 0x3) << 12)
			| ((USART_ConfigStruct->Databits & 0x7) << 9)
			| (1 << 1)  //Tx Enable
			| (1 << 0)  //Rx Enable
			; 
		USARTx->CR1 = tmp;

		tmp = 0
			|(1<<9)  // EN
			| ((USART_ConfigStruct->Stopbits & 0x1)<<2)
			;
		USARTx->CR2 = tmp;
			
        UsartBaseClock=SystemCoreClock >> 1;  //     UsartBaseClock = SystemCoreClock / 2
		HAL_USART_set_divisors(USARTx, USART_ConfigStruct->Baud_rate);
    }
	else if(USART_ConfigStruct->Mode == USART_SPI_MODE){ 
		tmp = 0
			| ((USART_ConfigStruct->Mode & 0x3) << 14)
			| ((USART_ConfigStruct->Databits & 0x7) << 9)
			| ((USART_ConfigStruct->Order & 0x1) << 8)
			| ((USART_ConfigStruct->ACK & 0x1) << 7)
			| ((USART_ConfigStruct->Edge & 0x1)  << 6)
			| (1 << 1)  //Tx Enable
			| (1 << 0)  //Rx Enable
			; 
		USARTx->CR1 = tmp;
		
		USARTx->CR2 &= ~(1<<3);   // FXCHn reset
	//    USARTx->CR2 |= (1<<3);    // FXCHn

		UsartBaseClock=SystemCoreClock >> 1;
		HAL_USART_set_divisors(USARTx, USART_ConfigStruct->Baud_rate);
		
		//Dummy Read
		tmp = HAL_USART_ReceiveByte(USARTx);
		tmp = HAL_USART_ReceiveByte(USARTx);		
    }	
    return HAL_OK;
}


/*********************************************************************//**
 * @brief		De-initializes the USARTx peripheral registers to their
 *              default reset values.
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @return 		 HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_USART_DeInit(USART_Type* USARTx)
{
	if(USARTx == USART10)
	{
		SCU->PER2&=~(1<<8);
		SCU->PCER2&=~(1<<8);		
	}
	else if(USARTx == USART11)
	{
		SCU->PER2&=~(1<<9);
		SCU->PCER2&=~(1<<9);		
	}
	else 
	{
	
	}
	return HAL_OK;

}


/*********************************************************************//**
 * @brief		USARTx enable control
 *              default reset values.
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 *              state ENABLE / DISABLE
 * @return 		 HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_USART_Enable(USART_Type* USARTx, FunctionalState state)
{
    if(state == ENABLE){
        USARTx->CR2 |= (1<<9);     // EN
    }else{
        USARTx->CR2 &= ~(1<<9);     // EN
    }
    return HAL_OK;
}


/******************************************************************************//**
 * @brief		Fills each HAL_USART_InitStruct member with its default value:
 * 					- 38400 bps
 * 					- 8-bit data
 * 					- 1 Stopbit
 * 					- None Parity
 * @param[in]	HAL_USART_InitStruct Pointer to a USART_CFG_Type structure which will
 * 				be initialized.
 * @return		 HAL Satus
 *******************************************************************************/
HAL_Status_Type HAL_USART_UART_Mode_Config(USART_CFG_Type *HAL_USART_InitStruct)
{
       HAL_USART_InitStruct->Mode = USART_UART_MODE;
	HAL_USART_InitStruct->Baud_rate = 115200;//38400;
	HAL_USART_InitStruct->Databits = USART_DATABIT_8;
	HAL_USART_InitStruct->Parity = USART_PARITY_NONE;
	HAL_USART_InitStruct->Stopbits = USART_STOPBIT_1;
	return HAL_OK;
}

/******************************************************************************//**
 * @brief		Fills each HAL_USART_InitStruct member with its default value:
 * @param[in]	HAL_USART_InitStruct Pointer to a USART_CFG_Type structure which will
 * 				be initialized.
 * @return		 HAL Satus
 *******************************************************************************/
HAL_Status_Type HAL_USART_SPI_Mode_Config(USART_CFG_Type *HAL_USART_InitStruct)
{
    HAL_USART_InitStruct->Mode = USART_SPI_MODE;
	HAL_USART_InitStruct->Baud_rate = 115200;//38400;
	HAL_USART_InitStruct->Databits = USART_DATABIT_8;
    
    //only SPI
    HAL_USART_InitStruct->Order = USART_SPI_LSB;
#if 1 // CPOLn : 0, CPHAn : 0
    HAL_USART_InitStruct->ACK = USART_SPI_TX_RISING;
    HAL_USART_InitStruct->Edge = USART_SPI_TX_LEADEDGE_SAMPLE;
#endif
#if 0 // CPOLn : 0, CPHAn : 1
    HAL_USART_InitStruct->ACK = USART_SPI_TX_RISING;
    HAL_USART_InitStruct->Edge = USART_SPI_TX_LEADEDGE_SETUP;
#endif
    
#if 0 // CPOLn : 1, CPHAn : 0 
    HAL_USART_InitStruct->ACK = USART_SPI_TX_FALLING;
    HAL_USART_InitStruct->Edge = USART_SPI_TX_LEADEDGE_SAMPLE;
#endif
    
#if 0 // CPOLn : 1, CPHAn : 1
    HAL_USART_InitStruct->ACK = USART_SPI_TX_FALLING;
    HAL_USART_InitStruct->Edge = USART_SPI_TX_LEADEDGE_SETUP;
#endif
    return HAL_OK;

    
}

/******************************************************************************//**
 * @brief		Fills each HAL_USART_InitStruct member with its default value:
 * 					- 38400 bps
 * 					- 8-bit data
 * 					- 1 Stopbit
 * 					- None Parity
 * @param[in]	HAL_USART_InitStruct Pointer to a USART_CFG_Type structure which will
 * 				be initialized.
 * @return		 HAL Satus
 *******************************************************************************/
HAL_Status_Type HAL_USART_USRT_Mode_Config(USART_CFG_Type *HAL_USART_InitStruct)
{
    	HAL_USART_InitStruct->Mode = USART_SYNC_MODE;
	HAL_USART_InitStruct->Baud_rate = 115200;//38400;
	HAL_USART_InitStruct->Databits = USART_DATABIT_8;
	HAL_USART_InitStruct->Parity = USART_PARITY_NONE;
	HAL_USART_InitStruct->Stopbits = USART_STOPBIT_1; 
	return HAL_OK;
}

/* USART Send/Recieve functions -------------------------------------------------*/
/**********************************************************************//**
 * @brief		Transmit a single data through USART peripheral
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * @param[in]	Data	Data to transmit (must be 8-bit long)
 * @return 		 HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_USART_TransmitByte(USART_Type* USARTx, uint8_t Data)
{
	USARTx->DR = Data;
	return HAL_OK;
}

/**********************************************************************//**
 * @brief		Receive a single data from USART peripheral
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * @return 		Data received
 **********************************************************************/
uint8_t HAL_USART_ReceiveByte(USART_Type* USARTx)
{
	return (USARTx->DR);
}

/**********************************************************************//**
 * @brief		Send a block of data via USART peripheral
 * @param[in]	USARTx	Selected USART peripheral used to send data, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @param[in]	txbuf 	Pointer to Transmit buffer
 * @param[in]	buflen 	Length of Transmit buffer
 * @return 		Number of bytes sent.
 *
 **********************************************************************/
uint32_t HAL_USART_Transmit(USART_Type *USARTx, uint8_t *txbuf, uint32_t buflen)
{
	uint32_t bToSend, bSent;
	uint8_t *pChar = txbuf;

	bToSend = buflen;
	bSent = 0;
	while (bToSend){          
		while(!(USARTx->ST & 0x80)){ }

		HAL_USART_TransmitByte(USARTx, (*pChar));

		pChar++;
		bToSend--;
		bSent++;
	}
	
	return bSent;
}

/*********************************************************************//**
 * @brief		Receive a block of data via USART peripheral
 * @param[in]	USARTx	Selected USART peripheral used to send data,
 * 				should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @param[out]	rxbuf 	Pointer to Received buffer
 * @param[in]	buflen 	Length of Received buffer
 * @param[in] 	flag 	Flag mode, should be:
 * 					- NONE_BLOCKING
 * 					- BLOCKING
 * @return 		Number of bytes received
 **********************************************************************/
uint32_t HAL_USART_Receive(USART_Type *USARTx, uint8_t *rxbuf, uint32_t buflen)
{
	uint32_t bToRecv, bRecv;
	uint8_t *pChar = rxbuf;

	bToRecv = buflen;
	bRecv = 0;
	while (bToRecv) {
		while(!(USARTx->ST & USART_SR_RXC)){ }
		
		(*pChar++) = HAL_USART_ReceiveByte(USARTx);
		bRecv++;
		bToRecv--;
	}
	return bRecv;
}


/********************************************************************//**
 * @brief 		Enable or disable specified USART interrupt.
 * @param[in]	USARTx	USART peripheral selected, should be
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @param[in]	USTIntCfg	Specifies the interrupt flag,
 * 				should be one of the following:
 * 					- USART_INTCFG_DR 	:DR Interrupt enable
 * 					- USART_INTCFG_TXC 	:TXC Interrupt enable
 * 					- USART_INTCFG_RXC 	:RXC interrupt enable
 * 					- USART_INTCFG_WAKE 	:WAKE Interrupt enable
 * @param[in]	NewState New state of specified USART interrupt type,
 * 				should be:
 * 					- ENALBE	:Enable this USART interrupt type.
 * 					- DISALBE	:Disable this USART interrupt type.
 * @return 		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_USART_ConfigInterrupt(USART_Type *USARTx, USART_INT_Type USTIntCfg, FunctionalState NewState)
{
	uint32_t tmp=0;
	
    
	switch(USTIntCfg){
		case USART_INTCFG_WAKE :
			tmp = USART_IER_WAKEINT_EN;
			break;
		case USART_INTCFG_RXC:
			tmp = USART_IER_RXCINT_EN;
			break;
		case USART_INTCFG_TXC:
			tmp = USART_IER_TXCINT_EN;
			break;
		case USART_INTCFG_DR:
			tmp = USART_IER_DR_EN;
			break;	
	}
    
	if (NewState == ENABLE)
	{
		USARTx->CR1 |= tmp;
	}
	else
	{
		USARTx->CR1 &= ~(tmp & USART_IER_BITMASK) ;
	}
	return HAL_OK;
}


/*********************************************************************//**
 * @brief 		Get current value of Line Status register in USART peripheral.
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @return		Current value of Status register in USART peripheral.
 *********************************************************************/
uint8_t HAL_USART_GetStatus(USART_Type* USARTx)
{
	return ((USARTx->ST) & USART_SR_BITMASK);
}

/*********************************************************************//**
 * @brief 		Clear Status register in USART peripheral.
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
  * @return 		 HAL Satus
 *********************************************************************/
HAL_Status_Type HAL_USART_ClearStatus(USART_Type* USARTx, USART_STATUS_Type Status)
{
    uint32_t tmp;
    
    switch(Status){
        case USART_STATUS_WAKE:
            tmp = USART_SR_WAKE;
            break;
        case USART_STATUS_RXC:
            tmp = USART_SR_RXC;
            break;
        case USART_STATUS_TXC:
            tmp = USART_SR_TXC;
            break;
        case USART_STATUS_DRE:
            tmp = USART_SR_DRE;
            break;
        default:
            break;
    }    
    
    USARTx->ST = tmp;
    return HAL_OK;
}

/**********************************************************************//**
 * @brief		Check whether if USART is busy or not
 * @param[in]	USARTx	USART peripheral selected, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @return		RESET if USART is not busy, otherwise return SET.
 **********************************************************************/
FlagStatus HAL_USART_CheckBusy(USART_Type *USARTx)
{
	if (USARTx->ST & USART_SR_DRE){
		return RESET;
	} else {
		return SET;
	}
}

/*********************************************************************//**
 * @brief		Configure Data Control mode for USART peripheral
 * @param[in]	USARTx
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @param[in]	Data Control mode, should be:
 * 					- USART_CR_USTEN     :Activate USARTn Block by supplying.
 * 					- USART_CR_DBLS      :Selects receiver sampling rate. (only UART mode)
 * 					- USART_CR_MASTER    :Selects master or slave in SPIn or Synchronous mode and controls the direction of SCKn pin.
 * 					- USART_CR_LOOPS     :Control the Loop Back mode of USARTn for test mode.
 * 					- USART_CR_DISSCK    :In synchronous mode operation, selects the waveform of SCKn output.
 * 					- USART_CR_USTSSEN   :This bit controls the SSn pin operation. (only SPI mode)
 * 					- USART_CR_FXCH      :SPIn port function exchange control bit. (only SPI mode)
 * 					- USART_CR_USTSB     :Selects the length of stop bit in Asynchronous or Synchronous mode.
 * 					- USART_CR_USTTX8    :The ninth bit of data frame in Asynchronous or Synchronous mode of operation. Write this bit first before loading the DR register.
 * 					- USART_CR_USTRX8    :The ninth bit of data frame in Asynchronous or Synchronous mode of operation. Read this bit first before reading the receive buffer (only UART mode)
 * @param[in]	NewState New State of this mode, should be:
 * 					- ENABLE	:Enable this mode.
					- DISABLE	:Disable this mode.
 * @return  HAL Satus
 **********************************************************************/
HAL_Status_Type HAL_USART_DataControlConfig(USART_Type *USARTx, USART_CONTROL_Type Mode, FunctionalState NewState)
{
	uint16_t tmp=0;
    
	switch(Mode){
	case USART_CONTROL_USTRX8:
		tmp = USART_CR2_USTRX8;
		break;
	case USART_CONTROL_USTTX8:
		tmp = USART_CR2_USTTX8;
		break;
	case USART_CONTROL_USTSB:
		tmp = USART_CR2_USTSB;
		break;
	case USART_CONTROL_FXCH:
		tmp = USART_CR2_FXCH;
		break;	
	case USART_CONTROL_USTSSEN:
		tmp = USART_CR2_USTSSEN;
		break;	
	case USART_CONTROL_DISSCK:
		tmp = USART_CR2_DISSCK;
		break;	
	case USART_CONTROL_LOOPS:
		tmp = USART_CR2_LOOPS;
		break;	
	case USART_CONTROL_MASTER:
		tmp = USART_CR2_MASTER;
		break;	
	case USART_CONTROL_DBLS:
		tmp = USART_CR2_DBLS;
		break;	
	case USART_CONTROL_USTEN:
		tmp = USART_CR2_USTEN;
		break;	
	default:
		break;
	}

	if (NewState == ENABLE)
	{
		USARTx->CR2 |= tmp;
	}
	else
	{
		USARTx->CR2 &= ~(tmp & USART_CR2_BITMASK) ;
	}
	return HAL_OK;
}
/*********************************************************************//**
 * @brief		Determines best dividers to get a target clock rate
 * @param[in]	USARTx	Pointer to selected USART peripheral, should be:
 * 					- USART10	:USART10 peripheral
 * 					- USART11	:USART11 peripheral
 * 					- USART12	:USART12 peripheral
 * 					- USART13	:USART13 peripheral 
 * @param[in]	baudrate Desired USART baud rate.
 * @return 		None
 **********************************************************************/
void HAL_USART_set_divisors(USART_Type *USARTx, uint32_t baudrate)
{
	uint32_t numerator; 
	uint32_t denominator; 
	uint32_t bdr;

	//------------------------------------------------------------------------------
	// numerator & denominator 
	// 
	// bdr = (UsartBaseClock) / n / baudrate  - 1
	//------------------------------------------------------------------------------
	numerator = UsartBaseClock; 
	denominator = baudrate; 

	bdr = numerator / denominator - 1; 

	USARTx->BDR= (uint16_t)(bdr&0xffff);

}

