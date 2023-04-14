/**********************************************************************
* @file		tas5806md.h
* @brief	IR code
* @version	1.0
* @date		
* @author	MS Kim
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#ifndef TAS5806MD_H
#define TAS5806MD_H

#include "i2c.h"

//Macro
typedef enum {
	Volume_Up,
	Volume_Down
}Vol_Setting;

#ifdef ADC_INPUT_ENABLE
#ifdef ADC_VOLUME_STEP_ENABLE
typedef enum {
	Attenuator_Volume_MAX, 	//-0dB
	Attenuator_Volume_19, 	//-1dB
	Attenuator_Volume_18, 	//-2dB
	Attenuator_Volume_17, 	//-3dB
	Attenuator_Volume_16, 	//-4dB
	Attenuator_Volume_15, 	//-5dB
	Attenuator_Volume_14, 	//-6dB
	Attenuator_Volume_13,	//-7dB
	Attenuator_Volume_12, 	//-8dB
	Attenuator_Volume_11, 	//-9dB
	Attenuator_Volume_10,	//-10dB
	Attenuator_Volume_9,	//-11dB
	Attenuator_Volume_8,	//-12dB
	Attenuator_Volume_7,	//-13dB
	Attenuator_Volume_6,	//-14dB
	Attenuator_Volume_5,	//-15dB
	Attenuator_Volume_4,	//-16dB
	Attenuator_Volume_3,	//-17dB
	Attenuator_Volume_2,	//-18dB
	Attenuator_Volume_1,	//-19dB
	Attenuator_Volume_MIN	//-20dB
}Attenuator_Volume_Level;
#else //ADC_VOLUME_STEP_ENABLE
typedef enum {
	Attenuator_Volume_Low,
	Attenuator_Volume_Mid,
	Attenuator_Volume_High
}Attenuator_Volume_Level;
#endif //ADC_VOLUME_STEP_ENABLE
#endif

typedef enum {
	LL_MODE,
	RR_MODE,
	STEREO_MODE
}Audio_Output_Setting;

typedef enum {
	TAS5806MD_PWR_Mode_DEEP_SLEEP,
	TAS5806MD_PWR_Mode_SLEEP,
	TAS5806MD_PWR_Mode_HIZ,
	TAS5806MD_PWR_Mode_PLAY
}TAS5806MD_Power_Mode;

#define TAS5806MD_I2C_ADDR						(TAS5806_DEVICE_ADDR_15K) //2022-11-14_1

//Function
Bool Is_Mute(void);
void Set_Is_Mute(Bool mute_on);
#ifdef AD82584F_USE_POWER_DOWN_MUTE
Bool IS_Display_Mute(void);
void Set_Display_Mute(Bool B_Mute_On_Display); //For LED Display
#endif
Bool TAS5806MD_Amp_Get_Cur_Mute_Status(uint8_t *buffer); //TRUE : Mute On / FALSE : Mute Off

void TAS5806MD_Amp_EQ_DRC_Control(EQ_Mode_Setting EQ_mode);

#ifdef FLASH_SELF_WRITE_ERASE
void TAS5806MD_Amp_Init(Bool Power_On_Init);
#else //FLASH_SELF_WRITE_ERASE
void TAS5806MD_Amp_Init();
#endif //FLASH_SELF_WRITE_ERASE
void TAS5806MD_Amp_Reset(Bool Reset_On);
void TAS5806MD_Amp_Mute_Toggle(void); //Toggle
void TAS5806MD_Amp_Move_to_Control_Page(void);
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
void TAS5806MD_Amp_Move_to_Volume_Control_Page(void);
#endif
#ifdef USEN_IT_AMP_EQ_ENABLE //2023-02-27_1
void TAS5806MD_Amp_Move_to_DSP_Control_Page(void);
void TAS58066MD_Amp_Move_to_DRC_band3_Page(void); //2023-03-02_2
#endif
void TAS5806MD_Amp_Mute(Bool Mute_On, Bool LED_Display);
void TAS5806MD_Amp_Volume_Control(Vol_Setting Vol_mode);
void TAS5806MD_Amp_Mode_Control(Audio_Output_Setting mode);

void TAS5806MD_Amp_Set_Cur_Volume_Level(uint8_t volume);
void TAS5806MD_Amp_Set_Default_Volume(void);

void TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_Power_Mode mode);
Bool TAS5806MD_Amp_Detect_FS(Bool BInit); //BInit - TRUE : Amp Init / FALSE : Not need Amp Init //2022-10-17_2
uint8_t TAS5806MD_Amp_Detect_Fault(Bool Return_Val_Only); //2022-10-25 : FAULT PIN

uint8_t TAS5806MD_Amp_Get_Cur_Volume_Level(void);
uint8_t TAS5806MD_Amp_Volume_Set_with_Index(uint8_t Vol_Level, Bool Inverse, Bool Actual_Key); //Actual Key says this is not SSP or BLE communication. So, we need to send same key to Slave SPK
uint8_t TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse(void); //Start count from Min(0)

void TAS5806MD_Register_Read(void); //2022-10-25 : FAULT PIN
void TAS5806MD_Fault_Clear_Reg(void); //2022-10-25 : FAULT PIN

#ifdef DRC_TOGGLE_TEST
void TAS5806MD_DRC_On(void);
void TAS5806MD_DRC_Off(void);
#endif
Bool Is_BAmp_Init(void);
Bool Is_I2C_Access_OK(void); //2023-02-27_2
#ifdef USEN_BAP
void TAS5806MD_AGL_Value_Change(void);
void TAS5806MD_EQ_OnOff(Bool BEQ_On);
void TAS5806MD_DRC_OnOff(Bool BDRC_On);
#endif
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
void TAS5806MD_Amp_Volume_Register_Writing(uint8_t uVolumeLevel);
#endif
#endif //TAS5806MD_H


