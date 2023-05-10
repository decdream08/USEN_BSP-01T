/**********************************************************************
* @file		timer20.h
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library on A31G21x
* @version	1.0
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
**********************************************************************/
#ifndef __TIMER20_H__
#define __TIMER20_H__

#ifdef TIMER20_COUNTER_ENABLE	
/* Includes ---------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
/* Private typedef -----------------------------------------------------------*/

typedef enum {
	factory_reset_delete_paired_list,
	factory_reset_firmware_version
} factory_reset_recovery_cmd;
	
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


#ifdef USEN_IT_AMP_EQ_ENABLE //2023-05-09_2
void TIMER20_drc_eq_set_flag_start(void);
void TIMER20_drc_eq_set_flag_stop(void);
#endif

#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) && defined(TIMER20_COUNTER_ENABLE) //2023-04-12_1
void TIMER20_aux_detection_flag_start(void);
#endif

#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : To display current EQ mode using Volume Indicator during 3 sec
void TIMER20_eq_mode_check_flag_start(void);
#endif
#ifdef USEN_BAP
#ifdef AUX_INPUT_DET_ENABLE
void TIMER20_aux_detect_check_flag_start(void);
void TIMER20_aux_detect_check_flag_stop(void);
#endif
#ifndef MASTER_MODE_ONLY  //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#ifdef ADC_VOLUME_STEP_ENABLE
void TIMER20_BT_send_extra_data_flag_start(void);
void TIMER20_BT_send_extra_data_flag_stop(void);
#endif //ADC_VOLUME_STEP_ENABLE
#endif
#endif //USEN_BAP

#ifdef TWS_MODE_ENABLE
void TIMER20_tws_mode_recovery_flag_Start(void);
void TIMER20_tws_mode_recovery_flag_Stop(void);
//void TWS_Slave_Amp_Init_Start(void); //2023-02-22_2
//void TWS_Slave_Amp_Init_Stop(void); //2023-02-22_2
#endif

void TIMER20_Forced_Input_Audio_Path_Setting_flag_start(void); //To avoid, Aux audio output NG
void TIMER20_Forced_Input_Audio_Path_Setting_flag_stop(void); //To avoid, Aux audio output NG

#ifdef MASTER_SLAVE_GROUPING
void TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(uint8_t cmd);
void TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop(void);
#ifdef TWS_MASTER_SLAVE_GROUPING //2023-02-20_2
void Auto_addtime_for_master_slave_grouping(void);
void TIMER20_TWS_Grouping_send_flag_start(void);
#endif
#endif

#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2 : To separate factory reset and BT HW reset which is action by Factory Reset
void TIMER20_BT_hw_reset_cmd_recovery_flag_start(void);
void TIMER20_BT_hw_reset_cmd_recovery_flag_stop(void);	
void TIMER20_factory_reset_cmd_recovery_flag_start(void);
#else //LR_360_FACTORY_ENABLE
void TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_recovery_cmd cmd);
#endif //LR_360_FACTORY_ENABLE
void TIMER20_factory_reset_cmd_recovery_flag_stop(void);

#ifdef FACTORY_RESET_LED_DISPLAY
void TIMER20_factory_reset_led_display_flag_Start(void);
Bool Get_factory_reset_led_display_flag(void);
#endif

#ifdef AUTO_VOLUME_LED_OFF
void TIMER20_auto_volume_led_off_flag_Start(void);
void TIMER20_auto_volume_led_off_flag_Stop(void);
#endif

#ifdef SOC_ERROR_ALARM
void TIMER20_SoC_error_flag_Start(void);
void TIMER20_uart_error_flag_Start(void);
void TIMER20_uart_error_flag_Stop(void);
#endif

#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01
Bool Get_Amp_error_flag(void);
void TIMER20_Amp_error_flag_Start(void);
void TIMER20_Amp_error_flag_Stop(void);

void TIMER20_Amp_access_error_flag_Start(void); //2023-04-07_1
void TIMER20_Amp_access_error_flag_Stop(void); //2023-04-07_1
#endif

void TIMER20_mute_flag_Start(void);
void TIMER20_mute_flag_Stop(void); //1.5sec delay of mute off to avoid pop-up noise

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
int32_t Is_TIMER20_mute_flag_set(void); //status check of mute_flag (1.5sec delay of mute off)
#endif

void TIMER20_auto_power_flag_Start(void);
void TIMER20_auto_power_flag_Stop(void);
Bool Get_auto_power_flag(void);

#ifdef SLAVE_AUTO_OFF_ENABLE
void TIMER20_Slave_auto_power_off_flag_Start(void); //This feature is only available under slave mode
void TIMER20_Slave_auto_power_off_flag_Stop(void); //This feature is only available under slave mode
Bool Get_Slave_auto_power_off_flag(void);
#endif

#ifdef FIVE_USER_EQ_ENABLE
void TIMER20_user_eq_mute_flag_start(void);
void TIMER20_user_eq_mute_flag_stop(void);
#endif

void TIMER20_Flag_init(void);

void TIMER20_Configure(void);
void TIMER20_Periodic_Mode_Run(Bool On);

int32_t TIMER20_1s_Count_Value(void);
int32_t TIMER20_500ms_Count_Value(void);
int32_t TIMER20_100ms_Count_Value(void);

#ifdef MASTER_SLAVE_GROUPING 
void TIMER20_Master_Slave_Grouping_flag_Start(void);
void TIMER20_Master_Slave_Grouping_flag_Stop(Bool Clear_Flag);
Bool Get_master_slave_grouping_flag(void);
#endif

#endif

#endif /* __TIMER21_H__ */


