/**********************************************************************
* @file		main.c
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
#include "main_conf.h"

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/
/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
/* Private define ----------------------------------------------------*/

#ifdef TIMER30_LED_PWM_ENABLE
#include "timer30.h"
#include "A31G21x_hal_timer30.h"

#define PWM_HZ			6000  /* pwm */

/* Private function prototypes ---------------------------------------*/
void TIMER30_Error_Handler(void);

/* Private variables ---------------------------------------------------*/

	
/**********************************************************************
 * @brief		TIMER30_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER30_IRQHandler_IT(void)
{
	if ((HAL_TIMER3n_GetStatus_IT(TIMER30) & TIMER30INT_PMATEN) == TIMER30INT_PMATEN)
	{
#if 0
		if (flag==0){
			flag=1;
			HAL_GPIO_ClearPin(PC, _BIT(0));					
		}
		else {
			flag=0;
			HAL_GPIO_SetPin(PC, _BIT(0));
		}
#endif
	}
	
	HAL_TIMER3n_ClearStatus_IT(TIMER30,0x7F);	
}


/**********************************************************************
 * @brief		TIMER30_Configure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER30_Configure(void)
{
	
	/*Timer30 clock source from PCLK*/
	HAL_SCU_Timer30_ClockConfig(TIMER30CLK_PCLK); 
	
	if(HAL_TIMER3n_Init(TIMER30) != HAL_OK)
	{
		TIMER30_Error_Handler();
	}
	
	HAL_TIMER3n_SetPeriod(TIMER30,0);		/* 32MHz/(0+1) = 32MHz*/
	
	HAL_TIMER3n_SetPeriod(TIMER30,(uint16_t)(((uint32_t)40000000L / (PWM_HZ)) / 2 -1)); //8000Hz = 125us //3,332
	HAL_TIMER3n_SetADuty(TIMER30,(TIMER30->PDR>>1)); 
	HAL_TIMER3n_SetBDuty(TIMER30,(TIMER30->PDR>>1));
	HAL_TIMER3n_SetCDuty(TIMER30,(TIMER30->PDR>>1));

#ifdef LED_PORT_HIGH_DISPLAY
	HAL_TIMER3n_OutputCtrl(TIMER30,ENABLE,TIMER30O_APOHIGH,TIMER30O_BPOHIGH);
#else //LED_PORT_HIGH_DISPLAY
	HAL_TIMER3n_OutputCtrl(TIMER30,ENABLE,TIMER30O_APOLOW,TIMER30O_BPOLOW);//HAL_TIMER3n_OutputCtrl(TIMER30,ENABLE,TIMER30O_APOLOW,TIMER30O_BPOHIGH);
#endif
	
	HAL_TIMER3n_ConfigInterrupt(TIMER30,ENABLE,TIMER30INT_PMATEN);
	HAL_TIMER3n_ClearStatus_IT(TIMER30,0x7F);
	
	HAL_TIMER3n_SetADCTrigger(TIMER30,TIMER30ADT_BTTGEN,0);
	
	HAL_TIMER3n_MPWMCmd(TIMER30,TIMER30_UPMATCH,TIMER30_E1PERIOD);
	
	HAL_TIMER3n_SetDelayTime(TIMER30,TIMER30_DLYINSEN,TIMER30_INSFRONT, 63);	// (63 + 1) / 32MHz = 2us
	

	NVIC_EnableIRQ(TIMER30_IRQn);
	NVIC_SetPriority(TIMER30_IRQn, 3);
}


/**********************************************************************
 * @brief		TIMER30_PWMRun
 * @param[in]	None
 * @return	None
 **********************************************************************/
void TIMER30_PWMRun(void)
{
	HAL_TIMER3n_Start(TIMER30,TIMER30_ENABLE);	
}


/**********************************************************************
  * @brief  Reports the name of the source file and the source line number
  *   where the check_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: check_param error line source number
  * @retval : None
 **********************************************************************/
void TIMER30_Error_Handler(void)
{
    while (1)
    {
    }
}

#endif //TIMER30_LED_PWM_ENABLE

