/**********************************************************************
* @file		A31G21x_it.h
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library
* @version	1.0
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
**********************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __A31G21x_IT_H
#define __A31G21x_IT_H

/* Includes ------------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif
	
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
#ifndef TOUCHKEY_ENABLE
void SysTick_Handler(void);
#endif

#ifdef I2C_0_ENABLE
void I2C0_IRQHandler(void);
#endif
#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(TIMER20_COUNTER_ENABLE)
void TIMER20_IRQHandler(void);
#endif
#ifdef SPI_11_ENABLE
void USART11_IRQHandler(void);
#endif
#ifdef TIMER30_LED_PWM_ENABLE
void TIMER30_IRQHandler(void);
#endif
#ifdef ADC_INTERRUPT_INPUT_ENABLE
void ADC_IRQHandler(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __A31G21x_IT_H */

