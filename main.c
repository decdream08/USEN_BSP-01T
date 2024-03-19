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
#include "serial.h"
#include "i2c.h"
#include "bt_MB3021.h"
#include "AD85050.h"
#include "timer1n.h"
#include "timer21.h"
#include "timer20.h"
#include "led_display.h"
#include "remocon_action.h"
#ifdef WATCHDOG_TIMER_RESET
#include "A31G21x_hal_wdt.h"
#endif
#include "flash.h"
#include "adc.h"
#ifdef PCM9211_ENABLE
#include "pcm9211.h"
#endif

#include "power.h"
#include "adc_polling.h"
#include "key.h"
#include "protection.h"

/* Private typedef ---------------------------------------------------*/
//BT_SPK_TACT_SWITCH - PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
//SWITCH_BUTTON_KEY_ENABLE BSP-01: PA0 / PA1 / PA2 /PA3 / PA4 / PA5 / PA6, BAP-01 : PA0 / PA1 / PA6
typedef enum
{
	button_push = 0,
	button_release
}button_status;

/* Private macro -----------------------------------------------------*/

/* Private variables -------------------------------------------------*/
const uint8_t menu[] =
"************************************************\n\r"
" LGD BT Speaker \n\r"
"\t - MCU: A31G21x \n\r"
"\t - Core: ARM Cortex-M0+ \n\r"
"\t - Communicate via: UART0 - 115200 bps \n\r"
"\t - 2022-02-17 \n\r"
"************************************************\n\r";
Bool B_AUX_DET; //Input(Aux Detec Pin) : TRUE -Aux In / FALSE - Aux Out

uint8_t UART10_Rx_Buffer[UART10_Rx_Buffer_Size];
uint8_t uBuffer_Count = 0;

#ifdef WATCHDOG_TIMER_RESET
uint32_t msec;
WDT_CFG_Type wdtCfg;
#endif
static uint32_t uAttenuator_Vol = 0xff; //2022-11-22_1

uint8_t	main_1ms_counter = 0;
uint8_t main_10ms_detect = OFF;

/* Private define ----------------------------------------------------*/
#define KEY_CHATTERING_DELAY_MS				(20) //40ms //20ms
#define KEY_FILTERING_TIME					(3)//(4) //over than X x 100ms

/* Private function prototypes ---------------------------------------*/
void mainloop(void);
void GPIO_Configure(void);
void EXTI_PortA_Configure(void);

void EXIT_PortF_Configure(void);

void Display_UART_Receive_Data(void);
static void Serial_Get_Data_Intterupt_Callback(uint8_t *Data);

uint8_t Convert_ADC_To_Attenuator(uint32_t ADC_Value);

void EXTI_PortC_Configure(void);

uint32_t ADC_Value_Update_to_send_Slave(void)
{
  uint32_t ADC_Value;
  uint8_t uCurVolLevel = 0, ADC_Level_Min, ADC_Level_Max, i;
  uint32_t l_volLevel = 0;
  uint8_t adc_count = 0;
  
  while(adc_count <= AREA2_VOLUME)
  {
    ADC_Value = ADC_PollingRun(4-adc_count);

#ifdef ADC_INPUT_DEBUG_MSG
    _DBG("\n\r === Master Volume ADC = 0x");
    _DBH32(ADC_Value);
#endif
    for(i=1;i<52;i++)
    {
      if(i==1)
        ADC_Level_Min = 0;
      else
        ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

      if(i==51)
        ADC_Level_Max = 255;
      else
        ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253

      if((ADC_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC_Value))
      {
        if(i==1)
          ADC_Level_Min = 0;
        else
          ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

        if(i==51)
          ADC_Level_Max = 255;
        else
          ADC_Level_Max = (i*5)-1; //4 9 14 19 ... 244 249~253

        if((ADC_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC_Value)) //2023-02-08_3 : Added additional code for Volume GAP
        {
#ifdef ADC_INPUT_DEBUG_MSG
          _DBG("\n\r Volume ADC Valid Value !!!! = ");
          _DBG("\n\r === Volume ADC Level Step = ");
          _DBD(i);
#endif
          uCurVolLevel = 51 - i;
        }
        else //2023-02-06_3 : Do not update cur volume level and current ADC value is not valid
        {
          uCurVolLevel = 50; //Vol Level 1
#ifdef ADC_INPUT_DEBUG_MSG
          _DBG("\n\r Volume ADC Invalid Value !!!! = ");
          _DBG("\n\r === Volume ADC Level Step = ");
          _DBD(i);
#endif
        }

        break;
      }
    }

    uCurVolLevel = 51 - i;
#ifdef ADC_INPUT_DEBUG_MSG
      _DBG("\n\r === Volume Level Setting = ");
      _DBD(uCurVolLevel);
#endif
    if(adc_count == 0) //Area 2
    {
      l_volLevel = uCurVolLevel;
    }
    else if(adc_count == 1) //Area 1
    {
      l_volLevel <<= 8;
      l_volLevel |= uCurVolLevel;
    }
    else if(adc_count == 2) //Slave BT
    {
      l_volLevel <<= 8;
      l_volLevel |= uCurVolLevel;
    }

    adc_count++;
  }

  AD85050_Amp_Volume_Set_with_Index(l_volLevel, FALSE, TRUE);

  return uCurVolLevel;
}

void Aux_Mode_Setting_After_Timer_Checking(Bool Aux_In)
{
#ifdef AUX_INPUT_DET_DEBUG
	_DBG("\n\r ### Aux_Mode_Setting_After_Timer_Checking()");
#endif

	B_AUX_DET = Aux_In;

	if(B_AUX_DET == FALSE)
	{
		MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);

		if(!Aux_In_Exist())
		{
			AD85050_Amp_Mute(TRUE, FALSE); //MUTE ON
		}

		Set_MB3021_BT_Module_Source_Change_Direct();
	}
}

void Set_Aux_Detection_flag(void) //2023-04-12_1
{	
	if(HAL_GPIO_ReadPin(PC) & (1<<3)) //Input(Aux Detec Pin) : High -Aux Out / Low -Aux In
	{
		B_AUX_DET = TRUE;
	}
	else
	{
    B_AUX_DET = FALSE;
	}
}

void SW_Reset(void) //2023-02-21_7 : After reboot, Slave SPK has Audio NG issue. So, Added more delay.
{
#ifdef COMMON_DEBUG_MSG
	_DBG("\n\rSW Reset");
#endif

#if 0
	HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN
	delay_ms(20);
	HAL_GPIO_ClearPin(PF, _BIT(3)); //+14V_DAMP_SW_1
	delay_ms(20);
	HAL_GPIO_ClearPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
#endif

	HAL_SCU_SetResetSrc(RST_SWRST, ENABLE);
	SCU->SCR |= (0x9EB30000|SCU_SCR_SWRST_Msk);
}

/**********************************************************************
 * @brief		All_Timer_Off
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void All_Timer_Off(void)
{
	TIMER12_Periodic_Mode_Run(FALSE, FALSE);
	TIMER13_Periodic_Mode_Run(FALSE, Timer13_BT_Pairing_Key);
	TIMER13_Periodic_Mode_Run(FALSE, Timer13_Factory_Reset_Key);
	TIMER21_Periodic_Mode_Run(FALSE);
}

/**********************************************************************
 * @brief		Aux In Exist
 * @param[in]	None
 * @return 		None
 **********************************************************************/
Bool Aux_In_Exist(void)
{
#ifdef AUX_INPUT_DET_DEBUG
	_DBG("\n\rAux_In_Exist: ");
	_DBH(B_AUX_DET);
#endif

	if(HAL_GPIO_ReadPin(PC) & (1<<3)) //High : Aux /Low : BT
		return TRUE;
	else
		return FALSE;
}

/**********************************************************************
 * @brief		Make all value as initial value
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void Factory_Reset_Value_Setting(void)
{
#ifdef COMMON_DEBUG_MSG
	_DBG("\n\rFactory_Reset_Value_Setting(void)");
#endif
	Remocon_EQ_Key_Action(EQ_NORMAL_MODE);
	MB3021_BT_Delete_Paired_List_All(TRUE);
}

/**********************************************************************
 * @brief		Make all value as initial value
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void Init_Value_Setting(Bool B_boot)
{
#ifdef COMMON_DEBUG_MSG
	_DBG("\n\rInit_Value_Setting(void)");
#endif
	if(!B_boot)
		TIMER20_Amp_error_flag_Stop();

	B_AUX_DET = FALSE; //FALSE - Aux Out //2023-04-12_1 : We need to make FALSE because HW make 5sec delay to keep Aux In(Low) even though there is no Aux In after DC In.
	TIMER20_aux_detection_flag_start();
}

void main_10ms_timer(void)
{
	if(main_1ms_counter >= df1msTimer10ms)
	{
		main_1ms_counter = df1msTimer0ms;
		main_10ms_detect = ON;

		Power_10ms_timer();
		MB3021_10ms_timer();
#ifdef PCM9211_ENABLE
		PCM9211_10ms_timer();
#endif
		AD85050_10ms_timer();
		ADC_Polling_10ms_timer();
		Key_10ms_timer();
	}
}

void main_timer_function(void)
{
	main_10ms_timer();
}

#ifdef WATCHDOG_TIMER_RESET
/**********************************************************************
 * @brief		SysTick handler sub-routine (1ms)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SysTick_Handler_IT (void) 
{
	if(msec)msec--;

	++main_1ms_counter;
}

/**********************************************************************
 * @brief		ADC_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void WDT_Configure(void)
{
  	/* WDT clock source from WDTRC. WDTRC must set LSI clock enable!!*/
	HAL_SCU_LSI_ClockConfig(LSIOSC_EN);
	HAL_SCU_WDT_ClockConfig(WDTCLK_WDTRC);	// 31250 hz	

	HAL_SCU_SetResetSrc(RST_WDTRST, ENABLE);
	HAL_SCU_ClearResetStatus(0xff); // clear all reset status 

	/* WDTDR(0.5s) < WDTWDR(1s), clear in 900ms */
	wdtCfg.wdtResetEn = ENABLE;
	wdtCfg.wdtClkDiv = WDT_DIV_4; 

	wdtCfg.wdtTmrConst = (7812*60)/2; 	// 30s //wdtCfg.wdtTmrConst = (7812*20)/2; 	// 10s
	wdtCfg.wdtWTmrConst = 7812*60; 		//60s //wdtCfg.wdtWTmrConst = 7812*20; 		//20s

	if(HAL_WDT_Init(wdtCfg)!= HAL_OK)
	{
		/* Initialization Error */
    		//Error_Handler();
	}
	
	HAL_WDT_ConfigInterrupt(WDT_INTCFG_UNFIEN, ENABLE);
	HAL_WDT_ConfigInterrupt(WDT_INTCFG_WINMIEN, ENABLE);	
	
	NVIC_SetPriority(WDT_IRQn, 3);
	NVIC_EnableIRQ(WDT_IRQn);
}
	
/**********************************************************************
 * @brief		SysTick_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void SysTick_Configure(void)
{	
	/*1msec interrupt */
   	SysTick_Config(SystemCoreClock/1000);
}

/**********************************************************************
 * @brief		WDT_ResetRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void WDT_ResetRun(void)
{	
   	HAL_WDT_Start(ENABLE);
}


/**********************************************************************
 * @brief		WDT_ReloadTimeRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void WDT_ReloadTimeRun(void)
{
	//msec = 900; 
	//while(msec);
	
#ifdef WATCHDOG_TIMER_RESET_DEBUG_MSG
	_DBG("\n\rWDT_ReloadTimeRun !!!");
#endif
	HAL_WDT_ReloadTimeCounter();
}
#endif

/**********************************************************************
 * @brief		Main loop
 * @param[in]	None
 * @return	None
 **********************************************************************/
void mainloop(void)
{
	uint8_t uFlash_Read_Buf1[FLASH_SAVE_DATA_END];

	/*Configure menu prinf*/
	DEBUG_MenuPrint();
#ifdef WATCHDOG_TIMER_RESET
	WDT_Configure();
	SysTick_Configure(); //2023-05-16_1 : Implemented WDT Reset
#endif
	/*Configure port peripheral*/
	GPIO_Configure();
	Serial_Open(SERIAL_PORT10, Serial_Get_Data_Intterupt_Callback);

	EXIT_PortE_Configure();
	EXTI_PortA_Configure();
	EXTI_PortC_Configure();

	EXIT_PortF_Configure();

	I2C_Configure();
#ifdef I2C_1_ENABLE
	I2C1_Configure();
#endif

	/*UART USART10 Configure*/
	Serial_Init(SERIAL_PORT10, 115200);

	TIMER21_Configure();
	TIMER20_Configure();

	TIMER12_Configure();
	TIMER13_Configure();
	//TIMER13_Periodic_Mode_Run(TRUE); //This function is called in Long Key Start

	ADC_Configure();

	/* Enable IRQ Interrupts */
	__enable_irq();

#ifdef WATCHDOG_TIMER_RESET //2023-05-16_1
	/*WDT Reset Start*/
	WDT_ResetRun();
#endif

	TIMER20_Periodic_Mode_Run(TRUE);

	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf1, FLASH_SAVE_DATA_END);

	if(uFlash_Read_Buf1[FLASH_SAVE_GENERAL_MODE_KEEP] == 0x01/* && uFlash_Read_Buf1[FLASH_SAVE_DATA_PDL_NUM] != 0x01*/) //Under Last connection mode, we don't need to use this statement
	{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
		_DBG("\n\rInit working ... Set General Mode after reading Flash Data !!!");
#endif
		BBT_Pairing_Key_In = TRUE; //Implemented GIA_PAIRING //To use, BT Pairing Key In is True only.
	}
	else
	{
		FlashSaveData(FLASH_SAVE_GENERAL_MODE_KEEP, 1); //Save GENERAL MODE Status(GENERAL Mode/GIA Mode) to Flash
	}

	BBT_Pairing_Key_In = TRUE;

	MB3021_PowerUp();
	Init_Value_Setting(TRUE); //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
	init_protect_values();

#ifdef COMMON_DEBUG_MSG
	_DBG("\n\rAll Init Done !!!");
#endif

	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf1, FLASH_SAVE_DATA_END);

	if(uFlash_Read_Buf1[FLASH_SAVE_DATA_POWER] == 0 || uFlash_Read_Buf1[FLASH_SAVE_DATA_POWER] == 0xff) //Power Off
	{
		Power_Mode_Set(PWR_OFF_START);
	}
	else //Power On - 0xff or 0x01
	{
		Power_Mode_Set(PWR_ON_START);		
	}

	while (1)
	{
		main_timer_function();

		if(main_10ms_detect == ON)
		{
			main_10ms_detect = OFF;

			Power_Process();
			Key_Process();

			MB3021_Process();			
#ifdef PCM9211_ENABLE
			PCM9211_Process();
#endif
#ifdef AD85050_ENABLE
			AD85050_Process();
#endif
			ADC_Polling_Process();
			Protection_Process();
		}
	}
}

/**********************************************************************
 * @brief		Main program
 * @param[in]	None
 * @return	None
 **********************************************************************/
int main (void)
{
	__disable_irq();
	
	SystemInit();
	 /* Initialize all port */
	Port_Init(); 

	/* Configure the system clock to HSI 32MHz */
	SystemClock_Config();
	
	/* Initialize Debug frame work through initializing UART port  */
	DEBUG_Init();
	
	/* Infinite loop */
	mainloop();  

	return (0);
}

/**********************************************************************
 * @brief		GPIOAB_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOCD_IRQHandler_IT2(void)
{
	static uint32_t status = 0, clear_bit = 0;
	static uint32_t shift_bit = 0;
	uint8_t key = 0;
	int ret = 0;

	static uint8_t key_bk = 0;
	static uint32_t filtering_time = 0, filtering_time_old = 0, filtering_time_very_old = 0;

	filtering_time = TIMER20_100ms_Count_Value();

	status = HAL_GPIO_EXTI_GetState(PC);

#ifdef AUX_INPUT_DET_DEBUG
	_DBG("\n\rstatus = ");
	_DBH32(status);
#endif

	shift_bit = 0xffffffff;

	if (status & (3UL<<(3<<1))) //Just check PC3
	{
		if(status & 0x00000080) //Rising Edge : High check - Aux
		{
			shift_bit = 0;

			delay_ms(80);

			if(HAL_GPIO_ReadPin(PC) & (1<<3)) //PC3 : High - Aux
			{
				key = INPUT_AUX_KEY;
			}
			else //Low Invalid value in here
			{
				shift_bit = 0xffffffff;
				key = NONE_KEY;
			}
		}
		else //0x00000040 //Falling Edge
		{
			shift_bit = 2;

			delay_ms(80);

			if(!(HAL_GPIO_ReadPin(PC) & (1<<3)) )
			{
				key = INPUT_BT_KEY;
			}
			else //High - Aux out //Invalid value in here
			{
				shift_bit = 0xffffffff;
				key = NONE_KEY;
			}
		}

		clear_bit = status & (3UL<<(3<<1));

		HAL_GPIO_EXTI_ClearPin(PC, status&clear_bit);
	}

	if(shift_bit != 0xffffffff)
	{
		if(key != NONE_KEY)
		{
			if(key == key_bk)
			{
				if((filtering_time - filtering_time_old) <= KEY_FILTERING_TIME)
				{													
					if((filtering_time - filtering_time_very_old) <= KEY_FILTERING_TIME)
					{
						ret = -1;
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
						_DBG("\n\rTime = ");_DBH32(filtering_time_very_old);_DBG("/");_DBH32(filtering_time_old);_DBG("/");_DBH32(filtering_time);
#endif
					}
					else
						ret = 0;

					filtering_time_very_old = filtering_time_old;
				}
			}

			filtering_time_old = filtering_time;
			key_bk = key;				

			if(ret != -1) //input select aux / bt
			{
				Send_Remote_Key_Event(key);
			}
		}
	}		
}

/**********************************************************************
 * @brief		EXTI_PortC_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void EXTI_PortC_Configure(void)
{	
	HAL_GPIO_EXTI_Config(PC, 3, IER_EDGE, ICR_BOTH_EDGE_INT);

	NVIC_SetPriority(GPIOCD_IRQn, 3);	
	NVIC_EnableIRQ(GPIOCD_IRQn);
}

Switch_BAP_EQ_Mode Get_Cur_BAP_EQ_Mode(void)
{
	Switch_BAP_EQ_Mode mode;

	if(HAL_GPIO_ReadPin(PA) & (1<<1)) //High
	{
		mode = Switch_EQ_NORMAL_Mode;
	}
	else //Low
	{
		mode = Switch_EQ_BSP_Mode;
	}

	return mode;
}

Switch_LR_Stereo_Mode Get_Cur_LR_Stereo_Mode(void)
{
	Switch_LR_Stereo_Mode mode;

	if(HAL_GPIO_ReadPin(PA) & (1<<0)) //High
	{
		mode = Switch_Stereo_Mode;
	}
	else //Low
	{
		mode = Switch_LR_Mode;
	}

	return mode;
}

/**********************************************************************
 * @brief		GPIOAB_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOAB_IRQHandler_IT(void)
{
	static uint32_t status = 0, clear_bit = 0, shift_bit = 0;
	uint8_t key = 0;
	button_status cur_button_status;
	int ret = 0;

	static uint8_t key_bk = 0;
	static uint32_t filtering_time = 0, filtering_time_old = 0, filtering_time_very_old = 0;
	
	filtering_time = TIMER20_100ms_Count_Value();

	status = HAL_GPIO_EXTI_GetState(PA);
	
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
	_DBG("\n\rstatus_A = ");
	_DBH32(status);
#endif

	if(!Power_State())
	{
		if(!(status & (3UL<<(6<<1))) //PA6
			&& !(status & (3UL<<(7<<1))) //PA7
			)
		{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
			_DBG("\n\r3. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
			HAL_GPIO_EXTI_ClearPin(PA, status);

			return;
		}
	}

	if (status & ((3UL<<(6<<1)) | (3UL<<(7<<1)))) //Just check PA0 / PA1 / PA4 / PA6 / PA7 (1111 0011 0000 1111)	
	{
		shift_bit = 0xffffffff;

#ifdef KEY_CHATTERING_ENABLE
		delay_ms(KEY_CHATTERING_DELAY_MS+60); //2023-05-04_2 : Under BAP-01, key chattering delay is increased from 20ms to 80ms.
#endif
    if(status & (3UL<<(6<<1))) //0x00003000 PA6 : POWER_Off(short)/POWER_ON(Long) //Implemented Power Key Feature //2022-10-07_3
		{
			shift_bit = 12;
			key = POWER_KEY;

			if(status & 0x00001000)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<6)) //PA6 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_push; //High -> Low
				}
			}
			else //status == 0x00002000
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<6))) //PA6 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				cur_button_status = button_release; //Low -> High
			}
			clear_bit = status & (3UL<<(6<<1));
		}
		else if(status & (3UL<<(7<<1))) //0x0000c000 PA7 : BT_UPDATE_DET(High) / Normal(Low)  //2022-10-12_4
		{
			shift_bit = 14;
			
#ifdef KEY_CHATTERING_ENABLE
			if(HAL_GPIO_ReadPin(PA) & (1<<6)) //PA7 is High and this says invalid value
			{
				if(!Power_State())
				{
					cur_button_status = button_release; //Falling Means : BT_UPDATE_DET On
					key = BT_UPDATE_KEY;
				}
				else
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
			}
			else
#endif
			{
				shift_bit = 0xffffffff;
				key = NONE_KEY;
			}			
			
			clear_bit = status & (3UL<<(7<<1));
		}
		else
			shift_bit = 0xffffffff; //Invalid

		HAL_GPIO_EXTI_ClearPin(PA, status&clear_bit);

		if(shift_bit != 0xffffffff)
		{
			if(cur_button_status == button_push) //Rising Edge 0x10
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\r2. Ready Button key = ");
				_DBH(key);
#endif
			}
			else //cur_button_status = button_release //Falling Edge 0x01
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG				
				_DBG("\n\r2. Release Button key = ");
				_DBH(key);
#endif
				if(key != NONE_KEY)
				{
					if(key == key_bk)
					{
						if((filtering_time - filtering_time_old) <= KEY_FILTERING_TIME)
						{													
							if((filtering_time - filtering_time_very_old) <= KEY_FILTERING_TIME)
							{
								ret = -1;
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
								_DBG("\n\rTime = ");_DBH32(filtering_time_very_old);_DBG("/");_DBH32(filtering_time_old);_DBG("/");_DBH32(filtering_time);
#endif
							}
							else
								ret = 0;

							filtering_time_very_old = filtering_time_old;
						}
					}
						
					filtering_time_old = filtering_time;
					key_bk = key;

					if(ret != -1)
					{
						Send_Remote_Key_Event(key);
					}
				}
			}
		}
	}
}

/**********************************************************************
 * @brief		EXTI_PortA_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void EXTI_PortA_Configure(void)
{	
  	/* external interrupt clock setting*/
	HAL_SCU_MiscClockConfig(4,PD0_TYPE,CLKSRC_LSI,100);

	HAL_GPIO_EXTI_Config(PA, 6, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 7, IER_EDGE, ICR_BOTH_EDGE_INT);

	NVIC_SetPriority(GPIOAB_IRQn, 3);	
	NVIC_EnableIRQ(GPIOAB_IRQn);
}

void GPIOE_IRQHandler_IT(void) // PE7 : Button Switch Input
{
	uint32_t status1 = 0, clear_bit = 0, shift_bit = 0;
	uint8_t key = 0;
	button_status cur_button_status;

	static uint8_t key_bk1 = 0;
	static uint32_t filtering_time1 = 0, filtering_time_old1 = 0, filtering_time_very_old1 = 0;

	filtering_time1 = TIMER20_100ms_Count_Value();

	status1 = HAL_GPIO_EXTI_GetState(PE);
	
#ifdef GPIO_DEBUG_MSG
	_DBG("\n\rstatus1 = ");
	_DBH32(status1);
#endif

	if(!Power_State())
	{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
		_DBG("\n\r4. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
		HAL_GPIO_EXTI_ClearPin(PE, status1);

		return;
	}

	if (status1 & ((3UL<<(7<<1)) | (3UL<<(0<<1)))) //0x0000c000 Just check PE7 or PE0 of PE //PE7 BT_KEY, PE0 BT_OUT_OFF
	{
		shift_bit = 0xffffffff;

#ifdef KEY_CHATTERING_ENABLE
		delay_ms(KEY_CHATTERING_DELAY_MS);
#endif

		if(status1 & (3UL<<(0<<1))) //0x00000003 PE0 : BT OUT OFF / BT OUT ON
		{
			shift_bit = 1;

			if(status1 & 0x00000001) //Falling Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PE) & (1<<0)) //PE0 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = BT_OUT_OFF_KEY;
					cur_button_status = button_release;
				}
			}
			else //0x00000002 Rising Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PE) & (1<<0))) //PE0 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = BT_OUT_ON_KEY;
					cur_button_status = button_release; //Low -> High
				}
			}

			clear_bit = status1 & (3UL<<(0<<1));
		}
		else if(status1 & (3UL<<(1<<1))) //0x0000000c PE1 : DIR Interrupt
		{
			clear_bit = status1 & (3UL<<(1<<1));
		}
		else if(status1 & (3UL<<(7<<1))) //0x0000c000 PE7 : BT_KEY(Long - 5Sec)
		{
			shift_bit = 4;

			key = BT_KEY;//BT Shor Key is available with this line

			if(status1 & 0x00004000) //Falling Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PE) & (1<<7)) //PE7 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					if(!Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
					{
						TIMER13_Periodic_Mode_Run(TRUE, Timer13_BT_Pairing_Key); //BT Key is implemented timer 13 interrupt routine
					}
					
					cur_button_status = button_push; //High -> Low
				}
			}
			else //status == 0x00008000 //Rising Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PE) & (1<<7))) //PE7 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = BT_KEY; //Need to delete timer when short key is input
					cur_button_status = button_release; //Low -> High
				}
			}

			clear_bit = status1 & (3UL<<(7<<1));
		}
		else
			shift_bit = 0xffffffff; //Invalid

		HAL_GPIO_EXTI_ClearPin(PE, status1&clear_bit);

		if(shift_bit != 0xffffffff)
		{
			if(cur_button_status == button_push) //Rising Edge 0x10
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\r4. Ready Button key = ");
				_DBH(key);
#endif
			}
			else //cur_button_status = button_release //Falling Edge 0x01
			{
				switch(key)
				{
					case BT_KEY: //BT Long Key / Short Key Action
						if(Is_BT_Long_Key()) //Long key is sent on Timer function so we don't need to send key again.
							key = NONE_KEY;
						
						TIMER13_Periodic_Mode_Run(FALSE, Timer13_BT_Pairing_Key);
					break;
						
					default:
						break;
				}

#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG				
				_DBG("\n\r4. Release Button key = ");
				_DBH(key);
#endif
				if(key != NONE_KEY)
				{
					int ret = 0;
					
					if(key == key_bk1)
					{
						if((filtering_time1 - filtering_time_old1) <= KEY_FILTERING_TIME)
						{													
							if((filtering_time1 - filtering_time_very_old1) <= KEY_FILTERING_TIME)
							{
								ret = -1;
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
								_DBG("\n\rTime = ");_DBH32(filtering_time_very_old1);_DBG("/");_DBH32(filtering_time_old1);_DBG("/");_DBH32(filtering_time1);
#endif
							}
							else
								ret = 0;

							filtering_time_very_old1 = filtering_time_old1;
						}
					}
						
					filtering_time_old1 = filtering_time1;
					key_bk1 = key;

					if(ret != -1)
					{
						Send_Remote_Key_Event(key);
					}
				}
			}
		}
	}
}

/**********************************************************************
 * @brief		EXIT_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void EXIT_PortE_Configure(void) //Implemented Interrupt Port E for BT Key(PE7) //2022-10-11_2
{
	HAL_GPIO_EXTI_Config(PE, 0, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 1, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 7, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_SCU_LVRCmd(DISABLE);
	
	NVIC_SetPriority(GPIOE_IRQn, 3);
	NVIC_EnableIRQ(GPIOE_IRQn);
	
	//HAL_SCU_WakeUpSRCCmd(WAKEUP_GPIOE, ENABLE);		
}

void EXIT_PortE_Disable(void) //2023-01-03_2 : Disable Interrupt Port E for BT Key(PE7)
{
	HAL_GPIO_EXTI_Config(PE, 0, IER_DISABLE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 1, IER_DISABLE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 7, IER_DISABLE, ICR_BOTH_EDGE_INT);
}

void GPIOF_IRQHandler_IT(void)
{
	uint32_t status, clear_bit = 0, shift_bit = 0;
	uint32_t status_buf = 0;
	uint8_t key = NONE_KEY;	
	button_status cur_button_status;
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
	static uint32_t status_bk = 0;
#endif

	status = HAL_GPIO_EXTI_GetState(PF);
	
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
	if(status_bk != status)
	{	
		status_bk = status;
		_DBG("\n\rstatus : ");
		_DBH32(status);
	}
#endif

	if (status & ((3UL<<(0<<1)) | (3UL<<(5<<1)) | (3UL<<(1<<1)) | (3UL<<(2<<1)) | (3UL<<(3<<1)))) //PF0, PF1, PF2, PF3, PF5
	{
		shift_bit = 0xffffffff;
					
#ifdef KEY_CHATTERING_ENABLE
		delay_ms(KEY_CHATTERING_DELAY_MS);
#endif
		if(status & (3UL<<(0<<1))) //0x00000003 PF0 : FACTORY RESET Button
		{
			shift_bit = 0;

			if(Power_State()) //Factory Reset Key is only available under Power On
				TIMER13_Periodic_Mode_Run(TRUE, Timer13_Factory_Reset_Key); //FACTORY RESET LONG Key is implemented timer 13 interrupt routine

			key = FACTORY_RESET_KEY;
			
			//Both Rising Edge and Falling Edge should work as switch input. so, in both case, the cur_button_status should be button_release.
      
			//For recovery, When we use capacitor on FACTORY RESET Line, the Rising Edge value is always 0x03(0x02 is correct) but Falling Edge is always 0x01
			status_buf = status & 0x00000003;

			if(status_buf == 0x00000001) //Falling Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PF) & (1<<0)) //PF0 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_push; //High -> Low
				}
			}
			else //status == 0x00000002 //Rising Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PF) & (1<<0))) //PF0 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //Low -> High
				}
			}

			if(!Power_State() && key == FACTORY_RESET_KEY) //FACTORY_RESET_KEY is invalid under Power off
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\rFACTORY_RESET_KEY is invalid under Power off ~~~ ");
				_DBH(key);
#endif
				shift_bit = 0xffffffff;
				key = NONE_KEY;
			}

			clear_bit = status & (3UL<<(0<<1));

			HAL_GPIO_EXTI_ClearPin(PF, status&clear_bit);
		}
		else if(status & (3UL<<(1<<1))) //0x0000000C PF1 : BT_OUT_AREA1
		{
			shift_bit = 1;

			status_buf = status & 0x0000000C;

			if(status_buf == 0x00000004) //Falling Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PF) & (1<<1)) //PF1 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = BT_OUT_AREA_1_KEY;
					cur_button_status = button_release; //High -> Low
				}
			}
			else //status == 0x00000008 //Rising Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PF) & (1<<1))) //PF1 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = NONE_KEY;
					cur_button_status = button_release; //Low -> High
				}
			}

			clear_bit = status & (3UL<<(1<<1));

			HAL_GPIO_EXTI_ClearPin(PF, status&clear_bit);
		}
		else if(status & (3UL<<(2<<1))) //0x00000030 PF2 : BT_OUT_AREA2
		{
			shift_bit = 2;

			status_buf = status & 0x00000030;

			if(status_buf == 0x00000010) //Falling Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PF) & (1<<2)) //PF1 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = BT_OUT_AREA_2_KEY;
					cur_button_status = button_release; //High -> Low
				}
			}
			else //status == 0x00000020 //Rising Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PF) & (1<<2))) //PF1 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = NONE_KEY;
					cur_button_status = button_release; //Low -> High
				}
			}

			clear_bit = status & (3UL<<(2<<1));

			HAL_GPIO_EXTI_ClearPin(PF, status&clear_bit);
		}
		else if(status & (3UL<<(3<<1))) //0x00000030 PF3 : BT_OUT_AREA1+2
		{
			shift_bit = 3;

			status_buf = status & 0x000000C0;

			if(status_buf == 0x00000040) //Falling Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PF) & (1<<3)) //PF1 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = BT_OUT_AREA_1_2_KEY;
					cur_button_status = button_release; //High -> Low
				}
			}
			else //status == 0x00000080 //Rising Edge
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PF) & (1<<3))) //PF1 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					key = NONE_KEY;
					cur_button_status = button_release; //Low -> High
				}
			}

			clear_bit = status & (3UL<<(3<<1));

			HAL_GPIO_EXTI_ClearPin(PF, status&clear_bit);
		}

		if(shift_bit != 0xffffffff)
		{
			if(cur_button_status == button_push) //Rising Edge 0x02
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\rReady Button key = ");
				_DBH(key);
#endif
			}
			else //cur_button_status = button_release //Falling Edge 0x01
			{
				if(key == FACTORY_RESET_KEY)
				{
					TIMER13_Periodic_Mode_Run(FALSE, Timer13_BT_Pairing_Key);
					key = NONE_KEY; //Short key has no meaning for Factory Reset Key
				}

				if(key != NONE_KEY)
					Send_Remote_Key_Event(key);
			}
		}

		if(status & (3UL<<(5<<1))) //0x00000c00 PF5 - DAMP_ERROR 
		{
#ifdef KEY_CHATTERING_ENABLE
			delay_ms(KEY_CHATTERING_DELAY_MS);
#endif
			clear_bit = status & (3UL<<(5<<1));//0x00000c00;
			HAL_GPIO_EXTI_ClearPin(PF, status&clear_bit);

			if(HAL_GPIO_ReadPin(PF) & (1<<5)) //PF5 - High
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\rDAMP_ERROR - CLEAR");
#endif
			}
			else
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\rDAMP_ERROR - ERROR");
#endif
#ifdef SOC_ERROR_ALARM_DEBUG_MSG
				_DBG("\n\rSOC_ERROR - 6");
#endif
				if(!Is_BAmp_Init())
				{
					if(AD85050_Amp_Detect_Fault(FALSE) == 0xFF)
					{
#ifdef AD85050_DEBUG_MSG
						_DBG("\n\rDAMP_ERROR - Recovery");
#endif
						TIMER20_Amp_access_error_flag_Start();
					}
				}
#ifdef AD85050_DEBUG_MSG
				else
				{
					_DBG("\n\r+++ Is_BAmp_Init is TRUE - 1");
				}
#endif
			}
		}
	}
}

void EXIT_PortF_Configure(void)
{
	HAL_GPIO_EXTI_Config(PF, 0, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PF, 1, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PF, 2, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PF, 3, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PF, 5, IER_EDGE, ICR_BOTH_EDGE_INT); //Added AMP error

	NVIC_SetPriority(GPIOF_IRQn, 3);	
	NVIC_EnableIRQ(GPIOF_IRQn);
}

/**********************************************************************
 * @brief		GPIO_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void GPIO_Configure(void)
{
#if 1 //USEN_BAP2 GPIO
	/* I2C0 PA1:SCL1, PA0:SDA1 */
	HAL_GPIO_ConfigOutput(PA, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 0, FUNC1);
	HAL_GPIO_ConfigOutput(PA, 1, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 1, FUNC1);

	/* ADC pin PA2 : BT_VOL */
	HAL_GPIO_ConfigOutput(PA, 2, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 2, FUNC3);
	HAL_GPIO_ConfigPullup(PA, 2, DISPUPD);

	/* ADC pin PA3 : AREA1_VOL */
	HAL_GPIO_ConfigOutput(PA, 3, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 3, FUNC3);
	HAL_GPIO_ConfigPullup(PA, 3, DISPUPD);

	/* ADC pin PA4 : AREA2_VOL */
	HAL_GPIO_ConfigOutput(PA, 4, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 4, FUNC3);
	HAL_GPIO_ConfigPullup(PA, 4, DISPUPD);

	/* GPIO Output setting PA5 - +3.3V DAMP Power enable(High:On, Low:off) */
	HAL_GPIO_ConfigOutput(PA, 5, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PA, 5, DISPUPD);
	HAL_GPIO_ClearPin(PA, _BIT(5));

	/* external interrupt pin PA6 : Power on/off toggle(Low) / Normal(High) */
	HAL_GPIO_ConfigOutput(PA, 6, INPUT);
	HAL_GPIO_ConfigPullup(PA, 6, ENPU); 
	HAL_GPIO_ClearPin(PA, _BIT(6));

	/* external interrupt pin PA7 : BT_UPDATE_DET(High) / Normal(Low) */
	HAL_GPIO_ConfigOutput(PA, 7, INPUT);
	HAL_GPIO_ConfigPullup(PA, 7, ENPU); 
	HAL_GPIO_ClearPin(PA, _BIT(7));

	/* Initialize USART10 pin connect - TX10 : PB0 / RX10 : PB1 */
	HAL_GPIO_ConfigOutput(PB, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 0, FUNC1);

	HAL_GPIO_ConfigOutput(PB, 1, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 1, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 1, ENPU);

	/* DAMP_GPIO0 */
	HAL_GPIO_ConfigOutput(PB, 2, INPUT);
	HAL_GPIO_ConfigPullup(PB, 2, ENPU); 
	HAL_GPIO_ClearPin(PB, _BIT(2));

#ifdef _DEBUG_MSG
	HAL_GPIO_ConfigOutput(PB, 6, ALTERN_FUNC); //TX1
	HAL_GPIO_ConfigFunction(PB, 6, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 6, 1);

	HAL_GPIO_ConfigOutput(PB, 7, ALTERN_FUNC); //RX1
	HAL_GPIO_ConfigFunction(PB, 7, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 7, 1); 
#else
	HAL_GPIO_ConfigOutput(PB, 6, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 6, ENPD);
	HAL_GPIO_ClearPin(PB, _BIT(6));

	HAL_GPIO_ConfigOutput(PB, 7, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 7, ENPD);
	HAL_GPIO_ClearPin(PB, _BIT(7));
#endif

	/* PC0 Output - MODULE_RESET */
	HAL_GPIO_ConfigOutput(PC, 0, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PC, 0, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(0));

	/* PC1 Output - STATUS_LED_W1 */
	HAL_GPIO_ConfigOutput(PC, 1, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PC, 1, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(1));

	/* GPIO Output setting PC2 - Outlet enable(High:On, Low:off) */
	HAL_GPIO_ConfigOutput(PC, 2, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PC, 2, DISPUPD);
	HAL_GPIO_ClearPin(PC, _BIT(2));

	/* External interrupt pin PC3 Input select BT(Low) / Aux(High)*/
	HAL_GPIO_ConfigOutput(PC, 3, INPUT);
	HAL_GPIO_ConfigPullup(PC, 3, ENPU);
	HAL_GPIO_ClearPin(PC, _BIT(3));

	/* external interrupt pin PC4 : 8.5V short Protection - Low : short detection */
	HAL_GPIO_ConfigOutput(PC, 4, INPUT);
	HAL_GPIO_ConfigPullup(PC, 4, ENPU); 
	HAL_GPIO_ClearPin(PC, _BIT(4));

	/* external interrupt pin PD0 : AMP short Protection - Low : short detection */
	HAL_GPIO_ConfigOutput(PD, 0, INPUT);
	HAL_GPIO_ConfigPullup(PD, 0, ENPU); 
	HAL_GPIO_ClearPin(PD, _BIT(0));

	/* external interrupt pin PD1 : LED 3.3V short Protection - Low : short detection */
	HAL_GPIO_ConfigOutput(PD, 1, INPUT);
	HAL_GPIO_ConfigPullup(PD, 1, ENPU); 
	HAL_GPIO_ClearPin(PD, _BIT(1));    

	/* PD2 Output - BT_PAIRING_B */
	HAL_GPIO_ConfigOutput(PD, 2, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 2, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(2));

	/* PD3 Output - BT_PAIRING_W */
	HAL_GPIO_ConfigOutput(PD, 3, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 3, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(3));

	/* GPIO Output setting PD4 - +24V_DAMP_SW */
	HAL_GPIO_ConfigOutput(PD, 4, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 4, DISPUPD);
	HAL_GPIO_ClearPin(PD, _BIT(4));

	/* GPIO Output setting PD5 - SW_+3.3V_SW(LED Power Control) */
	HAL_GPIO_ConfigOutput(PD, 5, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 5, DISPUPD);
	HAL_GPIO_ClearPin(PD, _BIT(5));

	/* External interrupt pin PE0 BT_OUT off - Pin Low : BT_OUT off*/
	HAL_GPIO_ConfigOutput(PE, 0, INPUT);
	HAL_GPIO_ConfigPullup(PE, 0, ENPU);
	HAL_GPIO_ClearPin(PE, _BIT(0));

	/* External interrupt pin PE1 DIR INT(INT0/INT1) - Pin Low : Interrupt*/
	HAL_GPIO_ConfigOutput(PE, 1, INPUT);
	HAL_GPIO_ConfigPullup(PE, 1, ENPU);
	HAL_GPIO_ClearPin(PE, _BIT(1));

	/* GPIO Output setting PE2 DIR RESET - Low : Reset / High : Normal*/
	HAL_GPIO_ConfigOutput(PE, 2, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PE, 2, DISPUPD);
	HAL_GPIO_ClearPin(PE, _BIT(2));

	/* GPIO Output setting pin PE3 BT_out4*/
	HAL_GPIO_ConfigOutput(PE, 3, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PE, 3, DISPUPD);
	HAL_GPIO_ClearPin(PE, _BIT(3));

	/* GPIO Output setting pin PE4 BT_out3*/
	HAL_GPIO_ConfigOutput(PE, 4, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PE, 4, DISPUPD);
	HAL_GPIO_ClearPin(PE, _BIT(4));

	/* GPIO Output setting pin PE5 BT_out2*/
	HAL_GPIO_ConfigOutput(PE, 5, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PE, 5, DISPUPD);
	HAL_GPIO_ClearPin(PE, _BIT(5));

	/* GPIO Output setting pin PE6 BT_out1*/
	HAL_GPIO_ConfigOutput(PE, 6, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PE, 6, DISPUPD);
	HAL_GPIO_ClearPin(PE, _BIT(6));

	/* External interrupt pin PE7 : BT KEY */
	HAL_GPIO_ConfigOutput(PE, 7, INPUT);
	HAL_GPIO_ConfigPullup(PE, 7, ENPD);
	HAL_GPIO_ClearPin(PE, _BIT(7));

	/* external interrupt pin PF0 : FACTORY RESET Button */
	HAL_GPIO_ConfigOutput(PF, 0, INPUT);
	HAL_GPIO_ConfigPullup(PF, 0, ENPU); 
	HAL_GPIO_ClearPin(PF, _BIT(0));

	/* external interrupt pin PF1 : BT_Out area1 - Low : BT_Out_Area1*/
	HAL_GPIO_ConfigOutput(PF, 1, INPUT);
	HAL_GPIO_ConfigPullup(PF, 1, ENPU); 
	HAL_GPIO_ClearPin(PF, _BIT(1));

	/* external interrupt pin PF2 : BT_Out area2 - Low : BT_Out_Area2*/
	HAL_GPIO_ConfigOutput(PF, 2, INPUT);
	HAL_GPIO_ConfigPullup(PF, 2, ENPU); 
	HAL_GPIO_ClearPin(PF, _BIT(2));

	/* external interrupt pin PF3 : BT_Out area1+2 - Low : BT_Out_Area1+2*/
	HAL_GPIO_ConfigOutput(PF, 3, INPUT);
	HAL_GPIO_ConfigPullup(PF, 3, ENPU); 
	HAL_GPIO_ClearPin(PF, _BIT(3));

	/* GPIO Output setting PF4 - AMP_SDB_CONT, SD Enabled : Low / SD Disabled : High */
	HAL_GPIO_ConfigOutput(PF, 4, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 4, DISPUPD);
	HAL_GPIO_ClearPin(PF, _BIT(4));

	/* External interrupt pin PF5 - DAMP_ERROR */
	HAL_GPIO_ConfigOutput(PF, 5, INPUT);
	HAL_GPIO_ConfigPullup(PF, 5, ENPU);
	HAL_GPIO_ClearPin(PF, _BIT(5));

	/* I2C0 PF6:SCL0, PF7:SDA0 */
	HAL_GPIO_ConfigOutput(PF, 6, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PF, 6, FUNC2);
	HAL_GPIO_ConfigOutput(PF, 7, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PF, 7, FUNC2);
#else
    int i;
  
    for(i=2;i<4;i++) //PB 2 ~ 3
    {
      HAL_GPIO_ConfigOutput(PB, i, PUSH_PULL_OUTPUT);
      HAL_GPIO_ConfigPullup(PB, i, ENPD);
      HAL_GPIO_ClearPin(PB, _BIT(i));
    }
  
    for(i=6;i<8;i++) //PB 6 ~ 7
    {
      HAL_GPIO_ConfigOutput(PB, i, PUSH_PULL_OUTPUT);
      HAL_GPIO_ConfigPullup(PB, i, ENPD);
      HAL_GPIO_ClearPin(PB, _BIT(i));
    }
  
    //PC 2
    i = 2;
    HAL_GPIO_ConfigOutput(PC, i, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PC, i, ENPD);
    HAL_GPIO_ClearPin(PC, _BIT(i));
  
    for(i=0;i<2;i++) //PD 0 ~ 1
    {
      HAL_GPIO_ConfigOutput(PD, i, PUSH_PULL_OUTPUT);
      HAL_GPIO_ConfigPullup(PD, i, ENPD);
      HAL_GPIO_ClearPin(PD, _BIT(i));
    }
  
    for(i=0;i<7;i++) //PE 0 ~ 6
    {
      HAL_GPIO_ConfigOutput(PE, i, PUSH_PULL_OUTPUT);
      HAL_GPIO_ConfigPullup(PE, i, ENPD);
      HAL_GPIO_ClearPin(PE, _BIT(i));
    }
  
    for(i=1;i<4;i++) //PF 1 ~ 3
    {
      HAL_GPIO_ConfigOutput(PF, i, PUSH_PULL_OUTPUT);
      HAL_GPIO_ConfigPullup(PF, i, ENPD);
      HAL_GPIO_ClearPin(PF, _BIT(i));
    }

    /* external interrupt pin PA0 : AUTO_SW */
    HAL_GPIO_ConfigOutput(PA, 0, INPUT);
    HAL_GPIO_ConfigPullup(PA, 0, ENPU); 
    HAL_GPIO_ClearPin(PA, _BIT(0));
  
    /* external interrupt pin PA1 : M/S_SWITCH_1 */
    HAL_GPIO_ConfigOutput(PA, 1, INPUT);
    HAL_GPIO_ConfigPullup(PA, 1, ENPU); 
    HAL_GPIO_ClearPin(PA, _BIT(1));
  
    /* external interrupt pin PA6 : POWER_Off(short)/POWER_ON(Long) */
    HAL_GPIO_ConfigOutput(PA, 6, INPUT);
    HAL_GPIO_ConfigPullup(PA, 6, ENPU); 
    HAL_GPIO_ClearPin(PA, _BIT(6));
  
    //To Do !!! PA4 : LOW_VOL_DETECT
    /* external interrupt pin PA7 : BT_UPDATE_DET(High) / Normal(Low) */ //2022-10-12_4
    HAL_GPIO_ConfigOutput(PA, 7, INPUT);
    HAL_GPIO_ConfigPullup(PA, 7, ENPU); 
    HAL_GPIO_ClearPin(PA, _BIT(7));
  
    /* External interrupt pin PE7 : BT KEY */ //Implemented Interrupt Port E for BT Key(PE7) //2022-10-11_2
    HAL_GPIO_ConfigOutput(PE, 7, INPUT);
    HAL_GPIO_ConfigPullup(PE, 7, ENPD); //2022-12-08 : Pull-Down Setting and controlled by externall Pull-up
    HAL_GPIO_ClearPin(PE, _BIT(7));
  
    /* ADC pin PA2 : BSP_VOL_A/D(Attenuator Volume) */
    HAL_GPIO_ConfigOutput(PA, 2, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PA, 2, FUNC3);
    HAL_GPIO_ConfigPullup(PA, 2, DISPUPD);
  
    /* ADC pin PA3 : VOL_CONT(Master Volume) */
    HAL_GPIO_ConfigOutput(PA, 3, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PA, 3, FUNC3);
    HAL_GPIO_ConfigPullup(PA, 3, DISPUPD);
  
    /* GPIO Output setting PA5 - +3.3V_DAMP_SW_1 */
    HAL_GPIO_ConfigOutput(PA, 5, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PA, 5, DISPUPD);
    HAL_GPIO_ClearPin(PA, _BIT(5));
  
    /* GPIO Output setting PC4 - +3.3V_SIG_SW */
    HAL_GPIO_ConfigOutput(PC, 4, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PC, 4, DISPUPD);
    HAL_GPIO_ClearPin(PC, _BIT(4));
  
    /* GPIO Output setting PD4 - +24V_DAMP_SW */
    HAL_GPIO_ConfigOutput(PD, 4, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PD, 4, DISPUPD);
    HAL_GPIO_ClearPin(PD, _BIT(4));
  
    /* GPIO Output setting PD5 - SW_+3.3V_SW(LED Power Control) */
    HAL_GPIO_ConfigOutput(PD, 5, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PD, 5, DISPUPD);
    HAL_GPIO_ClearPin(PD, _BIT(5));
  
    /* external interrupt pin PF0 : FACTORY RESET Button */
    HAL_GPIO_ConfigOutput(PF, 0, INPUT);
    HAL_GPIO_ConfigPullup(PF, 0, ENPU); 
    HAL_GPIO_ClearPin(PF, _BIT(0));
  
    /* setting PF4 - DAMP_PDN */
    HAL_GPIO_ConfigOutput(PF, 4, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PF, 4, DISPUPD);
    HAL_GPIO_ClearPin(PF, _BIT(4));
  
    /* External interrupt pin PF5 - DAMP_ERROR */
    HAL_GPIO_ConfigOutput(PF, 5, INPUT);
    HAL_GPIO_ConfigPullup(PF, 5, ENPU);
    HAL_GPIO_ClearPin(PF, _BIT(5));
  
    /* Initialize USART10 pin connect - TX10 : PB0 / RX10 : PB1 */
    HAL_GPIO_ConfigOutput(PB, 1, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PB, 1, FUNC1);
    HAL_GPIO_ConfigPullup(PB, 1, ENPU);
  
    HAL_GPIO_ConfigOutput(PB, 0, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PB, 0, FUNC1);
  
    /* PC0 Output - MODULE_RESET *///In referece flatrom, this port works as remote control port.
    HAL_GPIO_ConfigOutput(PC, 0, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PC, 0, DISPUPD);
    HAL_GPIO_SetPin(PC, _BIT(0));
    
    /* External interrupt pin PC3 */
    HAL_GPIO_ConfigOutput(PC, 3, INPUT);
    HAL_GPIO_ConfigPullup(PC, 3, ENPU);
    HAL_GPIO_ClearPin(PC, _BIT(3));
  
    /* PC1 Output - STATUS_LED_W1 */
    HAL_GPIO_ConfigOutput(PC, 1, PUSH_PULL_OUTPUT);
    HAL_GPIO_ConfigPullup(PC, 1, DISPUPD);
    HAL_GPIO_SetPin(PC, _BIT(1));
  
    /* PD2 Output - BT_PAIRING_B */
    HAL_GPIO_ConfigOutput(PD, 2, OPEN_DRAIN_OUTPUT);
    HAL_GPIO_ConfigPullup(PD, 2, DISPUPD);
    HAL_GPIO_SetPin(PD, _BIT(2));
  
    /* PD3 Output - BT_PAIRING_W */
    HAL_GPIO_ConfigOutput(PD, 3, OPEN_DRAIN_OUTPUT);
    HAL_GPIO_ConfigPullup(PD, 3, DISPUPD);
    HAL_GPIO_SetPin(PD, _BIT(3));

#ifdef _DEBUG_MSG //2023-05-12_1 : #ifndef _DEBUG_MSG //If we don't use DEBUG_MSG, we need to set some GPIO like below. Becasue these GPIOs can avoid USART10 UART error. But we don't know why.
    HAL_GPIO_ConfigOutput(PB, 7, ALTERN_FUNC); //RX1
    HAL_GPIO_ConfigFunction(PB, 7, FUNC1);
    HAL_GPIO_ConfigPullup(PB, 7, 1); 
  
    HAL_GPIO_ConfigOutput(PB, 6, ALTERN_FUNC); //TX1
    HAL_GPIO_ConfigFunction(PB, 6, FUNC1);
    HAL_GPIO_ConfigPullup(PB, 6, 1);
#endif

/*
    delay_ms(20);
    HAL_GPIO_SetPin(PA, _BIT(5)); //+3.3V_DAMP_SW_1
    delay_ms(20);
    HAL_GPIO_SetPin(PD, _BIT(4)); //+24V_DAMP_SW
    delay_ms(20);
    HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
*/
    /* GPIO Output setting PC4 - +3.3V_SIG_SW */
    HAL_GPIO_SetPin(PC, _BIT(4));
  
    /* GPIO Output setting PD5 - SW_+3.3V_SW(LED Power Control) */
    HAL_GPIO_SetPin(PD, _BIT(5));

    /* I2C0 PF6:SCL0, PF7:SDA0 */
    HAL_GPIO_ConfigOutput(PF, 6, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PF, 6, FUNC2);
    HAL_GPIO_ConfigOutput(PF, 7, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PF, 7, FUNC2);

#ifdef I2C_1_ENABLE
    /* I2C0 PA1:SCL1, PA0:SDA1 */
    HAL_GPIO_ConfigOutput(PA, 0, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PA, 0, FUNC1);
    HAL_GPIO_ConfigOutput(PA, 1, ALTERN_FUNC);
    HAL_GPIO_ConfigFunction(PA, 1, FUNC1);
#endif //I2C_1_ENABLE
#endif
}

/**********************************************************************
 * @brief		DEBUG_Init
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void DEBUG_Init(void)
{
	#ifdef _DEBUG_MSG
	debug_frmwrk_init();
	#endif
}

/**********************************************************************
 * @brief		menu Print
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void DEBUG_MenuPrint(void)
{
	#ifdef _DEBUG_MSG
	_DBG(menu);
	#endif
}

#ifdef  USE_FULL_ASSERT
/**********************************************************************
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
 **********************************************************************/
void check_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

   /* Infinite loop */
   while (1)
   {
   }
}
#endif

static void Serial_Get_Data_Intterupt_Callback(uint8_t *Data)
{	
	if(uBuffer_Count >= UART10_Rx_Buffer_Size)
		uBuffer_Count = 0;

	MB3021_BT_Module_Get_Auto_Response_Packet(uBuffer_Count, Data); //Type 0x02(Response)

	//Save data to buffer
	UART10_Rx_Buffer[uBuffer_Count++] = *Data;
}

void Serial_Data_Get(uint8_t *Buf, uint8_t length, uint8_t start_count)
{
	uint8_t i;

#if 0 //def _UART_DEBUG_MSG
	_DBG("\n\rstart_count : ");
	_DBD(start_count);
	_DBG("\n\rlength : ");
	_DBD(length);
	_DBG("\n\rGet Data : ");
	
	for(i=0;i<length;i++)
	{
		if(start_count+i >= UART10_Rx_Buffer_Size)
			Buf[i] = UART10_Rx_Buffer[start_count+i-UART10_Rx_Buffer_Size];
		else		
			Buf[i] = UART10_Rx_Buffer[start_count+i];
	
		_DBH(Buf[i]);
	}
	
	_DBG("\n\rGet Data : Exit!\n\r");
#else
	for(i=0;i<length;i++)
	{
		if(start_count+i >= UART10_Rx_Buffer_Size)
			Buf[i] = UART10_Rx_Buffer[start_count+i-UART10_Rx_Buffer_Size];
		else		
			Buf[i] = UART10_Rx_Buffer[start_count+i];

	}
#endif
}

void Serial_Data_Clear(uint8_t length, uint8_t start_count)
{
	uint16_t i, count =0;

	for(i=0;i<length;i++)
	{
		count = start_count+i;
		if(count >= UART10_Rx_Buffer_Size)
		{
#if 0//def _UART_DEBUG_MSG
			_DBG("\n\rClear Buf = ");
			_DBD(start_count+i);
#endif
			UART10_Rx_Buffer[count-255] = 0;	
		}
		else	
		{
#if 0//def _UART_DEBUG_MSG
			_DBG("\n\rClear Buf = ");
			_DBD(start_count+i);
#endif
			UART10_Rx_Buffer[count] = 0;
		}
	}
}

#ifdef _UART_DEBUG_MSG
void Display_UART_Receive_Data(void)
{
	uint8_t i;

	_DBG("\n\rGet UART Data1 : ");
	for(i=0;i<UART10_Rx_Buffer_Size;i++)
	{
		_DBH(UART10_Rx_Buffer[i]);
	}
	_DBG("\n\rGet Data1 : Exit!\n\r");
}
#endif //_BT_MODULE_DEBUG_MSG

void delay_ms(uint32_t m_ms)
{
	uint32_t i, j;

	for (i=0; i<m_ms; i++)
	{
		for (j=0; j<2070; j++)
		{
			__nop();
		}
	}
}

uint8_t Convert_ADC_To_Attenuator(uint32_t ADC_Value)
{
	uint8_t uConvert_Value = 0;

#ifdef ADC_INPUT_DEBUG_MSG
	_DBG("\n\r +++ Convert_ADC_To_Attenuator : ");
	_DBD32(ADC_Value);
#endif

	switch(ADC_Value)
	{
		case 0:
		uConvert_Value = Attenuator_Volume_MIN;	//-20dB
		break;
		case 1:
		uConvert_Value = Attenuator_Volume_1; 	//-19dB
		break;
		case 2:
		uConvert_Value = Attenuator_Volume_2; 	//-18dB
		break;
		case 3:
		uConvert_Value = Attenuator_Volume_3; 	//-17dB
		break;
		case 4:
		uConvert_Value = Attenuator_Volume_4; 	//-16dB
		break;
		case 5:
		uConvert_Value = Attenuator_Volume_5; 	//-15dB
		break;
		case 6:
		uConvert_Value = Attenuator_Volume_6; 	//-14dB
		break;
		case 7:
		uConvert_Value = Attenuator_Volume_7;	//-13dB
		break;
		case 8:
		uConvert_Value = Attenuator_Volume_8; 	//-12dB
		break;
		case 9:
		uConvert_Value = Attenuator_Volume_9; 	//-11dB
		break;
		case 10:
		uConvert_Value = Attenuator_Volume_10;	//-10dB
		break;
		case 11:
		uConvert_Value = Attenuator_Volume_11;	//-9dB
		break;
		case 12:
		uConvert_Value = Attenuator_Volume_12;	//-8dB
		break;
		case 13:
		uConvert_Value = Attenuator_Volume_13;	//-7dB
		break;
		case 14:
		uConvert_Value = Attenuator_Volume_14;	//-6dB
		break;
		case 15:
		uConvert_Value = Attenuator_Volume_15;	//-5dB
		break;
		case 16:
		uConvert_Value = Attenuator_Volume_16;	//-4dB
		break;
		case 17:
		uConvert_Value = Attenuator_Volume_17;	//-3dB
		break;
		case 18:
		uConvert_Value = Attenuator_Volume_18;	//-2dB
		break;
		case 19:
		uConvert_Value = Attenuator_Volume_19;	//-1dB
		break;
		case 20:
		uConvert_Value = Attenuator_Volume_MAX;	//-0dB
		break;
		default:
		uConvert_Value = uAttenuator_Vol; //To avoid noise, keep previous value
		break;
	}
	
	return uConvert_Value;
}

uint8_t ADC_Volume_Attenuator_Value_Init(Attenuator_Type attenuator_type)
{
  uint8_t uCurVolLevel = 0;
  static uint32_t ADC_Value = 0xffffffff;
  int i;
  uint8_t ADC_Level_Min, ADC_Level_Max;
  
  if(attenuator_type == AREA1_VOLUME)
    ADC_Value = ADC_PollingRun(3);
  else if(attenuator_type == AREA2_VOLUME)
    ADC_Value = ADC_PollingRun(4);
  else if(attenuator_type == SLAVE_BT_VOLUME)
    ADC_Value = ADC_PollingRun(2);  
  
#ifdef ADC_INPUT_DEBUG_MSG
    _DBG("\n\r === Master Volume ADC = 0x");
    _DBH32(ADC_Value);
#endif

  for(i=1;i<52;i++)
  {
    if(i==1)
      ADC_Level_Min = 0;
    else
      ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

    if(i==51)
      ADC_Level_Max = 255;
    else
      ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253

    if((ADC_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC_Value))
    {
      uCurVolLevel = 51 - i;

#ifdef ADC_INPUT_DEBUG_MSG
      _DBG("\n\r === ADC Level = ");
      _DBD(i);
#endif
      break;
    }
  }

  uCurVolLevel = 51 - i;
  
  return uCurVolLevel;
}

