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
#include "protection.h"

static void Power_On_Start_Process(void);
static void Power_Off_Start_Process(void);

static PowerModeDef mainPowerMode;
static Bool Power_state = FALSE;

uint16_t power_timer;
uint8_t  mainPowerStep;

extern Bool already_initialized;

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
		case PWR_OFF_PROTECTION_START:
		case PWR_LOW_LEVEL_START:
		case PWR_OFF_AMP_FAULT_START:
			Power_Off_Start_Process();
			break;

		default:
			break;
	}
}

void Power_Mode_Set(unsigned char mode)
{
#ifdef POWER_DEBUG_MSG
	  _DBG("\n\rPower_Mode_Set = ");
      _DBD(mode);
#endif

  mainPowerMode = (PowerModeDef)mode;
  mainPowerStep = 0;
  power_timer = df10msTimer0ms;
}

PowerModeDef Power_Get_Mode(void)
{
	return mainPowerMode;
}

static void Power_On_Start_Process(void)
{
	switch(mainPowerStep) {
#if 1
		case 0:
			{
#if 1
				uint8_t uFlash_Read_Buf3[FLASH_SAVE_DATA_END];
				Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf3, FLASH_SAVE_DATA_END);

				TIMER20_Amp_error_flag_Stop();

				Power_state = TRUE;

				if(IsInputSwitch_Aux()) //Keep Aux Mode LED When Power on
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
#else
				Power_state = TRUE;
#endif
				++mainPowerStep;
				break;
			}

		case 1:
			//if(IS_BBT_Init_OK())
			{
#if 0
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
#endif
				TIMER20_Amp_error_flag_Stop();

				PCM9211_PowerUp();

				MB3021_BT_AdvertisingOn_OnPowerOn();
				++mainPowerStep;
			}
			break;
#else
		case 0:
			Power_state = TRUE;

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

			TIMER20_Amp_error_flag_Stop();

			PCM9211_PowerUp();

			MB3021_BT_AdvertisingOn_OnPowerOn();
			//MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x01);
			++mainPowerStep;
			break;
		case 1:
			{
				uint8_t uFlash_Read_Buf3[FLASH_SAVE_DATA_END];
				Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf3, FLASH_SAVE_DATA_END);

				if(IsInputSwitch_Aux()) //Keep Aux Mode LED When Power on
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

				++mainPowerStep;
				break;
			}
#endif
		case 2:
			//HAL_GPIO_SetPin(PD, _BIT(5)); //LED POWER CONTROL - ON
			power_timer = df10msTimer30ms;
			++mainPowerStep;
			break;
		case 3:
			if(power_timer == df10msTimer0ms )
			{
				EXIT_PortE_Configure();
				++mainPowerStep;
			}
			break;
		case 4:
			Init_Value_Setting(FALSE);
			FlashSaveData(FLASH_SAVE_DATA_POWER, 1);
			++mainPowerStep;
			break;
		case 5:
			//MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x01);
			++mainPowerStep;
			break;
		default:
			{
#if 0
				uint8_t uFlash_Read_Buf3[FLASH_SAVE_DATA_END];
				Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf3, FLASH_SAVE_DATA_END);

				if(IsInputSwitch_Aux()) //Keep Aux Mode LED When Power on
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
#endif
				mainPowerStep = 0;
				//HAL_GPIO_SetPin(PC, _BIT(2)); //Outlet On
#if 1
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
#endif
				//protection_check_flag = ETC_PROTECTION_MONITOR | AMP_PROTECTION_MONITOR | LED_PROTECTION_MONITOR;
				Power_Mode_Set(PWR_ON_NORMAL);
			}
			break;
	}
}

static void Power_Off_Start_Process(void)
{
	switch(mainPowerStep) {
		case 0:
			protection_check_flag = OFF;

			PCM9211_PowerDown();
			AD85050_PowerDown();

			if(mainPowerMode == PWR_OFF_AMP_FAULT_START)
			{
				HAL_GPIO_ClearPin(PD, _BIT(4)); //+24V DAMP Power
				delay_ms(20);
				HAL_GPIO_ClearPin(PA, _BIT(5)); //+3.3V DAMP Power
			}

			Power_state = FALSE;

#ifdef AMP_AUTO_RECOVERY
			TIMER20_Amp_fault_flag_Stop();
#endif

			if(mainPowerMode != PWR_LOW_LEVEL_START && already_initialized)
				MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x00);

			++mainPowerStep;
			break;
		case 1:
			EXIT_PortE_Disable();
			//Set_Status_LED_Mode(STATUS_POWER_OFF_MODE);
			++mainPowerStep;
			break;
		case 2:
			All_Timer_Off();
			TIMER20_Flag_init(); //Init all Timer20 Flags			
			++mainPowerStep;
			break;
		case 3:
			//LED_Diplay_All_Off();
			//HAL_GPIO_ClearPin(PD, _BIT(5));
			++mainPowerStep;
			break;
		case 4:
			Init_uBLE_Remocon_Data();

			if(mainPowerMode != PWR_LOW_LEVEL_START)
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
			//MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x00);
			++mainPowerStep;
			break;

		default:
			mainPowerStep = 0;

			Set_Status_LED_Mode(STATUS_POWER_OFF_MODE);
			LED_Diplay_All_Off();
			HAL_GPIO_ClearPin(PD, _BIT(5));
			
			//HAL_GPIO_ClearPin(PC, _BIT(2)); //Outlet Off

			/* DAMP_GPIO0 */
			//HAL_GPIO_ConfigOutput(PB, 2, PUSH_PULL_OUTPUT);
			//HAL_GPIO_ConfigPullup(PB, 2, ENPU); 
			//HAL_GPIO_ClearPin(PB, _BIT(2));

			/* external interrupt pin PC4 : 8.5V short Protection - Low : short detection */
			HAL_GPIO_ConfigOutput(PC, 4, PUSH_PULL_OUTPUT);
			HAL_GPIO_ConfigPullup(PC, 4, ENPU); 
			HAL_GPIO_ClearPin(PC, _BIT(4));

			/* external interrupt pin PD0 : AMP short Protection - Low : short detection */
			HAL_GPIO_ConfigOutput(PD, 0, PUSH_PULL_OUTPUT);
			HAL_GPIO_ConfigPullup(PD, 0, ENPU); 
			HAL_GPIO_ClearPin(PD, _BIT(0));

			/* external interrupt pin PD1 : LED 3.3V short Protection - Low : short detection */
			HAL_GPIO_ConfigOutput(PD, 1, PUSH_PULL_OUTPUT);
			HAL_GPIO_ConfigPullup(PD, 1, ENPU); 
			HAL_GPIO_ClearPin(PD, _BIT(1));			

			//protection_check_flag = OFF;

			if(mainPowerMode == PWR_OFF_PROTECTION_START || mainPowerMode == PWR_OFF_AMP_FAULT_START)
			{
				if(protection_mode != ProtectionLED)
				{
					HAL_GPIO_SetPin(PD, _BIT(5)); //LED POWER CONTROL - ON
					Set_Status_LED_Mode(STATUS_PROTECTION_MODE);
					TIMER20_Amp_error_flag_Start();
				}

				protection_mode = ProtectionNone;
			}

			if(mainPowerMode == PWR_LOW_LEVEL_START)
				Power_Mode_Set(PWR_LOW_LEVEL_STNDBY);
			else
				Power_Mode_Set(PWR_STNDBY);
			break;
	}
}

void Power_SetState(Bool power_state)
{
	Power_state = power_state;
}

Bool Power_State(void)
{
	return Power_state;
}


