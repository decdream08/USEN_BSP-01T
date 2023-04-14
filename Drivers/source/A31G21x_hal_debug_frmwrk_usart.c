/**
* @file     A31G21x_hal_debug_frmwrk.c
* @brief    Contains all functions support for debugging through UART
* @author   AE Team, ABOV Semiconductor Co., Ltd.
* @version  1.00
* @date     
*
* Copyright(C) 2019, ABOV Semiconductor
*
*

*/

#ifndef _DEBUG_FRMWRK_
#define _DEBUG_FRMWRK_

/* Includes -------------------------------------------------------- */


/* Debug framework */

#include <stdarg.h>
#include <stdio.h>
#include "A31G21x_hal_debug_frmwrk_usart.h"
#include "A31G21x_hal_pcu.h"
#ifdef _DEBUG_MSG
void (*_db_msg)(USART_Type *UARTx, const void *s);
void (*_db_msg_)(USART_Type *UARTx, const void *s);
void (*_db_char)(USART_Type *UARTx, uint8_t ch);
void (*_db_dec)(USART_Type *UARTx, uint8_t decn);
void (*_db_dec_16)(USART_Type *UARTx, uint16_t decn);
void (*_db_dec_32)(USART_Type *UARTx, uint32_t decn);
void (*_db_hex)(USART_Type *UARTx, uint8_t hexn);
void (*_db_hex_16)(USART_Type *UARTx, uint16_t hexn);
void (*_db_hex_32)(USART_Type *UARTx, uint32_t hexn);
uint8_t (*_db_get_char)(USART_Type *UARTx);
uint8_t (*_db_get_ch)(USART_Type *UARTx, uint8_t *ch);

/*********************************************************************//**
 * @brief		Puts a character to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	ch		Character to put
 * @return		None
 **********************************************************************/
void USARTPutChar (USART_Type *UARTx, uint8_t ch)
{
	//HAL_USART_Transmit(UARTx, &ch, 1, BLOCKING);
	HAL_USART_Transmit(UARTx, &ch, 1 );
}


/*********************************************************************//**
 * @brief		Get a character to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @return		character value that returned
 **********************************************************************/
uint8_t USARTGetChar (USART_Type *UARTx)
{
	uint8_t tmp = 0;
	//HAL_USART_Receive(UARTx, &tmp, 1, BLOCKING);
	HAL_USART_Receive(UARTx, &tmp, 1);
	return(tmp);
}

/*********************************************************************//**
 * @brief		Get a character to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	*ch		Character to gett
 * @return		if getting value, return '1'. if not, return '0'
 **********************************************************************/
uint8_t USARTGetCh(USART_Type *UARTx, uint8_t *ch)
{
//	if (!(UARTx->LSR & UART_LSR_RDR)){
//		*ch=0;
//		return(0);		
//	}
//	else {
		*ch = HAL_USART_ReceiveByte(UARTx);
		return(1);	
//	}
}


/*********************************************************************//**
 * @brief		Puts a string to UART port
 * @param[in]	UARTx 	Pointer to UART peripheral
 * @param[in]	str 	string to put
 * @return		None
 **********************************************************************/
void USARTPuts(USART_Type *USARTx, const void *str)
{
	uint8_t *s = (uint8_t *) str;

	while (*s)
	{
		USARTPutChar(USARTx, *s++);
	} 
}


/*********************************************************************//**
 * @brief		Puts a string to UART port and print new line
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	str		String to put
 * @return		None
 **********************************************************************/
void USARTPuts_(USART_Type *UARTx, const void *str)
{
	USARTPuts (UARTx, str);
	USARTPuts (UARTx, "\n\r");
}


/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void USARTPutDec(USART_Type *UARTx, uint8_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	USARTPutChar(UARTx, '0'+c3);
	USARTPutChar(UARTx, '0'+c2);
	USARTPutChar(UARTx, '0'+c1);
}


/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void USARTPutDec16(USART_Type *UARTx, uint16_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	uint8_t c4=(decnum/1000)%10;
	uint8_t c5=(decnum/10000)%10;
	USARTPutChar(UARTx, '0'+c5);
	USARTPutChar(UARTx, '0'+c4);
	USARTPutChar(UARTx, '0'+c3);
	USARTPutChar(UARTx, '0'+c2);
	USARTPutChar(UARTx, '0'+c1);
}


/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void USARTPutDec32(USART_Type *UARTx, uint32_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	uint8_t c4=(decnum/1000)%10;
	uint8_t c5=(decnum/10000)%10;
	uint8_t c6=(decnum/100000)%10;
	uint8_t c7=(decnum/1000000)%10;
	uint8_t c8=(decnum/10000000)%10;
	uint8_t c9=(decnum/100000000)%10;
	uint8_t c10=(decnum/1000000000)%10;
	USARTPutChar(UARTx, '0'+c10);
	USARTPutChar(UARTx, '0'+c9);
	USARTPutChar(UARTx, '0'+c8);
	USARTPutChar(UARTx, '0'+c7);
	USARTPutChar(UARTx, '0'+c6);
	USARTPutChar(UARTx, '0'+c5);
	USARTPutChar(UARTx, '0'+c4);
	USARTPutChar(UARTx, '0'+c3);
	USARTPutChar(UARTx, '0'+c2);
	USARTPutChar(UARTx, '0'+c1);
}


/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (8-bit long)
 * @return		None
 **********************************************************************/
void USARTPutHex (USART_Type *UARTx, uint8_t hexnum)
{
	uint8_t nibble, i;

//	UARTPuts(UARTx, "0x");
	i = 1;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		USARTPutChar(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}


/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (16-bit long)
 * @return		None
 **********************************************************************/
void USARTPutHex16 (USART_Type *UARTx, uint16_t hexnum)
{
	uint8_t nibble, i;

//	UARTPuts(UARTx, "0x");
	i = 3;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		USARTPutChar(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}


/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (32-bit long)
 * @return		None
 **********************************************************************/
void USARTPutHex32 (USART_Type *UARTx, uint32_t hexnum)
{
	uint8_t nibble, i;

//	UARTPuts(UARTx, "0x");
	i = 7;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		USARTPutChar(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}


/*********************************************************************//**
 * @brief		print function that supports format as same as printf()
 * 				function of <stdio.h> library
 * @param[in]	format formated string to be print
 * @return		None
 **********************************************************************/
void  cprintf (const  char *format, ...)
{
    char  buffer[512 + 1];
            va_list     vArgs;
    va_start(vArgs, format);
    vsprintf((char *)buffer, (char const *)format, vArgs);
    va_end(vArgs);

    _DBG(buffer);
}


/*********************************************************************//**
 * @brief		Initialize Debug frame work through initializing UART port
 **********************************************************************/
void debug_frmwrk_init(void)
{
	USART_CFG_Type USARTConfigStruct;

#if (USED_UART_DEBUG_PORT==0)
/*
	* Initialize USART10 pin connect
	*/
	HAL_GPIO_ConfigOutput(PB, 1, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 1, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 1, ENPU);

	HAL_GPIO_ConfigOutput(PB, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 0, FUNC1);


	
#elif (USED_UART_DEBUG_PORT==1)
	/*
	* Initialize USART11 pin connect
	*/
	HAL_GPIO_ConfigOutput(PD, 3, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PD, 3, FUNC1);
	HAL_GPIO_ConfigPullup(PD, 3, ENPU);

	HAL_GPIO_ConfigOutput(PD, 2, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PD, 2, FUNC1);	
	
#endif

	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 38400bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	//HAL_USART_ConfigStructInit(&USARTConfigStruct);
	HAL_USART_UART_Mode_Config(&USARTConfigStruct);
	USARTConfigStruct.Baud_rate = 115200;//38400;
	
	// Initialize DEBUG_UART_PORT peripheral with given to corresponding parameter
	HAL_USART_Init((USART_Type*)DEBUG_UART_PORT, &USARTConfigStruct);

	_db_msg	= USARTPuts;
	_db_msg_ = USARTPuts_;
	_db_char = USARTPutChar;
	_db_hex = USARTPutHex;
	_db_hex_16 = USARTPutHex16;
	_db_hex_32 = USARTPutHex32;
	_db_dec = USARTPutDec;
	_db_dec_16 = USARTPutDec16;
	_db_dec_32 = USARTPutDec32;
	_db_get_char = USARTGetChar;
	_db_get_ch = USARTGetCh;	
}

#endif
#endif /* _DEBUG_FRMWRK_ */

/* --------------------------------- End Of File ------------------------------ */
