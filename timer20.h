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

void TIMER20_drc_eq_set_flag_start(void);
void TIMER20_drc_eq_set_flag_stop(void);

void TIMER20_aux_detection_flag_start(void);

void TIMER20_power_on_volume_sync_flag_start(void); //2023-07-19_1 
void TIMER20_power_on_volume_sync_flag_stop(void); //2023-07-19_1 

void TIMER20_aux_detect_check_flag_start(void);
void TIMER20_aux_detect_check_flag_stop(void);

void TIMER20_Forced_Input_Audio_Path_Setting_flag_start(void); //To avoid, Aux audio output NG
void TIMER20_Forced_Input_Audio_Path_Setting_flag_stop(void); //To avoid, Aux audio output NG

void TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(uint8_t cmd);
void TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop(void);


void TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_recovery_cmd cmd);
void TIMER20_factory_reset_cmd_recovery_flag_stop(void);

void TIMER20_factory_reset_led_display_flag_Start(void);
Bool Get_factory_reset_led_display_flag(void);

Bool Get_Amp_error_flag(void);
void TIMER20_Amp_error_flag_Start(void);
void TIMER20_Amp_error_flag_Stop(void);

void TIMER20_Amp_access_error_flag_Start(void); //2023-04-07_1
void TIMER20_Amp_access_error_flag_Stop(void); //2023-04-07_1

void TIMER20_Amp_error_no_diplay_flag_Start(void); //2023-06-30_1
void TIMER20_Amp_error_no_diplay_flag_Stop(void); //2023-06-30_1

void TIMER20_mute_flag_Start(void);
void TIMER20_mute_flag_Stop(void); //1.5sec delay of mute off to avoid pop-up noise

int32_t Is_TIMER20_mute_flag_set(void); //status check of mute_flag (1.5sec delay of mute off)

void TIMER20_auto_power_flag_Start(void);
void TIMER20_auto_power_flag_Stop(void);
Bool Get_auto_power_flag(void);

void TIMER20_user_eq_mute_flag_start(void);
void TIMER20_user_eq_mute_flag_stop(void);

void TIMER20_Flag_init(void);

void TIMER20_Configure(void);
void TIMER20_Periodic_Mode_Run(Bool On);

int32_t TIMER20_1s_Count_Value(void);
int32_t TIMER20_500ms_Count_Value(void);
int32_t TIMER20_100ms_Count_Value(void);

void TIMER20_Master_Slave_Grouping_flag_Start(void);
void TIMER20_Master_Slave_Grouping_flag_Stop(Bool Clear_Flag);
Bool Get_master_slave_grouping_flag(void);
#endif /* __TIMER21_H__ */


