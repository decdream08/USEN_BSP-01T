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

#ifdef TIMER21_LED_ENABLE
#include "timer21.h"
#include "led_display.h"
#include "timer20.h"

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/
/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
TIMER2n_PERIODICCFG_Type TIMER21_Config;

/* Private define ----------------------------------------------------*/
/* Private function prototypes ---------------------------------------*/
//void TIMER21_Error_Handler(void);

/* Private variables ---------------------------------------------------*/
uint16_t timer21_250ms_count = 0, timer21_500ms_count = 0, timer21_1s_count = 0;


void TIMER21_Configure(void)
{	
	/*Timer21 clock source from PCLK*/
	HAL_SCU_Timer20_ClockConfig(TIMER20CLK_PCLK);
	TIMER21_Config.CkSel = TIMER2n_MCCR2PCLK;    
	TIMER21_Config.Prescaler = 32;    /* 32Mhz / 32 = 1Mhz ->1us*/
	TIMER21_Config.ADR = (2500*100); //250ms //TIMER21_Config.ADR = (2500*200); //500ms
	TIMER21_Config.StartLevel = TIMER2n_START_LOW;   
	
	if(HAL_TIMER2n_Init(TIMER21, TIMER2n_PERIODIC_MODE, &TIMER21_Config) != HAL_OK)
	{
		/* Initialization Error */
    		//Error_Handler();
	}
	
	HAL_TIMER2n_ConfigInterrupt(TIMER21, TIMER2n_CR_MATCH_INTR, ENABLE);
	
	/* Enable Interrupt for TIMERx channel */
	NVIC_SetPriority(TIMER21_IRQn, 3);
	NVIC_EnableIRQ(TIMER21_IRQn); 
}


/**********************************************************************
 * @brief		TIMER21_OneShotRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void TIMER21_Periodic_Mode_Run(Bool On)
{
	static Bool On_bk = FALSE;

	if(On_bk == On) //Do not repeat same action with before
		return;

#ifdef LED_DISPLAY_CHANGE
	if(Get_master_slave_grouping_flag() && !On)
		return;
#endif	

	On_bk = On;
	
  /*timer start & clear*/
  	if(On)
  	{
  		timer21_250ms_count = 0;
  		timer21_500ms_count = 0;
		timer21_1s_count = 0;
		HAL_TIMER2n_Cmd(TIMER21, ENABLE);
  	}
	else
	{
		HAL_TIMER2n_Cmd(TIMER21, DISABLE);
		HAL_TIMER2n_ClearCounter(TIMER21);
		timer21_250ms_count = 0;
		timer21_500ms_count = 0;
		timer21_1s_count = 0;
	}
}

/**********************************************************************
 * @brief		TIMER21_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER21_IRQHandler_IT(void)
{	
	static Bool Very_Fast_Flag = FALSE; //250ms
	static Bool Fast_Flag = FALSE; //500ms
	static Bool Slow_Flag = FALSE; // 1sec
	Bool Blue_White_Flag;
	Bool Mute_On;  //Upon Mute On, this flag check whether need to set bliking mode or not. When mute is on, we don't need to run blinking mode except red led.
	Status_LED_Mode mode;

	mode = Get_Cur_Status_LED_Mode();
	Mute_On = IS_Display_Mute();

	if (HAL_TIMER2n_GetMatchInterrupt(TIMER21) == 1) //250ms
	{
		HAL_TIMER2n_ClearMatchInterrupt(TIMER21);
		
		if(timer21_250ms_count %2)
		{
			if(timer21_500ms_count%2)
			{
				if(timer21_1s_count%2)
					Slow_Flag = TRUE;
				else
					Slow_Flag = FALSE;

				timer21_1s_count++;
				
				Fast_Flag = TRUE;
			}
			else
			{
				Fast_Flag = FALSE;
			}

			Very_Fast_Flag = TRUE;
			
			timer21_500ms_count++;
		}
		else
		{
			Very_Fast_Flag = FALSE;			
		}

		timer21_250ms_count++;
		
		switch(mode)
		{ 				
			case STATUS_BT_PAIRING_MODE: // White Fast Blinking
				//White Fast Blinking
				if(!Mute_On)
					LED_Status_Display_Blinking(STATUS_LED_WHITE, Fast_Flag);

				LED_Status_Display_Blinking(L3_LED_WHITE, Fast_Flag); //Master Mode //Fast Blinking
				break;
				
			case STATUS_AMP_ERROR_MODE: // White Very Fast Blinking(250ms)
				//White Fast Blinking
				LED_Status_Display_Blinking(STATUS_LED_WHITE, Very_Fast_Flag);
				break;

			case STATUS_BT_MASTER_SLAVE_PAIRING_MODE: // White/Blue Fast Blinking (each 0.25sec)
				//White Fast Blinking
				if(!Mute_On)
					LED_Status_Display_Blinking(STATUS_LED_WHITE, Fast_Flag);
				
				//Master-Slave //White/Blue Fast Blinking
				if(Fast_Flag)
					Blue_White_Flag = FALSE;
				else
					Blue_White_Flag = TRUE;
				
				LED_Status_Display_Blinking(L3_LED_WHITE, Blue_White_Flag);
				LED_Status_Display_Blinking(L3_LED_BLUE, Fast_Flag);
			break;

			case STATUS_BT_GIA_PAIRING_MODE: // White Very Fast Blinking(250ms)
				//White Fast Blinking
				if(!Mute_On)
					LED_Status_Display_Blinking(STATUS_LED_WHITE, Very_Fast_Flag);

				LED_Status_Display_Blinking(L3_LED_WHITE, Very_Fast_Flag);
				break;

			case STATUS_BT_FAIL_OR_DISCONNECTION_MODE: // White Slow Blinking
				//White Slow Blinking
				if(!Mute_On)
					LED_Status_Display_Blinking(STATUS_LED_WHITE, Slow_Flag);

				LED_Status_Display_Blinking(L3_LED_WHITE, Slow_Flag);

				break;
 				
			case STATUS_SOC_ERROR_MODE: // Red Blinking
				break;
			
			default:
				break;
		}
	}
}

int16_t TIMER21_500ms_Count_Value(void)
{
	return timer21_500ms_count;	
}

#endif //TIMER21_LED_ENABLE


