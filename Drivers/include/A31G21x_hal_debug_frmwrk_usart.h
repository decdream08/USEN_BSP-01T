/**
* @file     A31G21x_hal_debug_frmwrk.h
* @brief    Contains some utilities that used for debugging through UART
* @author   AE Team, ABOV Semiconductor Co., Ltd.
* @version  1.00
* @date     
*
* Copyright(C) 2019, ABOV Semiconductor
*
*
*
*/

#ifndef _DEBUG_FRMWRK_H_
#define _DEBUG_FRMWRK_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "A31G21x_hal_usart1n.h"
#include "A31G21x_hal_libcfg.h"

#ifdef _DEBUG_MSG

#define USED_UART_DEBUG_PORT	0

#if (USED_UART_DEBUG_PORT==0)
#define DEBUG_UART_PORT	USART10
#elif (USED_UART_DEBUG_PORT==1)
#define DEBUG_UART_PORT	USART11
#endif

	

	
#define _DBG(x)	 	_db_msg((USART_Type*)DEBUG_UART_PORT, x)
#define _DBG_(x)	_db_msg_((USART_Type*)DEBUG_UART_PORT, x)
#define _DBC(x)	 	_db_char((USART_Type*)DEBUG_UART_PORT, x)
#define _DBD(x)	 	_db_dec((USART_Type*)DEBUG_UART_PORT, x)
#define _DBD16(x)	 _db_dec_16((USART_Type*)DEBUG_UART_PORT, x)
#define _DBD32(x)	 _db_dec_32((USART_Type*)DEBUG_UART_PORT, x)
#define _DBH(x)	 	_db_hex((USART_Type*)DEBUG_UART_PORT, x)
#define _DBH16(x)	 _db_hex_16((USART_Type*)DEBUG_UART_PORT, x)
#define _DBH32(x)	 _db_hex_32((USART_Type*)DEBUG_UART_PORT, x)
#define _DG			_db_get_char((USART_Type*)DEBUG_UART_PORT)
#define _DG_(x)	_db_get_ch((USART_Type*)DEBUG_UART_PORT,x)


extern void (*_db_msg)(USART_Type *UARTx, const void *s);
extern void (*_db_msg_)(USART_Type *UARTx, const void *s);
extern void (*_db_char)(USART_Type *UARTx, uint8_t ch);
extern void (*_db_dec)(USART_Type *UARTx, uint8_t decn);
extern void (*_db_dec_16)(USART_Type *UARTx, uint16_t decn);
extern void (*_db_dec_32)(USART_Type *UARTx, uint32_t decn);
extern void (*_db_hex)(USART_Type *UARTx, uint8_t hexn);
extern void (*_db_hex_16)(USART_Type *UARTx, uint16_t hexn);
extern void (*_db_hex_32)(USART_Type *UARTx, uint32_t hexn);
extern uint8_t (*_db_get_char)(USART_Type *UARTx);
extern uint8_t (*_db_get_ch)(USART_Type *UARTx, uint8_t *ch);

void USARTPutChar (USART_Type *UARTx, uint8_t ch);
void USARTPuts(USART_Type *UARTx, const void *str);
void USARTPuts_(USART_Type *UARTx, const void *str);
void USARTPutDec(USART_Type *UARTx, uint8_t decnum);
void USARTPutDec16(USART_Type *UARTx, uint16_t decnum);
void USARTPutDec32(USART_Type *UARTx, uint32_t decnum);
void USARTPutHex (USART_Type *UARTx, uint8_t hexnum);
void USARTPutHex16 (USART_Type *UARTx, uint16_t hexnum);
void USARTPutHex32 (USART_Type *UARTx, uint32_t hexnum);
uint8_t USARTGetChar (USART_Type *UARTx);
uint8_t USARTGetCh(USART_Type *UARTx, uint8_t *ch);
void debug_frmwrk_init(void);
void  cprintf(const  char *format, ...);

#ifdef __cplusplus
}
#endif

#endif  /* _DEBUG_MSG */

#endif /* _DEBUG_FRMWRK_H_ */

/* --------------------------------- End Of File ------------------------------ */
