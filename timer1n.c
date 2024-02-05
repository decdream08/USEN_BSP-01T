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

#if defined(TIMER1n_LED_PWM_ENABLE ) || defined(TIMER12_13_LONG_KEY_ENABLE)
#include "timer1n.h"
#include "A31G21x_hal_timer1n.h"
#include "led_display.h"

#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
#include "remocon_action.h"
#endif

/* Private typedef ---------------------------------------------------*/
//#define TIMER1N_DBG_ENABLE				(1)

/* Private define ----------------------------------------------------*/
/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
TIMER1n_PERIODICCFG_Type TIMER1n_Config;

/* Private define ----------------------------------------------------*/
/* Private function prototypes ---------------------------------------*/
void TIMER1n_Error_Handler(void);

/* Private variables ---------------------------------------------------*/

#ifdef TIMER12_13_LONG_KEY_ENABLE
uint16_t timer13_50ms_count = 0;
uint16_t timer12_50ms_count = 0;

Bool bVolume_Up = FALSE;
Bool bVol_Up_Long = FALSE, bVol_Down_Long = FALSE; //To avoid, sending same key again
Bool bBT_Long = FALSE;
Bool bPower_Long = FALSE;
Bool bTimer13_Long = FALSE;
#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
Bool bFactory_Reset_Long = FALSE;
#endif
#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : To display EQ Mode using volume level indicator during 3 sec
Bool bEQ_Long = FALSE;
#endif


Timer13_Long_Key_Type Long_Key_Type = Timer13_None_Key;

#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : To display EQ Mode using volume level indicator during 3 sec
Bool Is_EQ_Long_Key(void) //To avoid, sending same key again
{
#ifdef TIMER1N_DBG_ENABLE
	_DBG("\n\rIs_EQ_Long_Key = ");
	_DBD(bEQ_Long);
#endif

	return bEQ_Long;
}
#endif //EQ_TOGGLE_ENABLE

#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
Bool Is_Factory_Reset_Long_Key(void)
{
#ifdef TIMER1N_DBG_ENABLE
	_DBG("\n\rIs_Factory_Reset_Long_Key = ");
	_DBD(bFactory_Reset_Long);
#endif
	return bFactory_Reset_Long;
}
#endif


Bool Is_Power_Long_Key(void)
{
#ifdef TIMER1N_DBG_ENABLE
	_DBG("\n\rIs_Power_Long_Key = ");
	_DBD(bPower_Long);
#endif
	return bPower_Long;
}

Bool Is_BT_Long_Key(void) //To avoid, sending same key again
{
#ifdef TIMER1N_DBG_ENABLE
	_DBG("\n\rIs_BT_Long_Key = ");
	_DBD(bBT_Long);
#endif

	return bBT_Long;
}

Bool Is_Volume_Up_Long_Key(void) //To avoid, sending same key again
{
#ifdef TIMER1N_DBG_ENABLE
	_DBG("\n\rIs_Volume_Up_Long_Key = ");
	_DBD(bVol_Up_Long);
#endif

	return bVol_Up_Long;
}

Bool Is_Volume_Down_Long_Key(void) //To avoid, sending same key again
{
#ifdef TIMER1N_DBG_ENABLE
	_DBG("\n\rIs_Volume_Down_Long_Key = ");
	_DBD(bVol_Down_Long);
#endif

	return bVol_Down_Long;
}

Bool Is_Timer13_Long_Key(void) //To avoid, sending same key again
{
#ifdef TIMER1N_DBG_ENABLE
	_DBG("\n\rIs_Timer13_Long_Key = ");
	_DBD(bTimer13_Long);
#endif
	
	return bTimer13_Long;
}

void TIMER12_Configure(void)
{
	/*Timer1n clock source from PCLK*/
#if !defined(TIMER1n_LED_PWM_ENABLE) && (defined(USEN_BAP) || defined(USEN_BAP2))//Need to Call clock setting function to use Timer1n //2022-10-11_3 
	HAL_SCU_Timer1n_ClockConfig(TIMER1nCLK_PCLK); 
#endif
	TIMER1n_Config.CkSel = TIMER1n_MCCR1PCLK;    
	TIMER1n_Config.Prescaler = 32;    
  
	/*TIMER1n 50msec	*/
	TIMER1n_Config.ADR = (50000); 	// 1000 : 1msec
	TIMER1n_Config.StartLevel = TIMER1n_START_LOW;
	
	if(HAL_TIMER1n_Init(TIMER12, TIMER1n_PERIODIC_MODE, &TIMER1n_Config) != HAL_OK)
	{
		/* Initialization Error */
    		TIMER1n_Error_Handler();
	}

	/*TIMER10 ConfigInterrup*/
	HAL_TIMER1n_ConfigInterrupt(TIMER12, TIMER1n_INTCFG_MIE, ENABLE);
	
	/* Enable Interrupt for TIMERn channel */
	NVIC_SetPriority(TIMER12_IRQn, 3);
	NVIC_EnableIRQ(TIMER12_IRQn);
}

/**********************************************************************
 * @brief		TIMER1n_OneShotRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void TIMER12_Periodic_Mode_Run(Bool On, Bool Vol_Up)
{	
  /*timer start & clear*/
  	if(On)
  	{
  		timer12_50ms_count = 0;
		HAL_TIMER1n_Cmd(TIMER12, ENABLE);
  	}
	else
	{
		HAL_TIMER1n_Cmd(TIMER12, DISABLE);
		HAL_TIMER1n_ClearCounter(TIMER12);
		timer12_50ms_count = 0;
		bVol_Up_Long = FALSE;
		bVol_Down_Long = FALSE;
	}

	if(Vol_Up) //Just check whether volume key is volume up or volume down
		bVolume_Up = TRUE;
	else
		bVolume_Up = FALSE;
}

/**********************************************************************
 * @brief		TIMER12_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER12_IRQHandler_IT(void) //50msec timer
{	
	static uint32_t timer12_50ms_count_bk = 0;
	
	if (HAL_TIMER1n_GetMatchInterrupt(TIMER12) == 1)
	{
		HAL_TIMER1n_ClearMatchInterrupt(TIMER12);
		timer12_50ms_count++;

		if(timer12_50ms_count >= 60) // 3 Sec
		{
			if(timer12_50ms_count == 60)
				timer12_50ms_count_bk = timer12_50ms_count;

			if((timer12_50ms_count - timer12_50ms_count_bk) == 10) //0.5 Sec
			{
				if(bVolume_Up)
				{
#ifdef TIMER1N_DBG_ENABLE
					_DBG("\n\rLong key send - VOL_UP_KEY");
#endif
					bVol_Up_Long = TRUE;
#ifdef SWITCH_BUTTON_KEY_ENABLE
					Send_Remote_Key_Event(VOL_UP_KEY);
#endif
				}
				else
				{
#ifdef TIMER1N_DBG_ENABLE
					_DBG("\n\rLong key send - VOL_DOWN_KEY");
#endif
					bVol_Down_Long = TRUE;
#ifdef SWITCH_BUTTON_KEY_ENABLE
					Send_Remote_Key_Event(VOL_DOWN_KEY);
#endif
				}
				
				timer12_50ms_count_bk = timer12_50ms_count;
			}
		}
		else
			timer12_50ms_count_bk = 0;
	}
}

int16_t TIMER12_50ms_Count_Value(void)
{
	return timer12_50ms_count;	
}


void TIMER13_Configure(void)
{
	/*Timer1n clock source from PCLK*/
	//HAL_SCU_Timer1n_ClockConfig(TIMER1nCLK_PCLK); 
	TIMER1n_Config.CkSel = TIMER1n_MCCR1PCLK;    
	TIMER1n_Config.Prescaler = 32;    
  
	/*TIMER1n 50msec	*/
	TIMER1n_Config.ADR = (50000); 	// 1000 : 1msec
	TIMER1n_Config.StartLevel = TIMER1n_START_LOW;
	
	if(HAL_TIMER1n_Init(TIMER13, TIMER1n_PERIODIC_MODE, &TIMER1n_Config) != HAL_OK)
	{
		/* Initialization Error */
    		TIMER1n_Error_Handler();
	}

	/*TIMER10 ConfigInterrup*/
	HAL_TIMER1n_ConfigInterrupt(TIMER13, TIMER1n_INTCFG_MIE, ENABLE);
	
	/* Enable Interrupt for TIMERn channel */
	NVIC_SetPriority(TIMER13_IRQn, 3);
	NVIC_EnableIRQ(TIMER13_IRQn);
}

/**********************************************************************
 * @brief		TIMER1n_OneShotRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void TIMER13_Periodic_Mode_Run(Bool On, Timer13_Long_Key_Type Key_Type)
{	
	static Bool Enable = FALSE; //To avoid, calling same cmd without oppossite action
	
	Long_Key_Type = Key_Type;
	
  	/*timer start & clear*/
  	if(On)
  	{
  		if(Enable)
  		{
			HAL_TIMER1n_Cmd(TIMER13, DISABLE);
			HAL_TIMER1n_ClearCounter(TIMER13);
			Enable = FALSE;
  		}
		
  		timer13_50ms_count = 0;
		HAL_TIMER1n_Cmd(TIMER13, ENABLE);
		Enable = TRUE;
  	}
	else
	{
		HAL_TIMER1n_Cmd(TIMER13, DISABLE);
		HAL_TIMER1n_ClearCounter(TIMER13);

		timer13_50ms_count = 0;
		Long_Key_Type = Timer13_None_Key;
		bTimer13_Long = FALSE;
		Enable = FALSE;
		bBT_Long = FALSE;
		bPower_Long = FALSE;
#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
		bFactory_Reset_Long = FALSE;
#endif
#ifdef EQ_TOGGLE_ENABLE //2023-01-17
		bEQ_Long = FALSE;
#endif
	}
}

/**********************************************************************
 * @brief		TIMER13_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER13_IRQHandler_IT(void) //50ms timer
{	
	if (HAL_TIMER1n_GetMatchInterrupt(TIMER13) == 1)
	{
		HAL_TIMER1n_ClearMatchInterrupt(TIMER13);
		timer13_50ms_count++;

		bTimer13_Long = TRUE;
#if !defined(USEN_BAP) /*&& !defined(USEN_BAP_2)*/ //2022-10-07_3
		if(Long_Key_Type == Timer13_Power_Key)
		{
			//POWER ON KEY(Long Key) condiiton is over than 6 sec(50ms * 120) 
			if(timer13_50ms_count == /*120*/40)
			{
#ifdef TIMER1N_DBG_ENABLE
				_DBG("\n\rTIMER13_IRQHandler_IT = Timer13_Power_Key");
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
#ifdef POWER_KEY_TOGGLE_ENABLE
				Send_Remote_Key_Event(POWER_KEY);
#else //POWER_KEY_TOGGLE_ENABLE
				Send_Remote_Key_Event(POWER_ON_KEY);
#endif //POWER_KEY_TOGGLE_ENABLE
#endif //SWITCH_BUTTON_KEY_ENABLE
				bPower_Long = TRUE;
			}
		}
		else
#endif //USEN_BAP
		if(Long_Key_Type == Timer13_BT_Pairing_Key)
		{
			//BT PAIRING KEY(Long Key) condiiton is over than 5 sec(50ms * 100) 
			if(timer13_50ms_count == 100)
			{
#ifdef TIMER1N_DBG_ENABLE
				_DBG("\n\rTIMER13_IRQHandler_IT = Timer13_BT_Pairing_Key");
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
				Send_Remote_Key_Event(BT_PAIRING_KEY);
#endif
				bBT_Long = TRUE;
			}
		}
		else if(Long_Key_Type == Timer13_Factory_Reset_Key)
		{
#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
			if(timer13_50ms_count == 40) //BT PAIRING KEY(Long Key) condiiton is over than 2 sec(50ms * 40)//5 sec(50ms * 100) 
#else
			if(timer13_50ms_count == 120) //BT PAIRING KEY(Long Key) condiiton is over than 6 sec(50ms * 120) 
#endif
			{
#ifdef TIMER1N_DBG_ENABLE
				_DBG("\n\rTIMER13_IRQHandler_IT = Timer13_Factory_Reset_Key");
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
				Send_Remote_Key_Event(FACTORY_RESET_KEY);
				bFactory_Reset_Long = TRUE;
#else
				//Send_Remote_Key_Event(FACTORY_RESET_KEY);  //To Do !!! : Disable D-code and Enablue G-code
#endif
#endif
			}
		}
#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : To display EQ Mode using volume level indicator during 3 sec
		else if(Long_Key_Type == Timer13_EQ_Toggle_Key)
		{
			if(timer13_50ms_count == 60) //BT PAIRING KEY(Long Key) condiiton is over than 3 sec(50ms * 60)
			{
#ifdef TIMER1N_DBG_ENABLE
				_DBG("\n\rTIMER13_IRQHandler_IT = Timer13_EQ_Toggle_Key");
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
				Send_Remote_Key_Event(EQ_KEY);
				bEQ_Long = TRUE;
			}
#endif
		}
#endif //EQ_TOGGLE_ENABLE
		else
		{
#ifdef TIMER1N_DBG_ENABLE
			_DBG("\n\rError - Long_Key_Type is Timer13_BT_None_Key");
#endif
		}
	}
}

int16_t TIMER13_50ms_Count_Value(void)
{
	return timer13_50ms_count;	
}
#endif

#ifdef TIMER1n_LED_PWM_ENABLE
/**********************************************************************
 * @brief		TIMER10_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER10_IRQHandler_IT(void)
{
	if (HAL_TIMER1n_GetMatchInterrupt(TIMER10) == 1)
		HAL_TIMER1n_ClearMatchInterrupt(TIMER10);
}

/**********************************************************************
 * @brief		TIMER10_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER11_IRQHandler_IT(void)
{
	if (HAL_TIMER1n_GetMatchInterrupt(TIMER11) == 1)
		HAL_TIMER1n_ClearMatchInterrupt(TIMER11);
}

/**********************************************************************
 * @brief		TIMER1n_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void TIMER1n_Configure(Timer1n_type timer_type, uint8_t utime)
{
/***********************************************************************/
/********************  Total Period	/  High Period	/ Low Period *************/
/************ OFF			200us		0us			200us	************/
/************ LEVEL_1		200us		50us			150us	************/
/************ LEVEL_2		200us		100us		100us	************/
/************ FULL			200us		200us		0us		************/
/***********************************************************************/
	TIMER1n_Type *TIMER1n;

	if(timer_type == Timer1n_10)
		TIMER1n = TIMER10;
	else
		TIMER1n = TIMER11;
		
	/*Timer1n clock source from PCLK*/
	HAL_SCU_Timer1n_ClockConfig(TIMER1nCLK_PCLK); 
	TIMER1n_Config.CkSel = TIMER1n_MCCR1PCLK;    
	TIMER1n_Config.Prescaler = 32;    

	/*TIMER1n 200usec	*/
	TIMER1n_Config.ADR = (200); //200usec
	TIMER1n_Config.BDR = (utime); // 0usec	
#ifdef LED_PORT_HIGH_DISPLAY
	TIMER1n_Config.StartLevel = TIMER1n_START_HIGH;
#else
	TIMER1n_Config.StartLevel = TIMER1n_START_LOW;
#endif

	if(HAL_TIMER1n_Init(TIMER1n, TIMER1n_PWM_MODE, &TIMER1n_Config) != HAL_OK)
	{
		/* Initialization Error */
      		TIMER1n_Error_Handler();
	}
	
	/*TIMER10/11 ConfigInterrup*/
	HAL_TIMER1n_ConfigInterrupt(TIMER1n, TIMER1n_INTCFG_MIE, ENABLE);

	if(timer_type == Timer1n_10)
	{
  /* Enable Interrupt for TIMERn channel */
		NVIC_SetPriority(TIMER10_IRQn, 3);
		NVIC_EnableIRQ(TIMER10_IRQn);
	}
	else //TIMER11
	{
		NVIC_SetPriority(TIMER11_IRQn, 3);
		NVIC_EnableIRQ(TIMER11_IRQn);	
	}
}

/**********************************************************************
 * @brief		TIMER1n_PWMRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void TIMER1n_PWMRun(Timer1n_type timer_type)
{
	TIMER1n_Type *TIMER1n;

	if(timer_type == Timer1n_10)
		TIMER1n = TIMER10;
	else
		TIMER1n = TIMER11;
	
  /*timer start & clear*/
	HAL_TIMER1n_Cmd(TIMER1n, ENABLE);
}
#endif //TIMER1n_LED_PWM_ENABLE

/**********************************************************************
  * @brief  Reports the name of the source file and the source line number
  *   where the check_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: check_param error line source number
  * @retval : None
 **********************************************************************/
void TIMER1n_Error_Handler(void)
{
    while (1)
    {
    }
}
#endif //#if defined(TIMER1n_LED_PWM_ENABLE ) || defined(TIMER12_13_LONG_KEY_ENABLE)


