
#include "main_conf.h"

#ifdef SPI_11_ENABLE
#include "spi.h"
#include "A31G21x_hal_usart1n.h"
#include "A31G21x_it.h"

USART_CFG_Type USARTConfigStruct;

void SPI_GPIO_Configuration(USART_Type *USART_Port); 
void SPI_USART_Configuration(USART_Type *USART_Port, uint32_t Baudrate); 
void SPI_Interrupt_Configuration(USART_Type *USART_Port);

Serial_Handle_t SPI_HandleTable[SERIAL_PORT_MAX] = {(void *)0, };
uint8_t SPI_Rx10_Buffer;
uint8_t SPI_Rx11_Buffer;
uint8_t _usart11_tx_flag, _usart11_rx_flag;

void SPI_Init(SerialPort_t Port, uint32_t Baudrate)
{
	USART_Type *USART_Port;

	if(Port == SERIAL_PORT10)
	{
		USART_Port = USART10;
	}
	else if(Port == SERIAL_PORT11)
	{
		USART_Port = USART11;
	}

	SPI_GPIO_Configuration(USART_Port);
	SPI_USART_Configuration(USART_Port, Baudrate);        
	SPI_Interrupt_Configuration(USART_Port);
}

void SPI_Open(SerialPort_t Port, Serial_Handle_t Handle)
{
	SPI_HandleTable[Port] = Handle;
}

void SPI_Close(SerialPort_t Port)
{
    	SPI_HandleTable[Port] = NULL;
}

void USART_SPI_SS_control(USART_Type *USART1n, char ch)
{
	if (USART1n == USART10)
	{
		if (ch==0) 
			PB->OUTDR &= ~(1<<3);
		else 
			PB->OUTDR |= (1<<3);
	}
	else if (USART1n == USART11)
	{
		if (ch==0) 
			PD->OUTDR &= ~(1<<5);
		else 
			PD->OUTDR |= (1<<5);		
	}
	else
	{

	}
}

void SPI_WriteByte(uint8_t Data)
{
	uint32_t timeout;
	uint8_t *tx_flag;
	

	tx_flag = &_usart11_tx_flag;
		
	while(HAL_USART_CheckBusy(USART11)==SET); //check whether TX is busy or not

	//if(!bPowerOn) //When Power is up, we can't use this statement but else is OK.
	{
		timeout = 10000;
		HAL_USART_TransmitByte(USART11, Data); //Send Data
		
		while(timeout--)
		{
			if(*tx_flag) 
				break;
		}
		
		*tx_flag = 0;
	}
#if 0 //To Do(Need to check 2021-08-23 : If we add _DBG, ADAU1452 works well on MCU reset)
	else
	{
		uint8_t Status_Register = 0;
		static uint8_t check_flag = 0;

		Status_Register = HAL_USART_GetStatus(USART11);

		HAL_USART_TransmitByte(USART11, Data); //Send Data

		while(((Status_Register & USART_SR_TXC) != USART_SR_TXC))
		{
			if(check_flag == 0xff)
				{
				_DBG("\n\rR:");
				_DBH(Status_Register);
				break;
				}
			else
				check_flag++;
		}; //check whether TX is complete status or not
	}
#endif
}

void SPI_PutChar(SerialPort_t Port, uint8_t Data)
{
	USART_Type *USART_Port;

	if(Port == SERIAL_PORT10)
	{
		USART_Port = USART10;
	}
	else if(Port == SERIAL_PORT11)
	{
		USART_Port = USART11;
	}
	
	USART_SPI_SS_control(USART_Port, 0); //Clear(LOW) SS line
	
	while(HAL_USART_CheckBusy(USART_Port)==SET); //check whether TX is busy or not
	
	HAL_USART_TransmitByte(USART_Port, Data); //Send Data

	while(((HAL_USART_GetStatus(USART_Port)& USART_SR_TXC) != USART_SR_TXC)); //check whether TX is complete status or not
	
	USART_SPI_SS_control(USART_Port, 1); //Set(High) SS line
}

void SPI_PutString(SerialPort_t Port, uint8_t* String) //Need to change
{

	USART_Type *USART_Port;

	if(Port == SERIAL_PORT10)
	{
		USART_Port = USART10;
	}
	else if(Port == SERIAL_PORT11)
	{
		USART_Port = USART11;
	}

	USART_SPI_SS_control(USART_Port, 0); //Clear(LOW) SS line
	
	while(HAL_USART_CheckBusy(USART_Port)==SET); //check whether TX is busy or not
	
   	while(*String)
	{
		HAL_USART_TransmitByte(USART_Port, *String); //Send Data
		String++;
	}

	while(((HAL_USART_GetStatus(USART_Port)& USART_SR_TXC) != USART_SR_TXC)); //check whether TX is complete status or not
	
	USART_SPI_SS_control(USART_Port, 1); //Set(High) SS line
}

void SPI_GPIO_Configuration(USART_Type *USART_Port)
{
	if (USART_Port == USART10) 
	{
		//SPI I/F setting - MOSI:PB0/MISO:PB1/SCK:PB2/SS:PB3
		/* MOSI10 (PB0)	*/
		HAL_GPIO_ConfigOutput(PB, 0, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PB, 0, FUNC2);
		/* MISO10 (PB1)	*/
		HAL_GPIO_ConfigOutput(PB, 1, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PB, 1, FUNC2);
			/*SCK10 (PB2) */  
			HAL_GPIO_ConfigOutput(PB, 2, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PB, 2, FUNC2);

		/*SS10 (PB3)   : master control as GPIO */
		HAL_GPIO_ConfigOutput(PB, 3, PUSH_PULL_OUTPUT);
	}
	else if (USART_Port == USART11) 
	{
		//SPI I/F setting - MOSI:PD2/MISO:PD3/SCK:PD4/SS:PD5
		/* Initialize USART11 */
		// MOSI11 (PD2)	 
		HAL_GPIO_ConfigOutput(PD, 2, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PD, 2, FUNC2);
		// MISO11 (PD3)	
		HAL_GPIO_ConfigOutput(PD, 3, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PD, 3, FUNC2);
		//SCK11 (PD4) 
		HAL_GPIO_ConfigOutput(PD, 4, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PD, 4, FUNC2);
		
		//SS11 (PD5)   : master control as GPIO 
		HAL_GPIO_ConfigOutput(PD, 5, PUSH_PULL_OUTPUT);
	}
}

void SPI_USART_Configuration(USART_Type *USART_Port, uint32_t Baudrate)
{
	//USART_CFG_Type USARTConfigStruct;

	USARTConfigStruct.Mode = USART_SPI_MODE;
	USARTConfigStruct.Baud_rate = Baudrate;//1000000;//38400;//Baudrate;
	USARTConfigStruct.Databits = USART_DATABIT_8;

	//only SPI
	USARTConfigStruct.Order = USART_SPI_MSB;
#if 0 // CPOLn : 0, CPHAn : 0
	USARTConfigStruct.ACK = USART_SPI_TX_RISING;
	USARTConfigStruct.Edge = USART_SPI_TX_LEADEDGE_SAMPLE;
#endif
#if 0 // CPOLn : 0, CPHAn : 1
	USARTConfigStruct.ACK = USART_SPI_TX_RISING;
	USARTConfigStruct.Edge = USART_SPI_TX_LEADEDGE_SETUP;
#endif
#if 0 // CPOLn : 1, CPHAn : 0 
	USARTConfigStruct.ACK = USART_SPI_TX_FALLING;
	USARTConfigStruct.Edge = USART_SPI_TX_LEADEDGE_SAMPLE;
#endif
#if 1 // CPOLn : 1, CPHAn : 1
	USARTConfigStruct.ACK = USART_SPI_TX_FALLING;
	USARTConfigStruct.Edge = USART_SPI_TX_LEADEDGE_SETUP;
#endif

	if(USART_Port == USART10)
	{
		if(HAL_USART_Init(USART10, &USARTConfigStruct) != HAL_OK)
		{
		/* Initialization Error */
		//Error_Handler();
		}
	}
	else if(USART_Port == USART11)
	{
		if(HAL_USART_Init(USART11, &USARTConfigStruct) != HAL_OK)
		{
		/* Initialization Error */
		//Error_Handler();
		}
	}
}

void SPI_Interrupt_Configuration(USART_Type *USART_Port)
{
	if(USART_Port == USART10)
	{
		// SPI Master
		HAL_USART_DataControlConfig(USART10, USART_CONTROL_MASTER, ENABLE);
		USART_SPI_SS_control(USART10, 1); // SS init
		
		/*interrupt*/
		NVIC_SetPriority(USART10_IRQn, 3);
		NVIC_EnableIRQ(USART10_IRQn);
	    
		HAL_USART_ConfigInterrupt(USART10, USART_INTCFG_TXC, ENABLE);
		HAL_USART_ConfigInterrupt(USART10, USART_INTCFG_RXC, ENABLE);
	}
	else if(USART_Port == USART11)
	{
		// SPI Master
		HAL_USART_DataControlConfig(USART11, USART_CONTROL_MASTER, ENABLE);
		USART_SPI_SS_control(USART11, 1); // SS init
				
		//interrupt
		NVIC_SetPriority(USART11_IRQn, 3);
		NVIC_EnableIRQ(USART11_IRQn);

		HAL_USART_ConfigInterrupt(USART11, USART_INTCFG_TXC, ENABLE);
		HAL_USART_ConfigInterrupt(USART11, USART_INTCFG_RXC, ENABLE);
	}
}

#ifndef SPI_11_ENABLE
void USART10_IRQHandler(void)
{
	uint32_t intsrc, tmp;

	/* Determine the interrupt source */
	intsrc = HAL_USART_GetStatus(USART10);
	tmp = intsrc & USART_SR_BITMASK;

	// Receive Data Available or Character time-out
	if ((tmp & USART_SR_RXC) == USART_SR_RXC)
	{
		//_usart10_rx_buffer[_usart10_buffer_tail++] = HAL_USART_ReceiveByte(USART10);
		//_usart10_rx_flag = 1;
		SPI_Rx10_Buffer = HAL_USART_ReceiveByte(USART10);
		if(SPI_HandleTable[SERIAL_PORT10])
		{	
			SPI_HandleTable[SERIAL_PORT10](&SPI_Rx10_Buffer);
		}
	}

	// Transmit Holding Empty
	if ((tmp & USART_SR_TXC) == USART_SR_TXC)
	{
		HAL_USART_ClearStatus(USART10, USART_STATUS_TXC);
		//_usart10_tx_flag = 1;
	}
}
#endif //SPI_11_ENABLE

void USART11_IRQHandler_IT(void)
{
	uint32_t intsrc, tmp;

	/* Determine the interrupt source */
	intsrc = HAL_USART_GetStatus(USART11);
	tmp = intsrc & USART_SR_BITMASK;

	// Receive Data Available or Character time-out
	if ((tmp & USART_SR_RXC) == USART_SR_RXC){
        //_usart11_rx_buffer[_usart11_buffer_tail++] = HAL_USART_ReceiveByte(USART11);
		_usart11_rx_flag = 1;
		SPI_Rx11_Buffer = HAL_USART_ReceiveByte(USART11);
		if(SPI_HandleTable[SERIAL_PORT11])
		{	
			SPI_HandleTable[SERIAL_PORT11](&SPI_Rx11_Buffer);
		}
	}

	// Transmit Holding Empty
	if ((tmp & USART_SR_TXC) == USART_SR_TXC){
        HAL_USART_ClearStatus(USART11, USART_STATUS_TXC);
		_usart11_tx_flag = 1;
	}
}

void SPI_Send(SerialPort_t port, uint8_t *txbuf, uint8_t buflen)
{
	uint8_t i;

#ifdef _UART_DEBUG_MSG
	_DBG("\n\rSend Data : ");
#endif

	for(i=0;i<buflen;i++)
	{
		SPI_PutChar(port, txbuf[i]);
		
#ifdef _UART_DEBUG_MSG
		_DBH(txbuf[i]);
#endif
	}
}

#endif //SPI_11_ENABLE

