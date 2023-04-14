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
* //D:\02_ESTec\01_PRJ\05_USEN\03_SW\03_Amp(AD82584F)\220328_HQ\DRC.txt base
**********************************************************************/
#include "main_conf.h"
#ifdef AD82584F_ENABLE
#include "i2c.h"
#include "ad82584f.h"
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
#include "led_display.h"
#endif
#ifdef MB3021_ENABLE
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
#include "bt_MB3021.h"
#endif
#endif
#if defined(AUTO_VOLUME_LED_OFF) || defined(SLAVE_ADD_MUTE_DELAY_ENABLE)
#include "timer20.h"
#endif
#ifdef FLASH_SELF_WRITE_ERASE
#include "flash.h"
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
#include "remocon_action.h"
#endif
#endif //FLASH_SELF_WRITE_ERASE

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/

//#define AD82584F_DEBUG									(1)

#define AD82584F_DEVICE_ADDR							(0x31)

#define AD82584F_STATE_CTL3_REG						(0x02)
#define AD82584F_MASTER_MUTE_ON						(0x40)
#define AD82584F_MASTER_MUTE_OFF						(~(0x40))

#define AD82584F_STATE_CTL4_REG						(0x03)
#define AD82584F_MASTER_VOL_CTL_REG					(AD82584F_STATE_CTL4_REG)

#define AD82584F_STATE_CTL5_REG						(0x1A)
#define AD82584F_RESET_ON								(~(0x20))
#define AD82584F_RESET_OFF								(0x20)

#define AD82584F_STATE_CTL6_REG						(0x1B)

#define AD82584F_STATE_CTL7_REG						(0x1C)
#define AD82548F_A_SEL_FAULT_ON						(0x40)
#define AD82548F_FADE_IN_OUT_SPEED_10MS				(0x40)
#define AD82548F_FADE_IN_OUT_SPEED_1MS					(~(0x40))

//RAM Access Register
#define AD82584F_COEFFICIENT_RAM_BASE_ADDR_REG		(0x1D)
#define AD82584F_TOP_COFFICIENTS_A1_REG				(0x1E)
#define AD82584F_MID_COFFICIENTS_A1_REG				(0x1F)
#define AD82584F_BOTTOM_COFFICIENTS_A1_REG			(0x20)
#define AD82584F_TOP_COFFICIENTS_A2_REG				(0x21)
#define AD82584F_MID_COFFICIENTS_A2_REG				(0x22)
#define AD82584F_BOTTOM_COFFICIENTS_A2_REG			(0x23)
#define AD82584F_TOP_COFFICIENTS_B1_REG				(0x24)
#define AD82584F_MID_COFFICIENTS_B1_REG				(0x25)
#define AD82584F_BOTTOM_COFFICIENTS_B1_REG			(0x26)
#define AD82584F_TOP_COFFICIENTS_B2_REG				(0x27)
#define AD82584F_MID_COFFICIENTS_B2_REG				(0x28)
#define AD82584F_BOTTOM_COFFICIENTS_B2_REG			(0x29)
#define AD82584F_TOP_COFFICIENTS_A0_REG				(0x2A)
#define AD82584F_MID_COFFICIENTS_A0_REG				(0x2B)
#define AD82584F_BOTTOM_COFFICIENTS_A0_REG			(0x2C)
#define AD82584F_RAM_SETTING_REG						(0x2D)

#define AD82584F_MIXER1								(0x4B)
#define AD82584F_MIXER2								(0x4C)

#define AD82584F_EQ1									(0x00)
#define AD82584F_EQ2									(0x05)
#define AD82584F_EQ3									(0x0A)
#define AD82584F_EQ4									(0x0F)
#define AD82584F_EQ5									(0x14)
#define AD82584F_EQ6									(0x19)
#define AD82584F_EQ7									(0x1E)
#define AD82584F_EQ8									(0x23)
#define AD82584F_EQ9									(0x28)
#define AD82584F_EQ10									(0x2D)
#define AD82584F_EQ11									(0x32)
#define AD82584F_EQ12									(0x37)
#define AD82584F_EQ13									(0x3C)
#define AD82584F_EQ14									(0x41)
#define AD82584F_EQ15									(0x46)

#define AD82584F_LSRS_HPF_A0								(0x4F)
#define AD82584F_LSRS_LPF_A0								(0x52)
#define AD82584F_RSRS_HPF_A0								(0x4F)
#define AD82584F_RSRS_LPF_A0								(0x52)

#define AD82584F_PROTECTION_REG								(0x84)

#define MAX_VOLUME_LEVEL		(0)
#define VOLUME_LEVEL_NUMER	(sizeof(AD82584F_Volume_Table))
#define MIN_VOLUME_LEVEL		(VOLUME_LEVEL_NUMER - 1)

#define VOLUME_DEFAULT_LEVEL		(0x32)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
/* Private define ----------------------------------------------------*/
/* Private function prototypes ---------------------------------------*/
/* Private variables ---------------------------------------------------*/
uint8_t uCurrent_Vol_Level = 0;
static Bool IS_Mute = FALSE;
#ifdef AD82584F_USE_POWER_DOWN_MUTE
static Bool Display_Mute = FALSE;
#endif

uint8_t AD82584F_Volume_Table[] = { // 16-Step // Value 0(Max) ~ 15(Min)
#ifdef USEN_EQ_ENABLE
	0x00,	// MAX
	0x01,
	0x03,
	0x06,
	0x0B,	
	0x11, 
	0x16,
	0x1b,	
	0x22,	
	0x2a,	
	VOLUME_DEFAULT_LEVEL, //Default : 0x32
	0x37,
	0x3e,
	0x45,
	0x58,
	0xff	// Mute
#else //USEN_EQ_ENABLE
	0x23,	// + 12 dB
	0x2b, 	
	0x33,
	0x3b,	
	0x43,	
	0x4b,	// default
	0x53,	
	0x5b,
	0x63,
	0x6b,
	0x73,
	0x7b,
	0x83,
	0x8b,
	0x93,
	0xff	// Mute
#endif //USEN_EQ_ENABLE
};

#ifdef USEN_EQ_ENABLE
#ifdef SRS_FILTER_ENABLE
const uint8_t SRS_FILTER_Table [] = {
AD82584F_LSRS_HPF_A0, 0xf0, 0x1a, 0xa3, 0x0f, 0xe5, 0x5d, 0x0f, 0xca, 0xbb, 0x10,	\
AD82584F_RSRS_HPF_A0, 0xf0, 0x1a, 0xa3, 0x0f, 0xe5, 0x5d, 0x0f, 0xca, 0xbb, 0x50,	\
AD82584F_LSRS_LPF_A0, 0x1f, 0xe5, 0x5d, 0xe0, 0x4f, 0xe8, 0x0f, 0xca, 0xbb, 0x10,	\
AD82584F_RSRS_LPF_A0, 0x1f, 0xe5, 0x5d, 0xe0, 0x4f, 0xe8, 0x0f, 0xca, 0xbb, 0x50};
#endif

const uint8_t EQ_Normal_DRC_Table [] = {//Noraml
0x56, 0x10, 0xfd, 0x01, 0x01, \
0x57, 0x10, 0x81, 0x9d, 0x01, \
0x5a, 0x01, 0x12, 0x67, 0x01, \
0x5b, 0x01, 0x0a, 0x9e, 0x01, \
0x5c, 0x0d, 0xe3, 0x69, 0x01, \
0x5d, 0x0f, 0x24, 0x0e, 0x01, \
0x60, 0x04, 0x00, 0x00, 0x01, \
0x62, 0x04, 0x00, 0x00, 0x01, \
0x63, 0x04, 0x00, 0x00, 0x01};

const uint8_t EQ_Normal_Table [] = {//Noraml
AD82584F_EQ15, 0xe0, 0x9d, 0xc5, 0x00, 0x00, 0x00, 0x1e, 0xc4, 0x77, 0x00, 0x00, 0x00, 0x1f, 0x62, 0x3b, 0x02, 	\
AD82584F_EQ15, 0xe0, 0x9d, 0xc5, 0x00, 0x00, 0x00, 0x1e, 0xc4, 0x77, 0x00, 0x00, 0x00, 0x1f, 0x62, 0x3b, 0x42};

const uint8_t EQ_Table[4][85]={
	 //Rock
	{AD82584F_EQ1, 0xc5, 0xf5, 0x6e, 0x1a, 0x8a, 0xc7, 0x3a, 0x0a, 0x92, 0xe5, 0x75, 0x39, 0x20, 0x00, 0x00, 0x02, 	\
	AD82584F_EQ2, 0xc2, 0xc5, 0x45, 0x1e, 0x18, 0xa2, 0x3d, 0x3a, 0xbb, 0xe2, 0xb9, 0x2e, 0x1f, 0x2e, 0x2f, 0x02, 	\
	AD82584F_EQ3, 0xca, 0x18, 0x99, 0x15, 0x68, 0xa9, 0x35, 0xe7, 0x67, 0xe8, 0x31, 0xd6, 0x22, 0x65, 0x81, 0x02, 	\
	AD82584F_EQ4, 0xcf, 0x6b, 0x56, 0x11, 0x3e, 0x8c, 0x30, 0x94, 0xaa, 0xeb, 0x6a, 0xb7, 0x23, 0x56, 0xbc, 0x02, 	\
	AD82584F_EQ5, 0xe9, 0x5b, 0xd6, 0x01, 0xaf, 0x90, 0x16, 0xa4, 0x2a, 0xf2, 0xb8, 0x49, 0x2b, 0x98, 0x26, 0x02},
	//Hiphop
	{AD82584F_EQ1, 0xc0, 0x17, 0x21, 0x1f, 0xc6, 0xb7, 0x3f, 0xe8, 0xdf, 0xe0, 0x17, 0x00, 0x20, 0x22, 0x48, 0x02, 	\
	AD82584F_EQ2, 0xc0, 0x64, 0x50, 0x1f, 0x33, 0x14, 0x3f, 0x9b, 0xb0, 0xe0, 0x62, 0x78, 0x20, 0x6a, 0x74, 0x02, 	\
	AD82584F_EQ3, 0xc2, 0xb8, 0x0a, 0x1d, 0x63, 0xd3, 0x3d, 0x47, 0xf6, 0xe2, 0x9c, 0x2d, 0x20, 0x00, 0x00, 0x02, 	\
	AD82584F_EQ4, 0xc9, 0x6c, 0xc5, 0x16, 0x80, 0x67, 0x36, 0x93, 0x3b, 0xe7, 0xdf, 0xcd, 0x21, 0x9f, 0xcb, 0x02, 	\
	AD82584F_EQ5, 0xe2, 0x74, 0x0c, 0xff, 0xd0, 0x31, 0x1d, 0x8b, 0xf4, 0xef, 0x77, 0x48, 0x30, 0xb8, 0x88, 0x02},
	//Jazz
	{AD82584F_EQ1, 0xc0, 0xd6, 0x6f, 0x1e, 0xf0, 0x57, 0x3f, 0x29, 0x91, 0xe0, 0xd2, 0x31, 0x20, 0x3d, 0x78, 0x02, 	\
	AD82584F_EQ2, 0xc2, 0x4c, 0x6c, 0x1c, 0xfd, 0xae, 0x3d, 0xb3, 0x94, 0xe2, 0x2a, 0x88, 0x20, 0xd7, 0xca, 0x02, 	\
	AD82584F_EQ3, 0xc7, 0x90, 0x5c, 0x1a, 0x78, 0xc1, 0x38, 0x6f, 0xa4, 0xe7, 0x13, 0xb2, 0x1e, 0x73, 0x8c, 0x02, 	\
	AD82584F_EQ4, 0xd1, 0x9a, 0x14, 0x14, 0x47, 0xf7, 0x2e, 0x65, 0xec, 0xef, 0x00, 0x6f, 0x1c, 0xb7, 0x99, 0x02, 	\
	AD82584F_EQ5, 0xee, 0xed, 0xd3, 0x09, 0x99, 0x61, 0x11, 0x14, 0x2d, 0xfd, 0xd8, 0x1c, 0x18, 0x8e, 0x82, 0x02},
	//Pop
	{AD82584F_EQ1, 0xc0, 0xca, 0xd2, 0x1e, 0xec, 0x27, 0x3f, 0x35, 0x2e, 0xe0, 0xc6, 0x93, 0x20, 0x4d, 0x45, 0x02, 	\
	AD82584F_EQ2, 0xc2, 0x2e, 0x75, 0x1c, 0xee, 0x7d, 0x3d, 0xd1, 0x8b, 0xe2, 0x0c, 0x81, 0x21, 0x05, 0x02, 0x02, 	\
	AD82584F_EQ3, 0xd4, 0x02, 0x5a, 0x12, 0xa3, 0x71, 0x2b, 0xfd, 0xa6, 0xf0, 0x62, 0x8a, 0x1c, 0xfa, 0x04, 0x02, 	\
	AD82584F_EQ4, 0xd9, 0xc3, 0xc1, 0x0f, 0xd0, 0x0f, 0x26, 0x3c, 0x3f, 0xf3, 0xd9, 0x9b, 0x1c, 0x56, 0x55, 0x02, 	\
	AD82584F_EQ5, 0xee, 0x02, 0x42, 0x09, 0x26, 0x90, 0x11, 0xfd, 0xbe, 0xfc, 0x05, 0x02, 0x1a, 0xd4, 0x6d, 0x02}
};
#else //USEN_EQ_ENABLE
const uint8_t EQ_Table[4][85]={
	 //Rock
	{AD82584F_EQ1, 0xc5, 0xf5, 0x6e, 0x1a, 0x8a, 0xc7, 0x3a, 0x0a, 0x92, 0xe5, 0x75, 0x39, 0x20, 0x00, 0x00, 0x02, 	\
	AD82584F_EQ2, 0xc2, 0xc5, 0x45, 0x1e, 0x18, 0xa2, 0x3d, 0x3a, 0xbb, 0xe2, 0xb9, 0x2e, 0x1f, 0x2e, 0x2f, 0x02, 	\
	AD82584F_EQ3, 0xca, 0x18, 0x99, 0x15, 0x68, 0xa9, 0x35, 0xe7, 0x67, 0xe8, 0x31, 0xd6, 0x22, 0x65, 0x81, 0x02, 	\
	AD82584F_EQ4, 0xcf, 0x6b, 0x56, 0x11, 0x3e, 0x8c, 0x30, 0x94, 0xaa, 0xeb, 0x6a, 0xb7, 0x23, 0x56, 0xbc, 0x02, 	\
	AD82584F_EQ5, 0xe9, 0x5b, 0xd6, 0x01, 0xaf, 0x90, 0x16, 0xa4, 0x2a, 0xf2, 0xb8, 0x49, 0x2b, 0x98, 0x26, 0x02},
	//Jazz
	{AD82584F_EQ1, 0xc0, 0xd6, 0x6f, 0x1e, 0xf0, 0x57, 0x3f, 0x29, 0x91, 0xe0, 0xd2, 0x31, 0x20, 0x3d, 0x78, 0x02, 	\
	AD82584F_EQ2, 0xc2, 0x4c, 0x6c, 0x1c, 0xfd, 0xae, 0x3d, 0xb3, 0x94, 0xe2, 0x2a, 0x88, 0x20, 0xd7, 0xca, 0x02, 	\
	AD82584F_EQ3, 0xc7, 0x90, 0x5c, 0x1a, 0x78, 0xc1, 0x38, 0x6f, 0xa4, 0xe7, 0x13, 0xb2, 0x1e, 0x73, 0x8c, 0x02, 	\
	AD82584F_EQ4, 0xd1, 0x9a, 0x14, 0x14, 0x47, 0xf7, 0x2e, 0x65, 0xec, 0xef, 0x00, 0x6f, 0x1c, 0xb7, 0x99, 0x02, 	\
	AD82584F_EQ5, 0xee, 0xed, 0xd3, 0x09, 0x99, 0x61, 0x11, 0x14, 0x2d, 0xfd, 0xd8, 0x1c, 0x18, 0x8e, 0x82, 0x02},
	//Classic
	{AD82584F_EQ1, 0xc2, 0xb0, 0x8f, 0x1d, 0x1c, 0x56, 0x3d, 0x4f, 0x71, 0xe2, 0x8e, 0xe2, 0x20, 0x54, 0xc8, 0x02, 	\
	AD82584F_EQ2, 0xc7, 0x7c, 0x31, 0x17, 0xc1, 0xbd, 0x38, 0x83, 0xcf, 0xe6, 0x60, 0xc4, 0x21, 0xdd, 0x7f, 0x02, 	\
	AD82584F_EQ3, 0xc8, 0xc9, 0x0b, 0x16, 0x8f, 0x3d, 0x37, 0x36, 0xf5, 0xe7, 0x4d, 0xea, 0x22, 0x22, 0xd9, 0x02, 	\
	AD82584F_EQ4, 0xd4, 0x02, 0x5a, 0x12, 0xa3, 0x71, 0x2b, 0xfd, 0xa6, 0xf0, 0x62, 0x8a, 0x1c, 0xfa, 0x04, 0x02, 	\
	AD82584F_EQ5, 0xed, 0x8e, 0xa1, 0x08, 0xd7, 0xec, 0x12, 0x71, 0x5f, 0xfb, 0x1d, 0xc4, 0x1c, 0x0a, 0x50, 0x02},
	//Pop
	{AD82584F_EQ1, 0xc0, 0xca, 0xd2, 0x1e, 0xec, 0x27, 0x3f, 0x35, 0x2e, 0xe0, 0xc6, 0x93, 0x20, 0x4d, 0x45, 0x02, 	\
	AD82584F_EQ2, 0xc2, 0x2e, 0x75, 0x1c, 0xee, 0x7d, 0x3d, 0xd1, 0x8b, 0xe2, 0x0c, 0x81, 0x21, 0x05, 0x02, 0x02, 	\
	AD82584F_EQ3, 0xd4, 0x02, 0x5a, 0x12, 0xa3, 0x71, 0x2b, 0xfd, 0xa6, 0xf0, 0x62, 0x8a, 0x1c, 0xfa, 0x04, 0x02, 	\
	AD82584F_EQ4, 0xd9, 0xc3, 0xc1, 0x0f, 0xd0, 0x0f, 0x26, 0x3c, 0x3f, 0xf3, 0xd9, 0x9b, 0x1c, 0x56, 0x55, 0x02, 	\
	AD82584F_EQ5, 0xee, 0x02, 0x42, 0x09, 0x26, 0x90, 0x11, 0xfd, 0xbe, 0xfc, 0x05, 0x02, 0x1a, 0xd4, 0x6d, 0x02}
};
#endif //USEN_EQ_ENABLE

uint8_t LR_Mode_Ctl_Table[3][20]={
//CH1_MIX1: COEFFICIENT_RAM_BASE_ADDR, TOP_COFFICIENTS_A1, MID_COFFICIENTS_A1, BOTTOM_COFFICIENTS_A1, RAM_SETTING \
//CH1_MIX2: COEFFICIENT_RAM_BASE_ADDR, TOP_COFFICIENTS_A1, MID_COFFICIENTS_A1, BOTTOM_COFFICIENTS_A1, RAM_SETTING \
//CH2_MIX1: COEFFICIENT_RAM_BASE_ADDR, TOP_COFFICIENTS_A1, MID_COFFICIENTS_A1, BOTTOM_COFFICIENTS_A1, RAM_SETTING \
//CH2_MIX2: COEFFICIENT_RAM_BASE_ADDR, TOP_COFFICIENTS_A1, MID_COFFICIENTS_A1, BOTTOM_COFFICIENTS_A1, RAM_SETTING
	{AD82584F_MIXER1, 0x7f, 0xff, 0xff, 0x01, AD82584F_MIXER2, 0x00, 0x00, 0x00, 0x01, AD82584F_MIXER1, 0x7f, 0xff, 0xff, 0x41, AD82584F_MIXER2, 0x00, 0x00, 0x00, 0x41}, //LL Mode
	{AD82584F_MIXER1, 0x00, 0x00, 0x00, 0x01, AD82584F_MIXER2, 0x7f, 0xff, 0xff, 0x01, AD82584F_MIXER1, 0x00, 0x00, 0x00, 0x41, AD82584F_MIXER2, 0x7f, 0xff, 0xff, 0x41}, //RR Mode
	{AD82584F_MIXER1, 0x7f, 0xff, 0xff, 0x01, AD82584F_MIXER2, 0x00, 0x00, 0x00, 0x01, AD82584F_MIXER1, 0x00, 0x00, 0x00, 0x41, AD82584F_MIXER2, 0x7f, 0xff, 0xff, 0x41} //STEREO Mode	
};

#ifdef DRC_TOGGLE_TEST
const uint8_t AD82584F_DRC_On_Value[][2] = {
	{0x0d, 0x00}, //##Channel_1_configuration_registers
	{0x0e, 0x00}, // ##Channel_2_configuration_registers
	{0x0f, 0x00}, // ##Channel_3_configuration_registers
	{0x10, 0x00}, // ##Channel_4_configuration_registers
	{0x11, 0x00}, // ##Channel_5_configuration_registers
	{0x12, 0x00}, // ##Channel_6_configuration_registers
	{0x13, 0x00}, // ##Channel_7_configuration_registers
	{0x14, 0x00} // ##Channel_8_configuration_registers
};

const uint8_t AD82584F_DRC_Off_Value[][2] = {
	{0x0d, 0x04}, //##Channel_1_configuration_registers
	{0x0e, 0x04}, // ##Channel_2_configuration_registers
	{0x0f, 0x04}, // ##Channel_3_configuration_registers
	{0x10, 0x04}, // ##Channel_4_configuration_registers
	{0x11, 0x04}, // ##Channel_5_configuration_registers
	{0x12, 0x04}, // ##Channel_6_configuration_registers
	{0x13, 0x04}, // ##Channel_7_configuration_registers
	{0x14, 0x04} // ##Channel_8_configuration_registers
};
#endif

const uint8_t AD82584F_Register_Init_Value[][2] = {
	{0x01, 0x81}, //State_Control_2 //Under 48kHz //Do not changed Low nibble(0x1) !!!
	{0x02, 0x0c}, //State_Control_3
	{0x03, VOLUME_DEFAULT_LEVEL}, //{0x03, 0x11}, //Master_volume_control //Default Value
	{0x04, 0x00},//##Channel_1_volume_control
    {0x05, 0x00},//##Channel_2_volume_control
    {0x08, 0x08},//##Channel_5_volume_control
    {0x09, 0x08},//##Channel_6_volume_control
	{0x0c, 0x90}, //State_Control_4
	//{0x15, 0x6a},//##DRC1_limiter_attack/release_rate
	{0x17, 0x49},//##DRC3_limiter_attack/release_rate
	{0x18, 0x64},//##DRC4_limiter_attack/release_rate
	{0x1b, 0xa1}, //##HVUV_selection //!!!
	{0x1c, 0x04},//##State_Control_6 //Do not delete !!!(Bit2 : Fade time 10ms)
	/////////////EQ Side (Do we need these values???) To Do
	{0x1d, 0x46},//##Coefficient_RAM_Base_Address
	{0x1e, 0xe0},//##Top_8-bits_of_coefficients_A1
	{0x1f, 0x9d},//##Middle_8-bits_of_coefficients_A1
	{0x20, 0xc5},//##Bottom_8-bits_of_coefficients_A1
	{0x24, 0x1e},//##Top_8-bits_of_coefficients_B1
	{0x25, 0xc4},//##Middle_8-bits_of_coefficients_B1
	{0x26, 0x77},//##Bottom_8-bits_of_coefficients_B1
	{0x2a, 0x1f},//##Top_8-bits_of_coefficients_A0
	{0x2b, 0x62},//##Middle_8-bits_of_coefficients_A0
	{0x2c, 0x3b},//##Bottom_8-bits_of_coefficients_A0
	{0x2d, 0x40},//##Coefficient_R/W_control
};

Bool Is_Mute(void)
{
	return IS_Mute;
}

#ifdef AD82584F_USE_POWER_DOWN_MUTE
Bool IS_Display_Mute(void) //For LED Display
{
	return Display_Mute;
}
#endif

void Set_Is_Mute(Bool mute_on) //For Actual Mute Status
{
	IS_Mute = mute_on;
}

#ifdef DRC_TOGGLE_TEST
void AD82584F_DRC_On(void)
{
	uint32_t uSize = 0, i;
	uint8_t Data = 0;

	uSize = sizeof(AD82584F_DRC_On_Value)/2;

	for(i =0;i<uSize;i++)
	{
		Data = AD82584F_DRC_On_Value[i][1];
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_DRC_On_Value[i][0],&Data,1);
	}
}

void AD82584F_DRC_Off(void)
{
	uint32_t uSize = 0, i;
	uint8_t Data = 0;

	uSize = sizeof(AD82584F_DRC_Off_Value)/2;

	for(i =0;i<uSize;i++)
	{
		Data = AD82584F_DRC_Off_Value[i][1];
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_DRC_Off_Value[i][0],&Data,1);
	}
}
#endif

#ifdef FLASH_SELF_WRITE_ERASE
void AD82584F_Amp_Init(Bool Power_On_Init)
#else
void AD82584F_Amp_Init(void)
#endif
{
	uint32_t uSize = 0, i;
	uint8_t Data = 0;
#ifdef TIMER30_LED_PWM_ENABLE
	uint8_t uRead = 0, uCurVolLevel = 0;
#endif
#ifdef FLASH_SELF_WRITE_ERASE
	uint8_t uVol_Level = 0;
	uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END];
#endif
#ifdef AD82584F_DEBUG
	_DBG("\n\rAD82584F_Amp_Init");
#endif

#ifndef FLASH_SELF_WRITE_ERASE_EXTENSION //We don't need to init for Display_Mute because we need to keep mute status as spec change
#ifdef AD82584F_USE_POWER_DOWN_MUTE
	Display_Mute = FALSE;
#endif
#endif

	AD82584F_Amp_Reset(TRUE);
	delay_ms(10);
	AD82584F_Amp_Reset(FALSE);
	
	uSize = sizeof(AD82584F_Register_Init_Value)/2;

	for(i =0;i<uSize;i++)
	{
		Data = AD82584F_Register_Init_Value[i][1];
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_Register_Init_Value[i][0],&Data,1);
	}

#ifdef AD82584F_DEBUG
	_DBG("\n\rTAS3251_DSP_Amp_Init - OK !!!");
#endif


#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE) //Default Volume LED display
	LED_Display_Volume_All_Off(); //All Volume LEDs are turned off upon initializing
#ifdef FLASH_SELF_WRITE_ERASE
	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //Read Volume Level from Flash and set the value to Amp Device

	uVol_Level = uFlash_Read_Buf[FLASH_SAVE_DATA_VOLUME];

	if(uVol_Level == 0xff)
	{
#ifdef _DBG_FLASH_WRITE_ERASE
		_DBG("\n\r#### Flash Data : First Init(Volume Level is 0xff)");
#endif
		I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_MASTER_VOL_CTL_REG,&uRead,1);

		// Check whether current volume level is Max/Min or not
		if((uRead >= AD82584F_Volume_Table[MIN_VOLUME_LEVEL])	\
			|| (uRead <= AD82584F_Volume_Table[MAX_VOLUME_LEVEL]))
		{
#ifdef AD82584F_DEBUG
			_DBG("\n\rCan't change current volume level becuase it's a MAX or MIN !!! uRead = 0x");
			_DBH(uRead);
#endif
			return;
		}

		for(i =0; i < VOLUME_LEVEL_NUMER; i++)
		{
			if(AD82584F_Volume_Table[i] == uRead)
			{
				uCurVolLevel = i;
#ifdef AD82584F_DEBUG
				_DBG("\n\ruDefault CurVolLevel : 0x");
				_DBH(uCurVolLevel);
#endif
				break;
			}
		}		
		AD82584F_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
		LED_Display_Volume(uCurVolLevel);
#endif
	}
	else
	{
#ifdef _DBG_FLASH_WRITE_ERASE
		_DBG("\n\r#### Flash Data : Find Volume Level");
#endif
		if(Power_On_Init == TRUE)
			AD82584F_Amp_Volume_Set_with_Index(uVol_Level, FALSE, FALSE); //Power On Init Call
		else
			AD82584F_Amp_Volume_Set_with_Index(uVol_Level, FALSE, TRUE); //Actual Key Input

		AD82584F_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
		//LED_Display_Volume(uCurVolLevel); //Don't need to call duplicated. AD82584F_Amp_Volume_Set_with_Index() function calls LED_Display_Volume().
#endif
	}
#else //FLASH_SELF_WRITE_ERASE
	I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_MASTER_VOL_CTL_REG,&uRead,1);

	// Check whether current volume level is Max/Min or not
	if((uRead >= AD82584F_Volume_Table[MIN_VOLUME_LEVEL])	\
		|| (uRead <= AD82584F_Volume_Table[MAX_VOLUME_LEVEL]))
	{
#ifdef AD82584F_DEBUG
		_DBG("\n\rCan't change current volume level becuase it's a MAX or MIN !!! uRead = 0x");
		_DBH(uRead);
#endif
		return;
	}

	for(i =0; i < VOLUME_LEVEL_NUMER; i++)
	{
		if(AD82584F_Volume_Table[i] == uRead)
		{
			uCurVolLevel = i;
#ifdef AD82584F_DEBUG
			_DBG("\n\ruDefault CurVolLevel : 0x");
			_DBH(uCurVolLevel);
#endif
			break;
		}
	}
	
	AD82584F_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uCurVolLevel);
#endif
#endif //FLASH_SELF_WRITE_ERASE
#endif //TIMER30_LED_PWM_ENABLE

	//Error Pin Enable - 0x1C(B[6] = 1) //Fade In/Out Speed set to 10ms instead of default 1.25ms
	I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL7_REG,&uRead,1);
	Data = uRead | AD82548F_A_SEL_FAULT_ON;
	Data |= AD82548F_FADE_IN_OUT_SPEED_10MS;
	I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL7_REG,&Data,1);
}

void AD82584F_Amp_Reset(Bool Reset_On)
{
	uint8_t uRead = 0;

#ifdef AD82584F_DEBUG
	_DBG("\n\rAD82584F_Amp_Reset() : Reset_On = ");
	_DBD(Reset_On);
#endif

	if(Reset_On)
	{
		I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL5_REG,&uRead,1);
		uRead &= AD82584F_RESET_ON;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL5_REG,&uRead,1);
	}
	else
	{
		I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL5_REG,&uRead,1);
		uRead |= AD82584F_RESET_OFF;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL5_REG,&uRead,1);
	}
}

void AD82584F_Amp_Mute(Bool Mute_On, Bool LED_Display)
{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
#endif
#ifndef AD82584F_USE_POWER_DOWN_MUTE
	uint8_t uRead = 0;
#endif
#ifdef AUTO_VOLUME_LED_OFF //For SPP/BLE Data
	uint8_t uVolume_Level = 0;
#endif
	
#ifdef MUTE_CHECK_DEBUG_MSG	
	_DBG("\n\rAD82584F_Amp_Mute()");
#endif

#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION //When SPK goes to Power Off on booting by reading flash data, we need to return mute mode to avoid mute off
	if(!Power_State())
	{
#ifdef MUTE_CHECK_DEBUG_MSG	
		_DBG("\n\rPower Off mode : return ~!!!");
#endif		
		return;
	}
#endif

#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
#endif

#ifdef AD82584F_USE_POWER_DOWN_MUTE
	if(Mute_On) //Mute On
	{
#ifdef MUTE_CHECK_DEBUG_MSG	
		_DBG("\n\rMute On !!!");
#endif
		IS_Mute = TRUE;

		if(LED_Display)
		{
			Display_Mute = TRUE;
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
			if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
				FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
#endif
		}
		
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN : ON //Mute On
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		if(LED_Display)
			Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif
	}
	else //Mute Off
	{
#ifdef MUTE_CHECK_DEBUG_MSG	
		_DBG("\n\rMute Off !!!");
#endif
		IS_Mute = FALSE;

		if(LED_Display)
		{
			Display_Mute = FALSE;
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
			if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
				FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash
#endif
		}
		
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN : OFF //Mute Off
#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
		if(LED_Display)
		{
			if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
				Set_Status_LED_Mode(STATUS_AUX_MODE);
			else
				Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
		}
#endif
	}
#else //AD82584F_USE_POWER_DOWN_MUTE
	IS_Mute = AD82584F_Amp_Get_Cur_Mute_Status();

	if(IS_Mute == Mute_On)
		return;

	if(Mute_On) // A Cur_State is Mute Off and need to change to Mute On
	{ //Set Mute On
		uRead |= AD82584F_MASTER_MUTE_ON;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL3_REG,&uRead,1);
#ifdef AD82584F_DEBUG
		_DBG("\n\rMute On");
#endif
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		if(LED_Display)
		{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
			if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
				FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
#endif

			Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
		}
#endif
	}
	else // A Cur_State is Mute On and need to change to Mute Off
	{ //Set Mute Off
		uRead &= AD82584F_MASTER_MUTE_OFF;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL3_REG,&uRead,1);
#ifdef AD82584F_DEBUG
		_DBG("\n\rMute Off");
#endif
#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
		if(LED_Display)
		{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
			if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
				FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash
#endif
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
		}
#endif
	}
#endif //AD82584F_USE_POWER_DOWN_MUTE

#ifdef AUTO_VOLUME_LED_OFF //For SPP/BLE Data
	if(Display_Mute)
	{
		TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
		uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
		AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
	}
	else
		TIMER20_auto_volume_led_off_flag_Start(); //do not display LED volume after 10s Under Mute Off
#endif

}

void AD82584F_Amp_Mute_Toggle(void) //Toggle
{
#ifdef AD82584F_USE_POWER_DOWN_MUTE
#ifndef FLASH_SELF_WRITE_ERASE_EXTENSION
	static Bool Mute_On = FALSE; //We don't use this variable becasuse we need to set MUTE ON/OFF state using flash data on power on
#endif
#else	
	uint8_t uRead = 0;
	Bool Mute_On = FALSE;
#endif
#ifdef AUTO_VOLUME_LED_OFF
	uint8_t uVolume_Level = 0;
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
#endif

#ifdef MUTE_CHECK_DEBUG_MSG	
	_DBG("\n\rAD82584F_Amp_Mute_Toggle()");
#endif

#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
#endif

#ifdef AD82584F_USE_POWER_DOWN_MUTE
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0x01)
#else
	if(!Mute_On) //PF4 - High(Mute Off State) : Mute On
#endif
	{
		IS_Mute = TRUE;
		Display_Mute = TRUE;
		
#ifdef MUTE_CHECK_DEBUG_MSG
		_DBG("\n\rMute On");
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
#endif
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN : ON
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif
	}
	else //Mute Off
	{
		IS_Mute = FALSE;
		Display_Mute = FALSE;
		
#ifdef MUTE_CHECK_DEBUG_MSG
		_DBG("\n\rMute Off");
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash
#endif
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN : OFF
#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
		if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
#endif
	}
#ifndef FLASH_SELF_WRITE_ERASE_EXTENSION
	Mute_On = IS_Mute;
#endif
#else //AD82584F_USE_POWER_DOWN_MUTE
	Mute_On = AD82584F_Amp_Get_Cur_Mute_Status();

	if(Mute_On) // A Cur_State is Mute On and need to change to Mute Off
	{ //Set Mute Off
		uRead &= AD82584F_MASTER_MUTE_OFF;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL3_REG,&uRead,1);
#ifdef AD82584F_DEBUG
		_DBG("\n\rMute Off");
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash
#endif

#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
		Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
#endif
	}		
	else // A Cur_State is Mute Off and need to change to Mute On
	{ //Set Mute On
		uRead |= AD82584F_MASTER_MUTE_ON;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL3_REG,&uRead,1);
#ifdef AD82584F_DEBUG
		_DBG("\n\rMute On");
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
#endif

#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif
	}
#endif //AD82584F_USE_POWER_DOWN_MUTE

#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	if(IS_Mute)
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Mute, 0x01);
	else
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Mute, 0x00);
#endif
#ifdef AUTO_VOLUME_LED_OFF
	if(Display_Mute)
	{
		TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
		uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
		AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
	}
	else
		TIMER20_auto_volume_led_off_flag_Start(); //do not display LED volume after 10s Under Mute Off
#endif

}

void AD82584F_Amp_On_Off(void) //Toggle
{
	uint8_t uRead = 0;
	Bool Mute_On;

#ifdef AD82584F_DEBUG	
	_DBG("\n\rTAS3251_DSP_Amp_Mute()");
#endif

	Mute_On = AD82584F_Amp_Get_Cur_Mute_Status();

	if(Mute_On) // A Cur_State is Mute On and need to change to Mute Off
	{ //Set Mute Off
		uRead &= AD82584F_MASTER_MUTE_OFF;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL3_REG,&uRead,1);
#ifdef AD82584F_DEBUG
		_DBG("\n\rMute Off");
#endif
#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
		Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
#endif
	}		
	else // A Cur_State is Mute Off and need to change to Mute On
	{ //Set Mute On
		uRead |= AD82584F_MASTER_MUTE_ON;
		I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL3_REG,&uRead,1);
#ifdef AD82584F_DEBUG
		_DBG("\n\rMute On");
#endif
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif
	}
}

uint8_t AD82584F_Amp_Volume_Set_with_Index(uint8_t Vol_Level, Bool Inverse, Bool Actual_Key) //Actual Key says this is not SSP or BLE communication. So, we need to send same key to Slave SPK
{
	static uint8_t uCurVolLevel = 0;
	uint8_t uReg_Value = 0;
#ifdef FLASH_SELF_WRITE_ERASE
	uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END];
#endif

#ifdef AD82584F_DEBUG
	_DBG("\n\rAD82584F_Amp_Volume_Set_with_Index() !!!");
#endif

	if(Vol_Level > (VOLUME_LEVEL_NUMER-1))
	{
#ifdef AD82584F_DEBUG
		_DBG("\n\rInput Volume Level NG !!! : ");
		_DBH(Vol_Level);
#endif
		return 0xff;
	}

	if(Inverse)
	{
		uCurVolLevel = (VOLUME_LEVEL_NUMER-1) - Vol_Level;
	}
	else
		uCurVolLevel = Vol_Level;
	
	uReg_Value = AD82584F_Volume_Table[uCurVolLevel];
	
#ifdef AD82584F_DEBUG
	_DBG("\n\ruCurVol Reg value: 0x");
	_DBH(uReg_Value);
#endif
		
	I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MASTER_VOL_CTL_REG,&uReg_Value,1);
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uCurVolLevel);
#endif

#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	if(Actual_Key)
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, (VOLUME_LEVEL_NUMER-1) - Vol_Level);
#endif
#ifdef FLASH_SELF_WRITE_ERASE
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	else
#endif
	{
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //Read PDL Device Number from Flash		

		if(uFlash_Read_Buf[FLASH_SAVE_DATA_VOLUME] != uCurVolLevel)
			FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel); //FlashWriteErase(&uCurVolLevel, 1); //Save Current Volume Information to Flash memory for SPP/BLE Data
	}
#endif
	return uCurVolLevel;
}

void AD82584F_Amp_Volume_Control(Vol_Setting Vol_mode) //Volume Up/Down
{
	uint8_t uRead = 0, uCurVolLevel = 0, i;
#ifdef SLAVE_VOLUME_FORCED_SYNC
	static uint8_t uCount = 0;
#endif
	
#ifdef AD82584F_DEBUG	
	_DBG("\n\rAD82584F_Amp_Volume_Control!!! --- BVolumeUp =");
	_DBD(Vol_mode);
#endif

	//Check whether current volume level is available value to control volume
	uCurVolLevel = AD82584F_Amp_Get_Cur_Volume_Level();

	if(Vol_mode == Volume_Up)
		uCurVolLevel-=1;
	else
		uCurVolLevel+=1;

	if(uCurVolLevel > MIN_VOLUME_LEVEL)
	{
#ifdef SLAVE_VOLUME_FORCED_SYNC	//Need to update volume info even though invalid value becasue Slave can get different volume level.
		if(uCount%2)
			MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x03);
		else
			MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x04);

		uCount++;
#endif
		return;
	}

	I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_MASTER_VOL_CTL_REG,&uRead,1);
#ifdef _I2C_DEBUG_MSG
		_DBG("\n\rCur Value = 0x");
		_DBH(uRead);
#endif	
	// Check whether current volume level is Max/Min or not
	if(((uRead >= AD82584F_Volume_Table[MIN_VOLUME_LEVEL]) && (Vol_mode == Volume_Down))	\
		|| ((uRead <= AD82584F_Volume_Table[MAX_VOLUME_LEVEL]) && (Vol_mode == Volume_Up)))
	{
#ifdef AD82584F_DEBUG
		_DBG("\n\rCan't change current volume level becuase it's a MAX or MIN !!! uRead = 0x");
		_DBH(uRead);
#endif
		return;
	}

	for(i =0; i < VOLUME_LEVEL_NUMER; i++)
	{
		if(AD82584F_Volume_Table[i] == uRead)
		{
			uCurVolLevel = i;
#ifdef AD82584F_DEBUG
			_DBG("\n\ruCurVolLevel : 0x");
			_DBH(uCurVolLevel);
#endif
			break;
		}
	}

	if(Vol_mode == Volume_Up) //Volume up setting -A increasing number is smaller volume
		uCurVolLevel -= 1;
	else //Volume down setting
		uCurVolLevel += 1;
			
	uRead = AD82584F_Volume_Table[uCurVolLevel];
	
#ifdef AD82584F_DEBUG
	_DBG("\n\ruSet Volume Register : 0x");
	_DBH(uRead);
#endif

	I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MASTER_VOL_CTL_REG,&uRead,1);
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uCurVolLevel);
#endif
#ifdef FLASH_SELF_WRITE_ERASE
	FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel); //FlashWriteErase(&uCurVolLevel, 1); //Save Current Volume Information to Flash memory
#endif
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, (VOLUME_LEVEL_NUMER-1) - uCurVolLevel);
#endif
}

void AD82584F_Amp_RAM_Single_Write(uint8_t uCount, uint8_t uData)
{
	uint8_t Data = 0, count = 0;
#ifdef AD82584F_DEBUG
	_DBG("\n\rAD82584F_Amp_RAM_Single_Write() !!!");
#endif

	Data = uData;
	count = uCount%5;
			
	switch(count)
	{
		case 0:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_COEFFICIENT_RAM_BASE_ADDR_REG,&Data,1);
			break;
		case 1:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 2:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 3:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 4:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_RAM_SETTING_REG,&Data,1);
			break;
		default:
			break;
	}
}

void AD82584F_Amp_RAM_Three_Coeff_Write(uint8_t uCount, uint8_t uData)
{
	uint8_t Data = 0, count = 0;
#ifdef AD82584F_DEBUG
		_DBG("\n\rAD82584F_Amp_RAM_Three_Coeff_Write() !!!");
#endif

	Data = uData;
	count = uCount%11;

	switch(count)
	{
		case 0:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_COEFFICIENT_RAM_BASE_ADDR_REG,&Data,1);
			break;
		case 1:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 2:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 3:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 4:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 5:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 6:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 7:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 8:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 9:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 10:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_RAM_SETTING_REG,&Data,1);
			break;
		default:
			break;
	}
}

void AD82584F_Amp_RAM_Set_Write(uint8_t uCount, uint8_t uData)
{
	uint8_t Data = 0, count = 0;
#ifdef AD82584F_DEBUG
		_DBG("\n\rAD82584F_Amp_RAM_Set_Write() !!!");
#endif

	Data = uData;
	count = uCount%17;

	switch(count)
	{
		case 0:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_COEFFICIENT_RAM_BASE_ADDR_REG,&Data,1);
			break;
		case 1:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 2:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 3:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 4:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 5:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 6:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 7:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 8:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 9:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 10:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_B2_REG,&Data,1);
			break;
		case 11:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_B2_REG,&Data,1);
			break;
		case 12:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_B2_REG,&Data,1);
			break;
		case 13:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_TOP_COFFICIENTS_A0_REG,&Data,1);
			break;
		case 14:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MID_COFFICIENTS_A0_REG,&Data,1);
			break;
		case 15:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_BOTTOM_COFFICIENTS_A0_REG,&Data,1);
			break;
		case 16:
			I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_RAM_SETTING_REG,&Data,1);
			break;
		default:
			break;
	}
}

void AD82584F_Amp_Mode_Control(Audio_Output_Setting mode)
{
	uint8_t Data = 0, i;

#ifdef AD82584F_DEBUG
	_DBG("\n\rAD82584F_Amp_Mode_Control() !!! mode =");
	_DBD(mode);
#endif

	switch(mode)
	{
		case LL_MODE:
			for(i=0; i<20; i++)
			{
				Data = LR_Mode_Ctl_Table[LL_MODE][i];
				AD82584F_Amp_RAM_Single_Write(i, Data);
			}
			break;
		case RR_MODE:
			for(i=0; i<20; i++)
			{
				Data = LR_Mode_Ctl_Table[RR_MODE][i];
				AD82584F_Amp_RAM_Single_Write(i, Data);
			}
			break;
		case STEREO_MODE:
			for(i=0; i<20; i++)
			{
				Data = LR_Mode_Ctl_Table[STEREO_MODE][i];
				AD82584F_Amp_RAM_Single_Write(i, Data);
			}
			break;
		default:
			break;
	}
}

void AD82584F_Amp_EQ_DRC_Control(EQ_Mode_Setting EQ_mode)
{
	uint8_t uData = 0, uCount = 0, uSize = 0;

	uSize = sizeof(EQ_Normal_Table);
#ifdef AD82584F_DEBUG
	_DBG("\n\r AD82584F_Amp_EQ_DRC_Control() !!! size =");
	_DBD(uSize);
#endif
#ifdef USEN_EQ_ENABLE
	for(uCount=0; uCount<uSize; uCount++)
	{
		uData = EQ_Normal_Table[uCount];
		AD82584F_Amp_RAM_Set_Write(uCount, uData);
	}

	//0x1B B[6:5] = 1x Three band DRC Set. This setting is enabled by init value
	uSize = sizeof(EQ_Normal_DRC_Table);
#ifdef AD82584F_DEBUG
	_DBG("\n\r AD82584F_Amp_EQ_DRC_Control() !!! size =");
	_DBD(uSize);
#endif
	for(uCount = 0;uCount<uSize;uCount++)
	{
		uData = EQ_Normal_DRC_Table[uCount];
		AD82584F_Amp_RAM_Single_Write(uCount, uData);
	}
#ifdef SRS_FILTER_ENABLE
	uSize = sizeof(SRS_FILTER_Table);
#ifdef AD82584F_DEBUG
	_DBG("\n\r AD82584F_Amp_EQ_DRC_Control() !!! size =");
	_DBD(uSize);
#endif
	for(uCount = 0;uCount<uSize;uCount++)
	{
		uData = SRS_FILTER_Table[uCount];
		AD82584F_Amp_RAM_Three_Coeff_Write(uCount, uData);
	}
#endif //SRS_FILTER_ENABLE
#else //USEN_EQ_ENABLE
	switch(EQ_mode)
	{
		case EQ_ROCK_MODE:
#ifdef AD82584F_DEBUG
			_DBG("\n\rEQ_ROCK_MODE");
#endif
			for(uCount=0; uCount<85; uCount++)
			{
				uData = EQ_Table[EQ_ROCK_MODE][uCount];
				AD82584F_Amp_RAM_Set_Write(uCount, uData);
			}
			break;
			
		case EQ_JAZZ_MODE:
#ifdef AD82584F_DEBUG
			_DBG("\n\rEQ_JAZZ_MODE");
#endif
			for(uCount=0; uCount<85; uCount++)
			{
				uData = EQ_Table[EQ_JAZZ_MODE][uCount];
				AD82584F_Amp_RAM_Set_Write(uCount, uData);
			}
			break;
						
		case EQ_POP_MODE:
#ifdef AD82584F_DEBUG
			_DBG("\n\rEQ_POP_MODE");
#endif
			for(uCount=0; uCount<85; uCount++)
			{
				uData = EQ_Table[EQ_POP_MODE][uCount];
				AD82584F_Amp_RAM_Set_Write(uCount, uData);
			}
			break;
			
		default:
			break;
	}
#endif //USEN_EQ_ENABLE
}

Bool AD82584F_Amp_Get_Cur_Mute_Status(void) //TRUE : Mute On / FALSE : Mute Off
{
	uint8_t uRead = 0;
	Bool Mute_Status = FALSE;
#ifdef AD82584F_DEBUG	
	_DBG("\n\rAD82584F_Amp_Get_Cur_Mute_Status()");
#endif

	I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_STATE_CTL3_REG,&uRead,1); //Read Current mute status

	if(uRead & AD82584F_MASTER_MUTE_ON) //Current mute state is "Mute On"
	{
		Mute_Status = TRUE;
	}
	else //Current mute state is "Off"
	{
		Mute_Status = FALSE;
	}

	return Mute_Status;
}

void AD82584F_Amp_Set_Cur_Volume_Level(uint8_t volume)
{
#ifdef AD82584F_DEBUG
		_DBG("\n\rAD82584F_Amp_Set_Cur_Volume_Level() : volume =");
		_DBD(volume);
#endif
	uCurrent_Vol_Level = volume;
}

uint8_t AD82584F_Amp_Get_Cur_Volume_Level(void) //Start count from Max(15)
{
#ifdef AD82584F_DEBUG
		_DBG("\n\rAD82584F_Amp_Get_Cur_Volume_Level() : volume =");
		_DBD(uCurrent_Vol_Level);
#endif
	return uCurrent_Vol_Level;
}

uint8_t AD82584F_Amp_Get_Cur_Volume_Level_Inverse(void) //Start count from Min(0)
{
	uint8_t uInverse_Vol;
#ifdef AD82584F_DEBUG
		_DBG("\n\rAD82584F_Amp_Get_Cur_Volume_Level_Inverse() : volume =");
		_DBD(uCurrent_Vol_Level);
#endif
	uInverse_Vol = (VOLUME_LEVEL_NUMER-1) -uCurrent_Vol_Level;

	return uInverse_Vol;
}

Bool AD82584F_Amp_Get_Cur_CLK_Status(void) //TRUE : Clock Exist / FALSE : Clock absence
{	
	uint8_t uRead = 0;
	Bool Ret;
	
	I2C_Interrupt_Read_Data(AD82584F_DEVICE_ADDR, AD82584F_PROTECTION_REG, &uRead, 1);

#ifdef AD82584F_DEBUG
	_DBG("\n\rRead_Amp_Data : ");
	_DBH(uRead);
#endif

	if(uRead & 0x04) //Bit 2 - 0(CKERR occur - None) / 1(Normal)
		Ret = TRUE;
	else
		Ret = FALSE;
	
	return Ret;
}

void AD82584F_Amp_Set_Default_Volume(void)
{
	uint8_t uRead = 0, uVol = 0;
	int i;

	uRead = VOLUME_DEFAULT_LEVEL;

	for(i =0; i < VOLUME_LEVEL_NUMER; i++)
	{
		if(AD82584F_Volume_Table[i] == uRead)
		{
			uVol = i;
#ifdef AD82584F_DEBUG
			_DBG("\n\ruDefault CurVolLevel : 0x");
			_DBH(uCurVolLevel);
#endif
			break;
		}
	}
	
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uVol);
#endif
	I2C_Interrupt_Write_Data(AD82584F_DEVICE_ADDR, AD82584F_MASTER_VOL_CTL_REG,&uRead,1);
}


#endif //AD82584F_ENABLE


