/**********************************************************************
* @file		key.c
* @brief	KEY
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"
#include "key.h"
#include "AD85050.h"
#include "pcm9211.h"
#include "flash.h"
#include "led_display.h"
#include "bt_MB3021.h"
#include "timer20.h"
#include "remocon_action.h"
#include "power.h"

Bool bFactory_Reset_Mode = FALSE;
Bool BBT_Pairing_Key_In = FALSE;
Bool bFACTORY_MODE = FALSE;

uint8_t keyCode;
uint8_t keyOk;

uint16_t key_timer;

extern uint8_t uMode_Change;

void Key_10ms_timer(void)
{
	if(key_timer > df10msTimer0ms )
	{
		--key_timer;
	}
}

void Key_Process(void)
{
	if(keyOk == ON)
	{
		if(keyCode != INPUT_BT_KEY && keyCode != INPUT_AUX_KEY)
			keyOk = OFF;

		switch(keyCode)
		{
			case BT_PAIRING_KEY:
				Remocon_BT_Long_Key_Action();
#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_PAIRING_KEY");
#endif
				break;

			case BT_KEY:
				if(!(IsInputSwitch_Aux() && !IsBT_OUTSwitch_On()))
					Remocon_BT_Short_Key_Action();
#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_KEY");
#endif
				break;

			case POWER_KEY:
				Remocon_Power_Key_Action_Toggle();
#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rPOWER_KEY");
#endif
				break;

			case FACTORY_RESET_KEY:
				if(!Power_State())
				{
					Factory_Mode_Setting();
				}
				else
				{
					bFactory_Reset_Mode = TRUE;
					Factory_Reset_Value_Setting();
				}

#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rFACTORY_RESET_KEY");
#endif
				break;

			case BT_UPDATE_KEY:
				Factory_Mode_Setting();
#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_UPDATE_KEY");
#endif
				break;

			case INPUT_BT_KEY:
			case INPUT_AUX_KEY:
				if(key_timer == df10msTimer0ms)
				{
					if((uMode_Change == 0x50 && keyCode == INPUT_AUX_KEY) || (uMode_Change == 0x07 && keyCode == INPUT_BT_KEY))
					{
						keyOk = OFF;
						break;
					}

					AD85050_Amp_Mute(TRUE, FALSE);
					MB3021_BT_Module_Input_Key_Sync_With_Slave((Input_Key_Sync_With_Slave)(Input_key_Sync_Slave_Mute_Off | Input_key_Sync_Switch_Status), 0x02);
					AD85050_SetStatus(AD85050_CHANGE_SOURCE);
					//PCM9211_Set_Path_Init(FALSE);
					//Set_MB3021_BT_Module_Source_Change();
					Set_MB3021_BT_Module_Source_Change_Direct();
#ifdef KEY_CHECK_DEBUG_MSG
					if(keyCode == INPUT_BT_KEY)
						_DBG("\n\rINPUT_BT_KEY");
					else
						_DBG("\n\rINPUT_AUX_KEY");
#endif
					keyOk = OFF;
				}
				break;
#if 0
			case BT_OUT_AREA_1_KEY:
				HAL_GPIO_ClearPin(PE, _BIT(6)); //BT_OUT1
				HAL_GPIO_SetPin(PE, _BIT(5)); //BT_OUT2
				HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
				HAL_GPIO_ClearPin(PE, _BIT(3)); //BT_OUT4

#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_OUT_AREA_1_KEY");
#endif
				break;

			case BT_OUT_AREA_2_KEY:
				HAL_GPIO_SetPin(PE, _BIT(6)); //BT_OUT1
				HAL_GPIO_ClearPin(PE, _BIT(5)); //BT_OUT2
				HAL_GPIO_ClearPin(PE, _BIT(4)); //BT_OUT3
				HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4

#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_OUT_AREA_2_KEY");
#endif
				break;

			case BT_OUT_AREA_1_2_KEY:
				HAL_GPIO_ClearPin(PE, _BIT(6)); //BT_OUT1
				HAL_GPIO_ClearPin(PE, _BIT(5)); //BT_OUT2
				HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
				HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4

#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_OUT_AREA_1_2_KEY");
#endif
				break;
#endif
			case BT_OUT_OFF_KEY:
				if(IsInputSwitch_Aux())
				{
					if(Get_master_slave_grouping_flag())
						TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);

					PCM9211_Set_Status(PCM9211_CHANGE_PATH_TO_ADC);
				}
				else
				{
					PCM9211_Set_Status(PCM9211_CHANGE_PATH_TO_AUXIN0);
				}

#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_OUT_OFF_KEY");
#endif
				break;

			case BT_OUT_ON_KEY:
				PCM9211_Set_Status(PCM9211_CHANGE_PATH_TO_AUXIN0);

#ifdef KEY_CHECK_DEBUG_MSG
				_DBG("\n\rBT_OUT_ON_KEY");
#endif
				break;

			case MUTE_KEY:
				break;
			case VOL_UP_KEY:
				break;
			case VOL_DOWN_KEY:
				break;

			default:
			break;
		}
	}
}

void Factory_Mode_Setting(void)
{
	if(bFACTORY_MODE)
		return;

#ifdef COMMON_DEBUG_MSG
	_DBG("\n\r-----------------Factory_Mode_Setting !!!! ");
#endif

	//BLUE LED ON
	HAL_GPIO_ClearPin(PD, _BIT(2));
	
	//UART DISABLE
	bFACTORY_MODE = TRUE;
	
	//UART PORT change to OPEN DRAIN
	HAL_GPIO_ConfigOutput(PB, 1, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 1, DISPUPD);
	HAL_GPIO_SetPin(PB, _BIT(1));

	HAL_GPIO_ConfigOutput(PB, 0, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 0, DISPUPD);
	HAL_GPIO_SetPin(PB, _BIT(0));
}

void Remocon_MUTE_Key_Action(void)
{
    AD85050_Amp_Mute_Toggle();
}

void Remocon_VOL_Key_Action(Volume_Input Volume_In)
{
#ifdef REMOTE_CONTROL_ACTION_DBG
	_DBG("\n\rRemocon_VOL_Key_Action ");
	_DBH(Volume_In);
#endif
	
	if(Volume_In == Volume_Up_In)
	{
    AD85050_Amp_Volume_Control(Volume_Up);
	}
	else
	{
    AD85050_Amp_Volume_Control(Volume_Down);
	}
}

void Remocon_EQ_Toggle_Key_Action(void)
{
	static EQ_Mode_Setting EQ_mode = EQ_VOCAL_MODE;

	if(EQ_mode == EQ_NORMAL_MODE)
		EQ_mode = EQ_POP_ROCK_MODE;
	else if(EQ_mode == EQ_POP_ROCK_MODE)
		EQ_mode = EQ_CLUB_MODE;
	else if(EQ_mode == EQ_CLUB_MODE)
		EQ_mode = EQ_JAZZ_MODE;
	else if(EQ_mode == EQ_JAZZ_MODE)
		EQ_mode = EQ_VOCAL_MODE;
	else
		EQ_mode = EQ_NORMAL_MODE;

	MB3021_BT_Module_EQ(EQ_mode);
}


void Remocon_EQ_Key_Action(EQ_Mode_Setting EQ_mode)
{
	//To Do !!!Read EQ setting mode from Memory
	//static EQ_Mode_Setting EQ_mode = EQ_VOCAL_MODE;

	EQ_mode = EQ_NORMAL_MODE;
	AD85050_Amp_EQ_DRC_Control(EQ_mode);
}

void Remocon_Mode_Key_Action(void)
{
}

void Remocon_Power_Key_Action_Toggle(void) //For only Power Key input
{
#ifdef REMOTE_CONTROL_ACTION_DBG
	_DBG("\n\rRemocon_Power_Key_Action_Toggle(Power_state) = ");
#endif

	if(!Power_State()) //Execute Power On
	{
		if(Power_Get_Mode() == PWR_STNDBY)
			Power_Mode_Set(PWR_ON_START);
	}
	else //Execute Power Off
	{		
		if(Power_Get_Mode() == PWR_ON_NORMAL)
			Power_Mode_Set(PWR_OFF_START);
	}
}

void Remocon_Power_Key_Action(Bool Power_on, Bool Slave_Sync, Bool Vol_Sync) //For SPP/BLE Com or Auto Power On/Off
{		
	if(Power_State() == Power_on)
		return;

	if(Power_on == TRUE)
	{
#ifdef REMOCON_DEBUG_MSG
		_DBG("\n\rPower On 1!!!");
#endif
		Power_Mode_Set(PWR_ON_START);
	}
	else
	{
#ifdef REMOCON_DEBUG_MSG
		_DBG("\n\rPower Off 1!!!");
#endif
		Power_Mode_Set(PWR_OFF_START);
	}
}

void Remocon_BT_Long_Key_Action(void)
{
	MB3021_BT_Delete_Paired_List_All(FALSE); //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
}

void Remocon_BT_Short_Key_Action(void)
{
	MB3021_BT_Master_Slave_Grouping_Start();
}


