/**********************************************************************
* @file		remocon_action.c
* @brief	IR code
* @version	1.0
* @date		
* @author	MS Kim
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/
#include "main_conf.h"

#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
#include "remocon_action.h"
#if defined(UART_10_ENABLE) && defined(MB3021_ENABLE)
#include "serial.h"
#endif
#ifdef AD82584F_ENABLE
#include "ad82584f.h"
#endif
#ifdef TAS3251_ENABLE
#ifdef I2C_0_ENABLE
#include "i2c.h"
#endif
#include "dsp_amp.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h"
#endif
#ifdef MB3021_ENABLE
#include "bt_MB3021.h"
#endif
#ifdef F1DQ3007_ENABLE
#include "bt_F1DQ3007.h"
#endif
#ifdef F1DQ3021_ENABLE
#include "bt_F1DQ3021.h"
#endif
#if defined(TIMER30_LED_PWM_ENABLE) || defined(TIMER1n_LED_PWM_ENABLE) || defined(GPIO_LED_ENABLE) || defined(TIMER21_LED_ENABLE)
#include "led_display.h"
#endif
#ifdef SABRE9006A_ENABLE
#include "audio_dac.h"
#endif
#if defined(AUTO_VOLUME_LED_OFF) || defined(SLAVE_ADD_MUTE_DELAY_ENABLE) || defined(TIMER20_COUNTER_ENABLE)
#include "timer20.h"
#endif
#if defined(FLASH_SELF_WRITE_ERASE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //Save whether Paired Device exist in Flash
#include "flash.h"
#endif

/* Private typedef ---------------------------------------------------*/
typedef enum {
	Volume_Up_In,
	Volume_Down_In
}Volume_Input;

/* Private define ----------------------------------------------------*/
//#define REMOTE_CONTROL_ACTION_DBG							(1)

/* Private define ----------------------------------------------------*/
#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
const uint8_t ir_key_map[] =
{
	POWER_KEY,
	BT_PAIRING_KEY,
	MUTE_KEY,
	EXIT_KEY,
	DIRECTION_KEY,
	SPEAK_KEY,
	VOL_UP_KEY,
	VOL_DOWN_KEY,
	SUB_VOL_UP_KEY,
	SUB_VOL_DOWN_KEY,
	NUM_1_KEY,
	NUM_2_KEY,
	NUM_3_KEY,
	MOVIE_KEY,
	GAME_KEY,
	MUSIC_KEY,
	BASS_BOOST_KEY,
	PRESET_KEY,
};

#define IR_KEY_MAP_COUNT (sizeof(ir_key_map))
#endif //REMOCON_TIMER20_CAPTURE_ENABLE

//#define LED_DISPLAY_DEBUG

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
static Bool Power_state = TRUE; //main.c set AMP power on upon init //default Power on
#ifdef SWITCH_BUTTON_KEY_ENABLE
Bool bFactory_Reset_Mode = FALSE; //To use, new connection under slave mode only
Bool BBT_Pairing_Key_In = FALSE; //Implemented GIA_PAIRING //To use, BT Pairing Key In is True only.
#endif
#ifdef AUTO_ONOFF_ENABLE
Bool auto_power_off = FALSE;
#endif
#ifdef USEN_TABLET_AUTO_ON //When user selects power off button under USEN Tablet, we need to enable auto power on.
Bool USEN_Tablet_auto_power_on = FALSE;
#endif

#if (defined(POWER_KEY_TOGGLE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)) && (defined(AD82584F_ENABLE) ||defined(TAS5806MD_ENABLE))
static uint8_t bVolume_Level = 0;
#endif
#ifndef MASTER_MODE_ONLY
static Switch_Master_Slave_Mode Power_Off_MS_Mode; //Under Power Off mode, we need to enable Master/Slave Switch action 2022-09-29
#endif

/* Private define ----------------------------------------------------*/

#ifdef FACTORY_MODE
Bool bFACTORY_MODE = FALSE;

void Factory_Mode_Setting(void)
{
#ifdef USEN_BAP //2022-10-12_4
	if(bFACTORY_MODE)
		return;
#endif	
#ifdef COMMON_DEBUG_MSG
	_DBG("\n\r-----------------Factory_Mode_Setting !!!! ");
#endif

#ifdef USEN_BAP	//2022-10-12_4
	//BLUE LED ON
	HAL_GPIO_ClearPin(PD, _BIT(2));
#else
	//RED LED ON
	HAL_GPIO_ClearPin(PD, _BIT(5));
#endif
	
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
#endif

Bool Power_State(void)
{
	return Power_state;
}

#ifdef DRC_TOGGLE_TEST //(RED LED ON : DRC Off/WHITE LED ON : DRC On)
#ifdef LED_PORT_HIGH_DISPLAY
#define Make_LED_ON(x, y) 				(HAL_GPIO_SetPin(x, _BIT(y)))
#define Make_LED_OFF(x, y) 				(HAL_GPIO_ClearPin(x, _BIT(y)))
#else //LED_PORT_HIGH_DISPLAY
#define Make_LED_ON(x, y) 				(HAL_GPIO_ClearPin(x, _BIT(y)))
#define Make_LED_OFF(x, y) 				(HAL_GPIO_SetPin(x, _BIT(y)))
#endif //LED_PORT_HIGH_DISPLAY

//L1 LED
#define MUTE_LED_WHITE_ON						(Make_LED_ON(PD,4))
#define MUTE_LED_WHITE_OFF					(Make_LED_OFF(PD,4))

#define MUTE_LED_RED_ON						(Make_LED_ON(PD,5))
#define MUTE_LED_RED_OFF						(Make_LED_OFF(PD,5))

void Remocon_MUTE_Key_Action(void)
{
	static Bool Mute_On = FALSE;

	if(!Mute_On) //Execute Mute on
	{
		Mute_On = TRUE;
#ifdef TAS5806MD_ENABLE
		TAS5806MD_DRC_Off();
#else
		AD82584F_DRC_Off();
#endif
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on //RED LED ON : DRC Off
		MUTE_LED_RED_ON; 
		MUTE_LED_WHITE_OFF;
		//Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif
	}
	else //Exectue Mute off //WHITE LED ON : DRC On
	{
		Mute_On = FALSE;
#ifdef TAS5806MD_ENABLE
		TAS5806MD_DRC_On();
#else
		AD82584F_DRC_On();
#endif
#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
		MUTE_LED_RED_OFF; 
		MUTE_LED_WHITE_ON;
		//Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
#endif
	}
}
#else //DRC_TOGGLE_TEST
void Remocon_MUTE_Key_Action(void)
{
#ifdef TIMER21_LED_ENABLE
#ifdef AD82584F_ENABLE
	AD82584F_Amp_Mute_Toggle();
#endif
#ifdef TAS5806MD_ENABLE
	TAS5806MD_Amp_Mute_Toggle();
#endif
#elif defined(TAS3251_ENABLE) && defined(BT_SPK_GPIO_ENABLE)
	if(Cur_Mute_Status == Mute_Status_Unmute)
		AMP_DAC_MUTE(TRUE);
	else
		AMP_DAC_MUTE(FALSE);
#else // SABRE9006A_ENABLE
	Audio_Dac_Mute();
#endif
}
#endif //DRC_TOGGLE_TEST

void Remocon_VOL_Key_Action(Volume_Input Volume_In)
{
#ifdef REMOTE_CONTROL_ACTION_DBG
	_DBG("\n\rRemocon_VOL_Key_Action ");
	_DBH(Volume_In);
#endif
	
	if(Volume_In == Volume_Up_In)
	{
#if defined(TAS3251_ENABLE)
#ifdef ESTEC_BOARD
		TAS3251_DSP_Amp_Volume_Control(Volume_Up, TAS3251_DEVICE_ADDR_H);
#endif
		TAS3251_DSP_Amp_Volume_Control(Volume_Up, TAS3251_DEVICE_ADDR_L);
#elif defined(SABRE9006A_ENABLE)
		Audio_Dac_Volume_Control(Volume_Up);
#elif defined(AD82584F_ENABLE)
		AD82584F_Amp_Volume_Control(Volume_Up);
#elif defined(TAS5806MD_ENABLE)
		TAS5806MD_Amp_Volume_Control(Volume_Up);
#endif
	}
	else
	{
#if defined(TAS3251_ENABLE)
#ifdef ESTEC_BOARD
		TAS3251_DSP_Amp_Volume_Control(Volume_Down, TAS3251_DEVICE_ADDR_H);
#endif
		TAS3251_DSP_Amp_Volume_Control(Volume_Down, TAS3251_DEVICE_ADDR_L);
#elif defined(SABRE9006A_ENABLE)
		Audio_Dac_Volume_Control(Volume_Down);
#elif defined(AD82584F_ENABLE)
		AD82584F_Amp_Volume_Control(Volume_Down);
#elif defined(TAS5806MD_ENABLE)
		TAS5806MD_Amp_Volume_Control(Volume_Down);
#endif
	}
}

#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
void Remocon_EQ_Toggle_Key_Action(void)
{
	//To Do !!!Read EQ setting mode from Memory
#if defined(EQ_TOGGLE_ENABLE) && defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ) //2023-01-17 : Read EQ Mode from flash
	static EQ_Mode_Setting EQ_mode = EQ_NONE_MODE;

	if(EQ_mode == EQ_NONE_MODE)
	{
		uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END];

		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
	
		EQ_mode = (EQ_Mode_Setting)uFlash_Read_Buf[FLASH_SAVE_DATA_EQ]; //EQ	
	}

#else //defined(EQ_TOGGLE_ENABLE) && defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
	static EQ_Mode_Setting EQ_mode = EQ_VOCAL_MODE;
#endif //defined(EQ_TOGGLE_ENABLE) && defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)

#ifdef USEN_EQ_ENABLE
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
#else //USEN_EQ_ENABLE
	if(EQ_mode == EQ_POP_MODE)
		EQ_mode = EQ_ROCK_MODE;
	else if(EQ_mode == EQ_ROCK_MODE)
		EQ_mode = EQ_JAZZ_MODE;
	else if(EQ_mode == EQ_JAZZ_MODE)
		EQ_mode = EQ_CLASSIC_MODE;
	else
		EQ_mode = EQ_POP_MODE;
#endif //USEN_EQ_ENABLE

#if defined(FIVE_USER_EQ_ENABLE) && defined(MB3021_ENABLE)
	MB3021_BT_Module_EQ(EQ_mode);
#else
#ifdef AD82584F_ENABLE
	AD82584F_Amp_EQ_DRC_Control(EQ_mode);
#else //TAS5806MD_ENABLE
	TAS5806MD_Amp_EQ_DRC_Control(EQ_mode);
#endif //AD82584F_ENABLE
#endif

}


void Remocon_EQ_Key_Action(EQ_Mode_Setting EQ_mode)
{
	//To Do !!!Read EQ setting mode from Memory
	//static EQ_Mode_Setting EQ_mode = EQ_VOCAL_MODE;

#ifdef USEN_EQ_ENABLE
#if 0 //To Do...
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
#else
	EQ_mode = EQ_NORMAL_MODE;
#endif
#else //USEN_EQ_ENABLE
	if(EQ_mode == EQ_POP_MODE)
		EQ_mode = EQ_ROCK_MODE;
	else if(EQ_mode == EQ_ROCK_MODE)
		EQ_mode = EQ_JAZZ_MODE;
	else if(EQ_mode == EQ_JAZZ_MODE)
		EQ_mode = EQ_CLASSIC_MODE;
	else
		EQ_mode = EQ_POP_MODE;
#endif //USEN_EQ_ENABLE

#ifdef AD82584F_ENABLE
	AD82584F_Amp_EQ_DRC_Control(EQ_mode);
#else //TAS5806MD_ENABLE
	TAS5806MD_Amp_EQ_DRC_Control(EQ_mode);
#endif //AD82584F_ENABLE
}

#ifdef MASTER_MODE_ONLY //2023-03-27_4 : Under BAP-01 NORMAL mode, BAP-01 can get only NORMAL MODE.
void Remocon_BSP_NORMAL_Mode_Switch_Action(void)
{
#ifdef FIVE_USER_EQ_ENABLE
#ifdef AD82584F_ENABLE
	AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#else //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
	TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#endif											
#endif //TAS5806MD_ENABLE
	TIMER20_user_eq_mute_flag_start();
#endif //FIVE_USER_EQ_ENABLE

	if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_BSP_Mode)
	{
#ifdef MASTER_MODE_ONLY_DEBUG_MSG
		_DBG("\n\rSwitch_EQ_BSP_Mode");
#endif
		//EQ Setting from USEN Tablet
#ifdef USEN_IT_AMP_EQ_ENABLE //2023-02-27_1
		TAS5806MD_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting				
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_EQ, EQ_NORMAL_MODE);
#endif
#endif
	}
	else //Switch_EQ_NORMAL_Mode
	{
#ifdef MASTER_MODE_ONLY_DEBUG_MSG
		_DBG("\n\rSwitch_EQ_NORMAL_Mode");
#endif
		//EQ Normal Setting				
#ifdef USEN_IT_AMP_EQ_ENABLE //2023-02-27_1
		TAS5806MD_Amp_EQ_DRC_Control(EQ_BAP_NORMAL_MODE); //DRC / EQ Setting				
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_EQ, EQ_NORMAL_MODE);
#endif
#endif
	}
	
#ifdef SPP_CMD_AND_MASTER_INFO_SEND
	Send_Cur_Master_Info_To_Tablet(); //2023-03-28_3 : Send current information to USEN Tablet when user changes SW2_KEY(EQ NORMAL/EQ BSP) due to changed spec
#endif //#ifdef SPP_CMD_AND_MASTER_INFO_SEND

}
#endif


#ifdef MODE_KEY_TOGGLE_ENABLE
void Remocon_Mode_Key_Action(void)
{
	static Audio_Output_Setting output_mode = STEREO_MODE; //To Do !!!Read Soundmode setting mode from Key setting

	if(output_mode == STEREO_MODE)
		output_mode = LL_MODE;
	else if(output_mode == LL_MODE)
		output_mode = RR_MODE;
	else
		output_mode = STEREO_MODE;

#ifdef AD82584F_ENABLE
	AD82584F_Amp_Mode_Control(output_mode);
#else //TAS5806MD_ENABLE
	TAS5806MD_Amp_Mode_Control(output_mode);
#endif //AD82584F_ENABLE	

}
#else //MODE_KEY_TOGGLE_ENABLE
void Remocon_Mode_Key_Action(void)
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode Master_Slave;
#endif
	Switch_LR_Stereo_Mode mode;
	Audio_Output_Setting outmode;

#ifndef MASTER_MODE_ONLY
	Master_Slave = Get_Cur_Master_Slave_Mode();
#endif
	mode = Get_Cur_LR_Stereo_Mode();
	
	if(mode == Switch_LR_Mode)
	{
#ifdef MASTER_MODE_ONLY
		outmode = LL_MODE; //Master is abled to LL Only !!!
#else
		if(Master_Slave == Switch_Master_Mode) //Master
			outmode = LL_MODE; //Master is abled to LL Only !!!
		else //Slave
			outmode = RR_MODE; //Slave is abled to RR Only !!!
#endif
	}
	else
		outmode = STEREO_MODE;

#ifdef AD82584F_ENABLE
	AD82584F_Amp_Mode_Control(outmode);
#else //TAS5806MD_ENABLE
	TAS5806MD_Amp_Mode_Control(outmode);
#endif //AD82584F_ENABLE	
}
#endif //MODE_KEY_TOGGLE_ENABLE
#endif //AD82584F_ENABLE

#if defined(POWER_KEY_TOGGLE_ENABLE) || defined(USEN_BAP)
#if defined(POWER_KEY_TOGGLE_ENABLE)
void Remocon_Power_Key_Action_Toggle(void) //For only Power Key input
#else
void Remocon_Power_Key_Action_Toggle(uint8_t Input_Key) //For only Power Key input //2022-10-07_3
#endif
{
#if defined(TIMER20_COUNTER_ENABLE) && defined(MB3021_ENABLE)
	Bool BNeed_Mute_Off_Delay = FALSE;
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uFlash_Read_Buf3[FLASH_SAVE_DATA_END];
#endif
#if 0//defined(UART_10_ENABLE) && defined(MB3021_ENABLE) //When Power on/off under slave, slave miss BLE commnucation from Master very first one time.
	Switch_Master_Slave_Mode Master_Slave;
	
	Master_Slave = Get_Cur_Master_Slave_Mode();
#endif
#ifdef REMOTE_CONTROL_ACTION_DBG
	_DBG("\n\rRemocon_Power_Key_Action_Toggle(Power_state) = ");
#endif

#ifdef POWER_KEY_TOGGLE_ENABLE
	if(!Power_state) //Execute Power On
#else
	if(Input_Key == POWER_ON_KEY) //2022-10-07_3
#endif
	{
		Power_state = TRUE;
#ifdef AUTO_ONOFF_ENABLE
		auto_power_off = FALSE;
#endif
#ifdef USEN_TABLET_AUTO_ON //When user selects power off button under USEN Tablet, we need to enable auto power on.
		USEN_Tablet_auto_power_on = FALSE;
#endif
#ifndef MASTER_MODE_ONLY
		if(Power_Off_MS_Mode != Get_Cur_Master_Slave_Mode()
#if defined(SWITCH_BUTTON_KEY_ENABLE) && !defined(BT_GENERAL_MODE_KEEP_ENABLE) //2022-12-27 : Disable this statement to keep General mode after Power Off/On
			|| BBT_Pairing_Key_In
#endif
			) //Under Power Off mode, we need to enable Master/Slave Switch action 2022-09-29
		{
#ifdef MB3021_ENABLE
			MB3021_BT_Module_Init(FALSE);
#endif
#if defined(SWITCH_BUTTON_KEY_ENABLE) && !defined(BT_GENERAL_MODE_KEEP_ENABLE) //2022-12-27 : Disable this statement to keep General mode after Power Off/On
			if(BBT_Pairing_Key_In)
				BBT_Pairing_Key_In = FALSE;
#endif
			Set_Status_LED_Mode(STATUS_BT_PAIRING_MODE);
		}
#endif
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#ifndef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 //2022-10-17 //To Do !!! after implementing aux detection check
		if(HAL_GPIO_ReadPin(PA) & (1<<0)) //To Do !!! after implementing aux detection check
#else
		if(HAL_GPIO_ReadPin(PC) & (1<<3)) //Input(Aux Detec Pin) : High -Aux Out / Low -Aux In
#endif
		{
			BNeed_Mute_Off_Delay = FALSE;
		}
		else
		{
			BNeed_Mute_Off_Delay = TRUE;
		}
#ifdef MB3021_ENABLE
		if((BNeed_Mute_Off_Delay == FALSE) && (BT_Is_Routed() ==TRUE)) //When Aux is none and BT has Audio Stream, it need to use mute off delay
			BNeed_Mute_Off_Delay = TRUE;
#endif
#endif //AUX_INPUT_DET_ENABLE

#ifdef AD82584F_ENABLE
		delay_ms(20);
		HAL_GPIO_SetPin(PF, _BIT(3)); //+14V_DAMP_SW_1
		delay_ms(20);
		HAL_GPIO_SetPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
		delay_ms(20);	
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && defined(AD82584F_ENABLE)
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf3, FLASH_SAVE_DATA_END);

		if(uFlash_Read_Buf3[FLASH_SAVE_DATA_MUTE]) //Keep Mute
		{
			AD82584F_Amp_Mute(TRUE, TRUE); //Power Mute On
			BNeed_Mute_Off_Delay = TRUE; //To avoid mute off
		}
		else
#endif
		{
			if(!BNeed_Mute_Off_Delay)
				HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
			else
			{
				TIMER20_mute_flag_Start(); //Mute off delay when Aux is connected under power bootin on.
			}
		}
#else //#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#endif //#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#elif defined(TAS5806MD_ENABLE)
#ifdef TAS5806MD_ENABLE
#ifndef NOT_USE_POWER_DOWN_MUTE
		MB3021_BT_A2DP_Connection_Control(FALSE); //To Do !!!
		delay_ms(500);

		HAL_GPIO_SetPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
		HAL_GPIO_SetPin(PF, _BIT(3)); //+14V_DAMP_SW_1
#endif //NOT_USE_POWER_DOWN_MUTE
#endif //TAS5806MD_ENABLE

#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && defined(TAS5806MD_ENABLE) //defined(AD82584F_ENABLE) //2023-03-09_4 : When Master is mute state and then Power plug Out/In, Master can't keep mute on due to #ifdef error
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf3, FLASH_SAVE_DATA_END);

		if(uFlash_Read_Buf3[FLASH_SAVE_DATA_MUTE]) //Keep Mute
		{
			TAS5806MD_Amp_Mute(TRUE, TRUE); //Power Mute On
			BNeed_Mute_Off_Delay = TRUE; //To avoid mute off
		}
		else
#endif
		{
			if(!BNeed_Mute_Off_Delay)
			{
#ifdef NOT_USE_POWER_DOWN_MUTE
				TAS5806MD_Amp_Mute(FALSE, TRUE);
#else
				HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#endif
			}
			else
			{
				TIMER20_mute_flag_Start(); //Mute off delay when Aux is connected under power bootin on.
			}
		}
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY);
#endif
#else //#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Mute(FALSE, TRUE);
		TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY);
#else //NOT_USE_POWER_DOWN_MUTE
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#endif //NOT_USE_POWER_DOWN_MUTE
#endif //#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#ifdef TAS5806MD_ENABLE
#ifndef NOT_USE_POWER_DOWN_MUTE
		MB3021_BT_A2DP_Connection_Control(TRUE);//MB3021_BT_Module_HW_Reset(); //To Do !!!
#endif
#endif
#endif //AD82584F_ENABLE

#ifdef AD82584F_USE_POWER_DOWN_MUTE
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
		if(!BNeed_Mute_Off_Delay)
#endif
		Set_Is_Mute(FALSE);
#endif		
#ifdef TIMER21_LED_ENABLE
#ifdef AUX_INPUT_DET_ENABLE
		if(Aux_In_Exist()) //Keep Aux Mode LED When Power on
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
#endif
		{
			//Set_Status_LED_Mode(STATUS_POWER_ON_MODE); //Fixed LED display NG(White LED is fast blinking) under no pairing mode when user push factory reset --> Power Off/On
#ifdef BT_MODULE_ENABLE
			if(Get_Connection_State())
				Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);
			else
				Set_Status_LED_Mode(Get_Return_Status_LED_Mode()); //Fixed LED display NG(White LED is fast blinking) under no pairing mode when user push factory reset --> Power Off/On
#endif
		}
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uFlash_Read_Buf3[FLASH_SAVE_DATA_MUTE]) //Keep Mute
			Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif
#ifdef LED_POWER_CONTROL_ENABLE		/* PD0 Output - SW_+3.3V_SW(LED POWER CONTROL) */
#ifdef USEN_BAP //2023-02-09_2 : Turn on all LED
		HAL_GPIO_SetPin(PD, _BIT(5)); //LED POWER CONTROL - ON //To Do !!! - Need to use this after separating LED Power from Button Power
#ifndef BT_SPK_TACT_SWITCH
		delay_ms(25); //Do NOT Delete !!!
		EXIT_PortE_Configure(); //2023-01-03_2 : To enable BT_KEY Interrupt(PE7). After we fixed 2023-01-03_1 using LED Power control, it makes BT short key interrupt because LED POWER CONTROL line is used as pull-up of BT KEY
#endif
#else
		HAL_GPIO_SetPin(PD, _BIT(0)); //LED POWER CONTROL - ON //To Do !!! - Need to use this after separating LED Power from Button Power
#endif
#endif

#ifdef AD82584F_ENABLE
#ifdef FLASH_SELF_WRITE_ERASE
		AD82584F_Amp_Init(FALSE);
#else
		AD82584F_Amp_Init();
#endif
#ifndef FLASH_SELF_WRITE_ERASE
		AD82584F_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, TRUE);
#endif
#ifdef USEN_BAP //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
		Init_Value_Setting(FALSE);
#else
		Init_Value_Setting();
#endif
#elif defined(TAS5806MD_ENABLE)
#if 0 //When we use DeepSleep mode under TAS5806MD, we don't need to amp init upon power on and just use TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY) //2022-10-12_2
#ifdef FLASH_SELF_WRITE_ERASE
		TAS5806MD_Amp_Init(FALSE);
#else
		TAS5806MD_Amp_Init();
#endif
#endif
#ifdef FLASH_SELF_WRITE_ERASE //2022-11-10_1 : Set volume when power on
#ifdef USEN_BAP //When Master is turned off and turn on by actual power key of USEN Tablet //2023-01-02_3  
		TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE); //Actual key shoud be FALSE even though real actual key
#else
		TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, TRUE);
#endif
#endif
#ifdef USEN_BAP //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
		Init_Value_Setting(FALSE);
#else
		Init_Value_Setting();
#endif
#endif //AD82584F_ENABLE

#ifdef AUTO_VOLUME_LED_OFF
	TIMER20_auto_volume_led_off_flag_Start();
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		FlashSaveData(FLASH_SAVE_DATA_POWER, 1); //Save Power On/Off status - 0x01(Power On) / 0x00(Power Off)
#endif
#if 0//def SWITCH_BUTTON_KEY_ENABLE //Move to Upper 2022-09-29
		if(BBT_Pairing_Key_In)
		{
			BBT_Pairing_Key_In = FALSE;
			MB3021_BT_Module_Init(FALSE);
		}
#endif

#ifdef AUTO_ONOFF_ENABLE
		if(Get_auto_power_flag()) //Re-start count when power on 2022-09-14
		{
			TIMER20_auto_power_flag_Start();
		  
			if(Aux_In_Exist())
				TIMER20_auto_power_flag_Stop();
		}
#endif

#ifdef SLAVE_AUTO_OFF_ENABLE
		if(Get_Slave_auto_power_off_flag()) //Re-start count when power on 2022-09-14
			TIMER20_Slave_auto_power_off_flag_Start();
#endif
	}
	else //Execute Power Off
	{		
		Set_Status_LED_Mode(STATUS_POWER_OFF_MODE);
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
		LED_Display_Volume_All_Off();
#endif		
#ifdef AD82584F_ENABLE
		bVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
#elif defined(TAS5806MD_ENABLE)
		bVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
#endif
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Mute(TRUE, FALSE); //Power Mute Off //2022-10-12_5
#endif

		Power_state = FALSE;
#ifndef MASTER_MODE_ONLY
		Power_Off_MS_Mode = Get_Cur_Master_Slave_Mode();
#endif
		
		All_Timer_Off();
		TIMER20_Flag_init(); //Init all Timer20 Flags
				
		//delay_ms(500); //Need to delete because this statement makes Power On Error when user push power button(interrupt value is 0x3000)
#ifdef AD82584F_ENABLE
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN //Need to keep this Power Off sequence to avoid pop-up noise when power off
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(3)); //+14V_DAMP_SW_1
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
#elif defined(TAS5806MD_ENABLE)
#ifdef TAS5806MD_ENABLE
		//MB3021_BT_A2DP_Connection_Control(FALSE); //We can't use this function here because of disconnection with tablet. So, move to Power on side.
#endif
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_DEEP_SLEEP);
#else
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(3)); //+14V_DAMP_SW_1
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
#endif
#endif //defined(TAS5806MD_ENABLE)

#ifdef AD82584F_USE_POWER_DOWN_MUTE
		Set_Is_Mute(TRUE);
#endif
#ifdef LED_POWER_CONTROL_ENABLE /* PD0 Output - SW_+3.3V_SW(LED POWER CONTROL) */
#ifdef USEN_BAP //2023-02-09_2 : Turn off all LED on BAP-01
#ifndef BT_SPK_TACT_SWITCH
		EXIT_PortE_Disable(); //2023-01-03_2 : To disable BT_KEY Interrupt(PE7). After we fixed 2023-01-03_1 using LED Power control, it makes BT short key interrupt because LED POWER CONTROL line is used as pull-up of BT KEY
#endif
		HAL_GPIO_ClearPin(PD, _BIT(5)); //LED POWER CONTROL - OFF //To Do !!! - Need to use this after separating LED Power from Button Power
#else //USEN_BAP
#if 0//def LED_POWER_CONTROL_ENABLE /* PD0 Output - SW_+3.3V_SW(LED POWER CONTROL) */ //To Do !!! - FACTORY_MODE LED display
		HAL_GPIO_ClearPin(PD, _BIT(0)); //LED POWER CONTROL - OFF //To Do !!! - Need to use this after separating LED Power from Button Power
#endif
#endif //USEN_BAP
#endif //LED_POWER_CONTROL_ENABLE

#ifdef MB3021_ENABLE
		Init_uBLE_Remocon_Data(); //Under Slave and Power off mode, Slave work all CMD thru BLE
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		FlashSaveData(FLASH_SAVE_DATA_POWER, 0); //Save Power On/Off status - 0x01(Power On) / 0x00(Power Off)
#endif
#if defined(MASTER_SLAVE_GROUPING) && defined(LED_DISPLAY_CHANGE)
		if(Get_master_slave_grouping_flag()) //On this case, we can't execute "USEN_Tablet_auto_power_on = TRUE"
			TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
#ifdef USEN_TABLET_AUTO_ON
		else
#endif
#endif
#ifdef USEN_TABLET_AUTO_ON //When user selects power off button under USEN Tablet, we need to enable auto power on.
		if(bIs_USEN_Device)
		{
			//_DBG("\n\rWaitting Start : USEN Device Auto Power On ~~~");
			USEN_Tablet_auto_power_on = TRUE;
		}
#endif
	}
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	if(Power_state)
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x01);
	else
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x00);
#endif
#if 0//defined(UART_10_ENABLE) && defined(MB3021_ENABLE) //When Power on/off under slave, slave miss BLE commnucation from Master very first one time.
	/*UART USART10 Configure*/
	if(Master_Slave == Switch_Slave_Mode) //Slave
		Serial_Init(SERIAL_PORT10, 115200);
#endif
}
#endif //POWER_KEY_TOGGLE_ENABLE

#ifdef SWITCH_BUTTON_KEY_ENABLE
void Remocon_Power_Key_Action(Bool Power_on, Bool Slave_Sync, Bool Vol_Sync) //For SPP/BLE Com or Auto Power On/Off
{		
#if defined(TIMER20_COUNTER_ENABLE) && defined(MB3021_ENABLE)
	Bool BNeed_Mute_Off_Delay = FALSE;
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uFlash_Read_Buf2[FLASH_SAVE_DATA_END];
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When power is turned on, Slave SPK executes mute off delay.
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode Master_Slave;
	
	Master_Slave = Get_Cur_Master_Slave_Mode();
#endif
#endif

	if(Power_state == Power_on)
		return;

	//Power_state = Power_on; //We don't need this line because we assinged proper value below //2022-10-12_7

	if(Power_on == TRUE)
	{
#ifdef REMOCON_DEBUG_MSG
		_DBG("\n\rPower On 1!!!");
#endif
		Power_state = TRUE;
#ifdef AUTO_ONOFF_ENABLE
		auto_power_off = FALSE;
#endif
#ifdef USEN_TABLET_AUTO_ON //When user selects power off button under USEN Tablet, we need to enable auto power on.
		USEN_Tablet_auto_power_on = FALSE;
#endif		

#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When power is turned on, Slave SPK executes mute off delay.
#ifndef MASTER_MODE_ONLY
		if(Master_Slave == Switch_Master_Mode) //Master
#endif
#endif
		{
#ifndef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3
			if(HAL_GPIO_ReadPin(PA) & (1<<0)) //2022-10-17 //To Do !!! after implementing aux detection check
#else
			if(HAL_GPIO_ReadPin(PC) & (1<<3)) //Input(Aux Detec Pin) : High -Aux Out / Low -Aux In
#endif
			{
				BNeed_Mute_Off_Delay = FALSE;
			}
			else
			{
				BNeed_Mute_Off_Delay = TRUE;
			}

			if((BNeed_Mute_Off_Delay == FALSE) && (BT_Is_Routed() ==TRUE)) //When Aux is none and BT has Audio Stream, it need to use mute off delay
				BNeed_Mute_Off_Delay = TRUE;

			BNeed_Mute_Off_Delay = TRUE;
		}
#endif //AUX_INPUT_DET_ENABLE

#ifdef AD82584F_ENABLE
		delay_ms(20);
		HAL_GPIO_SetPin(PF, _BIT(3)); //+14V_DAMP_SW_1
		delay_ms(20);
		HAL_GPIO_SetPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
		delay_ms(20);	

#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && defined(AD82584F_ENABLE)
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf2, FLASH_SAVE_DATA_END);

		if(uFlash_Read_Buf2[FLASH_SAVE_DATA_MUTE]) //Keep Mute
		{
			AD82584F_Amp_Mute(TRUE, TRUE); //Power Mute On
			BNeed_Mute_Off_Delay = TRUE; //To avoid mute off
		}
		else
#endif
		{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When power is turned on, Slave SPK executes mute off delay.
#ifndef MASTER_MODE_ONLY
			if(Master_Slave == Switch_Master_Mode) //Master
#endif
#endif
			{
				if(!BNeed_Mute_Off_Delay)
					HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
				else
					TIMER20_mute_flag_Start(); //Mute off delay when Aux is connected under power bootin on.
			}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When power is turned on, Slave SPK executes mute off delay.
#ifndef MASTER_MODE_ONLY
			else
				TIMER20_mute_flag_Start(); //Mute off delay when Aux is connected under power bootin on.
#endif
#endif
		}
#else //#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#endif
#elif defined(TAS5806MD_ENABLE) //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
#ifndef NOT_USE_POWER_DOWN_MUTE
		MB3021_BT_A2DP_Connection_Control(FALSE);
		delay_ms(500);

		HAL_GPIO_SetPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
		HAL_GPIO_SetPin(PF, _BIT(3)); //+14V_DAMP_SW_1
#endif
#endif
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf2, FLASH_SAVE_DATA_END);

		if(uFlash_Read_Buf2[FLASH_SAVE_DATA_MUTE]) //Keep Mute
		{
			TAS5806MD_Amp_Mute(TRUE, TRUE); //Power Mute On
			BNeed_Mute_Off_Delay = TRUE; //To avoid mute off
		}
		else
#endif
		{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When power is turned on, Slave SPK executes mute off delay.
#ifndef MASTER_MODE_ONLY
			if(Master_Slave == Switch_Master_Mode) //Master
#endif
#endif
			{
				if(!BNeed_Mute_Off_Delay)
				{
#ifdef NOT_USE_POWER_DOWN_MUTE
					TAS5806MD_Amp_Mute(FALSE, TRUE); //Power Mute Off
#else
					HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#endif
				}
				else
					TIMER20_mute_flag_Start(); //Mute off delay when Aux is connected under power bootin on.
			}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When power is turned on, Slave SPK executes mute off delay.
#ifndef MASTER_MODE_ONLY
			else
				TIMER20_mute_flag_Start(); //Mute off delay when Aux is connected under power bootin on.
#endif
#endif
		}
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY);
#endif
#else //#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Mute(FALSE, TRUE); //Power Mute Off
		TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY);
#else
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#endif
#endif
#ifdef TAS5806MD_ENABLE
#ifndef NOT_USE_POWER_DOWN_MUTE
		MB3021_BT_A2DP_Connection_Control(TRUE);//MB3021_BT_Module_HW_Reset();
#endif
#endif
#endif //AD82584F_ENABLE

#ifdef AD82584F_USE_POWER_DOWN_MUTE
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
		if(!BNeed_Mute_Off_Delay
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When power is turned on, Slave SPK executes mute off delay.
#ifndef MASTER_MODE_ONLY
			&& Master_Slave == Switch_Master_Mode
#endif
#endif
		)
#endif //#if defined(TIMER20_COUNTER_ENABLE) && defined(AUX_INPUT_DET_ENABLE) && defined(MB3021_ENABLE)
		Set_Is_Mute(FALSE);
#endif
		
#ifdef TIMER21_LED_ENABLE		
#ifdef AUX_INPUT_DET_ENABLE
		if(Aux_In_Exist()) //Keep Aux Mode LED When Power on
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
#endif
		{
			//Set_Status_LED_Mode(STATUS_POWER_ON_MODE); //Fixed LED display NG(White LED is fast blinking) under no pairing mode when user push factory reset --> Power Off/On
#ifdef BT_MODULE_ENABLE
			if(Get_Connection_State())
				Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);
			else
				Set_Status_LED_Mode(Get_Return_Status_LED_Mode()); //Fixed LED display NG(White LED is fast blinking) under no pairing mode when user push factory reset --> Power Off/On
#endif
		}
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uFlash_Read_Buf2[FLASH_SAVE_DATA_MUTE]) //Keep Mute
			Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif

#ifdef LED_POWER_CONTROL_ENABLE /* PD0 Output - SW_+3.3V_SW(LED POWER CONTROL) */
#ifdef USEN_BAP //2023-01-03_1 : Turn on all LED by SSP communication on BAP-01
		HAL_GPIO_SetPin(PD, _BIT(5)); //LED POWER CONTROL - ON //To Do !!! - Need to use this after separating LED Power from Button Power
#ifndef BT_SPK_TACT_SWITCH
		delay_ms(25); //Do NOT Delete !!!
		EXIT_PortE_Configure(); //2023-01-03_2 : To enable BT_KEY Interrupt(PE7). After we fixed 2023-01-03_1 using LED Power control, it makes BT short key interrupt because LED POWER CONTROL line is used as pull-up of BT KEY
#endif
#else
		HAL_GPIO_SetPin(PD, _BIT(0)); //LED POWER CONTROL - ON //To Do !!! - Need to use this after separating LED Power from Button Power
#endif
#endif

#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
#ifdef FLASH_SELF_WRITE_ERASE
		AD82584F_Amp_Init(FALSE);
#else
		AD82584F_Amp_Init();
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
#ifdef MASTER_MODE_ONLY
		AD82584F_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, TRUE);
#else //MASTER_MODE_ONLY
		if(Master_Slave == Switch_Master_Mode)
			AD82584F_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, TRUE);
		else //Slave
		{
			 if(Vol_Sync == TRUE) //To set volume level from Master's information when power on and do not set volume level in power on function.
			 	AD82584F_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE);
		}
#endif //MASTER_MODE_ONLY
#else
		AD82584F_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE);
#endif
#ifdef USEN_BAP //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
		Init_Value_Setting(FALSE);
#else
		Init_Value_Setting();
#endif
#else //TAS5806MD_ENABLE
#if 0 //When we use DeepSleep mode under TAS5806MD, we don't need to amp init upon power on and just use TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY) //2022-10-12_2
#ifdef FLASH_SELF_WRITE_ERASE
		TAS5806MD_Amp_Init(FALSE);
#else
		TAS5806MD_Amp_Init();
#endif
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
#ifdef USEN_BAP //When Master is turned off and turn on by SSP communiication of USEN Tablet //2023-01-02_2
#ifdef MASTER_MODE_ONLY
		TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE); //Actual Key should be FALSE on BAP-01
#else //MASTER_MODE_ONLY
		if(Master_Slave == Switch_Master_Mode)
		{
			TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE); //Actual Key should be FALSE on BAP-01
		}
		else //Slave
		{
			 if(Vol_Sync == TRUE) //To set volume level from Master's information when power on and do not set volume level in power on function.
				TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE);
		}
#endif
#else
		if(Master_Slave == Switch_Master_Mode)
		{
			TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, TRUE);
		}
		else //Slave
		{
			 if(Vol_Sync == TRUE) //To set volume level from Master's information when power on and do not set volume level in power on function.
				TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE);
		}
#endif
#else
		TAS5806MD_Amp_Volume_Set_with_Index(bVolume_Level, FALSE, FALSE);
#endif
#ifdef USEN_BAP //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
		Init_Value_Setting(FALSE);
#else
		Init_Value_Setting();
#endif
#endif //TAS5806MD_ENABLE
#endif //defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
		
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		FlashSaveData(FLASH_SAVE_DATA_POWER, 1); //Save Power On/Off status - 0x01(Power On) / 0x00(Power Off)
#endif
#if defined(SWITCH_BUTTON_KEY_ENABLE) && !defined(BT_GENERAL_MODE_KEEP_ENABLE) //2022-12-27 : Disable this statement to keep General mode after Power Off/On
		if(BBT_Pairing_Key_In)
		{
			BBT_Pairing_Key_In = FALSE;
#ifdef MB3021_ENABLE
			MB3021_BT_Module_Init(FALSE);
#endif
		}
#endif

#ifdef AUTO_ONOFF_ENABLE
		if(Get_auto_power_flag()) //Re-start count when power on 2022-09-14
		{
			TIMER20_auto_power_flag_Start();
		  
			if(Aux_In_Exist())
				TIMER20_auto_power_flag_Stop();
		}
#endif
		
#ifdef SLAVE_AUTO_OFF_ENABLE
		if(Get_Slave_auto_power_off_flag()) //Re-start count when power on 2022-09-14
			TIMER20_Slave_auto_power_off_flag_Start();
#endif
	}
	else
	{
#ifdef REMOCON_DEBUG_MSG
		_DBG("\n\rPower Off 1!!!");
#endif
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Mute(TRUE, FALSE); //Power Mute Off //Execute Mute On when BAP-01 goes power off mode under power off-->Power Plug Out/In because when power on, BAP-01 has audio noise slightly. //2022-10-12_5
#endif
#ifdef AD82584F_ENABLE
		bVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
#elif defined(TAS5806MD_ENABLE)
		bVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
#endif
		Power_state = FALSE;
#ifndef MASTER_MODE_ONLY
		Power_Off_MS_Mode = Get_Cur_Master_Slave_Mode();
#endif
		All_Timer_Off();
		TIMER20_Flag_init(); //Init all Timer20 Flags
				
		//delay_ms(500);
#ifdef AD82584F_ENABLE
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN //Need to keep this Power Off sequence to avoid pop-up noise when power off
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(3)); //+14V_DAMP_SW_1
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
#elif defined(TAS5806MD_ENABLE)
		//MB3021_BT_A2DP_Connection_Control(FALSE); //We can't use this function here because of disconnection with tablet. So, move to Power on side.
#ifdef NOT_USE_POWER_DOWN_MUTE
		TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_DEEP_SLEEP);
#else
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(3)); //+14V_DAMP_SW_1
		delay_ms(20);
		HAL_GPIO_ClearPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
#endif
#endif

#ifdef AD82584F_USE_POWER_DOWN_MUTE
		Set_Is_Mute(TRUE);
#endif
#ifdef LED_POWER_CONTROL_ENABLE /* PD0 Output - SW_+3.3V_SW(LED POWER CONTROL) */
#ifdef USEN_BAP //2023-01-03_1 : Turn off all LED by SSP communication on BAP-01
#ifndef BT_SPK_TACT_SWITCH
		EXIT_PortE_Disable(); //2023-01-03_2 : To disable BT_KEY Interrupt(PE7). After we fixed 2023-01-03_1 using LED Power control, it makes BT short key interrupt because LED POWER CONTROL line is used as pull-up of BT KEY
#endif
		HAL_GPIO_ClearPin(PD, _BIT(5)); //LED POWER CONTROL - OFF //To Do !!! - Need to use this after separating LED Power from Button Power
#else
		HAL_GPIO_ClearPin(PD, _BIT(0)); //LED POWER CONTROL - OFF //To Do !!! - Need to use this after separating LED Power from Button Power
#endif
#endif
#ifdef TIMER21_LED_ENABLE		
		Set_Status_LED_Mode(STATUS_POWER_OFF_MODE);
#endif
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
		LED_Display_Volume_All_Off();
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		FlashSaveData(FLASH_SAVE_DATA_POWER, 0); //Save Power On/Off status - 0x01(Power On) / 0x00(Power Off)
#endif
	}

#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	if(Slave_Sync == TRUE)
	{
		if(Power_on == TRUE)
			MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x01);
		else
			MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Power, 0x00);
	}
#endif
#if 0//defined(UART_10_ENABLE) && defined(MB3021_ENABLE) //When Power on/off under slave, slave miss BLE commnucation from Master very first one time.
	/*UART USART10 Configure*/
	if(Master_Slave == Switch_Slave_Mode) //Slave
		Serial_Init(SERIAL_PORT10, 115200);
#endif
}
#endif

#if defined(F1DQ3021_ENABLE) || defined(MB3021_ENABLE)
#ifdef MB3021_ENABLE
#ifdef BT_ALWAYS_GENERAL_MODE //2023-01-31_3 : When user push BT Long Key, Master should delete PDL List and work as General mode under enabling "BT_ALWAYS_GENERAL_MODE"
void Remocon_BT_Long_Key_Action(void)
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;

	mode = Get_Cur_Master_Slave_Mode();

	if(mode == Switch_Master_Mode)
#endif
	{
		MB3021_BT_Delete_Paired_List_All(FALSE); //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
	}
}
#else //BT_ALWAYS_GENERAL_MODE
void Remocon_BT_Long_Key_Action(void)
{
#if defined(FLASH_SELF_WRITE_ERASE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //Save whether Paired Device exist in Flash
	uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END] = {0,};
#endif
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;

	mode = Get_Cur_Master_Slave_Mode();

	if(mode == Switch_Master_Mode)
#endif
	{
#if defined(FLASH_SELF_WRITE_ERASE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //Save whether Paired Device exist in Flash
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //Read PDL Device Number from Flash
		
		if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x01) //MASTER SPK doesn't have PDL(last connection info)
		{	//When user choose General mode by pusing BT button during 5 sec, BT White LED is turned on for 2sec and then start 0.5 sec blinking. So, deleted LED is turned on for 2sec.
			//Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE); //Move to below because LED is stopped by MB3021_BT_Module_Init(FALSE);
#ifdef SWITCH_BUTTON_KEY_ENABLE
			BBT_Pairing_Key_In = TRUE;
#endif
#ifdef MB3021_ENABLE
			MB3021_BT_Module_Init(FALSE);
#endif
			Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //To save FLASH_SAVE_GENERAL_MODE_KEEP to call save function //2022-12-23
			if(uFlash_Read_Buf[FLASH_SAVE_GENERAL_MODE_KEEP] != 0x01)
			{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
				_DBG("\n\rSave FLASH_SAVE_GENERAL_MODE_KEEP : 1");
#endif
				FlashSaveData(FLASH_SAVE_GENERAL_MODE_KEEP, 1); //Save GENERAL MODE Status(GENERAL Mode/GIA Mode) to Flash
			}
#endif
		}
		
#else //defined(FLASH_SELF_WRITE_ERASE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) 
		Set_Status_LED_Mode(STATUS_BT_PAIRING_MODE);

#ifdef SWITCH_BUTTON_KEY_ENABLE
		BBT_Pairing_Key_In = TRUE;
#endif
#ifdef MB3021_ENABLE
		MB3021_BT_Module_Init(FALSE);
#endif
#endif //defined(FLASH_SELF_WRITE_ERASE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) 
	}
}
#endif //BT_ALWAYS_GENERAL_MODE

#ifdef MASTER_SLAVE_GROUPING
void Remocon_BT_Short_Key_Action(void)
{
#if defined(FLASH_SELF_WRITE_ERASE) && defined(MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE) && defined(SW1_KEY_TWS_MODE)//Save whether Paired Device exist in Flash
	uint8_t Read_Buf1[FLASH_SAVE_DATA_END];
#endif

#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
	_DBG("\n\r1. Remocon_BT_Short_Key_Action()");
#endif
#if defined(FLASH_SELF_WRITE_ERASE) && defined(TWS_MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE) && defined(SW1_KEY_TWS_MODE) //2022-12-15 //TWS : BT Short Key Action for Master & Slave Grouping
	if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
	{
		Flash_Read(FLASH_SAVE_START_ADDR, Read_Buf1, FLASH_SAVE_DATA_END);	

		if(Read_Buf1[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && Read_Buf1[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff) //In case of valid SET_DEVICE_IDE
		{//It has the history that Master is connected with TWS Slave			
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rBT short Key is Invalid !!!");
#endif
			return;
		}

		if(
#ifndef MASTER_MODE_ONLY
			Get_Cur_Master_Slave_Mode() == Switch_Master_Mode &&
#endif
			!Get_Connection_State())
		{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rMaster is not connected with PeerDevice yet !!!");
#endif
			return;
		}
		
		MB3021_BT_TWS_Master_Slave_Grouping_Start();
	}
	else
#endif //#if defined(TWS_MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE) && defined(SW1_KEY_TWS_MODE)
	MB3021_BT_Master_Slave_Grouping_Start();
}
#endif
#else //MB3021_ENABLE
void Remocon_BT_Long_Key_Action(void)
{
	Switch_Master_Slave_Mode mode;

	mode = Get_Cur_Master_Slave_Mode();

	if(mode == Switch_Master_Mode)
		F1M22_BT_Module_Del_PDL_Set();
	
	Set_Status_LED_Mode(STATUS_BT_PAIRING_MODE);
	//F1M22_BT_Module_Set_Discoverable(FEATURE_ENABLE);
}

void Remocon_BT_Short_Key_Action(void)
{
	F1M22_BT_Module_BA_Association_Set();
}
#endif //MB3021_ENABLE
#endif // #if defined(F1DQ3021_ENABLE) || defined(MB3021_ENABLE)

#ifdef F1DQ3007_ENABLE
void Remocon_BT_Pairing_Key_Action(void)
{
	Set_BT_Pairing_Next_Stage(BT_PAIRING_DISCOVERABLE);
	F1M22_BT_Module_All_Disconnect();
}
#endif

//F1M22_BT_Module_AVRCP_Button(AVRCP_PAUSE); //To Do !!! AVRCP Action using NEXT_KEY, PREVIOUS_KEY, PAUSE_KEY, PLAY_KEY ... but now we don't have these keys.
//F1M22_BT_Module_AVRCP_Button(AVRCP_BACKWARD);
//F1M22_BT_Module_AVRCP_Button(AVRCP_PAUSE);
//F1M22_BT_Module_AVRCP_Button(AVRCP_PLAY);


void Send_Remote_Key_Event(uint8_t IR_KEY)
{
	uint8_t bCnt;
	Bool BValidKey = FALSE;
#ifdef AUTO_VOLUME_LED_OFF
	uint8_t uVolume_Level = 0;
#endif
#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(FLASH_SELF_WRITE_ERASE)
#ifndef MASTER_MODE_ONLY
	uint8_t uFlash_Read_Buf4[FLASH_SAVE_DATA_END];
#endif
#endif

	for(bCnt = 0; bCnt < IR_KEY_MAP_COUNT; bCnt++)
	{	
		if(IR_KEY == ir_key_map[bCnt]) // Is it valid??
		{
			BValidKey = TRUE;
			break;
		}
	}

	if(!BValidKey) // Not valid Key
		return;

#ifdef REMOTE_CONTROL_ACTION_DBG
	_DBG("\n\rInput IR_KEY = ");_DBH(IR_KEY);
#endif
#ifdef MB3021_ENABLE
	if(!IS_BBT_Init_OK())
	{
#ifdef REMOTE_CONTROL_ACTION_DBG
		_DBG("\n\rNeed to wait until Power on init in BT is finshed!!!");
#endif
#if defined(USEN_BAP) //2023-02-14_1 : When user makes power on with rotary switch after power plug Out --> In, this statement makes power on key is invalid key before finishing BT Init
		if(IR_KEY != POWER_ON_KEY && IR_KEY != POWER_OFF_KEY)
			return;
#elif defined(LR_360_FACTORY_ENABLE) //2023-04-05_2 : Need to recognize SW1_KEY(360/LR) and SW2_KEY(Master/Slave) even though BT intial is not finished.
		if(IR_KEY != SW1_KEY && IR_KEY != SW2_KEY)
			return;
#else
			return;
#endif
	}
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
	if(!Power_State())
	{
#ifdef POWER_KEY_TOGGLE_ENABLE
#ifdef FACTORY_MODE
		if(IR_KEY != POWER_KEY && IR_KEY != FACTORY_RESET_KEY && IR_KEY != SW1_KEY && IR_KEY != SW2_KEY) //2023-04-06_3 : Need to allow SW1_KEY and SW2_KEY even though Power off mode.
#else //FACTORY_MODE
		if(IR_KEY != POWER_KEY)
#endif //FACTORY_MODE
#else //POWER_KEY_TOGGLE_ENABLE
		if(IR_KEY != POWER_ON_KEY
#if defined(FACTORY_MODE) && defined(USEN_BAP) //2022-10-12_4
		&& IR_KEY != BT_UPDATE_KEY
#endif
			)
#endif //POWER_KEY_TOGGLE_ENABLE
		{
#if 1//def REMOTE_CONTROL_ACTION_DBG
			_DBG("\n\r5. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
			return;
		}
	}

#ifdef KEY_IS_INVALID_UNDER_MASTER_SLAVE_GROUPING //To make invalid key action excepting FACTORY_RESET_KEY/SW1_KEY/SW2_KEY under Master/Slave Grouping mode. To avoid minor issues(LED Display)
	if(Get_master_slave_grouping_flag())
	{
		if(IR_KEY != FACTORY_RESET_KEY && IR_KEY != SW1_KEY && IR_KEY != SW2_KEY)
			return;
	}
#endif	
#endif

#ifdef AUTO_VOLUME_LED_OFF
	if((IR_KEY != MUTE_KEY) &&(IR_KEY != POWER_KEY) && (IR_KEY != FACTORY_RESET_KEY)) //Must display LED volume Under Mute On
	{
		TIMER20_auto_volume_led_off_flag_Start();
		
		uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
		AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
	}
#endif

	switch(IR_KEY) //To do !!! Need to define some Key actions here!!!
	{
#if defined(EQ_TOGGLE_ENABLE) && (defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)) //2023-01-17 : Execute EQ Key
		case EQ_KEY: //2023-01-17
			Remocon_EQ_Toggle_Key_Action();
			break;
#endif //EQ_TOGGLE_ENABLE

#ifdef REMOCON_TIMER20_CAPTURE_ENABLE
		case NUM_1_KEY: 
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			Remocon_EQ_Toggle_Key_Action();
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			break;
#endif
#if defined(F1DQ3021_ENABLE) || defined(MB3021_ENABLE)
		case BT_PAIRING_KEY: //BT Long key
			Remocon_BT_Long_Key_Action();
			break;
#endif
		case MUTE_KEY:
			Remocon_MUTE_Key_Action();
			break;
			
		case VOL_UP_KEY:
#ifdef SIG_TEST
			MB3021_BT_Module_SIG_Test(A2DP_Abort_Request);
#else
			Remocon_VOL_Key_Action(Volume_Up_In);
#endif
			break;

		case VOL_DOWN_KEY:
#ifdef SIG_TEST
			MB3021_BT_Module_SIG_Test(A2DP_Close_Request);
#else
			Remocon_VOL_Key_Action(Volume_Down_In);
#endif
			break;

#ifdef SWITCH_BUTTON_KEY_ENABLE
#ifdef POWER_KEY_TOGGLE_ENABLE
		case POWER_KEY:
			Remocon_Power_Key_Action_Toggle();
		break;
#else //POWER_KEY_TOGGLE_ENABLE
#ifdef USEN_BAP //2022-10-07_3
		case POWER_ON_KEY:
#ifndef MASTER_MODE_ONLY
			if(Get_Cur_Master_Slave_Mode() != Switch_Slave_Mode
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
				|| (Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && !B_Master_Is_BAP) //2023-02-28_2 : Under BAP-01 Slave Mode, When BAP-01 Master is connected, we need to disable Power Key.
#endif			
				) //BAP-01 can't support actual POWER KEY under Slave mode and it's just followed the stauts of Master //2023-01-05_5
#endif
				Remocon_Power_Key_Action_Toggle(POWER_ON_KEY);
		break;
		case POWER_OFF_KEY:
#ifndef MASTER_MODE_ONLY
			if(Get_Cur_Master_Slave_Mode() != Switch_Slave_Mode
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
				|| (Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && !B_Master_Is_BAP) //2023-02-28_2 : Under BAP-01 Slave Mode, When BAP-01 Master is connected, we need to disable Power Key.
#endif			
				) //BAP-01 can't support actual POWER KEY under Slave mode and it's just followed the stauts of Master //2023-01-05_5
#endif
				Remocon_Power_Key_Action_Toggle(POWER_OFF_KEY);
		break;
		case BT_UPDATE_KEY: //2022-10-12_4
			Factory_Mode_Setting();
		break;
#else //USEN_BAP
		case POWER_ON_KEY: //220217
			Remocon_Power_Key_Action(TRUE, TRUE, TRUE);
		break;
		case POWER_OFF_KEY:
			Remocon_Power_Key_Action(FALSE, TRUE, TRUE);
		break;
#endif //USEN_BAP
#endif //POWER_KEY_TOGGLE_ENABLE
#if defined(F1DQ3021_ENABLE) ||defined(MASTER_SLAVE_GROUPING)
		case BT_KEY: //BT Short key
#ifdef MASTER_SLAVE_GROUPING
			Remocon_BT_Short_Key_Action();
#else //MASTER_SLAVE_GROUPING
#ifdef F1DQ3021_ENABLE
			Remocon_BT_Short_Key_Action();			
#endif
#endif //MASTER_SLAVE_GROUPING
			break;
#endif
		case FACTORY_RESET_KEY: //To do !!! Need to define Factory Reset Key actions here!!!
#ifdef FACTORY_MODE
			if(!Power_State())
			{
				Factory_Mode_Setting();
			}
			else
#endif
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE
				bFactory_Reset_Mode = TRUE;
#endif
				Factory_Reset_Value_Setting();
			}
			break;

		case SW1_KEY: //LR/360 KEY (SUB_VOL_UP_KEY)		//0xe0
#if defined(TWS_MODE_ENABLE) && defined(MB3021_ENABLE) //2022-11-09_1
#ifdef AD82584F_ENABLE
			MB3021_BT_Module_Init(FALSE);
			AD82584F_Amp_Init(FALSE);
#elif defined(TAS5806MD_ENABLE)
#ifdef LR_360_FACTORY_ENABLE //2023-04-06_1 : Changed BSP-01T Spec
#ifdef SWITCH_BUTTON_KEY_ENABLE
				bFactory_Reset_Mode = TRUE;
#endif
				Factory_Reset_Value_Setting();
#else //LR_360_FACTORY_ENABLE
			MB3021_BT_Module_Init(FALSE);
			TAS5806MD_Init_After_Clk_Detect(); //2022-12-06
#endif //LR_360_FACTORY_ENABLE
#endif
#else //TWS_MODE_ENABLE
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			Remocon_Mode_Key_Action();
#endif
#endif //TWS_MODE_ENABLE
			break;
			
		case SW2_KEY: //MASTER/SLAVE KEY (BASS_BOOST_KEY)				//0xb8
		{
#ifdef MASTER_MODE_ONLY //2023-03-27_2 : Implemented SW2_KEY action for BAP-01 EQ Setting
			Remocon_BSP_NORMAL_Mode_Switch_Action();
#else //MASTER_MODE_ONLY
#ifdef F1DQ3021_ENABLE
			Switch_Master_Slave_Mode Master_Slave;
			Master_Slave = Get_Cur_Master_Slave_Mode();
			//_DBG("Master_Slave(1:Master / 0:Slave) = ");
			//_DBD(Master_Slave);

			F1M22_BT_Module_Master_Set(Master_Slave);
#elif defined(MB3021_ENABLE)
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
			AD82584F_Amp_Mute(TRUE, FALSE); //When user changes Master/Slave, it needs mute on
#else //TAS5806MD_ENABLE
			TAS5806MD_Amp_Mute(TRUE, FALSE); //When user changes Master/Slave, it needs mute on
#endif //AD82584F_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)			
#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(FLASH_SELF_WRITE_ERASE) //2022-12-27 : When user switch master/slave key, general mode is only valid under master.
#ifndef MASTER_MODE_ONLY
			if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
			{
				Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf4, FLASH_SAVE_DATA_END);
			
				if(uFlash_Read_Buf4[FLASH_SAVE_GENERAL_MODE_KEEP] == 0x01)
				{
					BBT_Pairing_Key_In = TRUE;
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
					_DBG("\n\rSet BBT_Pairing_Key_In = TRUE");
#endif
				}
				else
					BBT_Pairing_Key_In = FALSE;
			}
#ifndef MASTER_MODE_ONLY
			else //Switch_Slave_Mode
				BBT_Pairing_Key_In = FALSE;
#endif
#else //#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(FLASH_SELF_WRITE_ERASE)
#ifdef SWITCH_BUTTON_KEY_ENABLE //Need to clear BBT_Pairing_Key_In 2022-09-29
			if(BBT_Pairing_Key_In)
				BBT_Pairing_Key_In = FALSE;
#endif
#endif //#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(FLASH_SELF_WRITE_ERASE)
			MB3021_BT_Module_Init(FALSE);
#if defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE) //2022-12-06
			TAS5806MD_Init_After_Clk_Detect();
#endif
#endif
			Set_Status_LED_Mode(STATUS_BT_PAIRING_MODE);
#endif //#ifdef MASTER_MODE_ONLY
		}
		//To Do !!! - Need to implement function //We just check this condition to display LED but If we need to implement other thing, we should implement it here.
			break;

#endif
			
		default:
			break;
	}
}
#endif //#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)

