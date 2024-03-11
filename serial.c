
#include "main_conf.h"
#ifdef UART_10_ENABLE
#include "serial.h"
#include "A31G21x_hal_usart1n.h"
#include "A31G21x_it.h"

static void Serial_GPIO_Configuration(USART_Type *USART_Port); 
static void Serial_USART_Configuration(USART_Type *USART_Port, uint32_t Baudrate); 
static void Serial_Interrupt_Configuration(USART_Type *USART_Port);

Serial_Handle_t Serial_HandleTable[SERIAL_PORT_MAX] = {(void *)0, };
uint8_t Rx10_Buffer;
uint8_t Rx11_Buffer;

void Serial_Init(SerialPort_t Port, uint32_t Baudrate)
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

	Serial_GPIO_Configuration(USART_Port);

	Serial_USART_Configuration(USART_Port, Baudrate);
        
	Serial_Interrupt_Configuration(USART_Port);
}

void Serial_Open(SerialPort_t Port, Serial_Handle_t Handle)
{
	Serial_HandleTable[Port] = Handle;
}

void Serial_Close(SerialPort_t Port)
{
    	Serial_HandleTable[Port] = NULL;
}

void Serial_PutChar(SerialPort_t Port, uint8_t Data)
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

	while(HAL_USART_CheckBusy(USART_Port)==SET);

	HAL_USART_TransmitByte(USART_Port, Data);
}

void Serial_PutString(SerialPort_t Port, uint8_t* String)
{
   	while(*String)
	{
		Serial_PutChar(Port, *String);
		String++;
	}
}

static void Serial_GPIO_Configuration(USART_Type *USART_Port)
{
	if (USART_Port == USART10) 
	{
		HAL_GPIO_ConfigOutput(PB, 1, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PB, 1, FUNC1);
		HAL_GPIO_ConfigPullup(PB, 1, ENPU);

		HAL_GPIO_ConfigOutput(PB, 0, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PB, 0, FUNC1);
	}
	else if (USART_Port == USART11) 
	{
		HAL_GPIO_ConfigOutput(PD, 3, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PD, 3, FUNC1);
		HAL_GPIO_ConfigPullup(PD, 3, ENPU);

		HAL_GPIO_ConfigOutput(PD, 2, ALTERN_FUNC);
		HAL_GPIO_ConfigFunction(PD, 2, FUNC1);
	}
}

static void Serial_USART_Configuration(USART_Type *USART_Port, uint32_t Baudrate)
{
	USART_CFG_Type USARTConfigStruct;

	USARTConfigStruct.Mode = USART_UART_MODE;
	USARTConfigStruct.Baud_rate = Baudrate;
	USARTConfigStruct.Databits = USART_DATABIT_8;
	USARTConfigStruct.Parity = USART_PARITY_NONE;
	USARTConfigStruct.Stopbits = USART_STOPBIT_1;

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
		if(HAL_USART_Init(USART10, &USARTConfigStruct) != HAL_OK)
		{
		/* Initialization Error */
		//Error_Handler();
		}
	}
}

static void Serial_Interrupt_Configuration(USART_Type *USART_Port)
{
	if(USART_Port == USART10)
	{
		HAL_USART_ConfigInterrupt((USART_Type *)USART10, USART_INTCFG_RXC, ENABLE);
		HAL_USART_ConfigInterrupt((USART_Type *)USART10, USART_INTCFG_TXC, ENABLE);

		NVIC_SetPriority(USART10_IRQn, 1); //To avoid UART10 data missing, changed priority from 3 to 1
		NVIC_EnableIRQ(USART10_IRQn);
	}
	else if(USART_Port == USART11)
	{
		HAL_USART_ConfigInterrupt((USART_Type *)USART11, USART_INTCFG_RXC, ENABLE);
		HAL_USART_ConfigInterrupt((USART_Type *)USART11, USART_INTCFG_TXC, ENABLE);

		NVIC_SetPriority(USART11_IRQn, 3);
		NVIC_EnableIRQ(USART11_IRQn);
	}
}

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
		Rx10_Buffer = HAL_USART_ReceiveByte(USART10);
		if(Serial_HandleTable[SERIAL_PORT10])
		{	
			Serial_HandleTable[SERIAL_PORT10](&Rx10_Buffer);
		}
	}

	// Transmit Holding Empty
	if ((tmp & USART_SR_TXC) == USART_SR_TXC)
	{
		HAL_USART_ClearStatus(USART10, USART_STATUS_TXC);
		//_usart10_tx_flag = 1;
	}
}

void Serial_Send(SerialPort_t port, uint8_t *txbuf, uint8_t buflen)
{
	uint8_t i;

	if(bFACTORY_MODE)
		return;	

#ifdef _UART_DEBUG_MSG
	_DBG("\n\rSend Data : ");
#endif

	for(i=0;i<buflen;i++)
	{
		Serial_PutChar(port, txbuf[i]);
		
#ifdef _UART_DEBUG_MSG
		_DBH(txbuf[i]);
#endif
	}
}
#endif //UART_10_ENABLE

