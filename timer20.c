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

#ifdef TIMER20_COUNTER_ENABLE
#include "timer20.h"

#ifdef WATCHDOG_TIMER_RESET
#include "A31G21x_hal_wdt.h"
#endif
#include "AD85050.h"
#include "led_display.h"
#include "remocon_action.h"
#include "key.h"
#include "bt_MB3021.h"

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/
//#define TIMER20_DEBUG_MSG					(1)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
TIMER2n_PERIODICCFG_Type TIMER20_Config;
static uint32_t timer20_100ms_count = 0, timer20_500ms_count = 0, timer20_1s_count = 0;
int32_t mute_flag = 0, auto_power_flag = 0;
int32_t aux_setting_flag = 0; //To avoid, Aux audio output NG
uint32_t user_eq_mute_flag = 0; //300ms mute

int32_t factory_reset_led_display_flag = 0;
int32_t factory_reset_cmd_recovery_flag = 0;

int32_t master_slave_grouping_flag = 0;
int32_t master_slave_Grouping_cmd_recovery_flag = 0;

int32_t amp_error_flag = 0;
int32_t amp_access_error_flag = 0; //2023-04-07_2 : To recovery TAS5806MD_Amp_Detect_Fault() function
int32_t amp_error_no_display_flag = 0; //2023-06-30_1 : Excepting the errors with LED error display, we need to recovery from error mode to normal mode.

factory_reset_recovery_cmd recovery_cmd;
#ifdef MASTER_SLAVE_GROUPING
uint8_t recovery_general_cmd;
#endif
int32_t aux_detect_check_flag = 0;
int32_t aux_detecttion_flag = 0;

int32_t drc_eq_set_recovery_flag = 0;

int32_t power_on_volume_sync_flag = 0;

#if defined(USEN_TI_AMP_EQ_ENABLE) || defined(AD85050_ENABLE) //2023-05-09_2
void TIMER20_drc_eq_set_flag_start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_drc_eq_set_flag_start() !!!");
#endif
	drc_eq_set_recovery_flag = 1;
}

void TIMER20_drc_eq_set_flag_stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_drc_eq_set_flag_stop() !!!");
#endif
	drc_eq_set_recovery_flag = 0;
}
#endif

#ifdef TIMER20_DEBUG_MSG
void TIMER20_Display_Flag(void)
{
	static int32_t mute_flag_bk = 0, auto_power_flag_bk = 0, user_eq_mute_flag_bk = 0, soc_error_flag_bk = 0, uart_error_flag_bk = 0, Slave_auto_power_off_flag_bk =0;
	static int32_t auto_volume_led_off_flag_bk = 0, factory_reset_led_display_flag_bk =0, factory_reset_cmd_recovery_flag_bk=0, master_slave_grouping_flag_bk=0;
	static int32_t master_slave_Grouping_cmd_recovery_flag_bk =0, aux_setting_flag_bk=0;
#ifdef AMP_ERROR_ALARM
	static int32_t amp_error_flag_bk = 0;
#endif
		
	//_DBG("\n\rTIMER20_Display_Flag");
	if(mute_flag != mute_flag_bk)
	{
		_DBG("\n\r^^^^^  mute_flag : ");_DBH32(mute_flag);
		mute_flag_bk = mute_flag;
	}
	if(auto_power_flag != auto_power_flag_bk)
	{
		_DBG("\n\r^^^^^ auto_power_flag : ");_DBH32(auto_power_flag);
		auto_power_flag_bk = auto_power_flag;
	}
#ifdef FIVE_USER_EQ_ENABLE
	if(user_eq_mute_flag != user_eq_mute_flag_bk)
	{
		_DBG("\n\r^^^^^ auto_power_flag : ");_DBH32(user_eq_mute_flag);
		user_eq_mute_flag_bk = user_eq_mute_flag;
	}
#endif
#ifdef SOC_ERROR_ALARM
	if(soc_error_flag_bk != soc_error_flag)
	{
		_DBG("\n\r^^^^^ soc_error_flag : ");_DBH32(soc_error_flag);
		soc_error_flag_bk = soc_error_flag;
	}
	
	if(uart_error_flag_bk != uart_error_flag)
	{
		_DBG("\n\r^^^^^ uart_error_flag : ");_DBH32(uart_error_flag);
		uart_error_flag_bk= uart_error_flag;
	}
#endif
#ifdef AMP_ERROR_ALARM
	if(amp_error_flag_bk != amp_error_flag)
	{
		_DBG("\n\r^^^^^ amp_error_flag : ");_DBH32(amp_error_flag);
		amp_error_flag_bk= amp_error_flag;
	}
#endif

#ifdef SLAVE_AUTO_OFF_ENABLE
	if(Slave_auto_power_off_flag_bk != Slave_auto_power_off_flag)
	{
		_DBG("\n\r^^^^^ Slave_auto_power_off_flag : ");_DBH32(Slave_auto_power_off_flag);
		Slave_auto_power_off_flag_bk = Slave_auto_power_off_flag;
	}
#endif
#ifdef AUTO_VOLUME_LED_OFF
	if(auto_volume_led_off_flag_bk != auto_volume_led_off_flag)
	{
		_DBG("\n\r^^^^^ auto_volume_led_off_flag : ");_DBH32(auto_volume_led_off_flag);
		auto_volume_led_off_flag_bk = auto_volume_led_off_flag;
	}
#endif
#ifdef FACTORY_RESET_LED_DISPLAY
	if(factory_reset_led_display_flag_bk != factory_reset_led_display_flag)
	{
		_DBG("\n\r^^^^^ factory_reset_led_display_flag : ");_DBH32(factory_reset_led_display_flag);
		factory_reset_led_display_flag_bk = factory_reset_led_display_flag;
	}
#endif
	if(factory_reset_cmd_recovery_flag_bk != factory_reset_cmd_recovery_flag)
	{
		_DBG("\n\r^^^^^ factory_reset_cmd_recovery_flag : ");_DBH32(factory_reset_cmd_recovery_flag);
		factory_reset_cmd_recovery_flag_bk = factory_reset_cmd_recovery_flag;
	}
#ifdef MASTER_SLAVE_GROUPING
	if(master_slave_grouping_flag_bk != master_slave_grouping_flag)
	{
		_DBG("\n\r^^^^^ master_slave_grouping_flag : ");_DBH32(master_slave_grouping_flag);
		master_slave_grouping_flag_bk = master_slave_grouping_flag;
	}

	if(master_slave_Grouping_cmd_recovery_flag_bk != master_slave_Grouping_cmd_recovery_flag)
	{
		_DBG("\n\r^^^^^ master_slave_Grouping_cmd_recovery_flag : ");_DBH32(master_slave_Grouping_cmd_recovery_flag);
		master_slave_Grouping_cmd_recovery_flag_bk = master_slave_Grouping_cmd_recovery_flag;
	}
#endif
	if(aux_setting_flag_bk != aux_setting_flag)
	{
		_DBG("\n\r^^^^^ aux_setting_flag : ");_DBH32(aux_setting_flag);
		aux_setting_flag_bk = aux_setting_flag;
	}
}
#endif

void TIMER20_power_on_volume_sync_flag_start(void) //2023-07-19_1 : To match volume sync with Slave on power-on under BAP-01 Master
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_power_on_volume_sync_flag_start() !!! ");
#endif
	power_on_volume_sync_flag = 1;
}
void TIMER20_power_on_volume_sync_flag_stop(void) //2023-07-19_1 : To match volume sync with Slave on power-on under BAP-01 Master
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_power_on_volume_sync_flag_stop() !!! ");
#endif
	power_on_volume_sync_flag = 0;
}

void TIMER20_aux_detection_flag_start(void) //To keep BT mode when Power On
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_aux_detection_flag_start() !!! ");
#endif

	aux_detecttion_flag = 1;
}

void TIMER20_aux_detection_flag_stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_aux_detecttion_flag_stop() !!! ");
#endif

	aux_detecttion_flag = 0;
}

void TIMER20_aux_detect_check_flag_start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_aux_detect_check_flag_start() !!! ");
#endif
	aux_detect_check_flag = 26; //2023-05-09_1 : Reduced the checking time from 5.3s to 2.6s //5; //2023-04-27_1 : Temparary SW Solution 500s check time to change Aux to BT.//41; //2023-04-12_4 : 4 Sec check time //51; //2023-02-21_9 : Reduced Aux detect check time from 20 sec(2023-01-10_3) to 5 sec(Total 10sec = HW 5sec + SW 5sec) 
	//aux_detect_check_flag = 201; //200 x 100ms timer = 20 Sec
}

void TIMER20_aux_detect_check_flag_stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_aux_detect_check_flag_stop() !!! ");
#endif
	aux_detect_check_flag = 0;
}

void TIMER20_Forced_Input_Audio_Path_Setting_flag_start(void) //To avoid, Aux audio output NG
{
#ifdef TIMER20_DEBUG_MSG
		_DBG("\n\rTIMER20_Forced_Aux_Setting_flag_start() !!! ");
#endif
	aux_setting_flag = 1;
}

void TIMER20_Forced_Input_Audio_Path_Setting_flag_stop(void) //To avoid, Aux audio output NG
{
#ifdef TIMER20_DEBUG_MSG
		_DBG("\n\rTIMER20_Forced_Aux_Setting_flag_stop() !!! ");
#endif
	aux_setting_flag = 0;
}

void TIMER20_user_eq_mute_flag_start(void) //To avoid noise when user tries eq mode change - 300ms mute
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_user_eq_mute_flag_start() !!! ");
#endif
	user_eq_mute_flag = 1;
}

void TIMER20_user_eq_mute_flag_stop(void) //To avoid noise when user tries eq mode change - 300ms mute
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_user_eq_mute_flag_stop() !!! ");
#endif
	user_eq_mute_flag = 0;	
}

void TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_recovery_cmd cmd) //To recovery when SPK sends Factory Reset Key
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_cmd_recovery_flag_start() !!! ");
#endif
	recovery_cmd = cmd;
	factory_reset_cmd_recovery_flag = 1;
}

void TIMER20_factory_reset_cmd_recovery_flag_stop(void) //To recovery when SPK sends Factory Reset Key
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_cmd_recovery_flag_stop() !!! ");
#endif
	factory_reset_cmd_recovery_flag = 0;	
}

void TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(uint8_t cmd) //To recovery when SPK sends Master_Slave_Grouping_cmd
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Master_Slave_Grouping_cmd_recovery_flag_start() !!! ");
#endif
	recovery_general_cmd = cmd;
	master_slave_Grouping_cmd_recovery_flag = 1;
}

void TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop(void) //To recovery when SPK sends Master_Slave_Grouping_cmd
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop() !!! ");
#endif
	master_slave_Grouping_cmd_recovery_flag = 0;
}

void TIMER20_Master_Slave_Grouping_flag_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Master_Slave_Grouping_flag_Start() !!! ");
#endif
	master_slave_grouping_flag = 1;
	Set_Status_LED_Mode(STATUS_BT_MASTER_SLAVE_PAIRING_MODE);
}

void TIMER20_Master_Slave_Grouping_flag_Stop(Bool Clear_Flag)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Master_Slave_Grouping_flag_Stop() !!! ");
#endif
	master_slave_grouping_flag = 0;

	if(!Clear_Flag)
	{
		MB3021_BT_Master_Slave_Grouping_Stop();
	}
}

Bool Get_master_slave_grouping_flag(void)
{
	Bool Ret;

	if(master_slave_grouping_flag)
		Ret = TRUE;
	else
		Ret = FALSE;

	return Ret;
}

void TIMER20_factory_reset_led_display_flag_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_factory_reset_led_display_flag_Start() !!! ");
#endif
	factory_reset_led_display_flag = 1;
}

void TIMER20_factory_reset_led_display_flag_Stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_factory_reset_led_display_flag_Stop() !!! ");
#endif
	factory_reset_led_display_flag = 0;
}

Bool Get_factory_reset_led_display_flag(void)
{
	Bool BRet;
	
	if(factory_reset_led_display_flag)
		BRet = TRUE;
	else
		BRet = FALSE;

	return BRet;
}

void TIMER20_Amp_access_error_flag_Start(void) //2023-04-07_1
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Amp_access_error_flag_Start() !!! ");
#endif
	amp_access_error_flag = 1;
}

void TIMER20_Amp_access_error_flag_Stop(void) //2023-04-07_1
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Amp_access_error_flag_Stop() !!! ");
#endif
	amp_access_error_flag = 0;
}

void TIMER20_Amp_error_no_diplay_flag_Start(void) //2023-06-30_1
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Amp_error_no_diplay_flag_Start() !!! ");
#endif
	amp_error_no_display_flag = 1;
}

void TIMER20_Amp_error_no_diplay_flag_Stop(void) //2023-06-30_1
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Amp_error_no_diplay_flag_Start() !!! ");
#endif
	amp_error_no_display_flag = 0;
}

Bool Get_Amp_error_flag(void)
{
	Bool BRet;
	
	if(amp_error_flag)
		BRet = TRUE;
	else
		BRet = FALSE;

	return BRet;
}

void TIMER20_Amp_error_flag_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Amp_error_flag_Start() !!! ");
#endif
	amp_error_flag = 1;
}

void TIMER20_Amp_error_flag_Stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Amp_error_flag_Stop() !!! ");
#endif
	amp_error_flag = 0;
}

void TIMER20_mute_flag_Start(void) //1.5sec delay of mute off to avoid pop-up noise
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_mute_flag_Start() !!! ");
#endif
	MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x01);
	mute_flag = 1;
}

void TIMER20_mute_flag_Stop(void) //1.5sec delay of mute off to avoid pop-up noise
{	
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_mute_flag_Stop() !!! ");
#endif
	mute_flag = 0;
}

int32_t Is_TIMER20_mute_flag_set(void) //status check of mute_flag (1.5sec delay of mute off)
{	
#ifdef AUTO_ONOFF_DEBUG_MSG
	_DBG("\n\rIs_TIMER20_mute_flag_set() !!! ");
#endif
	return mute_flag;
}

void TIMER20_auto_power_flag_Start(void) //This feature is only available under master mode
{
	if(Aux_In_Exist()) //we don't need to increase count under Aux mode. 2022-09-14
		return;

	auto_power_flag = 1;
}

void TIMER20_auto_power_flag_Stop()
{
#ifdef AUTO_ONOFF_DEBUG_MSG
	_DBG("\n\rTIMER20_auto_power_flag_Stop() !!! ");
#endif
	auto_power_flag = 0;
}

Bool Get_auto_power_flag(void)
{
	Bool Ret;

	if(auto_power_flag)
		Ret = TRUE;
	else
		Ret = FALSE;

	return Ret;
}

void TIMER20_Flag_init(void)
{
	TIMER20_factory_reset_led_display_flag_Stop();
	TIMER20_mute_flag_Stop();
}

int32_t TIMER20_1s_Count_Value(void)
{
	return timer20_1s_count;	
}

int32_t TIMER20_500ms_Count_Value(void)
{
	return timer20_500ms_count;	
}

int32_t TIMER20_100ms_Count_Value(void)
{
	return timer20_100ms_count;	
}

void TIMER20_Configure(void)
{	
	/*Timer20 clock source from PCLK*/
	HAL_SCU_Timer20_ClockConfig(TIMER20CLK_PCLK);
	TIMER20_Config.CkSel = TIMER2n_MCCR2PCLK;    
	TIMER20_Config.Prescaler = 32;    /* 32Mhz / 32 = 1Mhz ->1us*/
	TIMER20_Config.ADR = (1000*100); //100ms //(2500*200); //500ms
	TIMER20_Config.StartLevel = TIMER2n_START_LOW;   
	
	if(HAL_TIMER2n_Init(TIMER20, TIMER2n_PERIODIC_MODE, &TIMER20_Config) != HAL_OK)
	{
		/* Initialization Error */
    		//Error_Handler();
	}
	
	HAL_TIMER2n_ConfigInterrupt(TIMER20, TIMER2n_CR_MATCH_INTR, ENABLE);
	
	/* Enable Interrupt for TIMERx channel */
	NVIC_SetPriority(TIMER20_IRQn, 3);
	NVIC_EnableIRQ(TIMER20_IRQn); 
}


/**********************************************************************
 * @brief		TIMER20_OneShotRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void TIMER20_Periodic_Mode_Run(Bool On)
{
	static Bool On_bk = FALSE;

	if(On_bk == On) //Do not repeat same action with before
		return;
	
	On_bk = On;
	
  	/*timer start & clear*/
  	if(On)
  	{
  		timer20_100ms_count = 0;
  		timer20_500ms_count = 0;
		timer20_1s_count = 0;
		HAL_TIMER2n_Cmd(TIMER20, ENABLE);
  	}
	else
	{
		HAL_TIMER2n_Cmd(TIMER20, DISABLE);
		HAL_TIMER2n_ClearCounter(TIMER20);
		timer20_100ms_count = 0;
		timer20_500ms_count = 0;
		timer20_1s_count = 0;
	}
}

/**********************************************************************
 * @brief		TIMER20_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER20_IRQHandler_IT(void)
{
	uint32_t uVolume_Level = 0;
	Status_LED_Mode mode;

	if (HAL_TIMER2n_GetMatchInterrupt(TIMER20) == 1)
	{
		HAL_TIMER2n_ClearMatchInterrupt((TIMER20));
		
		if(!(timer20_100ms_count%5))
		{
			if(timer20_500ms_count%2)
			{
				timer20_1s_count++;
#ifdef TIMER20_DEBUG_MSG
				TIMER20_Display_Flag();
#endif
#ifdef WATCHDOG_TIMER_RESET
				WDT_ReloadTimeRun();
#endif
			}
#ifdef WATCHDOG_TIMER_RESET_DEBUG_MSG
				_DBG("\n\rStatus = ");_DBH32(HAL_WDT_GetStatus());
				_DBG("\n\rCount = ");_DBH32(HAL_WDT_GetCurrentCount());
#endif
			timer20_500ms_count++;
		}

 		if(aux_detect_check_flag)
		{
			aux_detect_check_flag--;
			
			if((aux_detect_check_flag%df100msTimer1s) == 0) //In every 1 sec
			{
#ifdef TIMER20_DEBUG_MSG
			_DBG("\n\rAux Detect Timer is working !!!");
#endif
				if(!(HAL_GPIO_ReadPin(PC) & (1<<3))) //Check if Aux Detect Pin is Low(Aux In)
				{
#ifdef TIMER20_DEBUG_MSG
					_DBG("\n\rAux Detect Timer : Detect Aux In !!!");
#endif
					Aux_Mode_Setting_After_Timer_Checking(TRUE);
					aux_detect_check_flag = 0;
				}
				else
				{
					if(aux_detect_check_flag == 0) //0 sec - final value during 10times checking
					{
#ifdef TIMER20_DEBUG_MSG
						_DBG("\n\rAux Detect Timer : Detect Aux Out !!!");
#endif
						Aux_Mode_Setting_After_Timer_Checking(FALSE);
					}
				}
			}
		}

		if(factory_reset_led_display_flag)
		{
			if(factory_reset_led_display_flag == df100msTimer1d6s)//After 1.5sec, Factory Reset LED Off and then update current LED display
			{
				factory_reset_led_display_flag = 0;
				LED_Diplay_All_Off(); //Clear LED Display

				uVolume_Level = AD85050_Amp_Get_Cur_Volume_Level();
				AD85050_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);

				mode = Get_Cur_Status_LED_Mode();

				if(mode == STATUS_BT_GIA_PAIRING_MODE)
					Set_Status_LED_Mode(STATUS_POWER_ON_MODE); //Do not use STATUS_BT_POWER_ON_MODE. Becasue the mode only control WHITE/RED LED
				else
					Set_Status_LED_Mode(mode); //Should be called this funciton after I2C/Amp init interrupt complete
					
				MB3021_BT_Module_Init(TRUE); //BT Module Init for Factor Reset
			}
			else
				factory_reset_led_display_flag++;
		}

		if(factory_reset_cmd_recovery_flag)
		{
			if(factory_reset_cmd_recovery_flag == df100msTimer1d6s)//After 1.5sec, If SPK does not receive response from BT Module since SPK sends CMD after 1.5sec. we need to send the CMD again.
			{
				//_DBG("\n\rfactory_reset_cmd_recovery_flag executes !!! ");
								
				//Changed factory_reset_cmd_recovery_flag value from 0 to 1 when factory_reset_cmd_recovery_flag timer is finished.
				//This is for Factory Reset Recovery.
				factory_reset_cmd_recovery_flag = 1;//factory_reset_cmd_recovery_flag = 0; 

				if(recovery_cmd == factory_reset_delete_paired_list)
				{
					if(!Is_Delete_PDL_by_Factory_Reset()) //2023-03-13_1 : sometimes, BT Long Key is worked as Factory Reset due to factory reset recovery action. So, we need to keep BT Long Key action here.)
						MB3021_BT_Delete_Paired_List_All(FALSE);
					else
						MB3021_BT_Delete_Paired_List_All(TRUE);
				}
				else //factory_reset_firmware_version
				{
					MB3021_BT_Module_HW_Reset(); //HW Reset
				}
			}
			else
				factory_reset_cmd_recovery_flag++;
		}

		if(master_slave_Grouping_cmd_recovery_flag)
		{
			if(master_slave_Grouping_cmd_recovery_flag == df100msTimer1d6s)//After 1.5sec, If SPK does not receive response from BT Module since SPK sends CMD after 1.5sec. we need to send the CMD again.
			{
				//_DBG("\n\rTIMER20_Master_Slave_Grouping_cmd_recovery_flag_start() !!! ");
				
				master_slave_Grouping_cmd_recovery_flag = 1;
				MB3021_BT_Master_Slave_Grouping_CMD_Set(recovery_general_cmd);
			}
			else
				master_slave_Grouping_cmd_recovery_flag++;
		}

		if(mute_flag)
		{
			if(mute_flag == df100msTimer1d6s)//After 1sec, Mute Off //2023-02-10_3 : Changed mute time from 1 sec to 1.5 sec due to ADC checking time reducing
			{
				mute_flag = 0;

				if(!IS_Display_Mute())//This is mute off delay and that's means this action should be worked in mute off. //if(Is_Mute())
				{
#ifdef TIMER20_DEBUG_MSG
          _DBG("\n\r+++ Mute off using mute_flag !!!");
#endif					
          AD85050_Amp_Mute(FALSE, FALSE); //MUTE OFF
				}
			}
			else
				mute_flag++;
		}
		
		if(amp_access_error_flag && !Is_BAmp_Init())
		{
			//TAS5806MD_Fault_Clear_Reg();
			//When Amp access error is ocurred, retry it again.
			if(amp_access_error_flag == df100msTimer1d1s) //After 1sec, check
			{				
				AD85050_Fault_Clear_Reg(); //Need to check fault status with TAS5806MD_Amp_Detect_Fault(TRUE) after call this function to update fault status

				if(AD85050_Amp_Detect_Fault(FALSE) == 1)
				{
#ifdef TIMER20_DEBUG_MSG
					_DBG("\n\rTAS5806MD_Amp_Detect_Fault Access OK !!! ");
#endif
					amp_access_error_flag = 0;
				}
				else
				{
					amp_access_error_flag = 1; //retry
				}
			}
			else
				amp_access_error_flag++;
		}

		if(amp_error_no_display_flag)
		{
#ifdef WATCHDOG_TIMER_RESET //2023-07-26_1 : When SW RESET(AMP Power down), this if(amp_error_no_display_flag) statement tries to read TAS5806MD_Amp_Detect_FS and it makes amp error condition. So, SW RESET time takes 40sec.
			if(Is_SSP_REBOOT_KEY_In())
			{
				amp_error_no_display_flag = 0;
			}
			else
#endif
			//When Amp access error is ocurred, retry it again.
			if(amp_error_no_display_flag == df100msTimer1d1s) //After 1sec, check
			{
				if(AD85050_Amp_Detect_FS(FALSE) == 1)
				{
#ifdef TIMER20_DEBUG_MSG
					_DBG("\n\rClock Error Recovery OK !!! ");
#endif					
					AD85050_Fault_Clear_Reg(); //Need to check fault status with TAS5806MD_Amp_Detect_Fault(TRUE) after call this function to update fault status
					amp_error_no_display_flag = 0;
				}
				else
				{
					amp_error_no_display_flag = 1; //retry
				}
			}
			else
				amp_error_no_display_flag++;
		}

		if(master_slave_grouping_flag)
		{
				if(master_slave_grouping_flag == df100msTimer30d1s) //After 30 sec, Return to Original product ID under Master mode
				{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
					_DBG("\n\rMaster Slave Gropuing Time Over under Master - 15sec !!! ");
#endif
					master_slave_grouping_flag = 0;
					TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
				}
				else
					master_slave_grouping_flag++;
		}

		if(user_eq_mute_flag)
		{
			if(user_eq_mute_flag == df100msTimer400ms)//After 300ms, Mute Off
			{
				user_eq_mute_flag = 0;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE) || defined(AD85050_ENABLE)
				if(!IS_Display_Mute())//This is mute off delay and that's means this action should be worked in mute off. //if(Is_Mute())
				{
					AD85050_Amp_Mute(FALSE, FALSE); //MUTE OFF
				}
#endif
			}
			else
				user_eq_mute_flag++;
		}

		if(power_on_volume_sync_flag)
		{
			if(!Power_State())
				power_on_volume_sync_flag = 0;
			
			if((power_on_volume_sync_flag == df100msTimer500ms) || (power_on_volume_sync_flag == df100msTimer1d5s) || (power_on_volume_sync_flag == df100msTimer2s)) //After 1.5sec
			{	
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\r##### power_on_volume_sync_flag meets condition to resend volume data!!! ");
				_DBD(power_on_volume_sync_flag);
#endif
				ADC_Value_Update_to_send_Slave(); //2023-07-20_1
								
				if((power_on_volume_sync_flag == df100msTimer500ms) || (power_on_volume_sync_flag == df100msTimer2s)) //After 3sec
				{
					MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x03);
					if(power_on_volume_sync_flag == df100msTimer2s)
						power_on_volume_sync_flag = 0;
					else
						power_on_volume_sync_flag++;
				}
				else
				{
					MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x04);
					power_on_volume_sync_flag++;
				}
			}
			else
				power_on_volume_sync_flag++;
		}

		if(aux_detecttion_flag)
		{
			if(aux_detecttion_flag == df100msTimer2d6s) //2023-05-09_1 : Reduced the checking time from 5.3s to 2.6s //54)//14) //2023-04-12_4 //54)//After 5.3ms, Check current BT/Aux Status
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\r##### aux_detecttion_flag meets 5.3ms condition !!! ");
#endif
				aux_detecttion_flag = 0;
				Set_Aux_Detection_flag();

				MB3021_BT_Module_Forced_Input_Audio_Path_Setting();
			}
			else
				aux_detecttion_flag++;
		}

		if(aux_setting_flag)
		{
			if(aux_setting_flag == df100msTimer1d6s)//After 1.5sec, Set current input audio path(Aux/BT) unless Master doesn't set input audio path
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\r##### Set current input audio path !!! ");
#endif
				MB3021_BT_Module_Forced_Input_Audio_Path_Setting();
				aux_setting_flag = 0;
			}
			else
				aux_setting_flag++;
		}

		timer20_100ms_count++;
	}// 100ms Timer
}

#endif //TIMER20_COUNTER_ENABLE

