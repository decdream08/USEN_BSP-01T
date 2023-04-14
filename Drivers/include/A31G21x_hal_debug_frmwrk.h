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


#include "A31G21x_hal_uartn.h"
#include "A31G21x_hal_libcfg.h"

#ifdef _DEBUG_MSG

#define USED_UART_DEBUG_PORT	1 //ms.kim.210611 : The UART Debug port is one of UART0/UART1. We'll use UART1 as the debug port.

#if (USED_UART_DEBUG_PORT==0)
#define DEBUG_UART_PORT	UART0
#elif (USED_UART_DEBUG_PORT==1)
#define DEBUG_UART_PORT	UART1
#endif

#define _DBG(x)	 	_db_msg((UART_Type*)DEBUG_UART_PORT, x)
#define _DBG_(x)	_db_msg_((UART_Type*)DEBUG_UART_PORT, x)
#define _DBC(x)	 	_db_char((UART_Type*)DEBUG_UART_PORT, x)
#define _DBD(x)	 	_db_dec((UART_Type*)DEBUG_UART_PORT, x)
#define _DBD16(x)	 _db_dec_16((UART_Type*)DEBUG_UART_PORT, x)
#define _DBD32(x)	 _db_dec_32((UART_Type*)DEBUG_UART_PORT, x)
#define _DBH(x)	 	_db_hex((UART_Type*)DEBUG_UART_PORT, x)
#define _DBH16(x)	 _db_hex_16((UART_Type*)DEBUG_UART_PORT, x)
#define _DBH32(x)	 _db_hex_32((UART_Type*)DEBUG_UART_PORT, x)
#define _DG			_db_get_char((UART_Type*)DEBUG_UART_PORT)
#define _DG_(x)	_db_get_ch((UART_Type*)DEBUG_UART_PORT,x)


extern void (*_db_msg)(UART_Type *UARTx, const void *s);
extern void (*_db_msg_)(UART_Type *UARTx, const void *s);
extern void (*_db_char)(UART_Type *UARTx, uint8_t ch);
extern void (*_db_dec)(UART_Type *UARTx, uint8_t decn);
extern void (*_db_dec_16)(UART_Type *UARTx, uint16_t decn);
extern void (*_db_dec_32)(UART_Type *UARTx, uint32_t decn);
extern void (*_db_hex)(UART_Type *UARTx, uint8_t hexn);
extern void (*_db_hex_16)(UART_Type *UARTx, uint16_t hexn);
extern void (*_db_hex_32)(UART_Type *UARTx, uint32_t hexn);
extern uint8_t (*_db_get_char)(UART_Type *UARTx);
extern uint8_t (*_db_get_ch)(UART_Type *UARTx, uint8_t *ch);

void UARTPutChar (UART_Type *UARTx, uint8_t ch);
void UARTPuts(UART_Type *UARTx, const void *str);
void UARTPuts_(UART_Type *UARTx, const void *str);
void UARTPutDec(UART_Type *UARTx, uint8_t decnum);
void UARTPutDec16(UART_Type *UARTx, uint16_t decnum);
void UARTPutDec32(UART_Type *UARTx, uint32_t decnum);
void UARTPutHex (UART_Type *UARTx, uint8_t hexnum);
void UARTPutHex16 (UART_Type *UARTx, uint16_t hexnum);
void UARTPutHex32 (UART_Type *UARTx, uint32_t hexnum);
uint8_t UARTGetChar (UART_Type *UARTx);
uint8_t UARTGetCh(UART_Type *UARTx, uint8_t *ch);
void debug_frmwrk_init(void);
void  cprintf(const  char *format, ...);

#ifdef __cplusplus
}
#endif

#else
#define _DBG(x)
#define _DBG_(x)
#define _DBC(x)	
#define _DBD(x)	
#define _DBD16(x)
#define _DBD32(x)
#define _DBH(x)	 
#define _DBH16(x)
#define _DBH32(x)
#define _DG		
#define _DG_(x)	
#endif  /* _DEBUG_MSG */

#endif /* _DEBUG_FRMWRK_H_ */

/* --------------------------------- End Of File ------------------------------ */
