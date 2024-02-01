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
#ifdef LED_DISPLAY_CHANGE
#include "timer20.h"
#endif

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
#if defined(MASTER_SLAVE_GROUPING_LED_DISPLAY) || defined(GIA_MODE_LED_DISPLAY_ENABLE)
	Bool Blue_White_Flag;
#endif
	Bool Mute_On;  //Upon Mute On, this flag check whether need to set bliking mode or not. When mute is on, we don't need to run blinking mode except red led.
	Status_LED_Mode mode;
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode Master_Slave;
	
	Master_Slave = Get_Cur_Master_Slave_Mode();
#endif
	mode = Get_Cur_Status_LED_Mode();
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_USE_POWER_DOWN_MUTE
	Mute_On = IS_Display_Mute();
#else
	Mute_On = Is_Mute(); //AD82584F_Amp_Get_Cur_Mute_Status();
#endif	
#endif
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
#ifdef MASTER_MODE_ONLY
				LED_Status_Display_Blinking(L3_LED_WHITE, Fast_Flag); //Master Mode //Fast Blinking
#else
				if(Master_Slave == Switch_Master_Mode) //Master Mode //Fast Blinking
				{
					LED_Status_Display_Blinking(L3_LED_WHITE, Fast_Flag);
				}
				else //Slave Mode
				{
					LED_Status_Display_Blinking(L3_LED_BLUE, Fast_Flag);
				}
#endif
				break;
				
#ifdef AMP_ERROR_ALARM
			case STATUS_AMP_ERROR_MODE: // White Very Fast Blinking(250ms)
				//White Fast Blinking
				LED_Status_Display_Blinking(STATUS_LED_WHITE, Very_Fast_Flag);
#ifndef USEN_BAP2 //2024-01-31_1 : BAP-02 display Power status LED blinking under AMP error(BAP-01 used BT status led White/Blue)
				LED_Status_Display_Blinking(L3_LED_WHITE, Very_Fast_Flag);
				LED_Status_Display_Blinking(L3_LED_BLUE, Very_Fast_Flag);
#endif
				break;
#endif

#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY
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

#ifdef GIA_MODE_LED_DISPLAY_ENABLE
			case STATUS_BT_GIA_PAIRING_MODE: // White Very Fast Blinking(250ms)
				//White Fast Blinking
				if(!Mute_On)
					LED_Status_Display_Blinking(STATUS_LED_WHITE, Very_Fast_Flag);
#ifndef MASTER_MODE_ONLY
				if(Master_Slave == Switch_Master_Mode)
#endif
				{
					LED_Status_Display_Blinking(L3_LED_WHITE, Very_Fast_Flag);
				}
				break;
#endif
#else //MASTER_SLAVE_GROUPING_LED_DISPLAY
#ifdef GIA_MODE_LED_DISPLAY_ENABLE
			case STATUS_BT_GIA_PAIRING_MODE: // White/Blue Slow Blinking
				//White Fast Blinking
				if(!Mute_On)
					LED_Status_Display_Blinking(STATUS_LED_WHITE, Fast_Flag);
				
#ifndef MASTER_MODE_ONLY				
				if(Master_Slave == Switch_Master_Mode) //Master Mode //White/Blue Slow Blinking
#endif
				{
					if(Slow_Flag)
					Blue_White_Flag = FALSE;
				else
					Blue_White_Flag = TRUE;
				
				LED_Status_Display_Blinking(L3_LED_WHITE, Blue_White_Flag);
					LED_Status_Display_Blinking(L3_LED_BLUE, Slow_Flag);
				}
			break;
#endif
#endif //MASTER_SLAVE_GROUPING_LED_DISPLAY

			case STATUS_BT_FAIL_OR_DISCONNECTION_MODE: // White Slow Blinking
				//White Slow Blinking
				if(!Mute_On)
					LED_Status_Display_Blinking(STATUS_LED_WHITE, Slow_Flag);

#ifdef MASTER_MODE_ONLY

				LED_Status_Display_Blinking(L3_LED_WHITE, Slow_Flag);
#else
				if(Master_Slave == Switch_Master_Mode) //Master Mode //Fast Blinking
				{
					LED_Status_Display_Blinking(L3_LED_WHITE, Slow_Flag);
				}
				else //Slave Mode
				{
					LED_Status_Display_Blinking(L3_LED_BLUE, Slow_Flag);
				}
#endif
				break;
 				
			case STATUS_SOC_ERROR_MODE: // Red Blinking
#ifndef USEN_BAP		//Red Fast Blinking
				LED_Status_Display_Blinking(STATUS_LED_RED, Fast_Flag);
#endif
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


