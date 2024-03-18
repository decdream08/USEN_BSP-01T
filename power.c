/**********************************************************************
* @file		power.c
* @brief	power
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"
#include "power.h"
#include "AD85050.h"
#include "pcm9211.h"
#include "flash.h"
#include "led_display.h"
#include "bt_MB3021.h"
#include "timer20.h"

static void Power_On_Start_Process(void);
static void Power_Off_Start_Process(void);

static PowerModeDef mainPowerMode;
static Bool Power_state = FALSE;

uint16_t power_timer;
uint8_t  mainPowerStep;

void Power_10ms_timer(void)
{
	if(power_timer > df10msTimer0ms )
	{
		--power_timer;
	}
}

void Power_Process(void)
{
	switch(mainPowerMode)
	{
		case PWR_STNDBY:
			break;

		case PWR_ON_START:
			Power_On_Start_Process();
			break;

		case PWR_ON_NORMAL:
			break;

		case PWR_OFF_START:
			Power_Off_Start_Process();
			break;

		default:
			break;
	}
}

void Power_Mode_Set(unsigned char mode)
{
  mainPowerMode = (PowerModeDef)mode;
  mainPowerStep = 0;
  power_timer = df10msTimer0ms;
}

static void Power_On_Start_Process(void)
{
	switch(mainPowerStep) {
		case 0:
			Power_state = TRUE;

			PCM9211_PowerUp();
			++mainPowerStep;
			break;
		case 1:
			{
			uint8_t uFlash_Read_Buf3[FLASH_SAVE_DATA_END];
			Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf3, FLASH_SAVE_DATA_END);

			if(Aux_In_Exist()) //Keep Aux Mode LED When Power on
				Set_Status_LED_Mode(STATUS_AUX_MODE);
			else
			{
				if(Get_Connection_State())
					Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);
				else
					Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
			}

			if(uFlash_Read_Buf3[FLASH_SAVE_DATA_MUTE])
				Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);

			HAL_GPIO_SetPin(PD, _BIT(5)); //LED POWER CONTROL - ON
			power_timer = df10msTimer30ms;

			++mainPowerStep;
			}
			break;
		case 2:
			if(power_timer == df10msTimer0ms )
			{
				EXIT_PortE_Configure();
				++mainPowerStep;
			}
			break;
		case 3:
			Init_Value_Setting(FALSE);
			FlashSaveData(FLASH_SAVE_DATA_POWER, 1);
			++mainPowerStep;
			break;
		case 4:
			MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x01);
			++mainPowerStep;
			break;

		default:
			mainPowerStep = 0;
			HAL_GPIO_SetPin(PC, _BIT(2)); //Outlet On
			Power_Mode_Set(PWR_ON_NORMAL);
			break;
	}
}

static void Power_Off_Start_Process(void)
{
	switch(mainPowerStep) {
		case 0:
			Power_state = FALSE;

			PCM9211_PowerDown();
			AD85050_PowerDown();
			++mainPowerStep;
			break;
		case 1:
			EXIT_PortE_Disable();
			Set_Status_LED_Mode(STATUS_POWER_OFF_MODE);
			++mainPowerStep;
			break;
		case 2:
			All_Timer_Off();
			TIMER20_Flag_init(); //Init all Timer20 Flags			
			++mainPowerStep;
			break;
		case 3:
			HAL_GPIO_ClearPin(PD, _BIT(5));
			++mainPowerStep;
			break;
		case 4:
			Init_uBLE_Remocon_Data();
			FlashSaveData(FLASH_SAVE_DATA_POWER, 0);
			++mainPowerStep;
			break;
		case 5:
			if(Get_master_slave_grouping_flag()) //On this case, we can't execute "USEN_Tablet_auto_power_on = TRUE"
				TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);

			TIMER20_power_on_volume_sync_flag_stop();
			++mainPowerStep;
			break;
		case 6:
			MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x00);
			++mainPowerStep;
			break;

		default:
			mainPowerStep = 0;
			HAL_GPIO_ClearPin(PC, _BIT(2)); //Outlet Off
			Power_Mode_Set(PWR_STNDBY);
			break;
	}
}

Bool Power_State(void)
{
	return Power_state;
}


