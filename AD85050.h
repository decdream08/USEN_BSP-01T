/**********************************************************************
* @file		AD85050.h
* @brief	IR code
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#ifndef AD85050_H
#define AD85050_H

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

#define AD85050_I2C_ADDR						(0x30) //DDD...TEST

//Function
void AD85050_Set_Cur_EQ_DRC_Mode(void);

Bool Is_Mute(void);
void Set_Is_Mute(Bool mute_on);
#ifdef AD82584F_USE_POWER_DOWN_MUTE
Bool IS_Display_Mute(void);
void Set_Display_Mute(Bool B_Mute_On_Display); //For LED Display
#endif
void AD85050_Amp_EQ_DRC_Control(EQ_Mode_Setting EQ_mode);
void AD85050_Amp_Init(Bool Power_On_Init);
void AD85050_Amp_Reset(Bool Reset_On);
void AD85050_Amp_Mute_Toggle(void); //Toggle
void AD85050_Amp_Move_to_Control_Page(void);
void AD85050_Amp_Mute(Bool Mute_On, Bool LED_Display);
void AD85050_Amp_Volume_Control(Vol_Setting Vol_mode);

void AD85050_Amp_RAM_Single_Write(uint8_t uCount, uint8_t uData);
void AD85050_Amp_RAM_Three_Coeff_Write(uint8_t uCount, uint8_t uData);
void AD85050_Amp_RAM_Set_Write(uint8_t uCount, uint8_t uData);

void AD85050_Amp_Mode_Control(Audio_Output_Setting mode);

void AD85050_Amp_Set_Cur_Volume_Level(uint32_t volume);

Bool AD85050_Amp_Get_Cur_CLK_Status(void);
void AD85050_Amp_Set_Default_Volume(void);

Bool AD85050_Amp_Detect_FS(Bool BInit); //BInit - TRUE : Amp Init / FALSE : Not need Amp Init //2022-10-17_2
uint8_t AD85050_Amp_Detect_Fault(Bool Return_Val_Only); //2022-10-25 : FAULT PIN

uint32_t AD85050_Amp_Get_Cur_Volume_Level(void);
uint32_t AD85050_Amp_Volume_Set_with_Index(uint32_t Vol_Level, Bool Inverse, Bool Actual_Key); //Actual Key says this is not SSP or BLE communication. So, we need to send same key to Slave SPK
uint8_t AD85050_Amp_Get_Cur_Volume_Level_Inverse(void); //Start count from Min(0)

void AD85050_Register_Read(void); //2022-10-25 : FAULT PIN
void AD85050_Fault_Clear_Reg(void); //2022-10-25 : FAULT PIN

#ifdef DRC_TOGGLE_TEST
void AD85050_DRC_On(void);
void AD85050_DRC_Off(void);
#endif
Bool Is_BAmp_Init(void);
Bool Is_I2C_Access_OK(void); //2023-02-27_2

void AD85050_AGL_Value_Change(Switch_BAP_EQ_Mode EQ_Mode, Bool BT_mode); //2023-06-13_1
void AD85050_EQ_OnOff(Bool BEQ_On);
void AD85050_DRC_OnOff(Bool BDRC_On);


void AD85050_Amp_Volume_Register_Writing(uint16_t uVolumeLevel);

void AD85050_Dac_Volume_Set(Bool Aux_Mode); //2023-06-13_1 : Added Parameter
#endif //AD85050_H


