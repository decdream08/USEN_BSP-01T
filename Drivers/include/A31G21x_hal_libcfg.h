/************************************************************************//**
* @file     A31G21x_hal_libcfg.h
* @brief    Contains all macro definitions and function prototypes support
*           for libcfg on A31G32x
* @author	PTS Team, ABOV Semiconductor Co., Ltd.
* @version	1.00
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
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


#ifndef _A31G21x_LIBCFG_H_
#define _A31G21x_LIBCFG_H_

/*******************************************************************************
* Included File
*******************************************************************************/
 /* Un-comment the line below to compile the library in DEBUG mode, this will expanse
 the "CHECK_PARAM" macro in the FW library code */
 
#define	USE_FULL_ASSERT
	
 /* DEBUG_FRAMWORK ------------------------------ */
//#define _DEBUG_MSG

///* SYSTICK --------------------------- */
//#define _SYSTICK

///* GPIO ------------------------------- */
//#define _GPIO

///* ADC ------------------------------- */
//#define _ADC


///* CRC -------------------------------- */
//#define _CRC


///* WDT --------------------------------- */
//#define _WDT

///* TIMER ------------------------------- */
//#define _TIMER

///* USART ------------------------------- */
//#define _USART
//#define _USART0
//#define _USART1
//#define _USART2
//#define _USART3
//#define _USART4
//#define _USART5


///* UART ------------------------------- */
//#define _UART0
//#define _UART1

///* SPI ------------------------------- */
//#define _SPI

///* I2C ------------------------------- */
//#define _I2C


///* WT ------------------------------- */
//#define _WT


///* TS ------------------------------- */
//#define _TS




/*******************************************************************************
* Public Macro
*******************************************************************************/
#ifdef  USE_FULL_ASSERT
/*******************************************************************************//**
* @brief                The CHECK_PARAM macro is used for function's parameters check.
*                               It is used only if the library is compiled in DEBUG mode.
* @param	    		expr - If expr is false, it calls check_failed() function
*                       which reports the name of the source file and the source
*                       line number of the call that failed.
*                    - If expr is true, it returns no value.
* @return               None
*******************************************************************************/
#define CHECK_PARAM(expr) ((expr) ? (void)0 : check_failed((uint8_t *)__FILE__, __LINE__))
#else
#define CHECK_PARAM(expr) ((void)0U)
#endif 

/*******************************************************************************
* Public Typedef
*******************************************************************************/
 
 
/*******************************************************************************//**
* Exported Public Function
*******************************************************************************/
#ifdef  USE_FULL_ASSERT
void check_failed(uint8_t *file, uint32_t line);
#endif

#endif


