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
#include <stdarg.h>
#include <stdio.h>
#include "A31G21x_hal_debug_frmwrk.h"
#include "A31G21x_hal_pcu.h"

/* Debug framework */
#ifdef _DEBUG_MSG
void (*_db_msg)(UART_Type *UARTx, const void *s);
void (*_db_msg_)(UART_Type *UARTx, const void *s);
void (*_db_char)(UART_Type *UARTx, uint8_t ch);
void (*_db_dec)(UART_Type *UARTx, uint8_t decn);
void (*_db_dec_16)(UART_Type *UARTx, uint16_t decn);
void (*_db_dec_32)(UART_Type *UARTx, uint32_t decn);
void (*_db_hex)(UART_Type *UARTx, uint8_t hexn);
void (*_db_hex_16)(UART_Type *UARTx, uint16_t hexn);
void (*_db_hex_32)(UART_Type *UARTx, uint32_t hexn);
uint8_t (*_db_get_char)(UART_Type *UARTx);
uint8_t (*_db_get_ch)(UART_Type *UARTx, uint8_t *ch);

/*********************************************************************//**
 * @brief		Puts a character to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	ch		Character to put
 * @return		None
 **********************************************************************/
void UARTPutChar (UART_Type *UARTx, uint8_t ch)
{
	HAL_UART_Transmit(UARTx, &ch, 1, BLOCKING);
}


/*********************************************************************//**
 * @brief		Get a character to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @return		character value that returned
 **********************************************************************/
uint8_t UARTGetChar (UART_Type *UARTx)
{
	uint8_t tmp = 0;
	HAL_UART_Receive(UARTx, &tmp, 1, BLOCKING);
	return(tmp);
}

/*********************************************************************//**
 * @brief		Get a character to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	*ch		Character to gett
 * @return		if getting value, return '1'. if not, return '0'
 **********************************************************************/
uint8_t UARTGetCh(UART_Type *UARTx, uint8_t *ch)
{
	if (!(UARTx->LSR & UART_LSR_RDR)){
		*ch=0;
		return(0);		
	}
	else {
		*ch = HAL_UART_ReceiveByte(UARTx);
		return(1);	
	}
}


/*********************************************************************//**
 * @brief		Puts a string to UART port
 * @param[in]	UARTx 	Pointer to UART peripheral
 * @param[in]	str 	string to put
 * @return		None
 **********************************************************************/
void UARTPuts(UART_Type *UARTx, const void *str)
{
	uint8_t *s = (uint8_t *) str;

	while (*s)
	{
		UARTPutChar(UARTx, *s++);
	} 
}


/*********************************************************************//**
 * @brief		Puts a string to UART port and print new line
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	str		String to put
 * @return		None
 **********************************************************************/
void UARTPuts_(UART_Type *UARTx, const void *str)
{
	UARTPuts (UARTx, str);
	UARTPuts (UARTx, "\n\r");
}


/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void UARTPutDec(UART_Type *UARTx, uint8_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	UARTPutChar(UARTx, '0'+c3);
	UARTPutChar(UARTx, '0'+c2);
	UARTPutChar(UARTx, '0'+c1);
}


/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void UARTPutDec16(UART_Type *UARTx, uint16_t decnum)
{
	uint8_t c1=decnum%10;
	uint8_t c2=(decnum/10)%10;
	uint8_t c3=(decnum/100)%10;
	uint8_t c4=(decnum/1000)%10;
	uint8_t c5=(decnum/10000)%10;
	UARTPutChar(UARTx, '0'+c5);
	UARTPutChar(UARTx, '0'+c4);
	UARTPutChar(UARTx, '0'+c3);
	UARTPutChar(UARTx, '0'+c2);
	UARTPutChar(UARTx, '0'+c1);
}


/*********************************************************************//**
 * @brief		Puts a decimal number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	decnum	Decimal number (8-bit long)
 * @return		None
 **********************************************************************/
void UARTPutDec32(UART_Type *UARTx, uint32_t decnum)
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
	UARTPutChar(UARTx, '0'+c10);
	UARTPutChar(UARTx, '0'+c9);
	UARTPutChar(UARTx, '0'+c8);
	UARTPutChar(UARTx, '0'+c7);
	UARTPutChar(UARTx, '0'+c6);
	UARTPutChar(UARTx, '0'+c5);
	UARTPutChar(UARTx, '0'+c4);
	UARTPutChar(UARTx, '0'+c3);
	UARTPutChar(UARTx, '0'+c2);
	UARTPutChar(UARTx, '0'+c1);
}


/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (8-bit long)
 * @return		None
 **********************************************************************/
void UARTPutHex (UART_Type *UARTx, uint8_t hexnum)
{
	uint8_t nibble, i;

//	UARTPuts(UARTx, "0x");
	i = 1;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		UARTPutChar(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}


/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (16-bit long)
 * @return		None
 **********************************************************************/
void UARTPutHex16 (UART_Type *UARTx, uint16_t hexnum)
{
	uint8_t nibble, i;

//	UARTPuts(UARTx, "0x");
	i = 3;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		UARTPutChar(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
	} while (i--);
}


/*********************************************************************//**
 * @brief		Puts a hex number to UART port
 * @param[in]	UARTx	Pointer to UART peripheral
 * @param[in]	hexnum	Hex number (32-bit long)
 * @return		None
 **********************************************************************/
void UARTPutHex32 (UART_Type *UARTx, uint32_t hexnum)
{
	uint8_t nibble, i;

//	UARTPuts(UARTx, "0x");
	i = 7;
	do {
		nibble = (hexnum >> (4*i)) & 0x0F;
		UARTPutChar(UARTx, (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble));
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
	UART_CFG_Type UARTConfigStruct;

#if (USED_UART_DEBUG_PORT==0)
	/*
	 * Initialize UART0 pin connect
	 */
	HAL_GPIO_ConfigOutput(PB, 5, ALTERN_FUNC); //RX0
	HAL_GPIO_ConfigFunction(PB, 5, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 5, 1);

	HAL_GPIO_ConfigOutput(PB, 4, ALTERN_FUNC); //TX0
	HAL_GPIO_ConfigFunction(PB, 4, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 4, 1);	
#elif (USED_UART_DEBUG_PORT==1)
	/*
	 * Initialize UART1 pin connect
	 */
	HAL_GPIO_ConfigOutput(PB, 7, ALTERN_FUNC); //RX1
	HAL_GPIO_ConfigFunction(PB, 7, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 7, 1);

	HAL_GPIO_ConfigOutput(PB, 6, ALTERN_FUNC); //TX1
	HAL_GPIO_ConfigFunction(PB, 6, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 6, 1);

//	HAL_GPIO_ConfigOutput(PF, 2, ALTERN_FUNC); //tX1
//	HAL_GPIO_ConfigFunction(PF, 2, FUNC1);
//	HAL_GPIO_ConfigPullup(PF, 2, 1);

//	HAL_GPIO_ConfigOutput(PF, 3, ALTERN_FUNC); //rX1
//	HAL_GPIO_ConfigFunction(PF, 3, FUNC1);
//	HAL_GPIO_ConfigPullup(PF, 3, 1);	
#endif

	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 38400bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	HAL_UART_ConfigStructInit(&UARTConfigStruct);
	UARTConfigStruct.Baud_rate = 115200;//38400;
	
	// Initialize DEBUG_UART_PORT peripheral with given to corresponding parameter
	HAL_UART_Init((UART_Type*)DEBUG_UART_PORT, &UARTConfigStruct);

	_db_msg	= UARTPuts;
	_db_msg_ = UARTPuts_;
	_db_char = UARTPutChar;
	_db_hex = UARTPutHex;
	_db_hex_16 = UARTPutHex16;
	_db_hex_32 = UARTPutHex32;
	_db_dec = UARTPutDec;
	_db_dec_16 = UARTPutDec16;
	_db_dec_32 = UARTPutDec32;
	_db_get_char = UARTGetChar;
	_db_get_ch = UARTGetCh;	
}

#endif

#endif /* _DEBUG_FRMWRK_ */

/* --------------------------------- End Of File ------------------------------ */
