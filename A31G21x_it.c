/***********************************************************************
* @file		A31G21x_it.c
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library
* @version	1.0
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
***********************************************************************/
#include "A31G21x_it.h"
#include "main_conf.h"

/* Private typedef ----------------------------------------------------*/
/* Private define -----------------------------------------------------*/
/* Private macro ------------------------------------------------------*/
/* Private variables --------------------------------------------------*/
/* Private define -----------------------------------------------------*/


/**********************************************************************/
/*            Cortex M0+ Processor Exceptions Handlers                                                        */
/**********************************************************************/

/**********************************************************************
 * @brief		This function handles NMI exception.
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void NMI_Handler(void)
{
}


/**********************************************************************
 * @brief		This function handles Hard Fault exception.
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}


/**********************************************************************
 * @brief		This function handles SVCall exception
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SVC_Handler(void)
{
}

/**********************************************************************
 * @brief		This function handles PendSVC exception
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void PendSV_Handler(void)
{
}

/**********************************************************************
 * @brief		This function handles SysTick Handler.
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SysTick_Handler(void)
{
#ifdef WATCHDOG_TIMER_RESET
	SysTick_Handler_IT();
#endif
}

void GPIOE_IRQHandler(void)
{
	GPIOE_IRQHandler_IT();
}

/**********************************************************************
 * @brief		GPIOCD Handler.
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOCD_IRQHandler(void)
{
	GPIOCD_IRQHandler_IT2();
}

void GPIOAB_IRQHandler(void)
{
	GPIOAB_IRQHandler_IT();
}

void GPIOF_IRQHandler(void)
{
	GPIOF_IRQHandler_IT();
}

/************************************************************************/
/*                 A31G21x Peripherals Interrupt Handlers                     */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_A31G21x.s).                                                 */
/*************************************************************************/
void I2C0_IRQHandler(void)
{
	I2C0_IRQHandler_IT();
}

#ifdef I2C_1_ENABLE
void I2C1_IRQHandler(void)
{
	I2C1_IRQHandler_IT();
}
#endif

void TIMER20_IRQHandler(void)
{
	TIMER20_IRQHandler_IT();
}

void TIMER12_IRQHandler(void)
{
	TIMER12_IRQHandler_IT();
}

void TIMER13_IRQHandler(void)
{
	TIMER13_IRQHandler_IT();
}

void TIMER21_IRQHandler(void)
{
	TIMER21_IRQHandler_IT();
}

