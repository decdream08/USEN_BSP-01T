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
#ifndef TOUCHKEY_ENABLE
void SysTick_Handler(void)
{
#ifdef WATCHDOG_TIMER_RESET
	SysTick_Handler_IT();
#endif
}
#endif

#if (defined(USEN_BAP) || defined(USEN_BAP2)) && defined(SWITCH_BUTTON_KEY_ENABLE) //2022-10-11_2
void GPIOE_IRQHandler(void)
{
	GPIOE_IRQHandler_IT();
}
#endif

#ifdef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 //2022-10-17_1 : Disable not used functions
#if defined(BT_SPK_GPIO_ENABLE) || defined(SOUND_BAR_GPIO_ENABLE) || defined(AUX_INPUT_DET_ENABLE) //PC4 / PD0 : Interrupt Input, PD2 / PD3 : Output || //PC3/PD0/PD1 || PC3
/**********************************************************************
 * @brief		GPIOCD Handler.
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOCD_IRQHandler(void)
{
#if defined(BT_SPK_GPIO_ENABLE)
	GPIOCD_IRQHandler_IT();
#elif defined(SOUND_BAR_GPIO_ENABLE)
	GPIOCD_IRQHandler_IT1();
#else // AUX_INPUT_DET_ENABLE
	GPIOCD_IRQHandler_IT2();
#endif
}

#ifdef BT_SPK_TACT_SWITCH
void GPIOE_IRQHandler(void)
{
	GPIOE_IRQHandler_IT();
}
#endif //BT_SPK_TACT_SWITCH
#endif //#if defined(BT_SPK_GPIO_ENABLE) || defined(SOUND_BAR_GPIO_ENABLE)
#endif

#ifdef SWITCH_BUTTON_KEY_ENABLE
void GPIOAB_IRQHandler(void)
{
	GPIOAB_IRQHandler_IT();
}
#endif

#ifdef USEN_GPIO_OTHERS_ENABLE //Use External INT for Switchs and Button Keys - PF0 / PF5(AMP_ERROR input)
void GPIOF_IRQHandler(void)
{
	GPIOF_IRQHandler_IT();
}
#endif


/************************************************************************/
/*                 A31G21x Peripherals Interrupt Handlers                     */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_A31G21x.s).                                                 */
/*************************************************************************/
#ifdef I2C_0_ENABLE
void I2C0_IRQHandler(void)
{
	I2C0_IRQHandler_IT();
}
#endif
#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(TIMER20_COUNTER_ENABLE)
void TIMER20_IRQHandler(void)
{
	TIMER20_IRQHandler_IT();
}
#endif
#ifdef SPI_11_ENABLE
void USART11_IRQHandler(void)
{
	USART11_IRQHandler_IT();
}
#endif
#ifdef TIMER30_LED_PWM_ENABLE
void TIMER30_IRQHandler(void)
{
	TIMER30_IRQHandler_IT();
}
#endif

#ifdef TIMER1n_LED_PWM_ENABLE
void TIMER10_IRQHandler(void)
{
	TIMER10_IRQHandler_IT();
}

void TIMER11_IRQHandler(void)
{
	TIMER11_IRQHandler_IT();
}
#endif

#ifdef TIMER12_13_LONG_KEY_ENABLE
void TIMER12_IRQHandler(void)
{
	TIMER12_IRQHandler_IT();
}

void TIMER13_IRQHandler(void)
{
	TIMER13_IRQHandler_IT();
}
#endif

#ifdef TIMER21_LED_ENABLE
void TIMER21_IRQHandler(void)
{
	TIMER21_IRQHandler_IT();
}
#endif

#ifdef ADC_INTERRUPT_INPUT_ENABLE
/**********************************************************************
 * @brief		This function handles SysTick Handler.
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void ADC_IRQHandler(void)
{
	ADC_IRQHandler_IT();
}
#endif

