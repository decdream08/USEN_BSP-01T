/**********************************************************************
* @file		AD85050.h
* @brief	AD85050
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

typedef enum {
	LL_MODE,
	RR_MODE,
	STEREO_MODE
}Audio_Output_Setting;

typedef enum {
    AD85050_POWER_DOWN,
    AD85050_POWER_UP,
	AD85050_POWER_UP_RESET_ON,
	AD85050_POWER_UP_RESET_OFF,
	AD85050_POWER_UP_INIT,
	AD85050_WAIT_CLK_STABLE,	
	AD85050_POWER_UP_COMPLETE,
    AD85050_CHECK_STATUS,
    AD85050_CHANGE_SOURCE,
    AD85050_ERROR_STATUS,
}AD85050_Status;

#define AD85050_I2C_ADDR						(0x30)

#define AD85050_STATE_CTL5_REG						(0x19)
#define AD85050_DRC_LINK_ENABLE						(0x18)
#define AD85050_DRC_LINK_DISABLE					(0x10)

#define AD85050_STATE_CTL6_REG						(0x1A)
#define AD85050_RESET_ON								(~(0x20))
#define AD85050_RESET_OFF								(0x20)

#define AD85050_DAC_GAIN_REG						(0x36)
#define AD85050_DAC_GAIN_MINUS_4DB					(0xC0)

#define AD85050_STATE_CTL2_REG						(0x01)
#define AD85050_M12D2_SAMPLE_FREQ_48K						(0x91)

#define AD85050_STATE_CTL3_REG						(0x02)
#define AD85050_MASTER_MUTE_ON						(0x40)
#define AD85050_MASTER_MUTE_OFF						(0x00)

#define AD85050_STATE_CTL4_REG						(0x0C)
#define AD85050_CHANNEL2_USE_CHANNEL1_EQ				(0x98)

//RAM Access Register
#define AD85050_COEFFICIENT_RAM_BASE_ADDR_REG		(0x1D)
#define AD85050_TOP_COFFICIENTS_A1_REG				(0x1E)
#define AD85050_MID_COFFICIENTS_A1_REG				(0x1F)
#define AD85050_BOTTOM_COFFICIENTS_A1_REG			(0x20)
#define AD85050_TOP_COFFICIENTS_A2_REG				(0x21)
#define AD85050_MID_COFFICIENTS_A2_REG				(0x22)
#define AD85050_BOTTOM_COFFICIENTS_A2_REG			(0x23)
#define AD85050_TOP_COFFICIENTS_B1_REG				(0x24)
#define AD85050_MID_COFFICIENTS_B1_REG				(0x25)
#define AD85050_BOTTOM_COFFICIENTS_B1_REG			(0x26)
#define AD85050_TOP_COFFICIENTS_B2_REG				(0x27)
#define AD85050_MID_COFFICIENTS_B2_REG				(0x28)
#define AD85050_BOTTOM_COFFICIENTS_B2_REG			(0x29)
#define AD85050_TOP_COFFICIENTS_A0_REG				(0x2A)
#define AD85050_MID_COFFICIENTS_A0_REG				(0x2B)
#define AD85050_BOTTOM_COFFICIENTS_A0_REG			(0x2C)
#define AD85050_RAM_SETTING_REG						(0x2D)

#define AD85050_VOL_CONTROL_REG1					(0x03)
#define AD85050_CHANNEL1_VOL_CONTROL_REG1					(0x04)
#define AD85050_CHANNEL2_VOL_CONTROL_REG1					(0x05)

#define AD85050_ERROR_REG								(0x84)

#define VOLUME_DEFAULT_LEVEL		            (0x32)
#ifdef PP_PBA
#define AUX_MASTER_VOLUME_LEVEL		            (0x03)
#define BT_MASTER_VOLUME_LEVEL		            (0x0C) //(0x0D)
#else
#define AUX_MASTER_VOLUME_LEVEL		            (0x04)//(0x05)
#define BT_MASTER_VOLUME_LEVEL		            (0x33)//(0x19)
#endif

#ifdef TP_PBA
#ifdef PP_PBA
#define AUX_MASTER_VOLUME_LEVEL_BT_OUT_ON		(0x06)
#else
#define AUX_MASTER_VOLUME_LEVEL_BT_OUT_ON		(0x07) //(0x06)
#endif
#endif

void AD85050_10ms_timer(void);
void AD85050_Process(void);

void AD85050_SetStatus(AD85050_Status status);
AD85050_Status AD85050_GetStatus(void);

void AD85050_ErrorProcess(void);
void AD85050_PowerUp(void);
void AD85050_PowerDown(void);
void AD85050_I2C_Init(void);

//Function
void AD85050_Set_Cur_EQ_DRC_Mode(void);

Bool Is_Mute(void);
void Set_Is_Mute(Bool mute_on);
Bool Get_Is_Mute(void);
Bool IS_Display_Mute(void);
void Set_Display_Mute(Bool B_Mute_On_Display); //For LED Display
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
uint8_t AD85050_Amp_Get_Cur_BT_Volume_Level_Inverse(void); //Start count from Min(0)
uint8_t AD85050_Amp_Get_Cur_BT_Volume_Level(void);

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
void AD85050_OutputLimit(Bool enable);
#endif //AD85050_H


