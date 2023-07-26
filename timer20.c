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

#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
#include "ad82584f.h"
#else //TAS5806MD_ENABLE
#include "tas5806md.h"
#endif
#endif
#if defined(SOC_ERROR_ALARM) || defined(FACTORY_RESET_LED_DISPLAY) || defined(USEN_BAP)
#include "led_display.h"
#endif
#if defined(AUTO_ONOFF_ENABLE) || defined(SLAVE_AUTO_OFF_ENABLE) || defined(USEN_BAP) //2023-07-19_1
#include "remocon_action.h"
#endif
#if defined(FACTORY_RESET_LED_DISPLAY) || defined(MASTER_SLAVE_GROUPING)
#include "bt_MB3021.h"
#endif

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/
//#define TIMER20_DEBUG_MSG					(1)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
TIMER2n_PERIODICCFG_Type TIMER20_Config;
static uint32_t timer20_100ms_count = 0, timer20_500ms_count = 0, timer20_1s_count = 0;
int32_t mute_flag = 0, auto_power_flag = 0;
int32_t aux_setting_flag = 0; //To avoid, Aux audio output NG
#ifdef FIVE_USER_EQ_ENABLE
uint32_t user_eq_mute_flag = 0; //300ms mute
#endif
#ifdef SOC_ERROR_ALARM
int32_t soc_error_flag = 0;
int32_t uart_error_flag = 0;
#endif
#ifdef SLAVE_AUTO_OFF_ENABLE
int32_t Slave_auto_power_off_flag = 0;
#endif
#ifdef AUTO_VOLUME_LED_OFF
int32_t auto_volume_led_off_flag = 0;
#endif
#ifdef FACTORY_RESET_LED_DISPLAY
int32_t factory_reset_led_display_flag = 0;
#endif
int32_t factory_reset_cmd_recovery_flag = 0;
#ifdef LR_360_FACTORY_ENABLE
int32_t BT_hw_reset_cmd_recovery_flag = 0;
#endif
#ifdef MASTER_SLAVE_GROUPING
int32_t master_slave_grouping_flag = 0;
int32_t master_slave_Grouping_cmd_recovery_flag = 0;
#endif
#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01
int32_t amp_error_flag = 0;
int32_t amp_access_error_flag = 0; //2023-04-07_2 : To recovery TAS5806MD_Amp_Detect_Fault() function
#ifdef TAS5806MD_ENABLE //2023-07-06_1 : Applied this solution(2023-06-30_1) under BSP-01T
int32_t amp_error_no_display_flag = 0; //2023-06-30_1 : Excepting the errors with LED error display, we need to recovery from error mode to normal mode.
#endif //TAS5806MD_ENABLE
#endif

factory_reset_recovery_cmd recovery_cmd;
#ifdef MASTER_SLAVE_GROUPING
uint8_t recovery_general_cmd;
#endif
#ifdef TWS_MODE_ENABLE
int32_t tws_mode_recovery_flag = 0;
#ifdef TWS_MASTER_SLAVE_COM_ENABLE //2023-02-22_3
int32_t TWS_powerinit_master_send_data_flag = 0;
#endif
#if 0//def TAS5806MD_ENABLE //2023-02-22_2
int32_t tws_slave_recovery_flag = 0;
#endif
#endif
#ifndef MASTER_MODE_ONLY  //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
int32_t bt_send_extra_data_flag = 0;
#endif
#endif
#ifdef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3
#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) //2023-01-10_3
int32_t aux_detect_check_flag = 0;
#endif
#endif
#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : To display current EQ mode using Volume Indicator during 3 sec
int32_t eq_mode_check_flag = 0;
#endif
#ifdef TWS_MASTER_SLAVE_GROUPING
int32_t tws_grouping_send_flag = 0; //2023-02-20_2
#endif
#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) && defined(TIMER20_COUNTER_ENABLE) //2023-04-12_1
int32_t aux_detecttion_flag = 0;
#endif

#ifdef USEN_TI_AMP_EQ_ENABLE //2023-05-09_2
int32_t drc_eq_set_recovery_flag = 0;
#endif

#ifdef USEN_BAP //2023-07-19_1 : To match volume sync with Slave on power-on under BAP-01 Master
int32_t power_on_volume_sync_flag = 0;
#endif

#ifdef USEN_TI_AMP_EQ_ENABLE //2023-05-09_2
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

#ifdef USEN_BAP
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
#endif

#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) && defined(TIMER20_COUNTER_ENABLE) //2023-04-12_1
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
#endif

#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : To display current EQ mode using Volume Indicator during 3 sec
void TIMER20_eq_mode_check_flag_start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_eq_mode_check_flag_start() !!! ");
#endif

	eq_mode_check_flag = 1;
}

void TIMER20_eq_mode_check_flag_stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_eq_mode_check_flag_stop() !!! ");
#endif

	eq_mode_check_flag = 0;
}
#endif //EQ_TOGGLE_ENABLE

#ifdef USEN_BAP
#ifdef AUX_INPUT_DET_ENABLE //2023-01-10_3
#ifdef AUX_DETECT_INTERRUPT_ENABLE
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
#endif
#endif //AUX_INPUT_DET_ENABLE

#ifndef MASTER_MODE_ONLY  //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#ifdef ADC_VOLUME_STEP_ENABLE
void TIMER20_BT_send_extra_data_flag_start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_BT_send_extra_data_flag_start() !!! ");
#endif
	bt_send_extra_data_flag = 1;
}

void TIMER20_BT_send_extra_data_flag_stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_BT_send_extra_data_flag_stop() !!! ");
#endif
	bt_send_extra_data_flag = 0;
}
#endif //ADC_VOLUME_STEP_ENABLE
#endif //#ifndef MASTER_MODE_ONLY  //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#endif //USEN_BAP

void TIMER20_Forced_Input_Audio_Path_Setting_flag_start(void) //To avoid, Aux audio output NG
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;

	mode = Get_Cur_Master_Slave_Mode();
	
	if(mode != Switch_Master_Mode) //only available under master mode
		return;
#endif
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


#ifdef FIVE_USER_EQ_ENABLE
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
#endif

#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2 : To separate factory reset and BT HW reset which is action by Factory Reset
void TIMER20_BT_hw_reset_cmd_recovery_flag_start(void) //To recovery when SPK executes BT HW Reset by Factory Reset Key
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_BT_hw_reset_cmd_recovery_flag_start() !!! ");
#endif
	BT_hw_reset_cmd_recovery_flag = 1;
}

void TIMER20_BT_hw_reset_cmd_recovery_flag_stop(void) //To recovery when SPK executes BT HW Reset by Factory Reset Key
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_BT_hw_reset_cmd_recovery_flag_stop() !!! ");
#endif
	BT_hw_reset_cmd_recovery_flag = 0;
}

void TIMER20_factory_reset_cmd_recovery_flag_start(void) //To recovery when SPK sends Factory Reset Key
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_cmd_recovery_flag_start() !!! ");
#endif
	factory_reset_cmd_recovery_flag = 1;
}
#else //LR_360_FACTORY_ENABLE
void TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_recovery_cmd cmd) //To recovery when SPK sends Factory Reset Key
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_cmd_recovery_flag_start() !!! ");
#endif
	recovery_cmd = cmd;
	factory_reset_cmd_recovery_flag = 1;
}
#endif //LR_360_FACTORY_ENABLE

void TIMER20_factory_reset_cmd_recovery_flag_stop(void) //To recovery when SPK sends Factory Reset Key
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_cmd_recovery_flag_stop() !!! ");
#endif
	factory_reset_cmd_recovery_flag = 0;	
}

#ifdef MASTER_SLAVE_GROUPING 
#ifdef TWS_MASTER_SLAVE_GROUPING //2023-02-20_2
void TIMER20_TWS_Grouping_send_flag_start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_TWS_Grouping_send_flag_start() !!! ");
#endif
	tws_grouping_send_flag = 1;
}

void TIMER20_TWS_Grouping_send_flag_stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_TWS_Grouping_send_flag_stop() !!! ");
#endif
	tws_grouping_send_flag = 0;
}
#endif

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
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY		
	Set_Status_LED_Mode(STATUS_BT_MASTER_SLAVE_PAIRING_MODE);
#endif
}

void TIMER20_Master_Slave_Grouping_flag_Stop(Bool Clear_Flag)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_Master_Slave_Grouping_flag_Stop() !!! ");
#endif
	master_slave_grouping_flag = 0;

	if(!Clear_Flag)
	{
#if defined(TWS_MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE)
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
			MB3021_BT_TWS_Master_Slave_Grouping_Stop();
		else
#endif
		MB3021_BT_Master_Slave_Grouping_Stop();
	}
}

#ifdef TWS_MASTER_SLAVE_GROUPING
void Auto_addtime_for_master_slave_grouping(void) //2023-02-20_2 : Need to send current information to TWS Slave to sync due to sync error, sometimes
{
	int32_t addtime = 0;
	
	if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
		addtime = 251;
	else //Slave
		addtime = 271;

	if(master_slave_grouping_flag > addtime) //Passed over than 25sec and remain less than 5sec
	{
		master_slave_grouping_flag = addtime;
#ifdef TIMER20_DEBUG_MSG
	    _DBG("\n\rAuto_addtime_for_master_slave_grouping() - Added 5sec !!! ");_DBD(master_slave_grouping_flag);
#endif
	}
	else
	{
#ifdef TIMER20_DEBUG_MSG
	    _DBG("\n\rAuto_addtime_for_master_slave_grouping() - No need Adding time !!! ");
#endif
	}
}
#endif

Bool Get_master_slave_grouping_flag(void)
{
	Bool Ret;

	if(master_slave_grouping_flag)
		Ret = TRUE;
	else
		Ret = FALSE;

	return Ret;
}
#endif //MASTER_SLAVE_GROUPING

#ifdef FACTORY_RESET_LED_DISPLAY
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
#endif

#ifdef AUTO_VOLUME_LED_OFF
void TIMER20_auto_volume_led_off_flag_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_auto_volume_led_off_flag_Start() !!! ");
#endif
	auto_volume_led_off_flag = 1;
}

void TIMER20_auto_volume_led_off_flag_Stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_auto_volume_led_off_flag_Stop() !!! ");
#endif
	auto_volume_led_off_flag = 0;
}
#endif

#ifdef SOC_ERROR_ALARM
void TIMER20_uart_error_flag_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_uart_error_flag_Start() !!! ");
#endif
	uart_error_flag = 1;
}

void TIMER20_uart_error_flag_Stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_uart_error_flag_Stop() !!! ");
#endif
	uart_error_flag = 0;
}

void TIMER20_SoC_error_flag_Start(void)
{	
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_SoC_error_flag_Start() !!! ");
#endif
	soc_error_flag = 1;
}
#endif

#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01
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

#ifdef TAS5806MD_ENABLE //2023-07-06_1 : Applied this solution(2023-06-30_1) under BSP-01T //2023-06-30_1 : Excepting the errors with LED error display, we need to recovery from error mode to normal mode.
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
#endif //TAS5806MD_ENABLE

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
#ifdef TAS5806MD_ENABLE
#ifndef USEN_BAP //2023-04-07_3
	TAS5806MD_Fault_Clear_Reg();
#endif
#endif
}
#endif //defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01

void TIMER20_mute_flag_Start(void) //1.5sec delay of mute off to avoid pop-up noise
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_mute_flag_Start() !!! ");
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
	MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x01);
#endif
	mute_flag = 1;
}

void TIMER20_mute_flag_Stop(void) //1.5sec delay of mute off to avoid pop-up noise
{	
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_mute_flag_Stop() !!! ");
#endif
	mute_flag = 0;
}

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
int32_t Is_TIMER20_mute_flag_set(void) //status check of mute_flag (1.5sec delay of mute off)
{	
#ifdef AUTO_ONOFF_DEBUG_MSG
	_DBG("\n\rIs_TIMER20_mute_flag_set() !!! ");
#endif
	return mute_flag;
}
#endif

void TIMER20_auto_power_flag_Start(void) //This feature is only available under master mode
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;
#endif
#ifdef AUTO_ONOFF_DEBUG_MSG
	_DBG("\n\rTIMER20_auto_power_flag_Start() !!! ");
#endif
#ifdef AUX_INPUT_DET_ENABLE
	if(Aux_In_Exist()) //we don't need to increase count under Aux mode. 2022-09-14
		return;
#endif

#ifdef MASTER_MODE_ONLY
	auto_power_flag = 1;
#else
	mode = Get_Cur_Master_Slave_Mode();
	
	if(mode == Switch_Master_Mode)
	auto_power_flag = 1;
	else
		auto_power_flag = 0;
#endif
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

#ifdef SLAVE_AUTO_OFF_ENABLE
void TIMER20_Slave_auto_power_off_flag_Start(void) //This feature is only available under slave mode
{
	Switch_Master_Slave_Mode mode;
#ifdef SLAVE_AUTO_OFF_DEBUG_MSG
	_DBG("\n\rTIMER20_Slave_auto_power_off_flag_Start() !!! ");
#endif	
	mode = Get_Cur_Master_Slave_Mode();
	
	if(mode == Switch_Slave_Mode)
		Slave_auto_power_off_flag = 1;
	else
		Slave_auto_power_off_flag = 0;
}

void TIMER20_Slave_auto_power_off_flag_Stop(void) //This feature is only available under slave mode
{
#ifdef SLAVE_AUTO_OFF_DEBUG_MSG
	_DBG("\n\rTIMER20_Slave_auto_power_off_flag_Stop() !!! ");
#endif
	Slave_auto_power_off_flag = 0;
}

Bool Get_Slave_auto_power_off_flag(void)
{
	Bool Ret;

	if(Slave_auto_power_off_flag)
		Ret = TRUE;
	else
		Ret = FALSE;

	return Ret;
}

#endif

#ifdef TWS_MODE_ENABLE
#ifdef TWS_MASTER_SLAVE_COM_ENABLE //2023-02-22_3
void TWS_Power_Init_Master_Send_Data_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTWS_Power_Init_Master_Send_Data_Start() !!! ");
#endif
	TWS_powerinit_master_send_data_flag = 1;
}

void TWS_Power_Init_Master_Send_Data_Stop(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTWS_Power_Init_Master_Send_Data_Stop() !!! ");
#endif
	TWS_powerinit_master_send_data_flag = 0;
}

#endif
#if 0//def TAS5806MD_ENABLE //2023-02-22_2
void TWS_Slave_Amp_Init_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTWS_Slave_Amp_Init_Start() !!! ");
#endif
	//tws_mode_even = FALSE;
	tws_slave_recovery_flag = 1;
}

void TWS_Slave_Amp_Init_Stop(void)
{	
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTWS_Slave_Amp_Init_Stop() !!! ");
#endif
	tws_slave_recovery_flag = 0;
}
#endif

void TIMER20_tws_mode_recovery_flag_Start(void)
{
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_tws_mode_recovery_flag_Start() !!! ");
#endif
	//tws_mode_even = FALSE;
	tws_mode_recovery_flag = 1;
}

void TIMER20_tws_mode_recovery_flag_Stop(void)
{	
#ifdef TIMER20_DEBUG_MSG
	_DBG("\n\rTIMER20_tws_mode_recovery_flag_Stop() !!! ");
#endif
	tws_mode_recovery_flag = 0;
}
#endif

void TIMER20_Flag_init(void)
{
#ifdef TIMER20_COUNTER_ENABLE
#ifdef FACTORY_RESET_LED_DISPLAY
	TIMER20_factory_reset_led_display_flag_Stop();
#endif
#ifdef AUTO_VOLUME_LED_OFF
	TIMER20_auto_volume_led_off_flag_Stop();
#endif
#if 0 //We don't need to use this functions here. because we'll init this other way under power on/off working 2022-09-14
	TIMER20_auto_power_flag_Stop();
#ifdef SLAVE_AUTO_OFF_ENABLE
	TIMER20_Slave_auto_power_off_flag_Stop();
#endif
#endif
	TIMER20_mute_flag_Stop();
#endif
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
#ifdef FACTORY_RESET_LED_DISPLAY
	uint8_t uVolume_Level = 0;
	Status_LED_Mode mode;
#endif

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

#ifdef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3
 #if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) //2023-01-10_3 : Implemented Aux detect. Since Aux out is detected by interrupt, Timer has been checking if Aux detect is True during 20 Sec.
 		if(aux_detect_check_flag)
		{
			aux_detect_check_flag--;
			
			if((aux_detect_check_flag%10) == 0) //In every 1 sec
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
#endif //#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE)
#endif //AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3
#ifdef FACTORY_RESET_LED_DISPLAY
		if(factory_reset_led_display_flag)
		{
			if(factory_reset_led_display_flag == 16)//After 1.5sec, Factory Reset LED Off and then update current LED display
			{
				factory_reset_led_display_flag = 0;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
				LED_Diplay_All_Off(); //Clear LED Display

#ifdef AD82584F_ENABLE
				uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
				AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#else //TAS5806MD_ENABLE
				uVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
				TAS5806MD_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#endif
				mode = Get_Cur_Status_LED_Mode();

#ifdef GIA_MODE_LED_DISPLAY_ENABLE
				if(mode == STATUS_BT_GIA_PAIRING_MODE)
					Set_Status_LED_Mode(STATUS_POWER_ON_MODE); //Do not use STATUS_BT_POWER_ON_MODE. Becasue the mode only control WHITE/RED LED
				else
#endif
					Set_Status_LED_Mode(mode); //Should be called this funciton after I2C/Amp init interrupt complete
					
#ifdef AUTO_VOLUME_LED_OFF
				TIMER20_auto_volume_led_off_flag_Start(); //do not display LED volume after 10s Under Mute Off
#endif
#endif
#ifdef MB3021_ENABLE
				MB3021_BT_Module_Init(TRUE); //BT Module Init for Factor Reset
#endif
			}
			else
				factory_reset_led_display_flag++;
		}
#endif

#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2 : To separate factory reset and BT HW reset which is action by Factory Reset
		if(BT_hw_reset_cmd_recovery_flag)
		{
			if(BT_hw_reset_cmd_recovery_flag == 16)//After 1.5sec, If SPK does not receive response from BT Module since SPK executes BT HW reset after 1.5sec. we need to execute the BT HW reset again.
			{
#ifdef TIMER20_DEBUG_MSG
				//_DBG("\n\rBT_hw_reset_cmd_recovery_flag is executed !!! ");
#endif		
				//Changed factory_reset_cmd_recovery_flag value from 0 to 1 when factory_reset_cmd_recovery_flag timer is finished.
				//This is for Factory Reset Recovery.
				BT_hw_reset_cmd_recovery_flag = 1;//factory_reset_cmd_recovery_flag = 0; 
#ifdef MB3021_ENABLE
				MB3021_BT_Module_HW_Reset(); //HW Reset
#endif
			}
			else
				BT_hw_reset_cmd_recovery_flag++;
		}
#endif //LR_360_FACTORY_ENABLE

		if(factory_reset_cmd_recovery_flag)
		{
			if(factory_reset_cmd_recovery_flag == 16)//After 1.5sec, If SPK does not receive response from BT Module since SPK sends CMD after 1.5sec. we need to send the CMD again.
			{
				//_DBG("\n\rfactory_reset_cmd_recovery_flag executes !!! ");
								
				//Changed factory_reset_cmd_recovery_flag value from 0 to 1 when factory_reset_cmd_recovery_flag timer is finished.
				//This is for Factory Reset Recovery.
				factory_reset_cmd_recovery_flag = 1;//factory_reset_cmd_recovery_flag = 0; 
#ifdef MB3021_ENABLE
#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2 : To separate factory reset and BT HW reset which is action by Factory Reset
#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
				if(!Is_Delete_PDL_by_Factory_Reset()) //2023-03-13_1 : sometimes, Separate.BT Long Key is worked as Factory Reset due to factory reset recovery action. So, we need to keep BT Long Key action here.)
					MB3021_BT_Delete_Paired_List_All(FALSE);
				else
					MB3021_BT_Delete_Paired_List_All(TRUE);
#else //BT_ALWAYS_GENERAL_MODE
				MB3021_BT_Delete_Paired_List_All();
#endif //BT_ALWAYS_GENERAL_MODE
#else //LR_360_FACTORY_ENABLE
				if(recovery_cmd == factory_reset_delete_paired_list)
				{
#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
					if(!Is_Delete_PDL_by_Factory_Reset()) //2023-03-13_1 : sometimes, BT Long Key is worked as Factory Reset due to factory reset recovery action. So, we need to keep BT Long Key action here.)
						MB3021_BT_Delete_Paired_List_All(FALSE);
					else
						MB3021_BT_Delete_Paired_List_All(TRUE);
#else //BT_ALWAYS_GENERAL_MODE
					MB3021_BT_Delete_Paired_List_All();
#endif //BT_ALWAYS_GENERAL_MODE
				}
				else //factory_reset_firmware_version
				{
#ifdef MB3021_ENABLE
					MB3021_BT_Module_HW_Reset(); //HW Reset
#endif
				}
#endif //LR_360_FACTORY_ENABLE
#endif //MB3021_ENABLE
			}
			else
				factory_reset_cmd_recovery_flag++;
		}

#ifdef MASTER_SLAVE_GROUPING 
		if(master_slave_Grouping_cmd_recovery_flag)
		{
			if(master_slave_Grouping_cmd_recovery_flag == 16)//After 1.5sec, If SPK does not receive response from BT Module since SPK sends CMD after 1.5sec. we need to send the CMD again.
			{
				//_DBG("\n\rTIMER20_Master_Slave_Grouping_cmd_recovery_flag_start() !!! ");
				
				master_slave_Grouping_cmd_recovery_flag = 1;
#ifdef MB3021_ENABLE
				MB3021_BT_Master_Slave_Grouping_CMD_Set(recovery_general_cmd);
#endif		
			}
			else
				master_slave_Grouping_cmd_recovery_flag++;
		}
#endif

		if(mute_flag)
		{
#ifdef USEN_BAP //2023-01-11_1 : Changed mute time from 2.5sec to 500msec
			if(mute_flag == 16)//After 1sec, Mute Off //2023-02-10_3 : Changed mute time from 1 sec to 1.5 sec due to ADC checking time reducing
#else
			if(mute_flag == 26)//After 2.5sec, Mute Off
#endif
			{
				mute_flag = 0;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_USE_POWER_DOWN_MUTE
				if(!IS_Display_Mute())//This is mute off delay and that's means this action should be worked in mute off. //if(Is_Mute())
#else
				if(Is_Mute())
#endif
				{
#ifdef TAS5806MD_ENABLE //2023-04-12_2 //2022-12-06
					if((TAS5806MD_CLK_Detect_Count() != 0xffffffff)
#ifdef TWS_MODE_ENABLE
						|| tws_mode_recovery_flag
#endif
						) /*&& Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode*/ //2023-03-10_2 : To avoid slave mute or noise under broadcast slave mode
					{
#ifdef TIMER20_DEBUG_MSG
						_DBG("\n\r+++ Do not execute Mute off using mute_flag because I2C access is not available!!!");
#endif					
						mute_flag = 1;
					}
					else
#endif
					{
#ifdef AD82584F_ENABLE
						AD82584F_Amp_Mute(FALSE, FALSE); //MUTE OFF
#else //TAS5806MD_ENABLE						
#ifdef TIMER20_DEBUG_MSG
						_DBG("\n\r+++ Mute off using mute_flag !!!");
#endif					
						TAS5806MD_Amp_Mute(FALSE, FALSE); //MUTE OFF
#endif //TAS5806MD_ENABLE
					}
				}
#endif
			}
			else
				mute_flag++;
		}
		
#ifdef AUTO_VOLUME_LED_OFF
		if(auto_volume_led_off_flag)
		{
			if(auto_volume_led_off_flag == 101) //After 10s, Power Off
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\rAuto Volume Led Off !!! ");
#endif				
				auto_volume_led_off_flag = 0;
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
				LED_Display_Volume_All_Off();
#endif
			}
			else
				auto_volume_led_off_flag++;
		}
#endif

#ifdef AUTO_ONOFF_ENABLE
		if(auto_power_flag && Power_State()) //We don't need to increase count under power off 2022-09-14
		{
			if(auto_power_flag == 3301) //After 5min 30sec, Power Off
			{
#ifndef MASTER_MODE_ONLY
				Switch_Master_Slave_Mode mode;
			
				mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef AUTO_ONOFF_DEBUG_MSG
				_DBG("\n\rAuto Power Off !!! ");
#endif
				auto_power_flag = 0;
				auto_power_off = TRUE;
#ifndef MASTER_MODE_ONLY
				if(mode == Switch_Master_Mode)
#endif
					Remocon_Power_Key_Action(FALSE, TRUE, TRUE);
			}
			else
				auto_power_flag++;
		}
#endif

#ifdef SLAVE_AUTO_OFF_ENABLE
		if(Slave_auto_power_off_flag && Power_State()) //We don't need to increase count under power off 2022-09-14
		{
			if(Slave_auto_power_off_flag == 6001) //After 10min, Slave SPK executes Auto Power Off
			{
				Switch_Master_Slave_Mode mode;
			
				mode = Get_Cur_Master_Slave_Mode();
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\rSlave auto Power Off !!! ");
#endif
				Slave_auto_power_off_flag = 0;

				if(mode == Switch_Slave_Mode)
					Remocon_Power_Key_Action(FALSE, FALSE, TRUE);
			}
			else
				Slave_auto_power_off_flag++;
		}
#endif
#ifdef SOC_ERROR_ALARM		
		if(uart_error_flag)
		{
			if(uart_error_flag == 251) //After 25sec, display SOC Error and then 1min, Self-Reset
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\rSoC Error Self Reset After 5min !!! ");
#ifdef SOC_ERROR_ALARM_DEBUG_MSG
                                _DBG("\n\rSOC_ERROR - 7");
#endif
#endif
				uart_error_flag = 0;
#ifdef TIMER21_LED_ENABLE
				Set_Status_LED_Mode(STATUS_SOC_ERROR_MODE); //over-temperature or short-circuit condition
#endif
				TIMER20_SoC_error_flag_Start();
			}
			else
				uart_error_flag++;
		}
		
		if(soc_error_flag)
		{
			if(soc_error_flag == 601) //After 1min, Self-Reset
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\rSoC Error Self Reset !!! ");
#endif
				soc_error_flag = 0;
				SW_Reset();
			}
			else
				soc_error_flag++;
		}
#endif


#if (defined(AMP_ERROR_ALARM) || defined(SOC_ERROR_ALARM)) && defined(TAS5806MD_ENABLE) //2023-04-07_2 : When we can't access TI amp (TAS5806MD_Amp_Detect_Fault()) becasue before amp init is not finished, we need to access TI amp again with this retry //2022-11-01
		if(amp_access_error_flag
#ifdef TAS5806MD_ENABLE
					&& !Is_BAmp_Init()
#endif
					)
		{
			//TAS5806MD_Fault_Clear_Reg();
			//When Amp access error is ocurred, retry it again.
			if(amp_access_error_flag == 11) //After 1sec, check
			{				
				TAS5806MD_Fault_Clear_Reg(); //Need to check fault status with TAS5806MD_Amp_Detect_Fault(TRUE) after call this function to update fault status

				if(TAS5806MD_Amp_Detect_Fault(FALSE) == 1)
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

#ifdef TAS5806MD_ENABLE //2023-07-06_1 : Applied this solution(2023-06-30_1) under BSP-01T //2023-06-30_1 : Excepting the errors with LED error display, we need to recovery from error mode to normal mode.
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
			if(amp_error_no_display_flag == 11) //After 1sec, check
			{
				if(TAS5806MD_Amp_Detect_FS(FALSE) == 1)
				{
#ifdef TIMER20_DEBUG_MSG
					_DBG("\n\rClock Error Recovery OK !!! ");
#endif					
					TAS5806MD_Fault_Clear_Reg(); //Need to check fault status with TAS5806MD_Amp_Detect_Fault(TRUE) after call this function to update fault status
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
#endif //TAS5806MD_ENABLE

#ifndef USEN_BAP //2023-04-07_3 : Do not use this under USEN_BAP
		if(amp_error_flag 
#ifdef TAS5806MD_ENABLE
			&& !Is_BAmp_Init()
#endif
			)
		{
			//TAS5806MD_Fault_Clear_Reg();
			//When Amp error is ocurred, execute volume down action. 2022-11-01
			if(amp_error_flag == 11) //After 1sec, Normal Display and then clear ERROR Pin flag
			{
				if(TAS5806MD_Amp_Detect_FS(FALSE))
				{
					TAS5806MD_Fault_Clear_Reg(); //Need to check fault status with TAS5806MD_Amp_Detect_Fault(TRUE) after call this function to update fault status

					if(TAS5806MD_Amp_Detect_Fault(TRUE) == 1)
					{
#ifdef TIMER20_DEBUG_MSG
						_DBG("\n\rAmp Error flag is needed to keep !!! ");
#endif
						amp_error_flag = 1;
#ifdef SWITCH_BUTTON_KEY_ENABLE
						Send_Remote_Key_Event(VOL_DOWN_KEY);
#endif
					}
					else
					{
#ifdef TIMER20_DEBUG_MSG
						_DBG("\n\rAmp Error flag Clear !!! ");
#endif
						amp_error_flag = 0;
#if defined(TIMER21_LED_ENABLE) && defined(AMP_ERROR_ALARM) //2022-11-01
						Set_Status_LED_Mode(Get_Return_Status_LED_Mode2()); //shoud be after amp_error_flag = 0; //over-temperature or short-circuit condition
#endif
					}
				}
				else
					amp_error_flag = 1;
			}
			else
				amp_error_flag++;
		}
#endif //USEN_BAP
#endif //#if (defined(AMP_ERROR_ALARM) || defined(SOC_ERROR_ALARM)) && defined(TAS5806MD_ENABLE) //2023-04-07_2 : When we can't access TI amp (TAS5806MD_Amp_Detect_Fault()) becasue before amp init is not finished, we need to access TI amp again with this retry //2022-11-01

#ifdef MASTER_SLAVE_GROUPING
#ifdef TWS_MASTER_SLAVE_GROUPING //2023-02-20_2
		if(tws_grouping_send_flag)
		{							
			if(
#ifndef MASTER_MODE_ONLY
				Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && 
#endif
				master_slave_grouping_flag)
			{	
				if(tws_grouping_send_flag == 11 || tws_grouping_send_flag == 16) //After 1 sec, Need to send DeviceID again
				{
					tws_grouping_send_flag++;
					Set_Cur_TWS_Grouping_Status(TWS_Grouping_Master_Send_DeviceID2);
					MASTER_SLAVE_Grouping_Send_SET_DEVICE_ID(TRUE); //Second time
				}
				else if(tws_grouping_send_flag == 21) //After 2 sec, Need to send Cur Status information to Slave
				{
					tws_grouping_send_flag++;
					Set_Cur_TWS_Grouping_Status(TWS_Grouping_Master_Send_Cur_Status1);
#if defined(MB3021_ENABLE) && defined(I2C_0_ENABLE)
#ifdef TAS5806MD_ENABLE
					MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse());//TAS5806MD_Amp_Get_Cur_Volume_Level());
#else
					MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, AD82584F_Amp_Get_Cur_Volume_Level_Inverse());//TAS5806MD_Amp_Get_Cur_Volume_Level());
#endif
#endif
				}
				else if(tws_grouping_send_flag == 31) //After 3 sec, Need to send Cur Status information to Slave again
				{
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_5 : To make New TWS Connection, we need to reset after 5sec since TWS Master sent SET_DEVICE_IDE to TWS Slave
					tws_grouping_send_flag++;
#else
					tws_grouping_send_flag = 0;
#endif
					Set_Cur_TWS_Grouping_Status(TWS_Grouping_Master_Send_Cur_Status2);
#if defined(MB3021_ENABLE) && defined(I2C_0_ENABLE)
#ifdef TAS5806MD_ENABLE
					MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse());//TAS5806MD_Amp_Get_Cur_Volume_Level());
#else
					MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, AD82584F_Amp_Get_Cur_Volume_Level_Inverse());//TAS5806MD_Amp_Get_Cur_Volume_Level());
#endif
#endif
				}
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_5 : To make New TWS Connection, we need to reset after 5sec since TWS Master sent SET_DEVICE_IDE to TWS Slave
				else if(tws_grouping_send_flag == 51)
				{
					tws_grouping_send_flag = 0;
					master_slave_grouping_flag = 0;
					TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
				}
#endif
				else
					tws_grouping_send_flag++;
			}
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_7 : To make New TWS Connection, we need to reset after 5sec since TWS Slave get SET_DEVICE_IDE from TWS Master.
			else if(
#ifndef MASTER_MODE_ONLY
				Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && 
#endif
				master_slave_grouping_flag)
			{
				if(tws_grouping_send_flag == 51)
				{
					tws_grouping_send_flag = 0;
					master_slave_grouping_flag = 0;
					TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
				}
				else
					tws_grouping_send_flag++;
			}
#endif
			else
			{
				tws_grouping_send_flag = 0;
			}
		}	
#endif

		if(master_slave_grouping_flag)
		{
#ifndef MASTER_MODE_ONLY
			if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
			{					
#if defined(TWS_MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE) //2022-12-15 : TWS - After 30 sec, Set new SET_DEVICE_ID under Master mode
				if(master_slave_grouping_flag == 301 && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
				{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
					_DBG("\n\rMaster Slave Gropuing Time Over under Master - 30sec !!! ");
#endif
#ifndef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_4 : To make New TWS Connection, we need to disable timer action to close TWS Master/Slave Grouping
					master_slave_grouping_flag = 0;
					TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
#endif //NEW_TWS_MASTER_SLAVE_LINK
				}
				else if(master_slave_grouping_flag == 151 && Get_Cur_LR_Stereo_Mode() == Switch_Stereo_Mode)
#else //#if defined(TWS_MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE)
				if(master_slave_grouping_flag == 151) //After 15 sec, Return to Original product ID under Master mode
#endif //#if defined(TWS_MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE)
				{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
					_DBG("\n\rMaster Slave Gropuing Time Over under Master - 15sec !!! ");
#endif
					master_slave_grouping_flag = 0;
					TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
				}
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-06_3 : To send BAP-01 Volume Sync Data before GROUPING disconnection 
#ifndef MASTER_MODE_ONLY //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
				else if(master_slave_grouping_flag == 146)
				{
					MB3021_BT_Module_Send_Extra_Data();
					master_slave_grouping_flag++;
				}
#endif //#ifndef MASTER_MODE_ONLY
#endif
				else
					master_slave_grouping_flag++;
			}
#ifndef MASTER_MODE_ONLY
			else
			{					
				if(master_slave_grouping_flag == 301) //After 30 sec, Return to Original product ID under Slave mode
				{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
					_DBG("\n\rMaster Slave Gropuing Time Over under Slave - 30sec !!! ");
#endif
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_4 : To make New TWS Connection, we need to disable timer action to close TWS Master/Slave Grouping
					if(Get_Cur_LR_Stereo_Mode() == Switch_Stereo_Mode)
#endif
					{
						master_slave_grouping_flag = 0;
						TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
					}
				}
				else
					master_slave_grouping_flag++;
			}
#endif
		}
#endif //MASTER_SLAVE_GROUPING

#ifdef FIVE_USER_EQ_ENABLE
		if(user_eq_mute_flag)
		{
			if(user_eq_mute_flag == 4)//After 300ms, Mute Off
			{
				user_eq_mute_flag = 0;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_USE_POWER_DOWN_MUTE
				if(!IS_Display_Mute())//This is mute off delay and that's means this action should be worked in mute off. //if(Is_Mute())
#else
				if(Is_Mute())
#endif
				{
#ifdef TAS5806MD_ENABLE //2023-04-12_2 //2022-12-06
					if((TAS5806MD_CLK_Detect_Count() != 0xffffffff)
#ifdef TWS_MODE_ENABLE
						|| tws_mode_recovery_flag
#endif
						) /*&& Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode*/ //2023-03-10_2 : To avoid slave mute or noise under broadcast slave mode
					{
#ifdef TIMER20_DEBUG_MSG
						_DBH32(TAS5806MD_CLK_Detect_Count());
						_DBG("\n\r+++ Do not execute Mute off using user_eq_mute_flag because I2C access is not available!!!");
#endif					
						user_eq_mute_flag = 1;
					}
					else
#endif
					{
#ifdef AD82584F_ENABLE
						AD82584F_Amp_Mute(FALSE, FALSE); //MUTE OFF
#else //TAS5806MD_ENABLE
#ifdef TIMER20_DEBUG_MSG
						_DBG("\n\r+++ Mute off using user_eq_mute_flag !!!");
#endif					
#ifdef USE_TI_AMP_HI_Z_MUTE //2023-05-22_1
						if(mute_flag == 0)
#endif
						TAS5806MD_Amp_Mute(FALSE, FALSE); //MUTE OFF

#endif //TAS5806MD_ENABLE
					}
				}
#endif
			}
			else
				user_eq_mute_flag++;
		}
#endif //FIVE_USER_EQ_ENABLE

#ifdef USEN_BAP //2023-07-19_1
	if(power_on_volume_sync_flag)
	{
		if(!Power_State())
			power_on_volume_sync_flag = 0;
		
		if((power_on_volume_sync_flag == 5) || (power_on_volume_sync_flag == 15) || (power_on_volume_sync_flag == 20)) //After 1.5sec
		{	
#ifdef TIMER20_DEBUG_MSG
			_DBG("\n\r##### power_on_volume_sync_flag meets condition to resend volume data!!! ");
			_DBD(power_on_volume_sync_flag);
#endif
			ADC_Value_Update_to_send_Slave(); //2023-07-20_1
							
			if((power_on_volume_sync_flag == 5) || (power_on_volume_sync_flag == 20)) //After 3sec
			{
				MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x03);
				if(power_on_volume_sync_flag == 20)
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
#endif

#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) && defined(TIMER20_COUNTER_ENABLE) //2023-04-12_1
		if(aux_detecttion_flag)
		{
			if(aux_detecttion_flag == 26) //2023-05-09_1 : Reduced the checking time from 5.3s to 2.6s //54)//14) //2023-04-12_4 //54)//After 5.3ms, Check current BT/Aux Status
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\r##### aux_detecttion_flag meets 5.3ms condition !!! ");
#endif
				aux_detecttion_flag = 0;
#ifdef AUX_DETECT_INTERRUPT_ENABLE
				Set_Aux_Detection_flag();
#endif
#ifdef MB3021_ENABLE
				MB3021_BT_Module_Forced_Input_Audio_Path_Setting();
#endif
			}
			else
				aux_detecttion_flag++;
		}
#endif //USEN_BAP

		if(aux_setting_flag)
		{
			if(aux_setting_flag == 16)//After 1.5sec, Set current input audio path(Aux/BT) unless Master doesn't set input audio path
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\r##### Set current input audio path !!! ");
#endif
#ifdef MB3021_ENABLE
				MB3021_BT_Module_Forced_Input_Audio_Path_Setting();
#endif
				aux_setting_flag = 0;
			}
			else
				aux_setting_flag++;
		}
				
#if defined(TWS_MODE_ENABLE) && defined(MB3021_ENABLE)
		if(tws_mode_recovery_flag)
		{
			tws_mode_recovery_flag++;
			
			if(tws_mode_recovery_flag == 21) //After 2sec, Set tws mode setting "exit" to "Maseter/Slave"
			{
				MB3021_BT_Module_TWS_Mode_Exit();
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\r##### Set TWS Recovery - 1!!!");
#endif
			}

			if(tws_mode_recovery_flag == 41) //After 4sec, Set tws mode setting "exit" to "Maseter/Slave"
			{
				MB3021_BT_Module_TWS_Mode_Set_Again();
				tws_mode_recovery_flag = 0;
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\r##### Set TWS Recovery - 2!!!");
#endif
			}
		}

#ifdef TWS_MASTER_SLAVE_COM_ENABLE //2023-02-22_3
		if(TWS_powerinit_master_send_data_flag)
		{
			TWS_powerinit_master_send_data_flag++;

			if(TWS_powerinit_master_send_data_flag == 16) //After 1.5sec(TWS connection), TWS Master send sync data again to Slave.
			{
				TWS_powerinit_master_send_data_flag = 0;

				if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
				{
#if defined(MB3021_ENABLE) && defined(I2C_0_ENABLE)
#ifdef TAS5806MD_ENABLE
					MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse());//TAS5806MD_Amp_Get_Cur_Volume_Level());
#else
					MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, AD82584F_Amp_Get_Cur_Volume_Level_Inverse());//TAS5806MD_Amp_Get_Cur_Volume_Level());
#endif
#endif					
				}
			}
		}
#endif
#if 0//def TAS5806MD_ENABLE //2023-02-22_2 : Disable AMP Init that is for AMP recovery
		if(tws_slave_recovery_flag)
		{
			tws_slave_recovery_flag++;

			if(tws_slave_recovery_flag == 31) //After 3sec, try Amp Init. FYI 2sec is too short.
			{
				tws_slave_recovery_flag = 0;

				if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
				{
					if(!TAS5806MD_Amp_Detect_FS(TRUE)) //2022-11-15_1
					   tws_slave_recovery_flag = 1;
				}
			}
		}
#endif
#endif //defined(TWS_MODE_ENABLE) && defined(MB3021_ENABLE)

#ifndef MASTER_MODE_ONLY  //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-06_1 : Changed SW which send 64 step volume to BAP
		if(bt_send_extra_data_flag)
		{
			if(bt_send_extra_data_flag == 5) //After 400ms (Need to keep this timing due to missing data issue from Slave)
			{
				bt_send_extra_data_flag = 0;

				if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
					MB3021_BT_Module_Send_Extra_Data(); //Send BT Extra data
			}
			else
				bt_send_extra_data_flag++;
		}
#endif //defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#endif

#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : To display current EQ mode using Volume Indicator during 3 sec
		if(eq_mode_check_flag)
		{
			if(!Power_State())
				eq_mode_check_flag = 0;
			
			if(eq_mode_check_flag == 31) //After 3 sec, Return to volume level indicator
			{
#ifdef TIMER20_DEBUG_MSG
				_DBG("\n\rEQ Mode Indicator off and display volume level !!! ");
#endif
				eq_mode_check_flag = 0;
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef TAS5806MD_ENABLE
				uVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
#else
				uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
#endif
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
				LED_Display_Volume(uVolume_Level);
#endif
			}
			else
				eq_mode_check_flag++;
		}
#endif

#ifdef USEN_TI_AMP_EQ_ENABLE //2023-05-09_2 : To recovery DRC/EQ Setting
		if(drc_eq_set_recovery_flag && !Is_BAmp_Init())
		{
			if(drc_eq_set_recovery_flag == 3) //check time 200ms
			{
				//_DBG("\n\rTIMER20_Master_Slave_Grouping_cmd_recovery_flag_start() !!! ");
				TAS5806MD_Set_Cur_EQ_DRC_Mode();
				drc_eq_set_recovery_flag = 0;
			}
			else
				drc_eq_set_recovery_flag++;
		}
#endif

		timer20_100ms_count++;
	}// 100ms Timer
}

#endif //TIMER20_COUNTER_ENABLE

