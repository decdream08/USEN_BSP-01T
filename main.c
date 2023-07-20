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
#ifdef SPI_11_ENABLE
#include "spi.h"
#endif
#ifdef UART_10_ENABLE
#include "serial.h"
#endif
#ifdef I2C_0_ENABLE
#include "i2c.h"
#endif
#ifdef REMOCON_TIMER20_CAPTURE_ENABLE
#include "ir.h"
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
#ifdef TAS3251_ENABLE
#include "dsp_amp.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h"
#endif
#ifdef TOUCHKEY_ENABLE
#include "touch_key.h"
#endif
#ifdef ADAU1452_ENABLE
#include "adau1452.h"
#endif
#ifdef SABRE9006A_ENABLE
#include "audio_dac.h"
#endif
#ifdef AD82584F_ENABLE
#include "ad82584f.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h"
#endif
#ifdef TIMER30_LED_PWM_ENABLE
#include "timer30.h"
#endif
#if defined(TIMER1n_LED_PWM_ENABLE) || defined(TIMER12_13_LONG_KEY_ENABLE)
#include "timer1n.h"
#endif
#ifdef TIMER21_LED_ENABLE
#include "timer21.h"
#endif
#ifdef TIMER20_COUNTER_ENABLE
#include "timer20.h"
#endif
#ifdef GPIO_LED_ENABLE
#include "led_display.h"
#endif
#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
#include "remocon_action.h"
#endif
#ifdef WATCHDOG_TIMER_RESET
#include "A31G21x_hal_wdt.h"
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
#include "flash.h"
#endif
#ifdef ADC_INPUT_ENABLE
#include "adc.h"
#endif
#ifdef ADC_INTERRUPT_INPUT_ENABLE
#include "adc_interrupt.h"
#endif

/* Private typedef ---------------------------------------------------*/
#ifndef TOUCHKEY_ENABLE
typedef enum
{
	TASK_UART = 0,
	TASK_GPIO,
#ifdef ADC_INPUT_ENABLE
	TASK_ADC,
#endif
	TASK_MAX
} e_task;

#endif

//BT_SPK_TACT_SWITCH - PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
//SWITCH_BUTTON_KEY_ENABLE BSP-01: PA0 / PA1 / PA2 /PA3 / PA4 / PA5 / PA6, BAP-01 : PA0 / PA1 / PA6
#if defined(BT_SPK_TACT_SWITCH) || defined(SWITCH_BUTTON_KEY_ENABLE)
typedef enum
{
	button_push = 0,
	button_release
}button_status;
#endif //#if defined(BT_SPK_TACT_SWITCH) || defined(SWITCH_BUTTON_KEY_ENABLE)

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
#ifdef BT_SPK_GPIO_ENABLE
Bool B_FALT_SW = TRUE; //Defualt High
Bool B_CLIP_OTW_SW = TRUE; //Default High
Mute_Status Cur_Mute_Status = Mute_Status_Unmute;
#endif
#ifdef AUX_INPUT_DET_ENABLE
Bool B_AUX_DET; //Input(Aux Detec Pin) : TRUE -Aux In / FALSE - Aux Out
#ifdef USEN_BAP //2022-10-17_1
Bool B_Auto_AUX_Mode; //TRUE - Aux Aux Mode / FALSE - Aux Fixed Mode
#endif
#endif
#ifdef UART_10_ENABLE
uint8_t UART10_Rx_Buffer[UART10_Rx_Buffer_Size];
uint8_t uBuffer_Count = 0;
#endif
#ifdef SPI_11_ENABLE
#define SPI_Rx_Buffer_Size			255

uint8_t SPI_Rx_Buffer[SPI_Rx_Buffer_Size];
uint8_t SPI_uBuffer_Count = 0;
#endif
#ifdef WATCHDOG_TIMER_RESET
uint32_t msec;
WDT_CFG_Type wdtCfg;
#endif
#if defined(AMP_1_1CH_WORKAROUND) || defined(TAS5806MD_ENABLE) //2022-10-05 //2022-10-17_3
static uint32_t Clk_detect_uCount = 0;
#endif
#if defined(ADC_INPUT_ENABLE) || defined(ADC_INTERRUPT_INPUT_ENABLE)
static uint32_t uAttenuator_Vol = 0xff; //2022-11-22_1
#endif

#ifdef KEY_CHATTERING_EXTENSION_ENABLE //2023-04-05_1 : Move to here for initializing for static variable to fix switch action
#ifdef USEN_BAP //2022-10-17_4
#ifdef MASTER_MODE_ONLY
static uint8_t IsEQ_BSP;
#else //MASTER_MODE_ONLY
static uint8_t IsMaster;
#endif //MASTER_MODE_ONLY
#else
static uint8_t IsMaster, IsStereo; //IsMaster - 0x01(Master)/0x00(Slave), IsStereo - 0x01(360)/0x00(LR)
#endif
#endif

/* Private define ----------------------------------------------------*/
#ifdef KEY_CHATTERING_ENABLE
#define KEY_CHATTERING_DELAY_MS				(20) //40ms //20ms
#endif
#ifdef VOLUME_KEY_FILTERING
#define KEY_FILTERING_TIME					(3)//(4) //over than X x 100ms
#endif

/* Private function prototypes ---------------------------------------*/
void mainloop(void);
void GPIO_Configure(void);
#ifdef BT_SPK_GPIO_ENABLE
void EXIT_Configure(void);
#ifdef BT_SPK_TACT_SWITCH //PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
void EXIT_Configure2(void);
#endif //BT_SPK_TACT_SWITCH
#endif //BT_SPK_GPIO_ENABLE
#ifdef SOUND_BAR_GPIO_ENABLE
void EXIT_Configure1(void);
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
void EXTI_PortA_Configure(void);
#endif
#ifdef USEN_GPIO_OTHERS_ENABLE //Use External INT for Switchs and Button Keys - PF0 / PF5(AMP_ERROR input)
void EXIT_PortF_Configure(void);
#endif
#if 1//def _UART_DEBUG_MSG
void Display_UART_Receive_Data(void);
#endif
#ifdef UART_10_ENABLE
static void Serial_Get_Data_Intterupt_Callback(uint8_t *Data);
#endif
#ifdef SPI_11_ENABLE
static void SPI_Get_Data_Interrupt_Callback(uint8_t *Data);
#endif

#if defined(ADC_VOLUME_STEP_ENABLE) && defined(TAS5806MD_ENABLE) && defined(ADC_INPUT_ENABLE) //Attenuator action is inversed. So fixed it. //2023-02-08_1 : make Attenuator GAP
uint8_t Convert_ADC_To_Attenuator(uint32_t ADC_Value);
#endif

#ifdef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 //2022-10-17
#ifdef AUX_INPUT_DET_ENABLE
void EXTI_PortC_Configure(void);
#endif

#if defined(USEN_BAP) && defined(ADC_INPUT_ENABLE) && defined(ADC_VOLUME_STEP_ENABLE) //2023-07-20_1 : Sometimes, BAP-01 can't set current ADC Value when user executes power on from power off using volume dial.
uint8_t ADC_Value_Update_to_send_Slave(void)
{
	uint32_t ADC3_Value;
	uint8_t uCurVolLevel = 0, ADC_Level_Min, ADC_Level_Max, i;
	
#ifndef ADC_INTERRUPT_INPUT_ENABLE
	ADC3_Value = ADC_PollingRun(3);

#ifdef ADC_INPUT_DEBUG_MSG
	_DBG("\n\r === Master Volume ADC = 0x");
	_DBH32(ADC3_Value);
#endif
#ifdef ADC_VOLUME_64_STEP_ENABLE
	for(i=1;i<65;i++)
#else //ADC_VOLUME_50_STEP_ENABLE
	for(i=1;i<51;i++)
#endif //ADC_VOLUME_64_STEP_ENABLE
	{
#ifdef ADC_VOLUME_64_STEP_ENABLE
		ADC_Level_Min = (i-1)*4; //0 4 8
		ADC_Level_Max = (i*4)-1; //3 7 11 //2023-02-06_3 : To make ADC Gap
#else //ADC_VOLUME_50_STEP_ENABLE //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
		if(i==1)
			ADC_Level_Min = 0;
		else
			ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

		if(i==50)
			ADC_Level_Max = 255;
		else
			ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253
#endif //ADC_VOLUME_64_STEP_ENABLE

		if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value))
		{
#ifdef ADC_VOLUME_64_STEP_ENABLE
			ADC_Level_Min = (i-1)*4; //0 4 8
			ADC_Level_Max = (i*4)-2; //2 6 10
#else //ADC_VOLUME_50_STEP_ENABLE //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
			if(i==1)
				ADC_Level_Min = 0;
			else
				ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

			if(i==50)
				ADC_Level_Max = 255;
			else
				ADC_Level_Max = (i*5)-1; //4 9 14 19 ... 244 249~253
#endif //ADC_VOLUME_64_STEP_ENABLE


			if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value)) //2023-02-08_3 : Added additional code for Volume GAP
			{
#ifdef ADC_INPUT_DEBUG_MSG
				_DBG("\n\r Volume ADC Valid Value !!!! = ");
				_DBG("\n\r === Volume ADC Level Step = ");
				_DBD(i);
#endif
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
				uCurVolLevel = 64 - i;
#else //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
				uCurVolLevel = 50 - i;
#endif
			}
			else //2023-02-06_3 : Do not update cur volume level and current ADC value is not valid
			{
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
				uCurVolLevel = 63; //Vol Level 1
#else //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
				uCurVolLevel = 49; //Vol Level 1
#endif
#ifdef ADC_INPUT_DEBUG_MSG
				_DBG("\n\r Volume ADC Invalid Value !!!! = ");
				_DBG("\n\r === Volume ADC Level Step = ");
				_DBD(i);
#endif
			}

			break;
		}
	}

#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
	uCurVolLevel = 64 - i; //0 ~ 63(64 Step / 0 - MAX)
#else //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
	uCurVolLevel = 50 - i;
#endif
#ifdef TAS5806MD_ENABLE
#ifdef ADC_INPUT_DEBUG_MSG
		_DBG("\n\r === Volume Level Setting = ");
		_DBD(uCurVolLevel);
#endif
		//uCurVolLevel_bk = uCurVolLevel;
		TAS5806MD_Amp_Volume_Set_with_Index(uCurVolLevel, FALSE, TRUE);
		//uCurVolLevel = 15 - ADC3_Value; //15 Step, Inverse Value, The integer value need to match with (VOLUME_LEVEL_NUMER)
#ifdef FLASH_SELF_WRITE_ERASE //2023-03-02_3 : BAP-01 do not use flash data to set volume level when power on.
		//FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel);
#endif
#endif

#endif //ADC_INTERRUPT_INPUT_ENABLE

	return uCurVolLevel;
}
#endif

#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) && defined(TIMER20_COUNTER_ENABLE) //2023-01-10_3
void Aux_Mode_Setting_After_Timer_Checking(Bool Aux_In)
{
#ifdef AUX_INPUT_DET_DEBUG
	_DBG("\n\r ### Aux_Mode_Setting_After_Timer_Checking()");
#endif

	B_AUX_DET = Aux_In;

	if(B_AUX_DET == FALSE)
	{
#ifdef MB3021_ENABLE
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUTO_ONOFF_ENABLE) //Fixed Master SPK do not work Auto power off even though No siganl from BT when user remove Aux jack
		TIMER20_auto_power_flag_Start();
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
		MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef USEN_BAP //2023-05-19_1 : Under BAP-01 Aux mode, customer wants to output audio at once when user change BT to Aux.
		if(!Aux_In_Exist())
#endif
		{
#ifdef AD82584F_ENABLE
			AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
			TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif
		}
#endif
#ifdef TWS_MODE_ENABLE
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //2022-11-17_2
		{
			if(Aux_In_Exist())
			{
				if(BTWS_LIAC != TWS_Status_Master_Mode_Control && Is_TWS_Master_Slave_Connect() == TWS_Get_Information_Ready)
					MB3021_BT_Module_TWS_Set_Discoverable_Mode(); //2022-11-17_2 : Under Aux mode, TWS Setting
			}
			else
			{
				if(Get_Connection_State() == FALSE)
					MB3021_BT_Module_Set_Discoverable_Mode(); //2022-11-17_2 : Under Non-Aux mode, Discoverable Setting
			}
		}
#endif
#ifdef USEN_BAP //2023-05-19_1 : Under BAP-01 Aux mode, customer wants to output audio at once when user change BT to Aux.
		Set_MB3021_BT_Module_Source_Change_Direct();
#else
		Set_MB3021_BT_Module_Source_Change();
#endif
#endif //MB3021_ENABLE
	}
}

void Set_Aux_Detection_flag(void) //2023-04-12_1
{	
	if(HAL_GPIO_ReadPin(PC) & (1<<3)) //Input(Aux Detec Pin) : High -Aux Out / Low -Aux In
	{
		B_AUX_DET = FALSE; //FALSE - Aux Out
	}
	else
	{
		B_AUX_DET = TRUE; //TURE -Aux In
	}
}
#endif
#endif

#if defined(AMP_1_1CH_WORKAROUND) || defined(TAS5806MD_ENABLE) //2022-10-17_3
void TAS5806MD_Init_After_Clk_Detect(void)
{
	Clk_detect_uCount = 0;
}
#ifdef TAS5806MD_ENABLE
uint32_t TAS5806MD_CLK_Detect_Count(void) //2022-12-06
{
	return Clk_detect_uCount;
}
#endif
#endif

#if (defined(ADC_INPUT_ENABLE) || defined(ADC_INTERRUPT_INPUT_ENABLE))&& defined(TAS5806MD_ENABLE) //2022-11-22_1 : Implemented Attenuator Action
int8_t uAttenuator_Vol_Value(void)
{
	int8_t Attenuator_Volume;
#if defined(ADC_DEBUG_MSG) || defined(ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG)
	_DBG("\n\ruAttenuator_Vol = 0x"); _DBH(uAttenuator_Vol);
#endif
#ifdef ADC_VOLUME_STEP_ENABLE
	Attenuator_Volume = uAttenuator_Vol; //0 ~ 20
#else //ADC_VOLUME_STEP_ENABLE
	switch(uAttenuator_Vol)
	{
		case Attenuator_Volume_Low: // -10dB
			Attenuator_Volume = 20;
		break;
		
		case Attenuator_Volume_Mid: // -6dB
			Attenuator_Volume = 12;
		break;

		case Attenuator_Volume_High: // 0dB
			Attenuator_Volume = 0;
		break;
	}
#endif //ADC_VOLUME_STEP_ENABLE
#if defined(ADC_DEBUG_MSG) || defined(ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG)
	_DBG("\n\rAttenuator_Volume = 0x"); _DBH(Attenuator_Volume);
#endif

	return Attenuator_Volume;
}
#endif

void SW_Reset(void) //2023-02-21_7 : After reboot, Slave SPK has Audio NG issue. So, Added more delay.
{
#ifdef COMMON_DEBUG_MSG
	_DBG("\n\rSW Reset");
#endif
#ifdef TAS5806MD_ENABLE //2023-03-14_2 : Under TWS first connection to avoid pop noise, we need to make mute on(w/o MUTE LED) before reboot.
	HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN
	delay_ms(20);
	HAL_GPIO_ClearPin(PF, _BIT(3)); //+14V_DAMP_SW_1
	delay_ms(20);
	HAL_GPIO_ClearPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
	delay_ms(1000);
#else //TAS5806MD_ENABLE
	HAL_GPIO_ClearPin(PF, _BIT(3)); //+14V_DAMP_SW_1
	delay_ms(30);
	HAL_GPIO_ClearPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
	delay_ms(30);	
	HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN
	delay_ms(1000);
#endif //TAS5806MD_ENABLE

	HAL_SCU_SetResetSrc(RST_SWRST, ENABLE);
	SCU->SCR |= (0x9EB30000|SCU_SCR_SWRST_Msk);
}

#if defined(TIMER21_LED_ENABLE) || defined(TIMER12_13_LONG_KEY_ENABLE)
/**********************************************************************
 * @brief		All_Timer_Off
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void All_Timer_Off(void)
{
#ifdef TIMER12_13_LONG_KEY_ENABLE
	TIMER12_Periodic_Mode_Run(FALSE, FALSE);
	TIMER13_Periodic_Mode_Run(FALSE, Timer13_BT_Pairing_Key);
#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
	TIMER13_Periodic_Mode_Run(FALSE, Timer13_Factory_Reset_Key);
#endif
#endif
#ifdef TIMER21_LED_ENABLE
	TIMER21_Periodic_Mode_Run(FALSE);
#endif
}
#endif

#ifdef AUX_INPUT_DET_ENABLE
/**********************************************************************
 * @brief		Aux In Exist
 * @param[in]	None
 * @return 		None
 **********************************************************************/
Bool Aux_In_Exist(void)
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode Master_Slave;

	Master_Slave = Get_Cur_Master_Slave_Mode();
#endif

#ifdef AUX_INPUT_DET_DEBUG
	_DBG("\n\rAux_In_Exist: ");
	_DBH(B_AUX_DET);
#endif
#ifndef MASTER_MODE_ONLY
	if(Master_Slave == Switch_Slave_Mode)
		return FALSE; //Under Slave mode, it ignore AUX mode.
	else
#endif
#ifndef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 after implementing aux detection check //2022-10-17
	{
		if(B_Auto_AUX_Mode)
			return FALSE;
		else
			return TRUE;
	}
#else
#ifdef USEN_BAP //2023-02-06_1 : If we use AUX_DETECT_INTERRUPT_ENABLE, we need to turn off BT LED under Aux Only Mode.
	if(HAL_GPIO_ReadPin(PA) & (1<<0)) //High : Aux Fix - Always Aux In /Low : Auto - Need to check B_AUX_DET
		return TRUE;
	else
#endif
		return B_AUX_DET; //Input(Aux Detec Pin) : TRUE -Aux In / FALSE - Aux Out
#endif
}
#endif

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
#if defined(SWITCH_BUTTON_KEY_ENABLE) && (defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE))
	Remocon_EQ_Key_Action(EQ_NORMAL_MODE);
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)

#ifdef MB3021_ENABLE
#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
	MB3021_BT_Delete_Paired_List_All(TRUE);
#else //BT_ALWAYS_GENERAL_MODE
	MB3021_BT_Delete_Paired_List_All();
#endif //BT_ALWAYS_GENERAL_MODE
#endif
}

#ifdef USEN_BT_SPK
/**********************************************************************
 * @brief		Make all value as initial value
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#ifdef USEN_BAP //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
void Init_Value_Setting(Bool B_boot)
#else
void Init_Value_Setting(void)
#endif
{
#ifdef COMMON_DEBUG_MSG
	_DBG("\n\rInit_Value_Setting(void)");
#endif
#ifdef USEN_BAP //2023-04-07_3
#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01
	if(!B_boot)
		TIMER20_Amp_error_flag_Stop();
#endif
#endif
#ifdef AUX_INPUT_DET_ENABLE
#ifndef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 //2022-10-17_1 : Implemented Auto Aux Switch(PA0)
	if(HAL_GPIO_ReadPin(PA) & (1<<0)) //Input(Auto Aux) : High - Aux / Low - BT //Modified Master/Slave Switch & Aux/BT Switch to work in inverse //2022-12-20_2
		B_Auto_AUX_Mode = FALSE; //B_Auto_AUX_Mode = TRUE;
	else
		B_Auto_AUX_Mode = TRUE; //B_Auto_AUX_Mode = FALSE;
#else //USEN_BAP & BSP-01T
#ifdef USEN_BAP
	B_AUX_DET = FALSE; //FALSE - Aux Out //2023-04-12_1 : We need to make FALSE because HW make 5sec delay to keep Aux In(Low) even though there is no Aux In after DC In.
#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) && defined(TIMER20_COUNTER_ENABLE) //2023-04-12_1
	TIMER20_aux_detection_flag_start();
#endif
#else //USEN_BAP
	if(HAL_GPIO_ReadPin(PC) & (1<<3)) //Input(Aux Detec Pin) : High -Aux Out / Low -Aux In
	{
		B_AUX_DET = FALSE; //FALSE - Aux Out
	}
	else
	{
		B_AUX_DET = TRUE; //TURE -Aux In
	}
#endif //USEN_BAP
#if defined(USEN_BAP) && defined(MB3021_ENABLE) //2023-04-06_4 : Under Power Off, when user changed Aux Auto/Fixed Switch and then power on using rotary key, we need to reflect this setting.
	if(B_boot)
	{
		if(HAL_GPIO_ReadPin(PA) & (1<<0)) //High : Aux Fix - Always Aux In /Low : Auto - Need to check B_AUX_DET
			B_Auto_AUX_Mode = FALSE;
		else
			B_Auto_AUX_Mode = TRUE;
	}
	else
	{
		if(HAL_GPIO_ReadPin(PA) & (1<<0)) //High : Aux Fix - Always Aux In /Low : Auto - Need to check B_AUX_DET
		{
			if(B_Auto_AUX_Mode)
			{
				Set_MB3021_BT_Module_Source_Change();
				B_Auto_AUX_Mode = FALSE;
			}
		}
		else
		{
			if(!B_Auto_AUX_Mode)
			{
				Set_MB3021_BT_Module_Source_Change();
				B_Auto_AUX_Mode = TRUE;
			}
		}
	}
#endif //#ifdef USEN_BAP //2023-04-06_4
#endif
#endif //AUX_INPUT_DET_ENABLE
#ifdef USEN_BAP
	if(HAL_GPIO_ReadPin(PA) & (1<<1)) //EQ BSP(High) //2023-05-04_1 : Need to make init for EQ NORMAL/EQ BSP position.
		IsEQ_BSP = 0x00; //EQ NORMAL(Low)
	else
		IsEQ_BSP = 0x01; //EQ BSP(High)
#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE)
	Remocon_BSP_NORMAL_Mode_Switch_Action(); //2023-03-27_4
#endif
#else //USEN_BAP
#if defined(SWITCH_BUTTON_KEY_ENABLE) && (defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE))
	Remocon_Mode_Key_Action(); //To set current mode(STEREO/LR Mode) when Power On
#endif
#endif //USEN_BAP
#ifdef KEY_CHATTERING_EXTENSION_ENABLE //2023-04-05_1 : Move to here for initializing for static variable to fix switch action
#ifndef USEN_BAP
	if(HAL_GPIO_ReadPin(PA) & (1<<0)) //STEREO Mode
		IsStereo = 0x01;
	else //LR Mode
		IsStereo = 0x00;

	if(HAL_GPIO_ReadPin(PA) & (1<<1)) //Master Mode
		IsMaster = 0x01;
	else //Slave Mode
		IsMaster = 0x00;
#endif
#endif
}
#endif

#ifdef WATCHDOG_TIMER_RESET
/**********************************************************************
 * @brief		SysTick handler sub-routine (1ms)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SysTick_Handler_IT (void) 
{
	if(msec)msec--;
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
#ifdef USEN_BAP //2023-05-16_1
	wdtCfg.wdtTmrConst = (7812*60)/2; 	// 30s //wdtCfg.wdtTmrConst = (7812*20)/2; 	// 10s
	wdtCfg.wdtWTmrConst = 7812*60; 		//60s //wdtCfg.wdtWTmrConst = 7812*20; 		//20s
#else
	wdtCfg.wdtTmrConst = (7812*60)/2; 	// 30s    //7812/2; 		// 0.5s    @31250
	wdtCfg.wdtWTmrConst = 7812*60; 		//60s	 	//7812; 		// 1s
#endif

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
#ifdef ADC_INPUT_ENABLE
#ifndef ADC_INTERRUPT_INPUT_ENABLE
	uint8_t uCurVolLevel = 0;
	static uint32_t ADC3_Value = 0xffffffff, ADC3_Value_bk = 0xffffffff;
	static uint8_t uSelect_Ch = 0;
	volatile Bool B_Update, B_Update1; //2023-02-06_3 
#endif
#ifdef TAS5806MD_ENABLE
	static uint32_t ADC2_Value = 0xffffffff, ADC2_Value_bk = 0xffffffff; //2023-02-08_1 : make Attenuator GAP
#endif
	static uint32_t uCount1 = 0;
#ifdef ADC_VOLUME_STEP_ENABLE
#ifndef ADC_INTERRUPT_INPUT_ENABLE
	int i;
	uint8_t ADC_Level_Min, ADC_Level_Max;
#endif
	static uint8_t uCurVolLevel_bk = 0xff, uCurAttenuator = 0xff; //2023-02-08_1 : make Attenuator GAP
#endif
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uFlash_Read_Buf1[FLASH_SAVE_DATA_END];
#endif
#if defined(TOUCHKEY_ENABLE) || defined(BT_MODULE_ENABLE) || defined(ADC_INPUT_ENABLE)
	e_task task;
#endif
#if 0//def FLASH_NOT_SAVE_POWER_MODE_SLAVE	//2023-02-06_2 : We don't need this statements
		Switch_Master_Slave_Mode mode;
	
		mode = Get_Cur_Master_Slave_Mode();
#endif

	/*Configure menu prinf*/
	DEBUG_MenuPrint();
#ifdef WATCHDOG_TIMER_RESET
	WDT_Configure();
	SysTick_Configure(); //2023-05-16_1 : Implemented WDT Reset
#endif

	/*Configure port peripheral*/
	GPIO_Configure();
#ifdef UART_10_ENABLE
	Serial_Open(SERIAL_PORT10, Serial_Get_Data_Intterupt_Callback);
#endif
#ifdef SPI_11_ENABLE
	SPI_Open(SERIAL_PORT11, SPI_Get_Data_Interrupt_Callback);
#endif
#ifdef BT_SPK_GPIO_ENABLE
#ifdef BT_SPK_TACT_SWITCH //PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
	EXIT_Configure2();
#endif
	EXIT_Configure();
#endif
#if defined(USEN_BAP) && !(BT_SPK_TACT_SWITCH) //Implemented Interrupt Port E for BT Key(PE7) //2022-10-11_2
	EXIT_PortE_Configure();
#endif
#ifdef SOUND_BAR_GPIO_ENABLE
	EXIT_Configure1();
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
	EXTI_PortA_Configure();
#endif
#ifdef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 //2022-10-17_1 : Disable not used fuctions
#ifdef AUX_INPUT_DET_ENABLE
	EXTI_PortC_Configure();
#endif
#endif
#ifdef USEN_GPIO_OTHERS_ENABLE //Use External INT for Switchs and Button Keys - PF0 / PF5(AMP_ERROR input)
	EXIT_PortF_Configure();
#endif
#ifdef I2C_0_ENABLE
	/*ADC Configure*/
	I2C_Configure();
#endif
#ifdef UART_10_ENABLE
	/*UART USART10 Configure*/
	Serial_Init(SERIAL_PORT10, 115200);
#endif
#ifdef SPI_11_ENABLE
	SPI_Init(SERIAL_PORT11, 1000000); // 1M Speed
#endif
#ifdef TIMER21_LED_ENABLE
	TIMER21_Configure();
#endif
#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(TIMER20_COUNTER_ENABLE)
#ifdef REMOCON_TIMER20_CAPTURE_ENABLE
	/* Timer20 capture Configure*/
	TIMER2n_Configure();
#else
	TIMER20_Configure();
#endif
#endif
#ifdef TOUCHKEY_ENABLE	
	TouchKey_Init();
#endif
#ifdef TIMER30_LED_PWM_ENABLE
	/*TIMER3n Configure*/
	TIMER30_Configure();
	TIMER30_PWMRun();
#endif //TIMER30_LED_PWM_ENABLE
#ifdef TIMER1n_LED_PWM_ENABLE
	TIMER1n_Configure(Timer1n_10, 0);
	TIMER1n_Configure(Timer1n_11, 0);
#endif
#ifdef TIMER12_13_LONG_KEY_ENABLE
	TIMER12_Configure();
	TIMER13_Configure();
	//TIMER13_Periodic_Mode_Run(TRUE); //This function is called in Long Key Start
#endif
#ifdef ADC_INPUT_ENABLE	/*ADC Configure*/
	ADC_Configure();
#endif
#ifdef ADC_INTERRUPT_INPUT_ENABLE /*ADC Interrupt Configure*/ //2023-02-06_3 
	ADC_Intterupt_Configure();
#endif

	/* Enable IRQ Interrupts */
	__enable_irq();

#ifdef WATCHDOG_TIMER_RESET //2023-05-16_1
	/*WDT Reset Start*/
	WDT_ResetRun();
#endif
#ifdef TIMER20_COUNTER_ENABLE //Always On Counter !!!
	TIMER20_Periodic_Mode_Run(TRUE);
#endif
#ifdef TIMER1n_LED_PWM_ENABLE
	TIMER1n_PWMRun(Timer1n_10);
	TIMER1n_PWMRun(Timer1n_11);
#endif
#ifdef ADAU1452_ENABLE
	delay_ms(500); //A Reset IC makes reset done status after 500ms even though MCU is already waked-up
	ADAU1452_DSP_Init(); //When the signal is recevied, The init is acceptable??
#endif
#ifdef SABRE9006A_ENABLE
	Audio_Dac_Init();
#endif
#ifdef TAS3251_ENABLE
	AMP_RESET(TRUE);
#ifdef ESTEC_BOARD
	TAS3251_DSP_Amp_Init(TAS3251_DEVICE_ADDR_H);
#ifdef AMP_1_1CH_WORKAROUND
	TAS3251_DSP_Amp_Mute(TAS3251_DEVICE_ADDR_H, TRUE);
#endif
#endif
	TAS3251_DSP_Amp_Init(TAS3251_DEVICE_ADDR_L);
#ifdef AMP_1_1CH_WORKAROUND
	TAS3251_DSP_Amp_Mute(TAS3251_DEVICE_ADDR_L, TRUE);
	AMP_RESET(FALSE);
#endif
#endif
#ifdef TAS5806MD_ENABLE
	delay_ms(300); //Need delay 300ms to guarantee I2C working with TI AMP //2022-11-24
#if defined(USEN_BT_SPK_TI) //2023-06-09_2
	TAS5806MD_Amp_Init(TAS5806MD_Init_Mode_Power_On);
#elif defined(FLASH_SELF_WRITE_ERASE)
	TAS5806MD_Amp_Init(TRUE);
#else
	TAS5806MD_Amp_Init();
#endif
#endif
#ifdef AD82584F_ENABLE
#ifdef FLASH_SELF_WRITE_ERASE
	AD82584F_Amp_Init(TRUE);
#else
	AD82584F_Amp_Init();
#endif
#endif
#ifdef BT_MODULE_ENABLE
#if defined(F1DQ3007_ENABLE) || defined(F1DQ3021_ENABLE)
	F1M22_BT_Module_Init();
#else //MB3021_ENABLE
#ifdef MB3021_ENABLE
#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) //2022-12-23 : To set BBT_Pairing_Key_In for GIA_PAIRING
#ifndef MASTER_MODE_ONLY
	if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
	{
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf1, FLASH_SAVE_DATA_END);

		if(uFlash_Read_Buf1[FLASH_SAVE_GENERAL_MODE_KEEP] == 0x01/* && uFlash_Read_Buf1[FLASH_SAVE_DATA_PDL_NUM] != 0x01*/) //Under Last connection mode, we don't need to use this statement
		{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
			_DBG("\n\rInit working ... Set General Mode after reading Flash Data !!!");
#endif
			BBT_Pairing_Key_In = TRUE; //Implemented GIA_PAIRING //To use, BT Pairing Key In is True only.
		}
#ifdef BT_ALWAYS_GENERAL_MODE //2023-01-31_1 : Implemented Alway General mode and do not use USEN mode.
		else
		{
			FlashSaveData(FLASH_SAVE_GENERAL_MODE_KEEP, 1); //Save GENERAL MODE Status(GENERAL Mode/GIA Mode) to Flash
		}
	
		BBT_Pairing_Key_In = TRUE;	
#endif
	}
#ifndef MASTER_MODE_ONLY
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_2 : To disable BLE_VOLUME_KEY using BLE DATA from Master under BAP slave when Master & Slave are BAP
	else
	{
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf1, FLASH_SAVE_DATA_END);

		if(uFlash_Read_Buf1[FLASH_SAVE_SLAVE_LAST_CONNECTION] == 0x02) //Master is BAP-01
			B_Master_Is_BAP = TRUE; //2023-01-09_2 : According to Last connection information, Master is BAP-01
		else
			B_Master_Is_BAP = FALSE; //Master is not BAP-01
	}
#endif //#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#endif //#ifndef MASTER_MODE_ONLY
#endif //#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE)
	MB3021_BT_Module_Init(FALSE);
#endif //MB3021_ENABLE
#endif //defined(F1DQ3007_ENABLE) || defined(F1DQ3021_ENABLE)
#endif //BT_MODULE_ENABLE
#ifdef USEN_BT_SPK
#ifdef USEN_BAP
	Init_Value_Setting(TRUE); //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
#else
	Init_Value_Setting();
#endif
#endif
#if defined(INPUT_KEY_SYNC_WITH_SLAVE_ENABLE) && defined(MB3021_ENABLE)
	MB3021_BT_Module_Input_Key_Init();
#endif
#ifdef COMMON_DEBUG_MSG
	_DBG("\n\rAll Init Done !!!");
#endif

#if defined(TOUCHKEY_ENABLE) || defined(BT_MODULE_ENABLE)
#ifdef TOUCHKEY_ENABLE
	task = TASK_TOUCH;
#else
	task = TASK_UART;
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION //Set Mute Setting and Power On/Off setting after Power On
	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf1, FLASH_SAVE_DATA_END);
#ifdef USEN_BAP //Implemented Power Key Feature //2022-10-07_3
	if((HAL_GPIO_ReadPin(PA) & (1<<6))
#if 0//2023-03-14_5 : Under Slave Mode, If User plug-in power cable when rotary power off and then power Plug-out, LED is turned on even though rotary power off. //def FLASH_NOT_SAVE_POWER_MODE_SLAVE //2023-02-06_2 : Changed "if" statemnet condition
		&&	(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
		) //Power Off
#else //USEN_BAP
	if(uFlash_Read_Buf1[FLASH_SAVE_DATA_POWER] == 0
#if 0//2023-03-10_4 : Keep Power State //def FLASH_NOT_SAVE_POWER_MODE_SLAVE //2023-02-06_2 : Changed "if" statemnet condition
		&&	(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
		) //Power Off
#endif //USEN_BAP
		Remocon_Power_Key_Action(FALSE, FALSE, TRUE); //Do not send BLE Data to Slave in here !!! Because we shall finish BT init(upto  INFORM_HOST_MODE)
	else //Power On - 0xff or 0x01
	{
#ifdef TIMER21_LED_ENABLE
		Set_Status_LED_Mode(STATUS_POWER_ON_MODE); //Should be called this funciton after I2C/Amp init interrupt complete
#endif
		if(
#ifdef AD82584F_USE_POWER_DOWN_MUTE
			!IS_Display_Mute() && 
#endif
			uFlash_Read_Buf1[FLASH_SAVE_DATA_MUTE] != 0 && uFlash_Read_Buf1[FLASH_SAVE_DATA_MUTE] != 0xff)//Set Mute Setting after Power On
		{
#ifdef _DBG_FLASH_WRITE_ERASE
			_DBG("\n\rPower On : Mute setting with flash data ===");
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
			AD82584F_Amp_Mute(TRUE, TRUE); //Power Mute On
#else //TAS5806MD_ENABLE
			TAS5806MD_Amp_Mute(TRUE, TRUE); //Power Mute On
#endif //AD82584F_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
		}
		else
		{
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
			AD82584F_Amp_Mute(TRUE, FALSE); //Power Mute On
#else //TAS5806MD_ENABLE
			TAS5806MD_Amp_Mute(TRUE, FALSE); //Power Mute On
#endif //AD82584F_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
		}
	}
#else //FLASH_SELF_WRITE_ERASE_EXTENSION
#ifdef TIMER21_LED_ENABLE
		Set_Status_LED_Mode(STATUS_POWER_ON_MODE); //Should be called this funciton after I2C/Amp init interrupt complete
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
		AD82584F_Amp_Mute(TRUE, FALSE); //Power Mute On
#else //TAS5806MD_ENABLE
		TAS5806MD_Amp_Mute(TRUE, FALSE); //Power Mute On
#endif //AD82584F_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#endif //FLASH_SELF_WRITE_ERASE_EXTENSION

#ifdef LED_POWER_CONTROL_ENABLE
#ifdef USEN_BAP //2023-02-06_2 : Turn on all LED on Power Init
	HAL_GPIO_SetPin(PD, _BIT(5)); //LED POWER CONTROL - ON //To Do !!! - Need to use this after separating LED Power from Button Power
#else //USEN_BAP
	HAL_GPIO_SetPin(PD, _BIT(0)); //LED POWER CONTROL - ON	- Need to use this after separating LED Power from Button Power
#endif
#endif

#if defined(TIMER20_COUNTER_ENABLE) && defined(AUTO_ONOFF_ENABLE)
	TIMER20_auto_power_flag_Start();
#endif
#ifdef AUTO_VOLUME_LED_OFF
	TIMER20_auto_volume_led_off_flag_Start();
#endif

	while (1)
	{
		switch (task)
		{
#ifdef TOUCHKEY_ENABLE
			case TASK_TOUCH :
				Do_taskTouch();
				break;
#endif
#ifdef BT_MODULE_ENABLE
			case TASK_UART :
				Do_taskUART();
				break;
#endif
			case TASK_GPIO:
#if 0//defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE) //2022-11-15_1 //2022-12-06
				if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
					Clk_detect_uCount = 0xffffffff;
#endif
#if defined(AMP_1_1CH_WORKAROUND) || defined(TAS5806MD_ENABLE) //2022-10-05
				if(Clk_detect_uCount == 0x7fff) //0xffff = 1s, 0xffff/2(0x7fff) = 500ms
				{
#ifdef TAS5806MD_ENABLE //When Power On after Plug in, we need to init again after I2S LRCK detection. This is TAS5806 spec. //2022-10-05
					if(TAS5806MD_Amp_Detect_FS(FALSE)) //2023-03-10_2 : Changed concept of AMP clock checking to recover audio noise when I2S clock is not stable. if(TAS5806MD_Amp_Detect_FS(TRUE)) //2022-10-17_2
					{
						Clk_detect_uCount++; //2023-03-10_2 : Recheck after 500ms //Complete //2022-10-05
					}
					else
					{
						Clk_detect_uCount = 0; //Retry
					}
#else
					TAS3251_DSP_Amp_Dect_FS();
#endif
				}
#ifdef TAS5806MD_ENABLE //2023-03-10_2 : Changed concept to recover audio noise when I2S clock is not stable
				else if(Clk_detect_uCount == (0xffff)) //After 500ms
				{
					if(TAS5806MD_Amp_Detect_FS(TRUE)) //2023-03-10_2 : Changed concept of AMP clock checking to recover audio noise when I2S clock is not stable. if(TAS5806MD_Amp_Detect_FS(TRUE)) //2022-10-17_2
					{
						Clk_detect_uCount = 0xffffffff; //Complete //2022-10-05
					}
					else
					{
						Clk_detect_uCount = 0; //Retry
					}
				}
#endif
				else
				{
					if(Clk_detect_uCount != 0xffffffff)
						Clk_detect_uCount++;
				}
#endif
#ifdef TOUCHKEY_ENABLE
				Do_TaskGPIO();
#endif
				break;

#ifdef ADC_INPUT_ENABLE
			case TASK_ADC:
#if defined(USEN_BAP) && !defined(MASTER_MODE_ONLY) //2022-12-13 To Slave volume sync with Master Volume, Changed SW
				if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
					break;
#endif
				if(uCount1 == (0x1999/4)) //2023-04-12_3 : 50ms setting //2023-02-06_3 : Need to fast action for Volume -100ms setting//0xffff = 1s, 0xffff/2(0x7fff) = 500ms/0x3332 = 200ms / 0x1999 = 100ms
				{
#ifndef ADC_INTERRUPT_INPUT_ENABLE
					if(uSelect_Ch%2)
					{
						ADC3_Value = ADC_PollingRun(3);

						if(ADC3_Value_bk != ADC3_Value)
						{
#ifdef ADC_INPUT_DEBUG_MSG
							_DBG("\n\r === Master Volume ADC = 0x");
							_DBH32(ADC3_Value);
#endif
#ifdef ADC_VOLUME_STEP_ENABLE
#ifdef ADC_VOLUME_64_STEP_ENABLE
							for(i=1;i<65;i++)
#else //ADC_VOLUME_50_STEP_ENABLE
							for(i=1;i<51;i++)
#endif //ADC_VOLUME_64_STEP_ENABLE
							{
								B_Update = FALSE; //2023-02-06_3 	
#ifdef ADC_VOLUME_64_STEP_ENABLE
								ADC_Level_Min = (i-1)*4; //0 4 8
								ADC_Level_Max = (i*4)-1; //3 7 11 //2023-02-06_3 : To make ADC Gap
#else //ADC_VOLUME_50_STEP_ENABLE //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
								if(i==1)
									ADC_Level_Min = 0;
								else
									ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

								if(i==50)
									ADC_Level_Max = 255;
								else
									ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253
#endif //ADC_VOLUME_64_STEP_ENABLE

								if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value))
								{
#ifdef ADC_VOLUME_64_STEP_ENABLE
									ADC_Level_Min = (i-1)*4; //0 4 8
									ADC_Level_Max = (i*4)-2; //2 6 10
#else //ADC_VOLUME_50_STEP_ENABLE //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
									if(i==1)
										ADC_Level_Min = 0;
									else
										ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

									if(i==50)
										ADC_Level_Max = 255;
									else
										ADC_Level_Max = (i*5)-1; //4 9 14 19 ... 244 249~253
#endif //ADC_VOLUME_64_STEP_ENABLE


									if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value)) //2023-02-08_3 : Added additional code for Volume GAP
									{
#ifdef ADC_INPUT_DEBUG_MSG
										_DBG("\n\r ++++ Volume ADC Valid Value !!!!");
										_DBG("\n\r === Volume ADC Level Step = ");
										_DBD(i);
#endif
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
										uCurVolLevel = 64 - i;
#else //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
										uCurVolLevel = 50 - i;
#endif
#ifdef ADC_INPUT_DEBUG_MSG
										_DBG("\n\rVolume Level cur - cur_bk = ");
										_DBD(uCurVolLevel);
										_DBG(" - ");
										_DBD(uCurVolLevel_bk);
#endif								

										B_Update = TRUE; //2023-02-06_3 
									}
									else //2023-02-06_3 : Do not update cur volume level and current ADC value is not valid
									{
										if(uCurVolLevel > uCurVolLevel_bk)
										{
											if((uCurVolLevel - uCurVolLevel_bk) > 1)
												B_Update = TRUE;
											else
												B_Update = FALSE;
										}
										else
										{
											if((uCurVolLevel_bk - uCurVolLevel) > 1)
												B_Update = TRUE;
											else
												B_Update = FALSE;
										}

#ifdef ADC_INPUT_DEBUG_MSG
										if(B_Update)
											_DBG("\n\r ++++ Volume ADC Gap - Valid Value even though if condition is FALSE");
										else
											_DBG("\n\r ++++ ADC Gap - Invalid Value");
#endif
									}

									break;
								}
							}

							if(B_Update) //2023-02-06_3
							{
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
								uCurVolLevel = 64 - i; //0 ~ 63(64 Step / 0 - MAX)
#else //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
								uCurVolLevel = 50 - i;
#endif
#ifdef TAS5806MD_ENABLE
								if(uCurVolLevel_bk != uCurVolLevel)
								{
#ifdef ADC_INPUT_DEBUG_MSG
									_DBG("\n\r === Volume Level Setting = ");
									_DBD(uCurVolLevel);
#endif
									uCurVolLevel_bk = uCurVolLevel;
									TAS5806MD_Amp_Volume_Set_with_Index(uCurVolLevel, FALSE, TRUE);
									//uCurVolLevel = 15 - ADC3_Value; //15 Step, Inverse Value, The integer value need to match with (VOLUME_LEVEL_NUMER)
#ifdef FLASH_SELF_WRITE_ERASE //2023-03-02_3 : BAP-01 do not use flash data to set volume level when power on.
									//FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel);
#endif
								}
								
								B_Update = FALSE;
#endif
							}
#else //ADC_VOLUME_STEP_ENABLE
#ifdef TAS5806MD_ENABLE
							TAS5806MD_Amp_Volume_Set_with_Index(ADC3_Value, TRUE, TRUE);
							
							uCurVolLevel = 15 - ADC3_Value; //15 Step, Inverse Value, The integer value need to match with (VOLUME_LEVEL_NUMER)
#ifdef FLASH_SELF_WRITE_ERASE //2023-03-02_3 : BAP-01 do not use flash data to set volume level when power on.
							//FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel);
#endif
#endif
#endif //ADC_VOLUME_STEP_ENABLE

							ADC3_Value_bk = ADC3_Value;
						}
					}
					else
#endif //ADC_INTERRUPT_INPUT_ENABLE
					{
//2023-02-08_1 : make Attenuator GAP
#ifdef TAS5806MD_ENABLE //2022-11-22_1
#ifdef ADC_VOLUME_STEP_ENABLE //Attenuator action is inversed. So fixed it. //2023-01-05_2						
						ADC2_Value = ADC_PollingRun(2); //0x00 ~ 0xA0(160)
						
						if(ADC2_Value != ADC2_Value_bk)
						{
#ifdef ADC_INPUT_DEBUG_MSG
							_DBG("\n\r ++++ ADC2 =");
							_DBD(ADC2_Value);
#endif
							for(i=0;i<21;i++) //0 ~ 21 is correct but we'll use that 21 is invalid ADC value.
							{				
								B_Update1 = FALSE;
								
								if(i == 0)
								{
									ADC_Level_Min = 0 ; //0 // 4 17 30
									ADC_Level_Max = 3; //3 //16 29 42
								}
								else
								{
									ADC_Level_Min = ((i-1)*13)+4 ; //0 //4 17 30 ... 251

									if(i == 20)
										ADC_Level_Max = 253; //3 //16 29 42 ... 253
									else
										ADC_Level_Max = (i*13)+3; //3 //16 29 42 ... 253
								}
								
								if((ADC2_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC2_Value))
								{
#ifdef ADC_INPUT_DEBUG_MSG
									_DBG("\n\r ++++ Attenuator ADC Gap Check !!!!");
#endif
									if(i == 0)
									{
										ADC_Level_Min = 0 ; //0 4 17 30
										ADC_Level_Max = 2; //2 15 28 41
									}
									else
									{
										ADC_Level_Min = ((i-1)*13)+4 ; //0 4 17 30 ... 251
										
										if(i == 20)
											ADC_Level_Max = 253; //3 //16 29 42 ... 253
										else
											ADC_Level_Max = (i*13)+2; //2 15 28 41 ... 253
									}

									if((ADC2_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC2_Value)) //2023-02-08_2 : Added additional code for Attenuator GAP
									{
										B_Update1 = TRUE;
										uCurAttenuator = Convert_ADC_To_Attenuator(i);
#ifdef ADC_INPUT_DEBUG_MSG
										_DBG("\n\r ++++ Attenuator ADC Valid Value !!!!");
										_DBG("\n\r === Attenuator ADC Level Step = ");
										_DBD(i);
#endif
									}
									else
									{
										uCurAttenuator = Convert_ADC_To_Attenuator(i);

										if(uCurAttenuator > uAttenuator_Vol)
										{
											if((uCurAttenuator - uAttenuator_Vol) > 1)
												B_Update1 = TRUE;
											else
												B_Update1 = FALSE;
										}
										else
										{
											if((uAttenuator_Vol - uCurAttenuator) > 1)
												B_Update1 = TRUE;
											else
												B_Update1 = FALSE;
										}

#ifdef ADC_INPUT_DEBUG_MSG
										if(B_Update1)
											_DBG("\n\r ++++ Attenuator ADC Gap - Valid Value even though if condition is FALSE");
										else
											_DBG("\n\r ++++ ADC Gap - Invalid Value ");

										_DBD(uCurAttenuator);
#endif
									}

									break;
								}
							}
						}

						ADC2_Value_bk = ADC2_Value;

						if(B_Update1)
						{							
							if(uAttenuator_Vol != uCurAttenuator)
							{
#ifdef ADC_INPUT_DEBUG_MSG
								_DBG("\n\r === Attenuator Volume = 0x");
								_DBH32(uCurAttenuator); //0(Max) ~ 20(Min)
#endif
								uAttenuator_Vol = uCurAttenuator; //Should be located in front of TAS5806MD_Amp_Volume_Set_with_Index() function.
#ifdef TAS5806MD_ENABLE
#ifdef ADC_VOLUME_STEP_ENABLE
								TAS5806MD_Amp_Volume_Set_with_Index(uCurVolLevel_bk, FALSE, TRUE);
#else
								TAS5806MD_Amp_Volume_Set_with_Index(ADC3_Value_bk, TRUE, TRUE);
#endif
#endif
							}

							B_Update1 = FALSE;
						}
#endif //ADC_VOLUME_STEP_ENABLE
#endif //TAS5806MD_ENABLE
					}
#ifndef ADC_INTERRUPT_INPUT_ENABLE
					uSelect_Ch++;
#endif
					uCount1 = 0; //Re-Do
				}
				else
				{
					uCount1++;
				}
			break;
#endif
			default :
				break;
		}

		task++;
		if (task == TASK_MAX)
		{
#ifdef TOUCHKEY_ENABLE
			task = TASK_TOUCH;
#else
			task = TASK_UART;
#endif
		}
		if (task > TASK_MAX)
		{
			break;
		}
	}
#else
	while(1)
	{
   /* Infinite loop */
		//UART_InterruptRun();
	}
#endif
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

#ifdef BT_SPK_GPIO_ENABLE //PC4 / PD0 : Interrupt Input, PD2 / PD3 : Output

//PD2 - DAC_MUTE_SW : High - Unmute / Low - Mute
void AMP_DAC_MUTE(Bool Mute)
{
	if(Mute)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\rAMP_DAC_MUTE - Mute");
#endif
		HAL_GPIO_ClearPin(PD, _BIT(2));
		Cur_Mute_Status = Mute_Status_Mute;
	}
	else
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\rAMP_DAC_MUTE - Unmute");
#endif
		HAL_GPIO_SetPin(PD, _BIT(2));
		Cur_Mute_Status = Mute_Status_Unmute;
	}
}

//PD3 - RESET_AMP_SW : High - Normal / Low - Reset
void AMP_RESET(Bool Reset)
{
	if(Reset)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\rAMP_RESET - Reset");
#endif
		HAL_GPIO_ClearPin(PD, _BIT(3));
	}
	else
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\rAMP_RESET - Normal");
#endif
		HAL_GPIO_SetPin(PD, _BIT(3));
	}
}

/**********************************************************************
 * @brief		GPIOE Handler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOCD_IRQHandler_IT(void)
{
	uint32_t	 status, status1, clear_bit = 0;
#ifdef GPIO_DEBUG_MSG
	static uint32_t status_bk = 0, status_bk1 = 0;
#endif

	status = HAL_GPIO_EXTI_GetState(PC);
	
#ifdef GPIO_DEBUG_MSG
	if(status_bk != status)
	{
		status_bk = status;
		_DBG("\n\rstatus : ");
		_DBH32(status);
	}
#endif

	if (status & 0x00000300)
	{
		clear_bit = status & 0x00000300;
		HAL_GPIO_EXTI_ClearPin(PC, status&clear_bit);
		
		if(HAL_GPIO_ReadPin(PC) & (1<<4))
		{
			B_FALT_SW = TRUE;
#ifdef GPIO_DEBUG_MSG
			_DBG("\n\rB_FALT_SW - TRUE");
#endif
		}
		else
		{
			B_FALT_SW = FALSE;
#ifdef GPIO_DEBUG_MSG
			_DBG("\n\rB_FALT_SW - FALSE");
#endif
		}
	}

	status1 = HAL_GPIO_EXTI_GetState(PD);

#ifdef GPIO_DEBUG_MSG
	if(status_bk1 != status1)
	{
		status_bk1 = status1;
		_DBG("\n\rstatus1 : ");
		_DBH32(status1);
	}
#endif

	if (status1 & 0x00000003) /* bit 0 : PD0 Falling Edge / bit 1 : PD0 Rising Edge */
	{
		clear_bit = status1 & 0x00000003;
		HAL_GPIO_EXTI_ClearPin(PD, status1&clear_bit);

		if(HAL_GPIO_ReadPin(PD) & (1<<0))
		{
			B_CLIP_OTW_SW = TRUE;
#ifdef GPIO_DEBUG_MSG
			_DBG("\n\rB_CLIP_OTW_SW - TRUE");
#endif
		}
		else
		{
			B_CLIP_OTW_SW = FALSE;
#ifdef GPIO_DEBUG_MSG
			_DBG("\n\rB_CLIP_OTW_SW - FALSE");
#endif
		}
	}
}

/**********************************************************************
 * @brief		EXIT_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void EXIT_Configure(void) //Interrupt edge of both side
{
	HAL_GPIO_EXTI_Config(PC, 4, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PD, 0, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_SCU_LVRCmd(DISABLE);
	
	NVIC_SetPriority(GPIOCD_IRQn, 3);
	NVIC_EnableIRQ(GPIOCD_IRQn);
	
	//HAL_SCU_WakeUpSRCCmd(WAKEUP_GPIOC, ENABLE);
	
	SCU->SMR = 0
	| (SCU_SMR_ROSCAON_Msk)
	| (SCU_SMR_BGRAON_Msk)
	| (SCU_SMR_VDCAON_Msk)
	;
		
}
#endif

#ifdef BT_SPK_TACT_SWITCH //PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
/**********************************************************************
 * @brief		GPIOE Handler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOE_IRQHandler_IT(void) //PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
{
	uint32_t status2 = 0, clear_bit = 0, shift_bit = 0;
	uint8_t key = 0;
	button_status cur_button_status;

	status2 = HAL_GPIO_EXTI_GetState(PE);
	
#ifdef GPIO_DEBUG_MSG
	_DBG("\n\rstatus2 = ");
	_DBH32(status2);
#endif

	if (status2 & 0x0000ffc0) //Just check PE3 / PE4 / PE5 / PE6 / PE7 of PE
	{
		clear_bit = status2 & 0x0000ffc0;
		HAL_GPIO_EXTI_ClearPin(PE, status2&clear_bit);

		shift_bit = 0;
		
		/* bit 0 : PD0 Falling Edge / bit 1 : PD0 Rising Edge *///Just check Rising Edge (Normal : High / Push : Low)
		if(status2 & 0x000000c0) //PE3 : MUTE KEY
		{
			shift_bit = 6;
			key = MUTE_KEY;

			if(status2 == 0x00000040)
				cur_button_status = button_push; //High -> Low
			else //status2 == 0x00000080
				cur_button_status = button_release; //Low -> High
		}
		else if(status2 & 0x00000300) //PE4 : NEXT KEY
		{
			shift_bit = 8;
			key = NUM_1_KEY;

			if(status2 == 0x00000100)
				cur_button_status = button_push; //High -> Low
			else //status2 == 0x00000200
				cur_button_status = button_release; //Low -> High
		}
		else if(status2 & 0x00000c00) //PE5 : PREVIOUS KEY
		{
			shift_bit = 10;
			key = NUM_2_KEY;

			if(status2 == 0x00000400)
				cur_button_status = button_push; //High -> Low
			else //status2 == 0x00000800
				cur_button_status = button_release; //Low -> High
		}
		else if(status2 & 0x00003000) //PE6 : VOLUME DOWN
		{
			shift_bit = 12;
			key = VOL_DOWN_KEY;

			if(status2 == 0x00001000)
				cur_button_status = button_push; //High -> Low
			else //status2 == 0x00002000
				cur_button_status = button_release; //Low -> High
		}
		else if(status2 & 0x0000c000) //PE7 : VOLUME UP
		{
			shift_bit = 14;
			key = VOL_UP_KEY;

			if(status2 == 0x00004000)
				cur_button_status = button_push; //High -> Low
			else //status2 == 0x00008000
				cur_button_status = button_release; //Low -> High
		}
		else
			shift_bit = 0xffffffff; //Invalid

		if(shift_bit != 0xffffffff)
		{
			
			if(cur_button_status == button_push) //Rising Edge 0x10
			{
#ifdef GPIO_DEBUG_MSG
				_DBG("\n\r1. Ready Button key = ");
				_DBH(key);
#endif
			}
			else //cur_button_status = button_release //Falling Edge 0x01
			{
#ifdef GPIO_DEBUG_MSG
				_DBG("\n\r1. Release Button key = ");
				_DBH(key);
#endif
				Send_Remote_Key_Event(key);
			}
		}
	}
}

/**********************************************************************
 * @brief		EXIT_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void EXIT_Configure2(void)
{
  	HAL_GPIO_EXTI_Config(PE, 3, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 4, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 5, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 6, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PE, 7, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_SCU_LVRCmd(DISABLE);
	
	NVIC_SetPriority(GPIOE_IRQn, 3);
	NVIC_EnableIRQ(GPIOE_IRQn);
	
	//HAL_SCU_WakeUpSRCCmd(WAKEUP_GPIOE, ENABLE);		
}
#endif

#ifdef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 //2022-10-17_1 : Disable not used functions
#ifdef AUX_INPUT_DET_ENABLE
/**********************************************************************
 * @brief		GPIOAB_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOCD_IRQHandler_IT2(void)
{
	static uint32_t status = 0, clear_bit = 0;
#ifdef AUX_INPUT_DET_ENABLE
	Bool BRet = FALSE;
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode Master_Slave;

	Master_Slave = Get_Cur_Master_Slave_Mode();
#endif
#endif
	status = HAL_GPIO_EXTI_GetState(PC);
	
#ifdef AUX_INPUT_DET_DEBUG
	_DBG("\n\rstatus = ");
	_DBH32(status);
#endif

	if (status & 0x000000c0) //Just check PC3
	{
		clear_bit = status & 0x000000c0;
				
		/* bit 0 : Falling Edge / bit 1 : Rising Edge */
#ifdef AUX_INPUT_DET_ENABLE //Input(Aux Detec Pin) : High -Aux Out / Low -Aux In
		if(status & 0x00000080) //Rising Edge : High check - Aux Out
		{
#ifdef AUX_CHATTERING_ENABLE
			delay_ms(80); //2023-04-28_3 : Changed Aux detection check delay from 40ms to 80ms.  //For Aux Chattering

			if(HAL_GPIO_ReadPin(PC) & (1<<3)) //PC3 : High - Aux Out
			{
				if(Aux_In_Exist())//if(B_AUX_DET) //Aux In
				{
#if defined(USEN_BAP) && defined(TIMER20_COUNTER_ENABLE) //2023-01-10_3
					TIMER20_aux_detect_check_flag_start();
#else //USEN_BAP
					B_AUX_DET = FALSE; //FALSE - Aux Out

					BRet = TRUE;
#endif //USEN_BAP
#ifdef AUX_INPUT_DET_DEBUG
					_DBG("\n\rB_AUX_DET = FALSE(Aux Out)");
#endif

				}
				else
				{
#ifdef USEN_BAP
					B_AUX_DET = FALSE; //2023-06-19_3
#endif
#ifdef AUX_INPUT_DET_DEBUG
					_DBG("\n\rB_AUX_DET = Already FALSE(Aux Out)");
#endif
				}
			}
			else //Low - Aux In //Invalid value in here
			{
#ifdef AUX_INPUT_DET_DEBUG
				_DBG("\n\rxxxxxx No Aux Out - Actually Aux In");
#endif

#ifdef TIMER20_COUNTER_ENABLE
#ifdef USEN_BAP //2023-01-10_3
				TIMER20_aux_detect_check_flag_stop();
#endif
#ifdef AUTO_ONOFF_ENABLE
				TIMER20_auto_power_flag_Stop();
#endif
#ifdef AD82584F_USE_POWER_DOWN_MUTE
				if(!IS_Display_Mute()) //&& AD82584F_Amp_Get_Cur_CLK_Status()) //When Mute On status, we don't need to mute off. This function is for LED Display
				{
#ifdef AUX_INPUT_DET_DEBUG
					_DBG("\n\r++++++++ Forced Mute Off w/1.5 sec delay");
#endif
					TIMER20_mute_flag_Start();
				}
#endif
#endif //TIMER20_COUNTER_ENABLE
			}
#endif
		}
		else //0x00000040 //Falling Edge : Low check - Aux In
		{
#ifndef MASTER_MODE_ONLY
			if(Master_Slave == Switch_Slave_Mode) //Under Slave, we don't need to set AUX mode
				BRet = FALSE;
			else
#endif
			{
#ifdef AUX_CHATTERING_ENABLE
				delay_ms(80); //2023-04-28_3 : Changed Aux detection check delay from 40ms to 80ms. //For Aux Chattering

				if(!(HAL_GPIO_ReadPin(PC) & (1<<3)) )//PC3 : Low -Aux In
				{
					if(!Aux_In_Exist())
					{
						B_AUX_DET = TRUE; //TURE -Aux In
#ifdef AUX_INPUT_DET_DEBUG
						_DBG("\n\rB_AUX_DET = TRUE(Aux In)");
#endif
						BRet = TRUE;
					}
					else //To recover mute off when user alternate Aux mode and BT mode repeatly
					{
#ifdef USEN_BAP
						B_AUX_DET = TRUE; //2023-06-19_3 : Under BAP-01, we need to set B_AUX_DET value in every interrupt condition. //TURE -Aux In
#endif
#ifdef AUX_INPUT_DET_DEBUG
						_DBG("\n\rB_AUX_DET = Already TRUE(Aux In)");
#endif
#ifdef TIMER20_COUNTER_ENABLE
#ifdef AUTO_ONOFF_ENABLE
						TIMER20_auto_power_flag_Stop();
#endif
#ifdef AD82584F_USE_POWER_DOWN_MUTE
						if(!IS_Display_Mute()) //&& AD82584F_Amp_Get_Cur_CLK_Status()) //When Mute On status, we don't need to mute off. This function is for LED Display
						{
#ifdef AUX_INPUT_DET_DEBUG
							_DBG("\n\r****** Forced Mute Off w/1.5 sec delay");
#endif
							TIMER20_mute_flag_Start();
						}
#endif
#endif
					}
				}
				else //High - Aux out //Invalid value in here
				{
#ifdef AUX_INPUT_DET_DEBUG
					_DBG("\n\rxxxxxx No Aux In - Actually Aux In");
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
					AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
					TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif //AD82584F_ENABLE
#endif
				}
#endif
			}
		}

#ifdef MB3021_ENABLE
		if(BRet)
		{
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUTO_ONOFF_ENABLE) //Fixed Master SPK do not work Auto power off even though No siganl from BT when user remove Aux jack
			TIMER20_auto_power_flag_Start();
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
			MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef USEN_BAP //2023-05-19_1 : Under BAP-01 Aux mode, customer wants to output audio at once when user change BT to Aux.
			if(!Aux_In_Exist())
#endif
			{
#ifdef AD82584F_ENABLE
				AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
				TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif
			}
#endif
#ifdef TWS_MODE_ENABLE
			if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //2022-11-17_2
			{
				if(Aux_In_Exist())
				{
					if(BTWS_LIAC != TWS_Status_Master_Mode_Control && Is_TWS_Master_Slave_Connect() == TWS_Get_Information_Ready)
						MB3021_BT_Module_TWS_Set_Discoverable_Mode(); //2022-11-17_2 : Under Aux mode, TWS Setting
				}
				else
				{
					if(Get_Connection_State() == FALSE)
						MB3021_BT_Module_Set_Discoverable_Mode(); //2022-11-17_2 : Under Non-Aux mode, Discoverable Setting
				}
			}
#endif
#ifdef USEN_BAP //2023-05-19_1 : Under BAP-01 Aux mode, customer wants to output audio at once when user change BT to Aux.
			Set_MB3021_BT_Module_Source_Change_Direct();
#else
			Set_MB3021_BT_Module_Source_Change();
#endif
		}
#endif

		HAL_GPIO_EXTI_ClearPin(PC, status&clear_bit);

#endif //AUX_INPUT_DET_ENABLE
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
#endif //AUX_INPUT_DET_ENABLE
#endif //AUX_DETECT_INTERRUPT_ENABLE

#ifdef MASTER_MODE_ONLY
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
#else //MASTER_MODE_ONLY
Switch_Master_Slave_Mode Get_Cur_Master_Slave_Mode(void)
{
	Switch_Master_Slave_Mode mode;

#ifdef SWITCH_BUTTON_KEY_ENABLE
#ifdef USEN_BAP //Modified Master/Slave Switch to work in inverse //2022-12-20_2
	if(HAL_GPIO_ReadPin(PA) & (1<<1)) //High
	{
		mode = Switch_Slave_Mode;
	}
	else //Low
	{
		mode = Switch_Master_Mode;
	}

#else //USEN_BAP
	if(HAL_GPIO_ReadPin(PA) & (1<<1)) //High
	{
		mode = Switch_Master_Mode;
	}
	else //Low
	{
		mode = Switch_Slave_Mode;
	}
#endif ////USEN_BAP
#else
	mode = Switch_Master_Mode;
#endif
	return mode;
}
#endif //MASTER_MODE_ONLY

Switch_LR_Stereo_Mode Get_Cur_LR_Stereo_Mode(void)
{
	Switch_LR_Stereo_Mode mode;

#ifdef SWITCH_BUTTON_KEY_ENABLE
	if(HAL_GPIO_ReadPin(PA) & (1<<0)) //High
	{
		mode = Switch_Stereo_Mode;
	}
	else //Low
	{
		mode = Switch_LR_Mode;
	}
#else
	mode = Switch_Stereo_Mode;
#endif
	return mode;
}

#ifdef SWITCH_BUTTON_KEY_ENABLE
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
#ifdef VOLUME_KEY_FILTERING
	int ret = 0;
#endif
#ifdef VOLUME_KEY_FILTERING
	static uint8_t key_bk = 0;
	static uint32_t filtering_time = 0, filtering_time_old = 0, filtering_time_very_old = 0;
	
	filtering_time = TIMER20_100ms_Count_Value();
#endif

	status = HAL_GPIO_EXTI_GetState(PA);
	
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
	_DBG("\n\rstatus_A = ");
	_DBH32(status);
#endif

#ifdef SWITCH_BUTTON_KEY_ENABLE //When the state is Power off, Power key is only available
	if(!Power_State())
	{
		if(!(status & 0x00003000)
#ifdef USEN_BAP //2022-10-12_4
			&& !(status & 0x0000C000)
#else
			&& !(status & 0x00000003) && !(status & 0x0000000c) //2023-04-06_3 : LR, MS Switch is acceptable uner power off mode
#endif
			) //PA6 : POWER_Off(short)/POWER_ON(Long)  //Long Key is implmented below if(key == POWER_KEY)
		{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
			_DBG("\n\r3. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
			HAL_GPIO_EXTI_ClearPin(PA, status);

			return;
		}
	}
#endif

#ifdef USEN_BAP
	if (status & 0x0000f30f) //Just check PA0 / PA1 / PA4 / PA6 / PA7 (1111 0011 0000 1111)
	{
		shift_bit = 0xffffffff;

#ifdef KEY_CHATTERING_ENABLE
		delay_ms(KEY_CHATTERING_DELAY_MS+60); //2023-05-04_2 : Under BAP-01, key chattering delay is increased from 20ms to 80ms.
#endif
		/* bit 0 : PA Falling Edge / bit 1 : PA Rising Edge *///Just check Rising Edge (Normal : High / Push : Low)
		if(status & 0x00000003) //PA0 : AUTO_SW //2022-10-17_1
		{
			shift_bit = 0;

			//Both Rising Edge and Falling Edge should work as switch input. so, in both case, the cur_button_status should be button_release.
			if(status & 0x00000001)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<0)) //PA0 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					if(!B_Auto_AUX_Mode) //2023-05-17_1 : To SW1_KEY chattering under BAP-01
					{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
						_DBG("\n\rSW1_KEY : Auto Aux Mode");
#endif
						cur_button_status = button_release; //High -> Low
						key = SW1_KEY;
						B_Auto_AUX_Mode = TRUE; //Auto Aux Mode //B_Auto_AUX_Mode = FALSE; //Aux Fixed Mode //2022-12-20_2
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
			}
			else //status == 0x00000002
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<0))) //PA0 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					if(B_Auto_AUX_Mode) //2023-05-17_1 : To SW1_KEY chattering under BAP-01
					{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
						_DBG("\n\rSW1_KEY : Aux Fixed Mode");
#endif
						cur_button_status = button_release; //Low -> High
						key = SW1_KEY;
						B_Auto_AUX_Mode = FALSE; //Aux Fixed Mode //B_Auto_AUX_Mode = TRUE; //Auto Aux Mode //2022-12-20_2
					}
				}
			}

#if 0//def MB3021_ENABLE //2023-05-04_3 : Move to the action of Auto Aux/Fixed Aux
			if(key != NONE_KEY)
			{
				key = NONE_KEY; //SW1_KEY set to NONE_KEY
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUTO_ONOFF_ENABLE) //Fixed Master SPK do not work Auto power off even though No siganl from BT when user remove Aux jack
				TIMER20_auto_power_flag_Start();
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
				MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef AD82584F_ENABLE
				AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
				TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif
#endif
				Set_MB3021_BT_Module_Source_Change();
			}
#endif

			clear_bit = status & 0x00000003;
		}
#ifdef MASTER_MODE_ONLY
		else if(status & 0x0000000c) //PA1 : EQ BSP(High)/EQ NORMAL(Low)
		{
			shift_bit = 2;

			//Both Rising Edge and Falling Edge should work as switch input. so, in both case, the cur_button_status should be button_release.
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
			if((status & 0x0c) == 0x0c) //To recovery, sometime status is 0xc0 even though GPIO is High. So, we need to make recovery.
			{
				if(HAL_GPIO_ReadPin(PA) & (1<<1)) //EQ BSP(High)
				{
					if(IsEQ_BSP) //Previous status should be Master Mode //2022-12-20_2
					{
						key = SW2_KEY;
						//IsEQ_BSP = 0x01; //EQ BSP(High) //2022-12-20_2
						IsEQ_BSP = 0x00; //EQ NORMAL(Low) //2022-12-20_2
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
				else //EQ NORMAL(Low)
				{
					if(!IsEQ_BSP) //Previous status should be EQ NORMAL(Low) Mode //2022-12-20_2
					{
						key = SW2_KEY;
						//IsEQ_BSP = 0x00; //EQ NORMAL(Low) //2022-12-20_2
						IsEQ_BSP = 0x01; //EQ BSP(High) //2022-12-20_2
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
			}
			else
#endif //KEY_CHATTERING_EXTENSION_ENABLE
			if(status & 0x00000004)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<1)) //PA1 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //High -> Low
					key = SW2_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsEQ_BSP = 0x01; //EQ BSP(High) //IsMaster = 0x00; //Slave //2022-12-20_2
#endif
				}
			}
			else //status == 0x00000008
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<1))) //PA1 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //Low -> High
					key = SW2_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsEQ_BSP = 0x00; //EQ NORMAL(Low) //IsMaster = 0x01; //Master //2022-12-20_2
#endif
				}
			}

			clear_bit = status & 0x0000000c;
		}
#else //MASTER_MODE_ONLY
		else if(status & 0x0000000c) //PA1 : M/S_SWITCH_1
		{
			shift_bit = 2;

			//Both Rising Edge and Falling Edge should work as switch input. so, in both case, the cur_button_status should be button_release.
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
			if((status & 0x0c) == 0x0c) //To recovery, sometime status is 0xc0 even though GPIO is High. So, we need to make recovery.
			{
				if(HAL_GPIO_ReadPin(PA) & (1<<1)) //Master Mode
				{
					//if(!IsMaster) //Previous status should be Slave Mode //2022-12-20_2
					if(IsMaster) //Previous status should be Master Mode //2022-12-20_2
					{
						key = SW2_KEY;
						//IsMaster = 0x01; //Master //2022-12-20_2
						IsMaster = 0x00; //Slave //2022-12-20_2
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
				else //Slave Mode
				{
					//if(IsMaster) //Previous status should be Master Mode //2022-12-20_2
					if(!IsMaster) //Previous status should be Slave Mode //2022-12-20_2
					{
						key = SW2_KEY;
						//IsMaster = 0x00; //Slave //2022-12-20_2
						IsMaster = 0x01; //Master //2022-12-20_2
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
			}
			else
#endif //KEY_CHATTERING_EXTENSION_ENABLE
			if(status & 0x00000004)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<1)) //PA1 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //High -> Low
					key = SW2_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsMaster = 0x01; //Master //IsMaster = 0x00; //Slave //2022-12-20_2
#endif
				}
			}
			else //status == 0x00000008
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<1))) //PA1 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //Low -> High
					key = SW2_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsMaster = 0x00; //Slave //IsMaster = 0x01; //Master //2022-12-20_2
#endif
				}
			}

			clear_bit = status & 0x0000000c;
		}
#endif //MASTER_MODE_ONLY
        else if(status & 0x00003000) //PA6 : POWER_Off(short)/POWER_ON(Long) //Implemented Power Key Feature //2022-10-07_3
		{
			shift_bit = 12;

			if(status & 0x00001000) //Falling
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
					if(!Power_State()) //2023-05-04_5 : Under BAP-01, we need to add power check condition for Power key chattering.
					{
						key = POWER_ON_KEY;
						cur_button_status = button_release; //Falling Means : Power On
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
			}
			else //status == 0x00002000 //Rising
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<6))) //PA6 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					if(Power_State()) //2023-05-04_5 : Under BAP-01, we need to add power check condition for Power key chattering.
					{
						key = POWER_OFF_KEY;
						cur_button_status = button_release; //Low -> High
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
			}
			clear_bit = status & 0x00003000;
		}
		else if(status & 0x0000c000) //PA7 : BT_UPDATE_DET(High) / Normal(Low)  //2022-10-12_4
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
			
			clear_bit = status & 0x0000c000;
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
#ifdef VOLUME_KEY_FILTERING					
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

#if defined(MB3021_ENABLE) && defined(USEN_BAP) //2023-05-16_2 : This is side effect of //2023-05-04_3
					if(key == SW1_KEY)
					{
						key = NONE_KEY; //SW1_KEY set to NONE_KEY
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUTO_ONOFF_ENABLE) //Fixed Master SPK do not work Auto power off even though No siganl from BT when user remove Aux jack
						TIMER20_auto_power_flag_Start();
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
						MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef AD82584F_ENABLE
						AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
						TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif
#endif
						Set_MB3021_BT_Module_Source_Change();

						ret = -1;
					}
#endif

					if(ret != -1)
					{
						Send_Remote_Key_Event(key);
					}
#else
						Send_Remote_Key_Event(key);
#endif
				}
			}
		}
	}

#else //USEN_BAP

	if (status & 0x00003fff) //Just check PA0 / PA1 / PA2 / PA3 / PA4 / PA5 / PA6
	{
		shift_bit = 0xffffffff;

#ifdef KEY_CHATTERING_ENABLE
		delay_ms(KEY_CHATTERING_DELAY_MS+10); //2023-02-21_8 : Add some more delay(10ms) for Master/Slave Key and LR/360 Key
#endif
		/* bit 0 : PA Falling Edge / bit 1 : PA Rising Edge *///Just check Rising Edge (Normal : High / Push : Low)
		if(status & 0x00000003) //PA0 : L/R_SWITCH_1
		{
			shift_bit = 0;

			//Both Rising Edge and Falling Edge should work as switch input. so, in both case, the cur_button_status should be button_release.
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
			if((status & 0x03) == 0x03) //To recovery, sometime status is 0x30 even though GPIO is High. So, we need to make recovery.
			{
				if(HAL_GPIO_ReadPin(PA) & (1<<0)) //STEREO Mode
				{
					if(!IsStereo) //Previous status should be Slave Mode //Low -> High
					{
						key = SW1_KEY;
						IsStereo = 0x01; //Stereo Mode
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
				else //L/R Mode
				{
					if(IsStereo) //Previous status should be Master Mode
					{
						key = SW1_KEY;
						IsStereo = 0x00; //L/R Mode
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
			}
			else
#endif //KEY_CHATTERING_EXTENSION_ENABLE
			if(status & 0x00000001) //High -> Low
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<0)) //PA0 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //High -> Low
					key = SW1_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsStereo = 0x00; //L/R Mode
#endif
				}
			}
			else //status == 0x00000002 //Low -> High
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<0))) //PA0 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //Low -> High
					key = SW1_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsStereo = 0x01; //Stereo Mode
#endif
				}
			}

			clear_bit = status & 0x00000003;
		}
		else if(status & 0x0000000c) //PA1 : M/S_SWITCH_1
		{
			shift_bit = 2;

			//Both Rising Edge and Falling Edge should work as switch input. so, in both case, the cur_button_status should be button_release.
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
			if((status & 0x0c) == 0x0c) //To recovery, sometime status is 0xc0 even though GPIO is High. So, we need to make recovery.
			{
				if(HAL_GPIO_ReadPin(PA) & (1<<1)) //Master Mode
				{
					if(!IsMaster) //Previous status should be Slave Mode
					{
						key = SW2_KEY;
						IsMaster = 0x01; //Master
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
				else //Slave Mode
				{
					if(IsMaster) //Previous status should be Master Mode
					{
						key = SW2_KEY;
						IsMaster = 0x00; //Slave
						cur_button_status = button_release; 
					}
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
				}
			}
			else
#endif //KEY_CHATTERING_EXTENSION_ENABLE
			if(status & 0x00000004)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<1)) //PA1 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //High -> Low
					key = SW2_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsMaster = 0x00; //Slave
#endif
				}
			}
			else //status == 0x00000008
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<1))) //PA1 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_release; //Low -> High
					key = SW2_KEY;
#ifdef KEY_CHATTERING_EXTENSION_ENABLE
					IsMaster = 0x01; //Master
#endif
				}
			}

			clear_bit = status & 0x0000000c;
		}
		else if(status & 0x00000030) //PA2 : BT_KEY(Long - 5Sec)
		{
			shift_bit = 4;

#ifdef MASTER_SLAVE_GROUPING
			key = BT_KEY;//BT Shor Key is available with this line
#else
			key = NONE_KEY; //BT Key is only valid for Long Key
#endif			
			if(status & 0x00000010)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<2)) //PA2 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
#ifdef AUX_INPUT_DET_ENABLE
					if(!Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
#endif
					{
						TIMER13_Periodic_Mode_Run(TRUE, Timer13_BT_Pairing_Key); //BT Key is implemented timer 13 interrupt routine
					}
					
					cur_button_status = button_push; //High -> Low
				}
			}
			else //status == 0x00000020
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<2))) //PA2 is Low and this says invalid value
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

			clear_bit = status & 0x00000030;
		}
		else if(status & 0x000000c0) //PA3 : VOLUME UP
		{
			shift_bit = 6;
			key = VOL_UP_KEY;

			if(status & 0x00000040)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<3)) //PA3 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_push; //High -> Low
					TIMER12_Periodic_Mode_Run(TRUE, TRUE); //A Long key action is executed in Timer interrupt function
				}
			}
			else //status == 0x00000080
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<3))) //PA3 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				cur_button_status = button_release; //Low -> High
			}

			clear_bit = status & 0x000000c0;
		}
		else if(status & 0x00000300) //PA4 : VOLUME DOWN
		{
			shift_bit = 8;
			key = VOL_DOWN_KEY;

			if(status & 0x00000100)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<4)) //PA4 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
					cur_button_status = button_push; //High -> Low
					TIMER12_Periodic_Mode_Run(TRUE, FALSE);  //A Long key action is executed in Timer interrupt function
				}
			}
			else //status == 0x00000200
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<4))) //PA4 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				cur_button_status = button_release; //Low -> High
			}

			clear_bit = status & 0x00000300;
		}
		else if(status & 0x00000c00) //PA5 : MUTE KEY //To Do !!! - Need to implement function
		{
			shift_bit = 10;
			key = MUTE_KEY;

			if(status & 0x00000400)
			{
#ifdef KEY_CHATTERING_ENABLE
				if(HAL_GPIO_ReadPin(PA) & (1<<5)) //PA5 is High and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				{
#ifdef EQ_TOGGLE_ENABLE //2023-01-17
#ifndef MASTER_MODE_ONLY
					if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
						TIMER13_Periodic_Mode_Run(TRUE, Timer13_EQ_Toggle_Key); //EQ Key(long key) is implemented timer 13 interrupt routine
#endif
					cur_button_status = button_push; //High -> Low
				}
			}
			else //status == 0x00000800
			{
#ifdef KEY_CHATTERING_ENABLE
				if(!(HAL_GPIO_ReadPin(PA) & (1<<5))) //PA5 is Low and this says invalid value
				{
					shift_bit = 0xffffffff;
					key = NONE_KEY;
				}
				else
#endif
				cur_button_status = button_release; //Low -> High
			}

			clear_bit = status & 0x00000c00;
		}
		else if(status & 0x00003000) //PA6 : POWER_Off(short)/POWER_ON(Long)  //Long Key is implmented below if(key == POWER_KEY)
		{
			shift_bit = 12;
#ifdef POWER_KEY_TOGGLE_ENABLE
			key = POWER_KEY;
#else
			key = POWER_OFF_KEY;
#endif
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
#ifdef PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE
#ifndef MASTER_MODE_ONLY
					Switch_Master_Slave_Mode mode;

					mode = Get_Cur_Master_Slave_Mode();

					if(mode == Switch_Master_Mode)
#endif
					{
						TIMER13_Periodic_Mode_Run(TRUE, Timer13_Power_Key);
						cur_button_status = button_push; //High -> Low
					}
#ifndef MASTER_MODE_ONLY
					else
					{
						shift_bit = 0xffffffff;
						key = NONE_KEY;
					}
#endif
#else
					TIMER13_Periodic_Mode_Run(TRUE, Timer13_Power_Key);
					cur_button_status = button_push; //High -> Low
#endif
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
			clear_bit = status & 0x00003000;
		}
		else
			shift_bit = 0xffffffff; //Invalid

		HAL_GPIO_EXTI_ClearPin(PA, status&clear_bit);

		if(shift_bit != 0xffffffff)
		{
			if(cur_button_status == button_push) //Rising Edge 0x10
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\r3. Ready Button key = ");
				_DBH(key);
#endif
			}
			else //cur_button_status = button_release //Falling Edge 0x01
			{
#if defined(TIMER12_13_LONG_KEY_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
				switch(key)
				{
#ifdef TIMER12_13_LONG_KEY_ENABLE
					case VOL_DOWN_KEY:
					{
						if(Is_Volume_Up_Long_Key()) //Long key is sent on Timer function so we don't need to send key again.
							key = NONE_KEY;
						
						TIMER12_Periodic_Mode_Run(FALSE, FALSE);
					}
					break;
					
					case VOL_UP_KEY:
					{
						if(Is_Volume_Down_Long_Key()) //Long key is sent on Timer function so we don't need to send key again.
							key = NONE_KEY;
						
						TIMER12_Periodic_Mode_Run(FALSE, TRUE);
					}
					break;
					
#endif //TIMER12_13_LONG_KEY_ENABLE
#ifdef SWITCH_BUTTON_KEY_ENABLE
#ifdef POWER_KEY_TOGGLE_ENABLE
					case POWER_KEY:
#else //POWER_KEY_TOGGLE_ENABLE
					case POWER_ON_KEY:
					case POWER_OFF_KEY:
#endif //POWER_KEY_TOGGLE_ENABLE
						if(Is_Power_Long_Key())
							key = NONE_KEY;
						
						TIMER13_Periodic_Mode_Run(FALSE, Timer13_Power_Key);
					break;

					case BT_KEY: // BT Short Key Action
						if(Is_BT_Long_Key()) //Long key is sent on Timer function so we don't need to send key again.
							key = NONE_KEY;
						
						TIMER13_Periodic_Mode_Run(FALSE, Timer13_BT_Pairing_Key);
#if defined(AUX_INPUT_DET_ENABLE) && !defined(USEN_BAP) //2023-04-13_1 : Under BAP-01, Aux mode shoud be accepted BT short key.
						if(Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
						{
							key = NONE_KEY;
						}
#endif
					break;

#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : Just use MUTE KEY(Long Key) for EQ KEY(EQ Toggle)
					case MUTE_KEY:
						if(Is_EQ_Long_Key())
							key = NONE_KEY;
						
						TIMER13_Periodic_Mode_Run(FALSE, Timer13_EQ_Toggle_Key);
#endif
#endif //SWITCH_BUTTON_KEY_ENABLE

					default:
						break;
				}
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG				
				_DBG("\n\r3. Release Button key = ");
				_DBH(key);
#endif
				if(key != NONE_KEY)
				{
#ifdef VOLUME_KEY_FILTERING			
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
#else //VOLUME_KEY_FILTERING
						Send_Remote_Key_Event(key);
#endif //VOLUME_KEY_FILTERING
				}
			}
		}
	}
#endif //USEN_BAP
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
#ifdef USEN_BAP //Interrupt setting for PA0/PA1/PA4/PA6 /PA7//2022-10-07
	HAL_GPIO_EXTI_Config(PA, 0, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 1, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 4, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 6, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 7, IER_EDGE, ICR_BOTH_EDGE_INT);
#else
	HAL_GPIO_EXTI_Config(PA, 0, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 1, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 2, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 3, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 4, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 5, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PA, 6, IER_EDGE, ICR_BOTH_EDGE_INT);
#endif
	NVIC_SetPriority(GPIOAB_IRQn, 3);	
	NVIC_EnableIRQ(GPIOAB_IRQn);
}

#if defined(USEN_BAP) && defined(SWITCH_BUTTON_KEY_ENABLE) //Implemented Interrupt Port E for BT Key(PE7) //2022-10-11_2
void GPIOE_IRQHandler_IT(void) // PE7 : Button Switch Input
{
	uint32_t status1 = 0, clear_bit = 0, shift_bit = 0;
	uint8_t key = 0;
	button_status cur_button_status;
#ifdef VOLUME_KEY_FILTERING
	static uint8_t key_bk1 = 0;
	static uint32_t filtering_time1 = 0, filtering_time_old1 = 0, filtering_time_very_old1 = 0;

	filtering_time1 = TIMER20_100ms_Count_Value();
#endif

	status1 = HAL_GPIO_EXTI_GetState(PE);
	
#ifdef GPIO_DEBUG_MSG
	_DBG("\n\rstatus1 = ");
	_DBH32(status1);
#endif

#ifdef SWITCH_BUTTON_KEY_ENABLE //When the state is Power off, Power key is only available
	if(!Power_State())
	{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
		_DBG("\n\r4. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
		HAL_GPIO_EXTI_ClearPin(PE, status1);

		return;
	}
#endif

	if (status1 & 0x0000c000) //Just check PE7 of PE
	{
		shift_bit = 0xffffffff;

#ifdef KEY_CHATTERING_ENABLE
		delay_ms(KEY_CHATTERING_DELAY_MS);
#endif
		if(status1 & 0x0000c000) //PA2 : BT_KEY(Long - 5Sec)
		{
			shift_bit = 4;

#ifdef MASTER_SLAVE_GROUPING
			key = BT_KEY;//BT Shor Key is available with this line
#else
			key = NONE_KEY; //BT Key is only valid for Long Key
#endif			
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
#ifdef AUX_INPUT_DET_ENABLE
					if(!Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
#endif
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

			clear_bit = status1 & 0x0000c000;
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
#if defined(TIMER12_13_LONG_KEY_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
				switch(key)
				{
#ifdef SWITCH_BUTTON_KEY_ENABLE
					case BT_KEY: //BT Long Key / Short Key Action
						if(Is_BT_Long_Key()) //Long key is sent on Timer function so we don't need to send key again.
							key = NONE_KEY;
						
						TIMER13_Periodic_Mode_Run(FALSE, Timer13_BT_Pairing_Key);
#if defined(AUX_INPUT_DET_ENABLE) && !defined(USEN_BAP) //2023-04-13_1 : Under BAP-01, Aux mode shoud be accepted BT short key.
						if(Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
						{
							key = NONE_KEY;
						}
#endif
					break;
#endif
					default:
						break;
				}
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG				
				_DBG("\n\r4. Release Button key = ");
				_DBH(key);
#endif
				if(key != NONE_KEY)
				{
#ifdef VOLUME_KEY_FILTERING
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
#else
						Send_Remote_Key_Event(key);
#endif
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
	HAL_GPIO_EXTI_Config(PE, 7, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_SCU_LVRCmd(DISABLE);
	
	NVIC_SetPriority(GPIOE_IRQn, 3);
	NVIC_EnableIRQ(GPIOE_IRQn);
	
	//HAL_SCU_WakeUpSRCCmd(WAKEUP_GPIOE, ENABLE);		
}

void EXIT_PortE_Disable(void) //2023-01-03_2 : Disable Interrupt Port E for BT Key(PE7)
{
	HAL_GPIO_EXTI_Config(PE, 7, IER_DISABLE, ICR_BOTH_EDGE_INT);
}
#endif //defined(USEN_BAP) && !(BT_SPK_TACT_SWITCH)

#endif //SWITCH_BUTTON_KEY_ENABLE

#ifdef USEN_GPIO_OTHERS_ENABLE //Use External INT for Switchs and Button Keys - PF0 / PF5(AMP_ERROR input)
void GPIOF_IRQHandler_IT(void)
{
	uint32_t status, clear_bit = 0, shift_bit = 0;
#ifdef FACTORY_RESET_KEY_CAPACITOR_APPLY
	uint32_t status_buf = 0;
#endif
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

	if (status & 0x00000c03) //0000 1100 0000 0011
	{
		shift_bit = 0xffffffff;
					
#ifdef KEY_CHATTERING_ENABLE
		delay_ms(KEY_CHATTERING_DELAY_MS);
#endif
		if(status & 0x00000003) //PF0 : FACTORY RESET Button
		{
			shift_bit = 0;
#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
			if(Power_State()
#if defined(USEN_BAP) && !defined(MASTER_MODE_ONLY) //FACTORY_RESET_KEY is invalid under BAP-01 Slave and only available it thru USEN Tablet SSP COM//2023-01-05_1
			&& Get_Cur_Master_Slave_Mode() != Switch_Slave_Mode 
#endif
				) //Factory Reset Key is only available under Power On
				TIMER13_Periodic_Mode_Run(TRUE, Timer13_Factory_Reset_Key); //FACTORY RESET LONG Key is implemented timer 13 interrupt routine
#endif
			key = FACTORY_RESET_KEY;
			
			//Both Rising Edge and Falling Edge should work as switch input. so, in both case, the cur_button_status should be button_release.
#ifdef FACTORY_RESET_KEY_CAPACITOR_APPLY //For recovery, When we use capacitor on FACTORY RESET Line, the Rising Edge value is always 0x03(0x02 is correct) but Falling Edge is always 0x01
			status_buf = status & 0x00000003;

			if(status_buf == 0x00000001) //Falling Edge
#else
			if(status & 0x00000001) //Falling Edge
#endif
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

#if defined(USEN_BAP) && !defined(MASTER_MODE_ONLY)
			if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && key == FACTORY_RESET_KEY) //FACTORY_RESET_KEY is invalid under BAP-01 Slave and only available it thru USEN Tablet SSP COM//2023-01-05_1
			{
#ifdef SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG
				_DBG("\n\rFACTORY_RESET_KEY is invalid under BAP-01 Slave ~~~ ");
				_DBH(key);
#endif
				shift_bit = 0xffffffff;
				key = NONE_KEY;
			}
#endif
			clear_bit = status & 0x00000003;

			HAL_GPIO_EXTI_ClearPin(PF, status&clear_bit);

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
#ifdef FACTORY_RESET_LONG_KEY_SUPPORT
					if(key == FACTORY_RESET_KEY)
					{
						TIMER13_Periodic_Mode_Run(FALSE, Timer13_BT_Pairing_Key);
						key = NONE_KEY; //Short key has no meaning for Factory Reset Key
					}
#endif
#if defined(TIMER12_13_LONG_KEY_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)	
					if(key != NONE_KEY)
						Send_Remote_Key_Event(key);
#endif
				}
			}
		}
	
		if(status & 0x00000c00) //PF5 - DAMP_ERROR 
		{
#ifdef KEY_CHATTERING_ENABLE
			delay_ms(KEY_CHATTERING_DELAY_MS);
#endif
			clear_bit = status & 0x00000c00;//0x00000c00;
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
#ifdef TAS5806MD_ENABLE
				if(!Is_BAmp_Init()) //2023-02-21_5 : We can't access TI AMP during TI AMP initializing.
				{
#ifdef TAS5806MD_ENABLE //When Amp error is ocurred, we don't need to alarm SOC_ERROR_MODE. 2022-10-31
					//TAS5806MD_Amp_Detect_Fault(FALSE); //2023-04-07_2 : Disable
					if(TAS5806MD_Amp_Detect_Fault(FALSE) == 0xFF) //2023-04-07_2 : To recovery TAS5806MD_Amp_Detect_Fault() function
					{
#ifdef TAS5806MD_DEBUG_MSG
						_DBG("\n\rDAMP_ERROR - Recovery");
#endif
#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01
						TIMER20_Amp_access_error_flag_Start();
#endif
					}
#else //TAS5806MD_ENABLE
#ifdef SOC_ERROR_ALARM
#ifdef TIMER21_LED_ENABLE
					Set_Status_LED_Mode(STATUS_SOC_ERROR_MODE); //over-temperature or short-circuit condition
#endif
					TIMER20_SoC_error_flag_Start();
#endif
#endif //TAS5806MD_ENABLE
				}
#ifdef TAS5806MD_DEBUG_MSG
				else
				{
					_DBG("\n\r+++ Is_BAmp_Init is TRUE - 1");
				}
#endif
#endif
			}
		}
	}
}

void EXIT_PortF_Configure(void)
{
	HAL_GPIO_EXTI_Config(PF, 0, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PF, 5, IER_EDGE, ICR_BOTH_EDGE_INT); //Added AMP error

	NVIC_SetPriority(GPIOF_IRQn, 3);	
	NVIC_EnableIRQ(GPIOF_IRQn);
}
#endif

#ifdef SOUND_BAR_GPIO_ENABLE
void DSP_Reset(Bool Reset_On)
{
	if(Reset_On)
	{
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
		_DBG("\n\rDSP_Reset - On");
#endif
		HAL_GPIO_ClearPin(PD, _BIT(1));
	}
	else
	{
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
		_DBG("\n\rDSP_Reset - Off");
#endif
		HAL_GPIO_SetPin(PD, _BIT(1));
	}
}
/**********************************************************************
 * @brief		GPIOE Handler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GPIOCD_IRQHandler_IT1(void)
{
	uint32_t	 status, status1, clear_bit = 0;
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
	static uint32_t status_bk = 0, status_bk1 = 0;
#endif

	status = HAL_GPIO_EXTI_GetState(PC);
	
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
	if(status_bk != status)
	{	
		status_bk = status;
		_DBG("\n\rstatus : ");
		_DBH32(status);
	}
#endif

	if (status & 0x00000300)
	{
		clear_bit = status & 0x00000300;
		HAL_GPIO_EXTI_ClearPin(PC, status&clear_bit);
		
		if(HAL_GPIO_ReadPin(PC) & (1<<4))
		{
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
			_DBG("\n\rB_FALT_SW - TRUE");
#endif
		}
		else
		{
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
			_DBG("\n\rB_FALT_SW - FALSE");
#endif
		}
	}

	status1 = HAL_GPIO_EXTI_GetState(PD);

#ifdef SOUND_BAR_GPIO_DEBUG_MSG
	if(status_bk1 != status1)
	{
		status_bk1 = status1;
		_DBG("\n\rstatus1 : ");
		_DBH32(status1);
	}
#endif

	if (status1 & 0x00000003) /* bit 0 : PD0 Falling Edge / bit 1 : PD0 Rising Edge */
	{
		clear_bit = status1 & 0x00000003;
		HAL_GPIO_EXTI_ClearPin(PD, status1&clear_bit);

		if(HAL_GPIO_ReadPin(PD) & (1<<0))
		{
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
			_DBG("\n\rB_CLIP_OTW_SW - TRUE");
#endif
		}
		else
		{
#ifdef SOUND_BAR_GPIO_DEBUG_MSG
			_DBG("\n\rB_CLIP_OTW_SW - FALSE");
#endif
		}
	}
}

/**********************************************************************
 * @brief		EXIT_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void EXIT_Configure1(void) //Interrupt edge of both side
{
	HAL_GPIO_EXTI_Config(PC, 4, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_GPIO_EXTI_Config(PD, 0, IER_EDGE, ICR_BOTH_EDGE_INT);
	HAL_SCU_LVRCmd(DISABLE);
	
	NVIC_SetPriority(GPIOCD_IRQn, 3);
	NVIC_EnableIRQ(GPIOCD_IRQn);
	
	//HAL_SCU_WakeUpSRCCmd(WAKEUP_GPIOC, ENABLE);
	
	SCU->SMR = 0
	| (SCU_SMR_ROSCAON_Msk)
	| (SCU_SMR_BGRAON_Msk)
	| (SCU_SMR_VDCAON_Msk)
	;
		
}
#endif //SOUND_BAR_GPIO_ENABLE

/**********************************************************************
 * @brief		GPIO_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void GPIO_Configure(void)
{
#ifdef USEN_BAP //2023-05-12_1 : make unused pins to PUSH_PULL_OUTPUT(Pull Down) //PB 2 ~ 3, 6 ~ 7 / PC 2, PC5 / PD 0 ~ 1 / PE 0 ~ 6 / PF 1 ~ 3
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

	//PC 2, PC5
	{
		i = 2;
		HAL_GPIO_ConfigOutput(PC, i, PUSH_PULL_OUTPUT);
		HAL_GPIO_ConfigPullup(PC, i, ENPD);
		HAL_GPIO_ClearPin(PC, _BIT(i));
#if 0 //2023-05-16_3 : To keep PC5(39pin) as RESET_N under BAP-01
		i = 5;
		HAL_GPIO_ConfigOutput(PC, i, PUSH_PULL_OUTPUT);
		HAL_GPIO_ConfigPullup(PC, i, ENPD);
		HAL_GPIO_ClearPin(PC, _BIT(i));
#endif
	}

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
#endif

#ifdef USEN_BAP //Port Setting for USEN_BAP //2022-10-07
#ifdef SWITCH_BUTTON_KEY_ENABLE //Use External INT for Switchs and Button Keys - PA0 / PA1 / PA6
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
#endif

#if defined(ADC_INTERRUPT_INPUT_ENABLE) || defined(ADC_INPUT_ENABLE)
#ifdef ADC_INPUT_ENABLE //Use ADC Input for Attenuator Volume and Main Volume -PA2 / PA3
	/* ADC pin PA2 : BSP_VOL_A/D(Attenuator Volume) */
	HAL_GPIO_ConfigOutput(PA, 2, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 2, FUNC3);
	HAL_GPIO_ConfigPullup(PA, 2, DISPUPD);

	/* ADC pin PA3 : VOL_CONT(Master Volume) */
	HAL_GPIO_ConfigOutput(PA, 3, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 3, FUNC3);
	HAL_GPIO_ConfigPullup(PA, 3, DISPUPD);
#else //ADC_INTERRUPT_INPUT_ENABLE
  	//PA2 - ATTENUATOR CONTROL
	HAL_GPIO_ConfigOutput(PA, 2, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 2, FUNC3);
	HAL_GPIO_ConfigPullup(PA, 2, DISPUPD);

  	//PA3 - VOLUME CONTROL
	HAL_GPIO_ConfigOutput(PA, 3, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PA, 3, FUNC3);
	HAL_GPIO_ConfigPullup(PA, 3, DISPUPD);
#endif
#endif //#if defined(ADC_INTERRUPT_INPUT_ENABLE) || defined(ADC_INPUT_ENABLE)

#ifdef USEN_GPIO_OTHERS_ENABLE //GPIO Output Setting
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
#endif	

#ifdef UART_10_ENABLE
	/* Initialize USART10 pin connect - TX10 : PB0 / RX10 : PB1 */
	HAL_GPIO_ConfigOutput(PB, 1, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 1, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 1, ENPU);

	HAL_GPIO_ConfigOutput(PB, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 0, FUNC1);
#else //UART update feature for MCS Logic BT Module
	HAL_GPIO_ConfigOutput(PB, 1, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 1, DISPUPD);
	HAL_GPIO_SetPin(PB, _BIT(1));

	HAL_GPIO_ConfigOutput(PB, 0, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 0, DISPUPD);
	HAL_GPIO_SetPin(PB, _BIT(0));
#endif //UART_10_ENABLE

#ifdef REMOCON_TIMER20_CAPTURE_ENABLE
	/* T20C pin config set if need - PC0 */
	HAL_GPIO_ConfigOutput(PC, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PC, 0, FUNC2);
#endif //REMOCON_TIMER20_CAPTURE_ENABLE

#if defined(USEN_BT_SPK) && !defined(USING_REFERENCE_FLATFORM)
	/* PC0 Output - MODULE_RESET *///In referece flatrom, this port works as remote control port.
	HAL_GPIO_ConfigOutput(PC, 0, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PC, 0, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(0));
#endif
	
#ifdef AUX_DETECT_INTERRUPT_ENABLE //2023-01-10_3 //2022-10-17  //To Do !!!
#ifdef AUX_INPUT_DET_ENABLE
	/* External interrupt pin PC3 */
	HAL_GPIO_ConfigOutput(PC, 3, INPUT);
	HAL_GPIO_ConfigPullup(PC, 3, ENPU);
	HAL_GPIO_ClearPin(PC, _BIT(3));
#endif
#endif

#ifdef GPIO_LED_ENABLE //Use GPIOs as LED output control - PC1 / PC2 / PD2 / PD3 / PD4 / PD5 / LED Power Control - PD0
	/* PC1 Output - STATUS_LED_W1 */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PC, 1, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PC, 1, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PC, 1, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(1));

	/* PD2 Output - BT_PAIRING_B */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PD, 2, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PD, 2, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PD, 2, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(2));

	/* PD3 Output - BT_PAIRING_W */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PD, 3, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PD, 3, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PD, 3, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(3));
#endif
#ifdef _DEBUG_MSG //2023-05-12_1 : #ifndef _DEBUG_MSG //If we don't use DEBUG_MSG, we need to set some GPIO like below. Becasue these GPIOs can avoid USART10 UART error. But we don't know why.
	HAL_GPIO_ConfigOutput(PB, 7, ALTERN_FUNC); //RX1
	HAL_GPIO_ConfigFunction(PB, 7, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 7, 1); 

	HAL_GPIO_ConfigOutput(PB, 6, ALTERN_FUNC); //TX1
	HAL_GPIO_ConfigFunction(PB, 6, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 6, 1);
#endif
#ifdef USEN_GPIO_OTHERS_ENABLE	//GPIO Output init
	delay_ms(20);
	HAL_GPIO_SetPin(PA, _BIT(5)); //+3.3V_DAMP_SW_1
	delay_ms(20);
	HAL_GPIO_SetPin(PD, _BIT(4)); //+24V_DAMP_SW
	delay_ms(20);
	HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN

	/* GPIO Output setting PC4 - +3.3V_SIG_SW */
	HAL_GPIO_SetPin(PC, _BIT(4));

	/* GPIO Output setting PD5 - SW_+3.3V_SW(LED Power Control) */
	HAL_GPIO_SetPin(PD, _BIT(5));
#endif
#ifdef I2C_0_ENABLE
	/* I2C0 PF6:SCL0, PF7:SDA0 */
	HAL_GPIO_ConfigOutput(PF, 6, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PF, 6, FUNC2);
	HAL_GPIO_ConfigOutput(PF, 7, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PF, 7, FUNC2);
#else //I2C_0_ENABLE
	HAL_GPIO_ConfigOutput(PF, 6, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 6, DISPUPD);
	HAL_GPIO_SetPin(PF, _BIT(6));

	HAL_GPIO_ConfigOutput(PF, 7, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 7, DISPUPD);
	HAL_GPIO_SetPin(PF, _BIT(7));
#endif //I2C_0_ENABLE

#else //USEN_BAP

#ifdef SWITCH_BUTTON_KEY_ENABLE //Use External INT for Switchs and Button Keys - PA0 / PA1 / PA2 / PA3 / PA4 / PA5 / PA6
	/* external interrupt pin PA0 : L/R_SWITCH_1 */
	HAL_GPIO_ConfigOutput(PA, 0, INPUT);
	HAL_GPIO_ConfigPullup(PA, 0, ENPU);	
	HAL_GPIO_ClearPin(PA, _BIT(0));

	/* external interrupt pin PA1 : M/S_SWITCH_1 */
	HAL_GPIO_ConfigOutput(PA, 1, INPUT);
	HAL_GPIO_ConfigPullup(PA, 1, ENPU);	
	HAL_GPIO_ClearPin(PA, _BIT(1));

	/* external interrupt pin PA2 : BT_KEY(Long - 5Sec) */
	HAL_GPIO_ConfigOutput(PA, 2, INPUT);
	HAL_GPIO_ConfigPullup(PA, 2, ENPU);	
	HAL_GPIO_ClearPin(PA, _BIT(2));

	/* external interrupt pin PA3 : VOLUME UP KEY */
	HAL_GPIO_ConfigOutput(PA, 3, INPUT);
	HAL_GPIO_ConfigPullup(PA, 3, ENPU);	
	HAL_GPIO_ClearPin(PA, _BIT(3));

	/* external interrupt pin PA4 : VOLUME DOWN KEY */
	HAL_GPIO_ConfigOutput(PA, 4, INPUT);
	HAL_GPIO_ConfigPullup(PA, 4, ENPU);	
	HAL_GPIO_ClearPin(PA, _BIT(4));

	/* external interrupt pin PA5 : MUTE KEY */
	HAL_GPIO_ConfigOutput(PA, 5, INPUT);
	HAL_GPIO_ConfigPullup(PA, 5, ENPU);	
	HAL_GPIO_ClearPin(PA, _BIT(5));

	/* external interrupt pin PA6 : POWER_Off(short)/POWER_ON(Long) */
	HAL_GPIO_ConfigOutput(PA, 6, INPUT);
	HAL_GPIO_ConfigPullup(PA, 6, ENPU);	
	HAL_GPIO_ClearPin(PA, _BIT(6));
#endif

#ifdef UART_10_ENABLE
	/* Initialize USART10 pin connect - TX10 : PB0 / RX10 : PB1 */
	HAL_GPIO_ConfigOutput(PB, 1, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 1, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 1, ENPU);

	HAL_GPIO_ConfigOutput(PB, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PB, 0, FUNC1);
#else //UART update feature for MCS Logic BT Module
	HAL_GPIO_ConfigOutput(PB, 1, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 1, DISPUPD);
	HAL_GPIO_SetPin(PB, _BIT(1));

	HAL_GPIO_ConfigOutput(PB, 0, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PB, 0, DISPUPD);
	HAL_GPIO_SetPin(PB, _BIT(0));
#endif //UART_10_ENABLE

#ifdef REMOCON_TIMER20_CAPTURE_ENABLE
	/* T20C pin config set if need - PC0 */
	HAL_GPIO_ConfigOutput(PC, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PC, 0, FUNC2);
#endif //REMOCON_TIMER20_CAPTURE_ENABLE

#if defined(USEN_BT_SPK) && !defined(USING_REFERENCE_FLATFORM)
	/* PC0 Output - MODULE_RESET *///In referece flatrom, this port works as remote control port.
	HAL_GPIO_ConfigOutput(PC, 0, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PC, 0, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(0));
#endif

#ifdef BT_SPK_GPIO_ENABLE //PC4 / PD0 : Interrupt Input, PD2 / PD3 : Output
	/* external interrupt pin PC4*/
	HAL_GPIO_ConfigOutput(PC, 4, INPUT);
	HAL_GPIO_ConfigPullup(PC, 4, ENPU);	
	HAL_GPIO_ClearPin(PC, _BIT(4));

	/* external interrupt test pin PD0*/
	HAL_GPIO_ConfigOutput(PD, 0, INPUT);
	HAL_GPIO_ConfigPullup(PD, 0, ENPU);	
	HAL_GPIO_ClearPin(PD, _BIT(0));

	HAL_GPIO_ConfigOutput(PD, 2, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 2, DISPUPD);
#ifdef AMP_1_1CH_WORKAROUND
	HAL_GPIO_ClearPin(PD, _BIT(2));
#else //AMP_1_1CH_WORKAROUND
	HAL_GPIO_SetPin(PD, _BIT(2));
#endif //AMP_1_1CH_WORKAROUND

	HAL_GPIO_ConfigOutput(PD, 3, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 3, DISPUPD);
#ifdef AMP_1_1CH_WORKAROUND
	HAL_GPIO_ClearPin(PD, _BIT(3));
#else //AMP_1_1CH_WORKAROUND
	HAL_GPIO_SetPin(PD, _BIT(3));
#endif //AMP_1_1CH_WORKAROUND

#ifdef BT_SPK_TACT_SWITCH //PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
	/* External interrupt pin PE3 */
	HAL_GPIO_ConfigOutput(PE, 3, INPUT);
	HAL_GPIO_ConfigPullup(PE, 3, ENPU);
	HAL_GPIO_ClearPin(PE, _BIT(3));

	/* External interrupt pin PE4 */
	HAL_GPIO_ConfigOutput(PE, 4, INPUT);
	HAL_GPIO_ConfigPullup(PE, 4, ENPU);
	HAL_GPIO_ClearPin(PE, _BIT(4));

	/* External interrupt pin PE5 */
	HAL_GPIO_ConfigOutput(PE, 5, INPUT);
	HAL_GPIO_ConfigPullup(PE, 5, ENPU);
	HAL_GPIO_ClearPin(PE, _BIT(5));

	/* External interrupt pin PE6 */
	HAL_GPIO_ConfigOutput(PE, 6, INPUT);
	HAL_GPIO_ConfigPullup(PE, 6, ENPU);
	HAL_GPIO_ClearPin(PE, _BIT(6));

	/* External interrupt pin PE7 */
	HAL_GPIO_ConfigOutput(PE, 7, INPUT);
	HAL_GPIO_ConfigPullup(PE, 7, ENPU);
	HAL_GPIO_ClearPin(PE, _BIT(7));
#endif //BT_SPK_TACT_SWITCH
#endif //BT_SPK_GPIO_ENABLE

#ifdef AUX_INPUT_DET_ENABLE
	/* External interrupt pin PC3 */
	HAL_GPIO_ConfigOutput(PC, 3, INPUT);
	HAL_GPIO_ConfigPullup(PC, 3, ENPU);
	HAL_GPIO_ClearPin(PC, _BIT(3));
#endif

#ifdef SOUND_BAR_GPIO_ENABLE
	/* external interrupt pin PC4*/
	HAL_GPIO_ConfigOutput(PC, 4, INPUT);
	HAL_GPIO_ConfigPullup(PC, 4, ENPU);	
	HAL_GPIO_ClearPin(PC, _BIT(4));

	/* external interrupt test pin PD0*/
	HAL_GPIO_ConfigOutput(PD, 0, INPUT);
	HAL_GPIO_ConfigPullup(PD, 0, ENPU);	
	HAL_GPIO_ClearPin(PD, _BIT(0));

	/* PC3 Output */
	HAL_GPIO_ConfigOutput(PC, 3, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PC, 3, DISPUPD);

	//HAL_GPIO_SetPin(PC, _BIT(3));
	HAL_GPIO_ClearPin(PC, _BIT(3));

	/* PD1 Output */
	HAL_GPIO_ConfigOutput(PD, 1, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 1, DISPUPD);

	HAL_GPIO_SetPin(PD, _BIT(1));
#endif //SOUND_BAR_GPIO_ENABLE

#ifdef GPIO_LED_ENABLE //Use GPIOs as LED output control - PC1 / PC2 / PD2 / PD3 / PD4 / PD5 / LED Power Control - PD0
#ifdef LED_TEST_PC4
	/* PC4 Output */
#ifdef USING_REFERENCE_FLATFORM
	HAL_GPIO_ConfigOutput(PC, 4, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PC, 4, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PC, 4, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(4));
#endif
	/* PC1 Output - STATUS_LED_W1 */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PC, 1, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PC, 1, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PC, 1, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(1));
	
	/* PC2 Output - STATUS_LED_R1 */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PC, 2, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PC, 2, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PC, 2, DISPUPD);
	HAL_GPIO_SetPin(PC, _BIT(2));

	/* PD2 Output - BT_PAIRING_B */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PD, 2, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PD, 2, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PD, 2, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(2));

	/* PD3 Output - BT_PAIRING_W */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PD, 3, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PD, 3, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PD, 3, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(3));

	/* PD4 Output - MUTE_W */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PD, 4, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PD, 4, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PD, 4, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(4));

	/* PD5 Output - MUTE_R */
#ifdef LED_PORT_PUSH_PULL
	HAL_GPIO_ConfigOutput(PD, 5, PUSH_PULL_OUTPUT);
#else
	HAL_GPIO_ConfigOutput(PD, 5, OPEN_DRAIN_OUTPUT);
#endif
	HAL_GPIO_ConfigPullup(PD, 5, DISPUPD);
	HAL_GPIO_SetPin(PD, _BIT(5));

#ifndef OLD_BOARD
#ifdef LED_POWER_CONTROL_ENABLE 	/* PD0 Output - SW_+3.3V_SW(LED POWER CONTROL) */
	HAL_GPIO_ConfigOutput(PD, 0, PUSH_PULL_OUTPUT);//HAL_GPIO_ConfigOutput(PD, 0, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PD, 0, DISPUPD);
	HAL_GPIO_ClearPin(PD, _BIT(0)); //LED POWER CONTROL - ON //Need to use this after separating LED Power from Button Power
#else //LED_POWER_CONTROL_ENABLE
	HAL_GPIO_SetPin(PD, _BIT(0));// /* PD0 Output - SW_+3.3V_SW(LED POWER CONTROL) : Low */
#endif //LED_POWER_CONTROL_ENABLE
#endif //OLD_BOARD
#endif

#ifdef TIMER30_LED_PWM_ENABLE
	/* PWM30AB(PE1)*/
	HAL_GPIO_ConfigOutput(PE, 1, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PE, 1, FUNC1);
	/* PWM30BB(PE3)*/
	HAL_GPIO_ConfigOutput(PE, 3, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PE, 3, FUNC1);
	/* PWM30CB(PE5)*/
	HAL_GPIO_ConfigOutput(PE, 5, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PE, 5, FUNC1);
#endif //TIMER30_LED_PWM_ENABLE

#ifdef TIMER1n_LED_PWM_ENABLE
	/* setting PE6 - Timer10*/
  	HAL_GPIO_ConfigOutput(PE, 6, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PE, 6, FUNC1);
	/* setting PE7 - Timer11*/
  	HAL_GPIO_ConfigOutput(PE, 7, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PE, 7, FUNC1);
#endif

#ifdef USEN_GPIO_OTHERS_ENABLE
	/* external interrupt pin PF0 : FACTORY RESET Button */
	HAL_GPIO_ConfigOutput(PF, 0, INPUT);
	HAL_GPIO_ConfigPullup(PF, 0, ENPU);	
	HAL_GPIO_ClearPin(PF, _BIT(0));

	/* setting PF2 - +3.3V_DAMP_SW_1 */
	HAL_GPIO_ConfigOutput(PF, 2, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 2, DISPUPD);
	HAL_GPIO_ClearPin(PF, _BIT(2));

	/* setting PF3 - +14V_DAMP_SW_1 */
	HAL_GPIO_ConfigOutput(PF, 3, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 3, DISPUPD);
	HAL_GPIO_ClearPin(PF, _BIT(3));

	/* setting PF4 - DAMP_PDN */
	HAL_GPIO_ConfigOutput(PF, 4, PUSH_PULL_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 4, DISPUPD);
	HAL_GPIO_ClearPin(PF, _BIT(4));

	/* External interrupt pin PF5 - DAMP_ERROR */
	HAL_GPIO_ConfigOutput(PF, 5, INPUT);
	HAL_GPIO_ConfigPullup(PF, 5, ENPU);
	HAL_GPIO_ClearPin(PF, _BIT(5));
#endif

#ifdef USEN_BT_SPK
#ifdef AD82584F_ENABLE
	delay_ms(20);
	HAL_GPIO_SetPin(PF, _BIT(3)); //+14V_DAMP_SW_1
	delay_ms(20);
	HAL_GPIO_SetPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
	delay_ms(20);	
	HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#else //TAS5806MD_ENABLE
	delay_ms(20);
	HAL_GPIO_SetPin(PF, _BIT(2)); //+3.3V_DAMP_SW_1
	delay_ms(20);
	HAL_GPIO_SetPin(PF, _BIT(3)); //+14V_DAMP_SW_1
	delay_ms(20);	
	HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN
#endif
#endif

#ifdef I2C_0_ENABLE
#ifdef USING_REFERENCE_FLATFORM
	/* I2C0 PD0:SCL0, PD1:SDA0 */
	HAL_GPIO_ConfigOutput(PD, 0, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PD, 0, FUNC1);
	HAL_GPIO_ConfigOutput(PD, 1, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PD, 1, FUNC1);
#else //USING_REFERENCE_FLATFORM
	/* I2C0 PF6:SCL0, PF7:SDA0 */
	HAL_GPIO_ConfigOutput(PF, 6, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PF, 6, FUNC2);
	HAL_GPIO_ConfigOutput(PF, 7, ALTERN_FUNC);
	HAL_GPIO_ConfigFunction(PF, 7, FUNC2);
#endif //USING_REFERENCE_FLATFORM
#else //I2C_0_ENABLE
	HAL_GPIO_ConfigOutput(PF, 6, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 6, DISPUPD);
	HAL_GPIO_SetPin(PF, _BIT(6));

	HAL_GPIO_ConfigOutput(PF, 7, OPEN_DRAIN_OUTPUT);
	HAL_GPIO_ConfigPullup(PF, 7, DISPUPD);
	HAL_GPIO_SetPin(PF, _BIT(7));
#endif //I2C_0_ENABLE
#ifndef _DEBUG_MSG //If we don't use DEBUG_MSG, we need to set some GPIO like below. Becasue these GPIOs can avoid USART10 UART error. But we don't know why.
	HAL_GPIO_ConfigOutput(PB, 7, ALTERN_FUNC); //RX1
	HAL_GPIO_ConfigFunction(PB, 7, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 7, 1);

	HAL_GPIO_ConfigOutput(PB, 6, ALTERN_FUNC); //TX1
	HAL_GPIO_ConfigFunction(PB, 6, FUNC1);
	HAL_GPIO_ConfigPullup(PB, 6, 1);
#endif
#endif //USEN_BAP
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

#ifdef UART_10_ENABLE
static void Serial_Get_Data_Intterupt_Callback(uint8_t *Data)
{	
	if(uBuffer_Count >= UART10_Rx_Buffer_Size)
		uBuffer_Count = 0;
#if defined(USEN_BT_SPK) && defined(BT_MODULE_ENABLE)
#if defined(F1DQ3007_ENABLE) || defined(F1DQ3021_ENABLE)
	F1M22_BT_Module_Get_Auto_Resp(uBuffer_Count, Data); //Type 0x03(Alam)
	F1M22_BT_Module_Get_Resp(uBuffer_Count, Data); //Type 0x02(Response)
#else
	MB3021_BT_Module_Get_Auto_Response_Packet(uBuffer_Count, Data); //Type 0x02(Response)
#endif
#endif
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
#endif //UART_10_ENABLE

#ifdef SPI_11_ENABLE
static void SPI_Get_Data_Interrupt_Callback(uint8_t *Data)
{
//	_DBG("\n\rGet SPI Data : ");
	if(uBuffer_Count >= SPI_Rx_Buffer_Size)
		uBuffer_Count = 0;
	//Save data to buffer
	UART10_Rx_Buffer[uBuffer_Count++] = *Data;

	//_DBH(*Data);
	delay_ms(1);
	//_DBG("\n\rGet Data : Exit!\n\r");
}
#endif

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

#if defined(ADC_VOLUME_STEP_ENABLE) && defined(TAS5806MD_ENABLE) && defined(ADC_INPUT_ENABLE) //Attenuator action is inversed. So fixed it. //2023-02-08_1 : make Attenuator GAP
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
#endif //ADC_VOLUME_STEP_ENABLE


#if defined(USEN_BAP) && defined(TAS5806MD_ENABLE) && defined(ADC_INPUT_ENABLE)
uint8_t ADC_Volume_Attenuator_Value_Init(void) //2023-03-02_3 : Changed the concept of volume level update under BAP-01, we just use ADC value for Amp_Init() instead of flash data.
{//return cur_volume_level
	uint8_t uCurVolLevel = 0;
	static uint32_t ADC3_Value = 0xffffffff;
	static uint32_t ADC2_Value = 0xffffffff;
	int i;
	uint8_t ADC_Level_Min, ADC_Level_Max;
	
	//************ Volume Setting Start ************//

	ADC3_Value = ADC_PollingRun(3);
	
#ifdef ADC_INPUT_DEBUG_MSG
		_DBG("\n\r === Master Volume ADC = 0x");
		_DBH32(ADC3_Value);
#endif

#ifdef ADC_VOLUME_64_STEP_ENABLE
	for(i=1;i<65;i++)
#else //ADC_VOLUME_50_STEP_ENABLE
	for(i=1;i<51;i++)
#endif //ADC_VOLUME_64_STEP_ENABLE
	{
#ifdef ADC_VOLUME_64_STEP_ENABLE
		ADC_Level_Min = (i-1)*4; //0 4 8
		ADC_Level_Max = (i*4)-1; //3 7 11 //2023-02-06_3 : To make ADC Gap
#else //ADC_VOLUME_50_STEP_ENABLE //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
		if(i==1)
			ADC_Level_Min = 0;
		else
			ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

		if(i==50)
			ADC_Level_Max = 255;
		else
			ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253
#endif //ADC_VOLUME_64_STEP_ENABLE

		if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value))
		{
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
			uCurVolLevel = 64 - i;
#else //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
			uCurVolLevel = 50 - i;
#endif

#ifdef ADC_INPUT_DEBUG_MSG
			_DBG("\n\r === ADC Level = ");
			_DBD(i);
#endif
			break;
		}
	}

#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
	uCurVolLevel = 64 - i; //0 ~ 63(64 Step / 0 - MAX)
#else //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
	uCurVolLevel = 50 - i;
#endif

	//************ Volume Setting End ************//

	//************ Attenuator Setting Start ************//
	//Attenuator action is inversed. So fixed it. //2023-01-05_2						
	ADC2_Value = ADC_PollingRun(2); //0x00 ~ 0xA0(160)
	
#ifdef ADC_INPUT_DEBUG_MSG
	_DBG("\n\r ++++ ADC2 =");
	_DBD(ADC2_Value);
#endif

	for(i=0;i<21;i++) //0 ~ 21 is correct but we'll use that 21 is invalid ADC value.
	{					
		if(i == 0)
		{
			ADC_Level_Min = 0 ; //0 // 4 17 30
			ADC_Level_Max = 3; //3 //16 29 42
		}
		else
		{
			ADC_Level_Min = ((i-1)*13)+4 ; //0 //4 17 30 ... 251

			if(i == 20)
				ADC_Level_Max = 253; //3 //16 29 42 ... 253
			else
				ADC_Level_Max = (i*13)+3; //3 //16 29 42 ... 253
		}
		
		if((ADC2_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC2_Value))
		{
			uAttenuator_Vol = Convert_ADC_To_Attenuator(i);
#ifdef ADC_INPUT_DEBUG_MSG
			_DBG("\n\r === Attenuator ADC Level Step = ");
			_DBD(i);
#endif	
			break;
		}
	}

	//************ Attenuator Setting End ************//

#ifdef ADC_INPUT_DEBUG_MSG
	_DBG("\n\r === Init Volume = ");
	_DBD(uCurVolLevel);
	_DBG("\n\r === Init Attenuator = ");
	_DBD(uAttenuator_Vol);
#endif

	//TAS5806MD_Amp_Volume_Set_with_Index(uCurVolLevel, FALSE, TRUE);
	return uCurVolLevel;
}
#endif //defined(ADC_INPUT_ENABLE) && defined(USEN_BAP)

