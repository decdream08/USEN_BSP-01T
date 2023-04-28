/**********************************************************************
* @file		tas5806dm.c
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
#ifdef TAS5806MD_ENABLE
#include "i2c.h"
#include "tas5806md.h"
#include "tas5806md_register.h" //Separted TAS5806MD.c to two files(TAS5806MD_register.h) to move Init function. //2022-10-11_1
#if (defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE) || defined(TIMER21_LED_ENABLE))
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
#define MOVE_PAGE		(0x00)
#define MOVE_BOOK		(0x7F)

#define TAS5806MD_RESET_CONTROL_REG					(0x01)
#define TAS5806MD_RESET_ALL_ON					(0x10)
#define TAS5806MD_RESET_ALL_OFF					(~(0x10))

#define TAS5806MD_MUTE_CONTROL_REG					(0x03)
#define TAS5806MD_MUTE_ON					(0x08)
#define TAS5806MD_MUTE_OFF					(~(0x08))

#define TAS5806MD_PWR_CONTROL_REG					(TAS5806MD_MUTE_CONTROL_REG)
#define TAS5806MD_PWR_DEEP_SLEEP					(0x0)
#define TAS5806MD_PWR_SLEEP							(0x1)
#define TAS5806MD_PWR_HIZ							(0x2)
#define TAS5806MD_PWR_PLAY							(0x3)
#define TAS5806MD_PWR_REG_CLEAR 					(~(0x3))

#define TAS5806MD_DAC_GAIN_CONTROL_REG					(0x4C) //DAC Gain

#define TAS5806MD_FS_DET_REG						(0x37)
#define I2S_FS_32KHZ								(0x6) //0110:32KHz
#define I2S_FS_48KHZ								(0x9) //1001:48KHz
#define I2S_FS_96KHZ								(0xB) //1011:96KHz

#define TAS5806MD_DSP_MISC_REG						(0x66)
#define TAS5806MD_DRC_BYPASS_CONTROL				(0x02)
#define TAS5806MD_EQ_BYPASS_CONTROL					(0x01)

#define TAS5806MD_CHAN_FAULT_REG					(0x70)
#define TAS5806MD_GLOBAL_FAULT1_REG					(0x71)
#define TAS5806MD_GLOBAL_FAULT2_REG					(0x72)
#define TAS5806MD_OT_WARNING_REG					(0x73)

#define TAS5806MD_FAULT_CLEAR_REG					(0x78)

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE //Need to move volume control page, first !!!
#define TAS5806MD_VOL_CONTROL_REG1					(0x24)
#define TAS5806MD_VOL_CONTROL_REG2					(0x28)
#endif

#define MAX_VOLUME_LEVEL		(0)
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
#define VOLUME_LEVEL_NUMER	(sizeof(Find_Volume_Level))
#define MIN_VOLUME_LEVEL		(VOLUME_LEVEL_NUMER - 1)
#else //TI_AMP_DSP_VOLUME_CONTROL_ENABLE
#define VOLUME_LEVEL_NUMER	(sizeof(TAS5806MD_Volume_Table))
#define MIN_VOLUME_LEVEL		(VOLUME_LEVEL_NUMER - 1)
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
uint8_t uCurrent_Vol_Level = 0;
static Bool IS_Mute = FALSE;
#ifdef AD82584F_USE_POWER_DOWN_MUTE
static Bool Display_Mute = FALSE;
#endif
Bool volatile BAmp_Init = TRUE; //2023-02-21_5 : To aovid AMP access after boot on
Bool volatile BAmp_COM = FALSE; //2023-02-27_3 : To check whether AMP is busy(can't access - TRUE) or not(FALSE)

#ifdef USEN_IT_AMP_EQ_ENABLE //2023-04-28_1 : To apply BSP-01T EQ Setting to BAP-01 under EQ BSP Mode //#if !defined(USEN_BAP) && defined(USEN_IT_AMP_EQ_ENABLE)  //2023-03-08_3
static EQ_Mode_Setting Cur_EQ_Mode = EQ_NORMAL_MODE;
#endif

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
#ifdef ADC_VOLUME_STEP_ENABLE
#define ADJUST_VOLUME_GAIN 			(8) //2023-02-24_1 : BAP-01/BSP-01T Model, DSP Volume Level Tuning for EQ BYPASS BT FW. Max Volume = DSP(-8dB) + DAC(20.5dB) = 12.5 dB Max(DAC Gain Setting)

#ifdef ADC_VOLUME_64_STEP_ENABLE
const uint8_t Find_Volume_Level[64] = { //2023-01-12_1 //dB 0dB  ~ -28.5dB(Actual volume level is Analog Gain + DAC Gain + Volume and Max output is 20.5dB)
	0 + ADJUST_VOLUME_GAIN,
	0 + ADJUST_VOLUME_GAIN,
	0 + ADJUST_VOLUME_GAIN,
	0 + ADJUST_VOLUME_GAIN,
	0 + ADJUST_VOLUME_GAIN,
	0 + ADJUST_VOLUME_GAIN,
	0 + ADJUST_VOLUME_GAIN,
	0 + ADJUST_VOLUME_GAIN,
	1 + ADJUST_VOLUME_GAIN,
	1 + ADJUST_VOLUME_GAIN,
	1 + ADJUST_VOLUME_GAIN,
	1 + ADJUST_VOLUME_GAIN,
	1 + ADJUST_VOLUME_GAIN,
	2 + ADJUST_VOLUME_GAIN,
	2 + ADJUST_VOLUME_GAIN,
	2 + ADJUST_VOLUME_GAIN,
	3 + ADJUST_VOLUME_GAIN,
	3 + ADJUST_VOLUME_GAIN,
	4 + ADJUST_VOLUME_GAIN,
	4 + ADJUST_VOLUME_GAIN,
	5 + ADJUST_VOLUME_GAIN,
	6 + ADJUST_VOLUME_GAIN,
	7 + ADJUST_VOLUME_GAIN,
	8 + ADJUST_VOLUME_GAIN,
	8 + ADJUST_VOLUME_GAIN,
	9 + ADJUST_VOLUME_GAIN,
	10 + ADJUST_VOLUME_GAIN,
	10 + ADJUST_VOLUME_GAIN,
	11 + ADJUST_VOLUME_GAIN,
	11 + ADJUST_VOLUME_GAIN,
	12 + ADJUST_VOLUME_GAIN,
	13 + ADJUST_VOLUME_GAIN,
	13 + ADJUST_VOLUME_GAIN,
	14 + ADJUST_VOLUME_GAIN,
	15 + ADJUST_VOLUME_GAIN,
	16 + ADJUST_VOLUME_GAIN,
	17 + ADJUST_VOLUME_GAIN,
	18 + ADJUST_VOLUME_GAIN,
	19 + ADJUST_VOLUME_GAIN,
	20 + ADJUST_VOLUME_GAIN,
	21 + ADJUST_VOLUME_GAIN,
	22 + ADJUST_VOLUME_GAIN,
	23 + ADJUST_VOLUME_GAIN,
	24 + ADJUST_VOLUME_GAIN,
	25 + ADJUST_VOLUME_GAIN,
	26 + ADJUST_VOLUME_GAIN,
	26 + ADJUST_VOLUME_GAIN,
	27 + ADJUST_VOLUME_GAIN,
	27 + ADJUST_VOLUME_GAIN,
	28 + ADJUST_VOLUME_GAIN,
	29 + ADJUST_VOLUME_GAIN,
	30 + ADJUST_VOLUME_GAIN,
	31 + ADJUST_VOLUME_GAIN,
	32 + ADJUST_VOLUME_GAIN,
	33 + ADJUST_VOLUME_GAIN,
	34 + ADJUST_VOLUME_GAIN,
	35 + ADJUST_VOLUME_GAIN,
	36 + ADJUST_VOLUME_GAIN,
	37 + ADJUST_VOLUME_GAIN,
	39 + ADJUST_VOLUME_GAIN,
	41 + ADJUST_VOLUME_GAIN,
	43 + ADJUST_VOLUME_GAIN,
	46 + ADJUST_VOLUME_GAIN,
	49 + ADJUST_VOLUME_GAIN
};
	
#else //ADC_VOLUME_50_STEP_ENABLE

const uint8_t Find_Volume_Level[50] = { //2023-02-27_3 : Changed ADC volume step from 64 step to 50 step.
	0 + ADJUST_VOLUME_GAIN,
	1 + ADJUST_VOLUME_GAIN,
	2 + ADJUST_VOLUME_GAIN,
	3 + ADJUST_VOLUME_GAIN,
	4 + ADJUST_VOLUME_GAIN,
	5 + ADJUST_VOLUME_GAIN,
	6 + ADJUST_VOLUME_GAIN,
	7 + ADJUST_VOLUME_GAIN,
	8 + ADJUST_VOLUME_GAIN,
	9 + ADJUST_VOLUME_GAIN,
	10 + ADJUST_VOLUME_GAIN,
	11 + ADJUST_VOLUME_GAIN,
	12 + ADJUST_VOLUME_GAIN,
	13 + ADJUST_VOLUME_GAIN,
	14 + ADJUST_VOLUME_GAIN,
	15 + ADJUST_VOLUME_GAIN,
	16 + ADJUST_VOLUME_GAIN,
	17 + ADJUST_VOLUME_GAIN,
	18 + ADJUST_VOLUME_GAIN,
	19 + ADJUST_VOLUME_GAIN,
	20 + ADJUST_VOLUME_GAIN,
	21 + ADJUST_VOLUME_GAIN,
	22 + ADJUST_VOLUME_GAIN,
	23 + ADJUST_VOLUME_GAIN,
	24 + ADJUST_VOLUME_GAIN,
	25 + ADJUST_VOLUME_GAIN,
	26 + ADJUST_VOLUME_GAIN,
	27 + ADJUST_VOLUME_GAIN,
	28 + ADJUST_VOLUME_GAIN,
	29 + ADJUST_VOLUME_GAIN,
	30 + ADJUST_VOLUME_GAIN,
	31 + ADJUST_VOLUME_GAIN,
	32 + ADJUST_VOLUME_GAIN,
	33 + ADJUST_VOLUME_GAIN,
	34 + ADJUST_VOLUME_GAIN,
	35 + ADJUST_VOLUME_GAIN,
	36 + ADJUST_VOLUME_GAIN,
	37 + ADJUST_VOLUME_GAIN,
	38 + ADJUST_VOLUME_GAIN,
	39 + ADJUST_VOLUME_GAIN,
	40 + ADJUST_VOLUME_GAIN,
	41 + ADJUST_VOLUME_GAIN,
	42 + ADJUST_VOLUME_GAIN,
	43 + ADJUST_VOLUME_GAIN,
	44 + ADJUST_VOLUME_GAIN,
	45 + ADJUST_VOLUME_GAIN,
	46 + ADJUST_VOLUME_GAIN,
	47 + ADJUST_VOLUME_GAIN,
	48 + ADJUST_VOLUME_GAIN,
	49 + ADJUST_VOLUME_GAIN
};
#endif //ADC_VOLUME_50_STEP_ENABLE

#else //ADC_VOLUME_STEP_ENABLE

#ifndef USEN_BAP //2023-02-09_3 : Changed volume table to adjust volume level like BSP-01
#define ADJUST_VOLUME_GAIN			(8) //2023-02-23_2 //(0)//2023-02-24_2 : Revert +9dB

const uint8_t Find_Volume_Level[16] = { //2023-02-23_1 : In BSP-01T Model, DSP Volume Level Tuning for EQ BYPASS BT FW
	0+ ADJUST_VOLUME_GAIN, //DSP(-8dB) + DAC(15.5dB) = 7.5 dB Max(DAC Gain Setting)
	1+ ADJUST_VOLUME_GAIN,
	2+ ADJUST_VOLUME_GAIN,
	3+ ADJUST_VOLUME_GAIN,//4,
	6+ ADJUST_VOLUME_GAIN,
	9+ ADJUST_VOLUME_GAIN,
	11+ ADJUST_VOLUME_GAIN,//12,
	14+ ADJUST_VOLUME_GAIN,
	17+ ADJUST_VOLUME_GAIN,//18,
	21+ ADJUST_VOLUME_GAIN,//22,
	25+ ADJUST_VOLUME_GAIN,//26, //Default
	28+ ADJUST_VOLUME_GAIN,
	31+ ADJUST_VOLUME_GAIN,//32,
	34+ ADJUST_VOLUME_GAIN,//35,
	42+ ADJUST_VOLUME_GAIN,//45,
	110 //MUTE
};
#endif //USEN_BAP
#endif //ADC_VOLUME_STEP_ENABLE

const uint8_t TAS5806MD_Volume_Table[111][4] = { //111-Step //0dB ~ 110dB(1dB Step)
	{0x0,0x80,0x0,0x0},
	{0x0,0x72,0x14,0x83},
	{0x0,0x65,0xac,0x8c},
	{0x0,0x5a,0x9d,0xf8},
	{0x0,0x50,0xc3,0x36},
	{0x0,0x47,0xfa,0xcd},
	{0x0,0x40,0x26,0xe7},
	{0x0,0x39,0x2c,0xee},
	{0x0,0x32,0xf5,0x2d},
	{0x0,0x2d,0x6a,0x86},
	{0x0,0x28,0x7a,0x27},
	{0x0,0x24,0x13,0x47},
	{0x0,0x20,0x26,0xf3},
	{0x0,0x1c,0xa7,0xd7},
	{0x0,0x19,0x8a,0x13},
	{0x0,0x16,0xc3,0x11},
	{0x0,0x14,0x49,0x61},
	{0x0,0x12,0x14,0x9a},
	{0x0,0x10,0x1d,0x3f},
	{0x0,0x0e,0x5c,0xa1},
	{0x0,0x0c,0xcc,0xcd},
	{0x0,0x0b,0x68,0x73},
	{0x0,0x0a,0x2a,0xdb},
	{0x0,0x9,0x0f,0xcc},
	{0x0,0x8,0x13,0x85},
	{0x0,0x7,0x32,0xae},
	{0x0,0x6,0x6a,0x4a},
	{0x0,0x5,0xb7,0xb1},
	{0x0,0x5,0x18,0x84},
	{0x0,0x4,0x8a,0xa7},
	{0x0,0x4,0x0c,0x37},
	{0x0,0x3,0x9b,0x87},
	{0x0,0x3,0x37,0x18},
	{0x0,0x2,0xdd,0x96},
	{0x0,0x2,0x8d,0xcf},
	{0x0,0x2,0x46,0xb5},
	{0x0,0x2,0x7,0x56},
	{0x0,0x1,0xce,0xdc},
	{0x0,0x1,0x9c,0x86},
	{0x0,0x1,0x6f,0xaa},
	{0x0,0x1,0x47,0xae},
	{0x0,0x1,0x24,0x0c},
	{0x0,0x1,0x4,0x49},
	{0x0,0x0,0xe7,0xfb},
	{0x0,0x0,0xce,0xc1},
	{0x0,0x0,0xb8,0x45},
	{0x0,0x0,0xa4,0x3b},
	{0x0,0x0,0x92,0x5f},
	{0x0,0x0,0x82,0x74},
	{0x0,0x0,0x74,0x44},
	{0x0,0x0,0x67,0x9f},
	{0x0,0x0,0x5c,0x5a},
	{0x0,0x0,0x52,0x4f},
	{0x0,0x0,0x49,0x5c},
	{0x0,0x0,0x41,0x61},
	{0x0,0x0,0x3a,0x45},
	{0x0,0x0,0x33,0xef},
	{0x0,0x0,0x2e,0x49},
	{0x0,0x0,0x29,0x41},
	{0x0,0x0,0x24,0xc4},
	{0x0,0x0,0x20,0xc5},
	{0x0,0x0,0x1d,0x34},
	{0x0,0x0,0x1a,0x7},
	{0x0,0x0,0x17,0x33},
	{0x0,0x0,0x14,0xad},
	{0x0,0x0,0x12,0x6d},
	{0x0,0x0,0x10,0x6c},
	{0x0,0x0,0x0e,0xa3},
	{0x0,0x0,0x0d,0x0c},
	{0x0,0x0,0x0b,0xa0},
	{0x0,0x0,0x0a,0x5d},
	{0x0,0x0,0x9,0x3c},
	{0x0,0x0,0x8,0x3b},
	{0x0,0x0,0x7,0x56},
	{0x0,0x0,0x6,0x8a},
	{0x0,0x0,0x5,0xd4},
	{0x0,0x0,0x5,0x32},
	{0x0,0x0,0x4,0xa1},
	{0x0,0x0,0x4,0x20},
	{0x0,0x0,0x3,0xad},
	{0x0,0x0,0x3,0x47},
	{0x0,0x0,0x2,0xec},
	{0x0,0x0,0x2,0x9a},
	{0x0,0x0,0x2,0x52},
	{0x0,0x0,0x2,0x11},
	{0x0,0x0,0x1,0xd8},
	{0x0,0x0,0x1,0xa4},
	{0x0,0x0,0x1,0x77},
	{0x0,0x0,0x1,0x4e},
	{0x0,0x0,0x1,0x2a},
	{0x0,0x0,0x1,0x9},
	{0x0,0x0,0x0,0xec},
	{0x0,0x0,0x0,0xd3},
	{0x0,0x0,0x0,0xbc},
	{0x0,0x0,0x0,0xa7},
	{0x0,0x0,0x0,0x95},
	{0x0,0x0,0x0,0x85},
	{0x0,0x0,0x0,0x76},
	{0x0,0x0,0x0,0x6a},
	{0x0,0x0,0x0,0x5e},
	{0x0,0x0,0x0,0x54},
	{0x0,0x0,0x0,0x4b},
	{0x0,0x0,0x0,0x43},
	{0x0,0x0,0x0,0x3b},
	{0x0,0x0,0x0,0x35},
	{0x0,0x0,0x0,0x2f},
	{0x0,0x0,0x0,0x2a},
	{0x0,0x0,0x0,0x25},
	{0x0,0x0,0x0,0x21},
	{0x0,0x0,0x0,0x1e},
	{0x0,0x0,0x0,0x1b}
};

#else //TI_AMP_DSP_VOLUME_CONTROL_ENABLE
uint8_t TAS5806MD_Volume_Table[] = { // 16-Step
       //2022-11-22 : Fixed Volume Level from HW team
		(0x10+VOLUME_ADJUSTMENT),	//24dB MAX
		(0x11+VOLUME_ADJUSTMENT),
		(0x13+VOLUME_ADJUSTMENT),
		(0x16+VOLUME_ADJUSTMENT),
		(0x1b+VOLUME_ADJUSTMENT),	
		(0x21+VOLUME_ADJUSTMENT), 
		(0x26+VOLUME_ADJUSTMENT), 
		(0x2b+VOLUME_ADJUSTMENT),	
		(0x32+VOLUME_ADJUSTMENT),	
		(0x3a+VOLUME_ADJUSTMENT),	
		(0x42+VOLUME_ADJUSTMENT),	//Default
		(0x47+VOLUME_ADJUSTMENT),
		(0x4e+VOLUME_ADJUSTMENT),
		(0x55+VOLUME_ADJUSTMENT),
		(0x68+VOLUME_ADJUSTMENT),
		0xff //0x10	// Mute
};
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

//TAS5806_DEVICE_ADDR_15K = (0x2D), //8bit - 0x5A
#ifdef USEN_BAP
const uint8_t TAS5806MD_AGL_Table[][2] = 
{
	{ 0x00, 0x00 }, //0x00 : 0x00
	{ 0x7F, 0x8C }, //0x7F : 0x8C
	{ 0x00, 0x2C }, //0x00 : 0x2C
    { 0x5c, 0x00 }, //  AGL Release Rate: 0.05
    { 0x5d, 0x11 },
    { 0x5e, 0x11 },
    { 0x5f, 0x11 },
    { 0x60, 0x00 }, //  AGL Attack Rate: 0.05
    { 0x61, 0x44 },
    { 0x62, 0x44 },
    { 0x63, 0x44 },
    { 0x64, 0x00 }, //  AGL Threshold: -22 dB
    { 0x65, 0xa4 },
    { 0x66, 0x3a },
    { 0x67, 0xa2 },
};
#endif

#ifdef TAS5806MD_ENABLE
Bool Is_BAmp_Init(void)
{
	Bool BRet = FALSE;
	
	if(BAmp_Init)
		BRet = TRUE;

	return BRet;
}
#endif

Bool Is_I2C_Access_OK(void) //2023-02-27_2 : Added some codes to avoid interrupt during I2C commnunication. Please refer to BAmp_COM(FALSE : Can't Access / TRUE : Can Access).
{
	Bool BRet = FALSE;

	if(Is_BAmp_Init() || BAmp_COM)
		BRet = FALSE;
	else
		BRet = TRUE;

	return BRet;
}

Bool Is_Mute(void)
{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	Bool bRet;
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];

	//_DBG("\n\rIs_Mute() : ");
	
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
	//_DBH(uRead_Buf[FLASH_SAVE_DATA_MUTE]);
	if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
		bRet = FALSE;
	else
		bRet = TRUE;

	return bRet;
#else
	return IS_Mute;
#endif
}

#ifdef AD82584F_USE_POWER_DOWN_MUTE
void Set_Display_Mute(Bool B_Mute_On_Display) //2023-03-08_4 : For LED Display
{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
	
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
#endif

	Display_Mute = B_Mute_On_Display;

	if(Display_Mute)
	{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
#endif
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif	
	}
	else
	{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash
#endif
#ifdef AUX_INPUT_DET_ENABLE
		if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
#endif
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());

	}
}

Bool IS_Display_Mute(void) //For LED Display
{
	return Display_Mute;
}
#endif

void Set_Is_Mute(Bool mute_on) //For Actual Mute Status
{
	IS_Mute = mute_on;
}

#ifdef FLASH_SELF_WRITE_ERASE
void TAS5806MD_Amp_Init(Bool Power_On_Init)
#else
void TAS5806MD_Amp_Init(void)
#endif
{
	uint32_t uSize = 0, i;
	uint8_t Data = 0, uCommand = 0;
#if defined(TIMER30_LED_PWM_ENABLE) || defined(TIMER21_LED_ENABLE)
#ifndef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	uint8_t uRead = 0;
#endif
	uint8_t uCurVolLevel = 0;
#endif
#ifdef FLASH_SELF_WRITE_ERASE
	uint8_t uVol_Level = 0;
	uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END];
#endif
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	uint8_t uBuffer = 0;
#endif

	BAmp_Init = TRUE;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Init");
#endif

#ifndef FLASH_SELF_WRITE_ERASE_EXTENSION //We don't need to init for Display_Mute because we need to keep mute status as spec change
#ifdef AD82584F_USE_POWER_DOWN_MUTE
	Display_Mute = FALSE;
#endif
#endif
	
	TAS5806MD_Amp_Reset(TRUE);
	delay_ms(10);
	TAS5806MD_Amp_Reset(FALSE);

	//Write TAS5806MD_Init	
	uSize = sizeof(TAS5806MD_registers)/2;

	for(i =0;i<uSize;i++)
	{
		uCommand = TAS5806MD_registers[i][0]; //2022-11-14
		
		switch(uCommand)
		{
			case CFG_META_DELAY:
				Data = TAS5806MD_registers[i][1]; //2023-03-10_3 : Need to add delay for Data value but we don't set Data value before.
				delay_ms(Data*10);
				break;

			default:
			Data = TAS5806MD_registers[i][1];
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			break;
		}
	}

	TAS5806MD_Amp_Move_to_Control_Page(); //2023-03-10_6 : In TAS5806MD_Amp_Init(), we move to the position of TAS5806MD_Amp_Move_to_Control_Page() to protect I2C Access.

	BAmp_Init = FALSE;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Init - OK !!!");
#endif

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE) //Default Volume LED display
	LED_Display_Volume_All_Off(); //All Volume LEDs are turned off upon initializing
#endif //TIMER30_LED_PWM_ENABLE

	//TAS5806MD_Amp_Move_to_Control_Page(); //2023-03-10_6 : move to up side

#ifdef FLASH_SELF_WRITE_ERASE
	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //Read Volume Level from Flash and set the value to Amp Device

#if defined(NOT_USE_POWER_DOWN_MUTE) && defined(TAS5806MD_ENABLE) //2023-02-21_1 : After Factory Reset or Reboot, SPK Should keep power stage(PLAY) under PLAY MODE and it do not set uRead = 0
#ifdef USEN_BAP //Implemented Power Key Feature //2022-10-07_3
		if((HAL_GPIO_ReadPin(PA) & (1<<6))) //Power Off
#else //USEN_BAP
		if(uFlash_Read_Buf[FLASH_SAVE_DATA_POWER] == 0) //Power Off
#endif //USEN_BAP
		{
			TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_DEEP_SLEEP);
		}
		else //Power On - 0xff or 0x01
		{
			TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY);
		}
#endif

#if defined(USEN_BAP) && defined(ADC_INPUT_ENABLE) //2023-03-02_3 : BAP-01 do not use flash data to set volume level when power on.
	uVol_Level = ADC_Volume_Attenuator_Value_Init();
#else
	uVol_Level = uFlash_Read_Buf[FLASH_SAVE_DATA_VOLUME];
#endif

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	//DAC Gain Default Volume Setting
#ifdef USEN_BAP
	uBuffer = 0x07; //20.5dB
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uBuffer,1);
#else
	uBuffer = 0x11; //15.5dB //2023-02-23_2 : Changed Default DAC GAIN for EQ BYPASS BT FW //0x0F; //16.5dB //2023-02-09_3 : Changed Default DAC GAIN
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uBuffer,1);
#endif
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

	if(uVol_Level == 0xff)
	{
#ifdef _DBG_FLASH_WRITE_ERASE
		_DBG("\n\r#### Flash Data : First Init(Volume Level is 0xff)");
#endif
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
#ifdef ADC_VOLUME_STEP_ENABLE
		//Volume Level 0 setting.
		uCurVolLevel = 64; //Level 64
		TAS5806MD_Amp_Volume_Register_Writing(uCurVolLevel);
#else //ADC_VOLUME_STEP_ENABLE
		//-10dB default
		uCurVolLevel = 10; //Level 10
		TAS5806MD_Amp_Volume_Register_Writing(uCurVolLevel);
#endif //ADC_VOLUME_STEP_ENABLE
#else //TI_AMP_DSP_VOLUME_CONTROL_ENABLE
		I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uRead,1);

		// Check whether current volume level is Max/Min or not
		if((uRead >= TAS5806MD_Volume_Table[MIN_VOLUME_LEVEL])	\
			|| (uRead <= TAS5806MD_Volume_Table[MAX_VOLUME_LEVEL]))
		{
#ifdef AD82584F_DEBUG
			_DBG("\n\rCan't change current volume level becuase it's a MAX or MIN !!! uRead = 0x");
			_DBH(uRead);
#endif
			return;
		}

		for(i =0; i < VOLUME_LEVEL_NUMER; i++)
		{
			if(TAS5806MD_Volume_Table[i] == uRead)
			{
				uCurVolLevel = i;
#ifdef AD82584F_DEBUG
				_DBG("\n\ruDefault CurVolLevel : 0x");
				_DBH(uCurVolLevel);
#endif
				break;
			}
		}		
#ifndef USEN_IT_AMP_EQ_ENABLE //2023-02-27_1 : Move to down side
		TAS5806MD_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting
#endif
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
		LED_Display_Volume(uCurVolLevel);
#elif defined(USEN_BAP) //Need to update volume level when we don't use LED_Display_Volume() //2022-10-27_1
		TAS5806MD_Amp_Set_Cur_Volume_Level(uCurVolLevel); //Save current volume level
#endif
	}
	else
	{
#ifdef _DBG_FLASH_WRITE_ERASE
		_DBG("\n\r#### Flash Data : Find Volume Level");
#endif

#if defined(USEN_BAP) && defined(USEN_IT_AMP_EQ_ENABLE) //2023-04-25_1 : Need to set EQ Mode after AMP Init
		TAS5806MD_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting
#endif

		if(Power_On_Init == TRUE)
#ifdef USEN_BAP //2023-04-28_2 : Under BSP-01T broadcast mode, we need to return back to original code to avoid to send "BLE_SET_MANUFACTURER_DATA" when DC Power on.
			TAS5806MD_Amp_Volume_Set_with_Index(uVol_Level, FALSE, TRUE); //Power On Init Call //2023-03-28_5 : Changed condition actual_key from FALSE to TRUE. When power on init, BAP-01 Master send wrong volume data until user changed voluem thru rotary button or remote app. Also, BAP-01 can use ACTUAL KEY in this case.
#else
			TAS5806MD_Amp_Volume_Set_with_Index(uVol_Level, FALSE, FALSE); //Power On Init Call
#endif
		else
			TAS5806MD_Amp_Volume_Set_with_Index(uVol_Level, FALSE, TRUE); //Actual Key Input

	//	TAS5806MD_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting // To Do !!! EQ and DRC Setting here !!!

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
		//LED_Display_Volume(uCurVolLevel); //Don't need to call duplicated. AD82584F_Amp_Volume_Set_with_Index() function calls LED_Display_Volume().
#endif
	}
#else //FLASH_SELF_WRITE_ERASE	
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uRead,1);
#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rInit Volume Level !!! uRead = 0x");
	_DBH(uRead);
#endif

#ifdef USEN_IT_AMP_EQ_ENABLE //2023-02-27_1
	TAS5806MD_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); //DRC / EQ Setting
#endif

	// Check whether current volume level is Max/Min or not
	if((uRead >= TAS5806MD_Volume_Table[MIN_VOLUME_LEVEL])	\
		|| (uRead <= TAS5806MD_Volume_Table[MAX_VOLUME_LEVEL]))
	{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rCan't change current volume level becuase it's a MAX or MIN !!! uRead = 0x");
		_DBH(uRead);
#endif
		return;
	}

	for(i =0; i < VOLUME_LEVEL_NUMER; i++)
	{
		if(TAS5806MD_Volume_Table[i] == uRead)
		{
			uCurVolLevel = i;
#ifdef TAS5806MD_DEBUG_MSG
			_DBG("\n\ruDefault CurVolLevel : 0x");
			_DBH(uCurVolLevel);
#endif
			break;
		}
	}

	//AD82584F_Amp_EQ_DRC_Control(EQ_NORMAL_MODE); // To Do !!! EQ and DRC Setting here !!!

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uCurVolLevel);
#elif defined(USEN_BAP) //Need to update volume level when we don't use LED_Display_Volume() //2022-10-27_1
	TAS5806MD_Amp_Set_Cur_Volume_Level(uCurVolLevel); //Save current volume level
#endif
#endif //FLASH_SELF_WRITE_ERASE
//To Do !!!  //Fade In/Out
}

void TAS5806MD_Amp_Reset(Bool Reset_On)
{
	uint8_t uRead = 0;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Reset() : Reset_On = ");
	_DBD(Reset_On);
#endif
#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE) 
	{

		_DBG("\n\r+++ TAS5806MD_Amp_Reset() don't execute this !!!!");

		return;
	}
#endif
	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Control_Page();

	if(Reset_On)
	{
		I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_RESET_CONTROL_REG,&uRead,1);
		uRead |= TAS5806MD_RESET_ALL_ON;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_RESET_CONTROL_REG,&uRead,1);
	}
	else
	{
		I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_RESET_CONTROL_REG,&uRead,1);
		uRead &= TAS5806MD_RESET_ALL_OFF;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_RESET_CONTROL_REG,&uRead,1);
	}

	BAmp_COM = FALSE;
}

void TAS5806MD_Amp_Mute(Bool Mute_On, Bool LED_Display) // First of all, You need to make sure if current page is Book 00 & Page00
{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
#endif
#if !defined(AD82584F_USE_POWER_DOWN_MUTE) || defined(NOT_USE_POWER_DOWN_MUTE)
	uint8_t uRead = 0;
#endif
#ifdef AUTO_VOLUME_LED_OFF //For SPP/BLE Data
	uint8_t uVolume_Level = 0;
#endif
	
#ifdef MUTE_CHECK_DEBUG_MSG	
	_DBG("\n\rTAS5806MD_Amp_Mute!!!  ");_DBD(Mute_On);_DBD(LED_Display);
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

#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE)
	{
		_DBG("\n\r+++ TAS5806MD_Amp_Mute() don't execute this !!!!");_DBD(Mute_On);_DBD(LED_Display);

		return;
	}
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
#endif

	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Control_Page();

#ifdef NOT_USE_POWER_DOWN_MUTE
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1); //Read Current mute status
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
#ifdef NOT_USE_POWER_DOWN_MUTE
		//Set Mute On
		uRead |= TAS5806MD_MUTE_ON;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
#else //NOT_USE_POWER_DOWN_MUTE
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN : ON //Mute On
#endif //NOT_USE_POWER_DOWN_MUTE
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
#ifdef NOT_USE_POWER_DOWN_MUTE
		if(TAS5806MD_CLK_Detect_Count() != 0xffffffff) //2023-03-10_5 : When mute off, we need to check whether I2S Clock is stable or not. if I2S Clock is not stable, we use TIMER20_mute_flag_Start() instead of actual mute off.
		{
#ifdef MUTE_CHECK_DEBUG_MSG	
			_DBG("\n\rMute Off - TIMER20_mute_flag_Start !!!");
#endif
			TIMER20_mute_flag_Start();
		}
		else
		{
#ifdef MUTE_CHECK_DEBUG_MSG	
			_DBG("\n\rMute Off - Actual Mute Off !!!");
#endif

			//Set Mute Off
			uRead &= TAS5806MD_MUTE_OFF;
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
		}
#else //NOT_USE_POWER_DOWN_MUTE
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN : OFF //Mute Off
#endif //NOT_USE_POWER_DOWN_MUTE
#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
		if(LED_Display)
		{
#ifdef AUX_INPUT_DET_ENABLE
			if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
				Set_Status_LED_Mode(STATUS_AUX_MODE);
			else
#endif
				Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
		}
#endif
	}
#else //AD82584F_USE_POWER_DOWN_MUTE
	IS_Mute = TAS5806MD_Amp_Get_Cur_Mute_Status(&uRead);

	if(IS_Mute == Mute_On)
	{
		BAmp_COM = FALSE;

		return;
	}

	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1); //Read Current mute status

	if(Mute_On) // A Cur_State is Mute Off and need to change to Mute On
	{ //Set Mute On
		uRead |= TAS5806MD_MUTE_ON;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
#ifdef TAS5806MD_DEBUG_MSG
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
		uRead &= TAS5806MD_MUTE_OFF;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
#ifdef TAS5806MD_DEBUG_MSG
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
		uVolume_Level = TAS5806MD_Amp_Set_Cur_Volume_Level();
		TAS5806MD_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
	}
	else
		TIMER20_auto_volume_led_off_flag_Start(); //do not display LED volume after 10s Under Mute Off
#endif
	BAmp_COM = FALSE;
}

void TAS5806MD_Amp_Mute_Toggle(void) //Toggle
{
#ifdef AD82584F_USE_POWER_DOWN_MUTE
#ifndef FLASH_SELF_WRITE_ERASE_EXTENSION
	static Bool Mute_On = FALSE; //We don't use this variable becasuse we need to set MUTE ON/OFF state using flash data on power on
#endif
#else	
	Bool Mute_On = FALSE;
#endif
#if !defined(AD82584F_USE_POWER_DOWN_MUTE) || defined(NOT_USE_POWER_DOWN_MUTE)
	uint8_t uRead = 0;
#endif
#ifdef AUTO_VOLUME_LED_OFF
	uint8_t uVolume_Level = 0;
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
#endif

#ifdef MUTE_CHECK_DEBUG_MSG	
	_DBG("\n\rTAS5806MD_Amp_Mute_Toggle()");
#endif

#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
#endif
	
#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE)
	{
		_DBG("\n\r+++ TAS5806MD_Amp_Mute_Toggle() don't execute this !!!!");

		return;
	}
#endif
	BAmp_COM = TRUE;

#ifdef NOT_USE_POWER_DOWN_MUTE
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1); //Read Current mute status
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
		_DBG("\n\rMute On 1!!!");
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
#endif
#ifdef NOT_USE_POWER_DOWN_MUTE
		//Set Mute On
		uRead |= TAS5806MD_MUTE_ON;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
#else //NOT_USE_POWER_DOWN_MUTE
		HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN : ON
#endif //NOT_USE_POWER_DOWN_MUTE
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif
	}
	else //Mute Off
	{
		IS_Mute = FALSE;
		Display_Mute = FALSE;
		
#ifdef MUTE_CHECK_DEBUG_MSG
		_DBG("\n\rMute Off 1 !!!");
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash
#endif
#ifdef NOT_USE_POWER_DOWN_MUTE
		if(TAS5806MD_CLK_Detect_Count() != 0xffffffff) //2023-03-16_1 : Add 2023-03-10_5 solution //2023-03-10_5 : When mute off, we need to check whether I2S Clock is stable or not. if I2S Clock is not stable, we use TIMER20_mute_flag_Start() instead of actual mute off.
		{
#ifdef MUTE_CHECK_DEBUG_MSG	
			_DBG("\n\rMute Off for Actual Mute Key - TIMER20_mute_flag_Start !!!");
#endif
			TIMER20_mute_flag_Start();
		}
		else
		{
#ifdef MUTE_CHECK_DEBUG_MSG	
			_DBG("\n\rMute Off for Actual Mute Key - Actual Mute Off !!!");
#endif
			//Set Mute Off
			uRead &= TAS5806MD_MUTE_OFF;
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
		}
#else //NOT_USE_POWER_DOWN_MUTE
		HAL_GPIO_SetPin(PF, _BIT(4)); //DAMP_PDN : OFF
#endif //NOT_USE_POWER_DOWN_MUTE
#ifdef TIMER21_LED_ENABLE //Need to return latest status led mode
#ifdef AUX_INPUT_DET_ENABLE
		if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
#endif
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
#endif
	}
#ifndef FLASH_SELF_WRITE_ERASE_EXTENSION
	Mute_On = IS_Mute;
#endif
#else //AD82584F_USE_POWER_DOWN_MUTE
	Mute_On = TAS5806MD_Amp_Get_Cur_Mute_Status(&uRead);

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rRead Value =");
	_DBH(uRead);
#endif

	TAS5806MD_Amp_Move_to_Control_Page();

	if(Mute_On) // A Cur_State is Mute On and need to change to Mute Off
	{ //Set Mute Off
		uRead &= TAS5806MD_MUTE_OFF;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
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
		uRead |= TAS5806MD_MUTE_ON;
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
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
	{
		if(TAS5806MD_CLK_Detect_Count() == 0xffffffff) //2023-04-12_2 : if this condition is not true, we'll retry again and agin under mute off function. So, we don't need to send same mute off CMD to slave
			MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Mute, 0x00);
	}
#endif
#ifdef AUTO_VOLUME_LED_OFF
	if(Display_Mute)
	{
		TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
		uVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
		TAS5806MD_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
	}
	else
		TIMER20_auto_volume_led_off_flag_Start(); //do not display LED volume after 10s Under Mute Off
#endif

	BAmp_COM = FALSE;
}

uint8_t TAS5806MD_Amp_Volume_Set_with_Index(uint8_t Vol_Level, Bool Inverse, Bool Actual_Key) //Actual Key says this is not SSP or BLE communication. So, we need to send same key to Slave SPK
{
	uint8_t uCurVolLevel = 0;
#ifndef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	uint8_t uReg_Value = 0;
#endif
#if defined(FLASH_SELF_WRITE_ERASE) && !defined(USEN_BAP) //2023-03-02_3 : BAP-01 do not use flash data to set volume level when power on.
	uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END];
#endif

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Volume_Set_with_Index() !!!");
#endif

	if(Vol_Level > (VOLUME_LEVEL_NUMER-1))
	{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rInput Volume Level NG !!! : ");
		_DBD(Vol_Level);
#endif
		return 0xff;
	}

#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE)
	{
		_DBG("\n\r+++ TAS5806MD_Amp_Volume_Set_with_Index() don't execute this !!!!");

		return 0xff;
	}
#endif

	if(Inverse)
	{
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_3 : To Fit BLE volume data from Master under BAP
		uCurVolLevel = VOLUME_LEVEL_NUMER - Vol_Level;
#else
		uCurVolLevel = (VOLUME_LEVEL_NUMER-1) - Vol_Level;
#endif
	}
	else
		uCurVolLevel = Vol_Level;
	
	BAmp_COM = TRUE;

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	TAS5806MD_Amp_Volume_Register_Writing(uCurVolLevel);
#else //TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	uReg_Value = TAS5806MD_Volume_Table[uCurVolLevel];
	
#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\ruCurVol Reg value: 0x");
	_DBH(uReg_Value);
#endif

	TAS5806MD_Amp_Move_to_Control_Page();
		
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uReg_Value,1);
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

	BAmp_COM = FALSE;

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uCurVolLevel);
#elif defined(USEN_BAP) //Need to update volume level when we don't use LED_Display_Volume() //2022-10-27_1
	TAS5806MD_Amp_Set_Cur_Volume_Level(uCurVolLevel); //Save current volume level
#endif

#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	if(Actual_Key)
	{
#if defined(USEN_BAP) && !defined(ADC_VOLUME_STEP_ENABLE) //Fixed that BSP-01 slave is working in inverse when BAP-01 Master volume is changed //2023-01-05_7
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, Vol_Level); //Fixed that BSP-01 slave is working in inverse when BAP-01 Master volume is changed //2023-01-02_1
#else
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, (VOLUME_LEVEL_NUMER-1) - Vol_Level);
#endif
	}
#endif
#if defined(FLASH_SELF_WRITE_ERASE) && !defined(USEN_BAP) //2023-03-02_3 : BAP-01 do not use flash data to set volume level when power on.
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	else
#endif
	{
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //Read Volume Level from Flash		

		if(uFlash_Read_Buf[FLASH_SAVE_DATA_VOLUME] != uCurVolLevel)
			FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel); //FlashWriteErase(&uCurVolLevel, 1); //Save Current Volume Information to Flash memory for SPP/BLE Data
	}
#endif
	return uCurVolLevel;
}

// First of all, You need to make sure if current page is Book 00 & Page00
void TAS5806MD_Amp_Volume_Control(Vol_Setting Vol_mode) //Volume Up/Down
{
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	uint8_t uCurVolLevel = 0;
#else
	uint8_t uRead = 0, uCurVolLevel = 0, i;
#endif
#ifdef SLAVE_VOLUME_FORCED_SYNC
	static uint8_t uCount = 0;
#endif

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Volume_Control!!! --- BVolumeUp =");
	_DBD(Vol_mode);
#endif

#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE)
	{
		_DBG("\n\r+++ TAS5806MD_Amp_Volume_Control() don't execute this !!!!");

		return;
	}
#endif
	//Check whether current volume level is available value to control volume
	uCurVolLevel = TAS5806MD_Amp_Get_Cur_Volume_Level();

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

	BAmp_COM = TRUE;

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	TAS5806MD_Amp_Volume_Register_Writing(uCurVolLevel);
#else //TI_AMP_DSP_VOLUME_CONTROL_ENABLE
	TAS5806MD_Amp_Move_to_Control_Page();
	
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uRead,1);
#ifdef _I2C_DEBUG_MSG
		_DBG("\n\rCur Value = 0x");
		_DBH(uRead);
#endif	
	// Check whether current volume level is Max/Min or not
	if(((uRead >= TAS5806MD_Volume_Table[MIN_VOLUME_LEVEL]) && (Vol_mode == Volume_Down))	\
		|| ((uRead <= TAS5806MD_Volume_Table[MAX_VOLUME_LEVEL]) && (Vol_mode == Volume_Up)))
	{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rCan't change current volume level becuase it's a MAX or MIN !!! uRead = 0x");
		_DBH(uRead);
#endif
		BAmp_COM = FALSE;

		return;
	}

	for(i =0; i < VOLUME_LEVEL_NUMER; i++)
	{
		if(TAS5806MD_Volume_Table[i] == uRead)
		{
			uCurVolLevel = i;
#ifdef TAS5806MD_DEBUG_MSG
			_DBG("\n\ruCurVolLevel : ");
			_DBH(uCurVolLevel);
#endif
			break;
		}
	}

	if(Vol_mode == Volume_Up) //Volume up setting -A increasing number is smaller volume
		uCurVolLevel -= 1;
	else //Volume down setting
		uCurVolLevel += 1;
			
	uRead = TAS5806MD_Volume_Table[uCurVolLevel];

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\ruSet Volume Register : 0x");
	_DBH(uRead);
#endif

	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uRead,1);
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

	BAmp_COM = FALSE;

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uCurVolLevel);
#elif defined(USEN_BAP) //Need to update volume level when we don't use LED_Display_Volume() //2022-10-27_1
	TAS5806MD_Amp_Set_Cur_Volume_Level(uCurVolLevel); //Save current volume level
#endif
#ifdef FLASH_SELF_WRITE_ERASE
#ifndef USEN_BAP //2023-03-02_3 : BAP-01 do not use flash data to set volume level when power on.
	FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel); //FlashWriteErase(&uCurVolLevel, 1); //Save Current Volume Information to Flash memory
#endif
#endif
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, (VOLUME_LEVEL_NUMER-1) - uCurVolLevel);
#endif
}

void TAS5806MD_Amp_Mode_Control(Audio_Output_Setting mode) //To Do !!!
{
#if 0
	uint8_t Data = 0, i;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Mode_Control() !!! mode =");
	_DBD(mode);
#endif

	TAS5806MD_Amp_Move_to_Control_Page();

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
#endif
}

void TAS5806MD_Amp_EQ_DRC_Control(EQ_Mode_Setting EQ_mode)
{
#ifdef USEN_IT_AMP_EQ_ENABLE //2023-02-27_1
	uint16_t uSize, i;
	uint8_t Data, uCommand;
#ifdef USEN_IT_AMP_EQ_ENABLE //2023-04-28_1 : To apply BSP-01T EQ Setting to BAP-01 under EQ BSP Mode  //#if !defined(USEN_BAP) && defined(USEN_IT_AMP_EQ_ENABLE) //2023-03-08_3 : Control volume level for each EQ Mode
	uint8_t uCurVolLevel;
#endif

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\r+++ TAS5806MD_Amp_EQ_DRC_Control() !!!!");
#endif

	if(Is_BAmp_Init()) //2023-02-22_1
		return;
	
#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE)
	{
		_DBG("\n\r+++ TAS5806MD_Amp_EQ_DRC_Control() don't execute this !!!!");

		return;
	}
#endif

	BAmp_COM = TRUE;

#ifdef USEN_BAP //2023-04-25_1 : To keep EQ Mode for Amp_Init, we need to use cur_EQ_Mode under BAP-01.
	if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode && EQ_mode == EQ_NORMAL_MODE)
	{
		EQ_mode = EQ_BAP_NORMAL_MODE;
	}
#endif
	
#ifdef USEN_IT_AMP_EQ_ENABLE //2023-04-28_1 : To apply BSP-01T EQ Setting to BAP-01 under EQ BSP Mode  //#if !defined(USEN_BAP) && defined(USEN_IT_AMP_EQ_ENABLE)
	Cur_EQ_Mode = EQ_mode; //2023-03-08_3
#endif
	
//2023-04-28_1 //#ifndef USEN_BAP //2023-03-23_1 : EQ On/Off setting
	TAS5806MD_Amp_Move_to_DSP_Control_Page();
//#endif

#ifdef I2C_ACCESS_ERROR_DEBUG
	_DBG("\n\r+++ TAS5806MD_Amp_EQ_DRC_Control() !!!!");
	_DBD(EQ_mode);
#endif

	//Write TAS5806MD_Init
	switch(EQ_mode)
	{
#if defined(USEN_BAP) && defined(USEN_IT_AMP_EQ_ENABLE) //2023-03-28_6 : Added EQ NORMAL switch mode from EJT
		case EQ_BAP_NORMAL_MODE:
		{
#if 0//2023-04-28_1 //def USEN_BAP 
			TAS5806MD_EQ_OnOff(TRUE); //EQ On			
			TAS5806MD_Amp_Move_to_DSP_Control_Page();
#endif
			uSize = sizeof(TAS5806MD_EQ_Table_BAP_NORMAL_MODE)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_EQ_Table_BAP_NORMAL_MODE[i][0];
				
				Data = TAS5806MD_EQ_Table_BAP_NORMAL_MODE[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;
#endif
		case EQ_NORMAL_MODE:
		{
#if 0//2023-04-28_1 //def USEN_BAP //2023-03-23_1  : EQ Setting from EJT
			TAS5806MD_EQ_OnOff(FALSE); //EQ Off
#else //USEN_BAP
			uSize = sizeof(TAS5806MD_EQ_Table_NORMAL)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_EQ_Table_NORMAL[i][0];
				
				Data = TAS5806MD_EQ_Table_NORMAL[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
#endif //USEN_BAP
		}
		break;
		case EQ_POP_ROCK_MODE:
		{
#if 0//2023-04-28_1 //2023-03-23_1 : EQ Setting from EJT
			TAS5806MD_EQ_OnOff(TRUE); //EQ On			
			TAS5806MD_Amp_Move_to_DSP_Control_Page();
#endif
			uSize = sizeof(TAS5806MD_EQ_Table_POP_ROCK)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_EQ_Table_POP_ROCK[i][0];
				
				Data = TAS5806MD_EQ_Table_POP_ROCK[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;
		case EQ_CLUB_MODE:
		{
#if 0//2023-04-28_1 //2023-04-28_1 //#ifdef USEN_BAP //2023-03-23_1 : EQ Setting from EJT
			TAS5806MD_EQ_OnOff(TRUE); //EQ On			
			TAS5806MD_Amp_Move_to_DSP_Control_Page();
#endif
			uSize = sizeof(TAS5806MD_EQ_Table_CLUB)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_EQ_Table_CLUB[i][0];
				
				Data = TAS5806MD_EQ_Table_CLUB[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;
		case EQ_JAZZ_MODE:
		{
#if 0//2023-04-28_1 //#ifdef USEN_BAP //2023-03-23_1 : EQ Setting from EJT
			TAS5806MD_EQ_OnOff(TRUE); //EQ On			
			TAS5806MD_Amp_Move_to_DSP_Control_Page();
#endif
			uSize = sizeof(TAS5806MD_EQ_Table_JAZZ)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_EQ_Table_JAZZ[i][0];
				
				Data = TAS5806MD_EQ_Table_JAZZ[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;
		case EQ_VOCAL_MODE:
		{
#if 0//2023-04-28_1 //2023-04-28_1 //#ifdef USEN_BAP //2023-03-23_1 : EQ Setting from EJT
			TAS5806MD_EQ_OnOff(TRUE); //EQ On			
			TAS5806MD_Amp_Move_to_DSP_Control_Page();
#endif
			uSize = sizeof(TAS5806MD_EQ_Table_VOCAL)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_EQ_Table_VOCAL[i][0];
				
				Data = TAS5806MD_EQ_Table_VOCAL[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;
	}
		
#if 0//def USEN_BAP	
	TAS5806MD_Amp_Move_to_Control_Page();

	switch(EQ_mode) //2023-03-23_1 : Added DRC from OYM
	{
		case EQ_POP_ROCK_MODE: //POP & ROCK
		case EQ_CLUB_MODE: //CLUB
		case EQ_NORMAL_MODE: //Normal
		case EQ_BAP_NORMAL_MODE: //2023-03-28_6 //EQ NORMAL SWITCH MODE
		{
			//DRC_OFF
			TAS5806MD_DRC_OnOff(FALSE);
		}
		break;

		default: //NORMAL //JAZZ //VOCAL
		{
			//DRC_ON
			TAS5806MD_DRC_OnOff(TRUE);
			
			//DRC Setting
			TAS58066MD_Amp_Move_to_DRC_band3_Page();
			
			uSize = sizeof(TAS5806MD_DRC_Table_NORMAL)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_DRC_Table_NORMAL[i][0];
				
				Data = TAS5806MD_DRC_Table_NORMAL[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;
	}
#else //USEN_BAP
	switch(EQ_mode) //2023-03-02_2 : Added DRC due to 230302_001_Normal_OYM.h
	{
#ifdef USEN_BAP //2023-04-28_1
		case EQ_BAP_NORMAL_MODE:
		{
			//DRC_OFF
			TAS5806MD_Amp_Move_to_Control_Page();
			TAS5806MD_DRC_OnOff(FALSE);
		}
		break;
#endif
		case EQ_POP_ROCK_MODE: //POP & ROCK
		{
#ifdef USEN_BAP //2023-04-28_1
			//DRC_ON
			TAS5806MD_Amp_Move_to_Control_Page();
			TAS5806MD_DRC_OnOff(TRUE);
#endif
			TAS58066MD_Amp_Move_to_DRC_band3_Page();
			
			uSize = sizeof(TAS5806MD_DRC_Table_POP_ROCK)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_DRC_Table_POP_ROCK[i][0];
				
				Data = TAS5806MD_DRC_Table_POP_ROCK[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;

		default: //NORMAL
		{
#ifdef USEN_BAP //2023-04-28_1
			//DRC_ON
			TAS5806MD_Amp_Move_to_Control_Page();
			TAS5806MD_DRC_OnOff(TRUE);
#endif

			TAS58066MD_Amp_Move_to_DRC_band3_Page();
			
			uSize = sizeof(TAS5806MD_DRC_Table_NORMAL)/2;

			for(i =0;i<uSize;i++)
			{
				uCommand = TAS5806MD_DRC_Table_NORMAL[i][0];
				
				Data = TAS5806MD_DRC_Table_NORMAL[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, uCommand,&Data,1);
			}
		}
		break;
	}
#endif //USEN_BAP
	
#ifdef USEN_IT_AMP_EQ_ENABLE
//2023-04-28_1 //#ifndef USEN_BAP //2023-03-23_1 : Changed condition //#if !defined(USEN_BAP) && defined(USEN_IT_AMP_EQ_ENABLE) //2023-03-08_3 : Control volume level for each EQ Mode
	uCurVolLevel = TAS5806MD_Amp_Get_Cur_Volume_Level();
	TAS5806MD_Amp_Volume_Register_Writing(uCurVolLevel);
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_EQ, EQ_mode);
#endif
//2023-04-28_1 //#endif
#else //USEN_IT_AMP_EQ_ENABLE
	TAS5806MD_Amp_Move_to_Control_Page();
#endif //USEN_IT_AMP_EQ_ENABLE

	BAmp_COM = FALSE;
#endif
}

Bool TAS5806MD_Amp_Get_Cur_Mute_Status(uint8_t *buffer) //TRUE : Mute On / FALSE : Mute Off
{
	uint8_t uRead = 0;
	Bool Mute_Status = FALSE;
#ifdef TAS5806MD_DEBUG_MSG	
	_DBG("\n\rTAS5806MD_Amp_Get_Cur_Mute_Status()");
#endif
#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE)
	{
		_DBG("\n\r+++ TAS5806MD_Amp_Get_Cur_Mute_Status() don't execute this !!!!");

		return TRUE;
	}
#endif
	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Control_Page();

	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1); //Read Current mute status

	BAmp_COM = FALSE;

	if(buffer != NULL)
	{
		*buffer = uRead;
#ifdef TAS5806MD_DEBUG_MSG	
		_DBG("\n\r*buffer =");
		_DBH(*buffer);
#endif
	}
	
	if(uRead & TAS5806MD_MUTE_ON)
		Mute_Status = TRUE;
	else
		Mute_Status = FALSE;
	
	return Mute_Status;
}

void TAS5806MD_Amp_Set_Cur_Volume_Level(uint8_t volume)
{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rTAS5806MD_Amp_Set_Cur_Volume_Level() : volume =");
		_DBD(volume);
#endif

	uCurrent_Vol_Level = volume;
}

uint8_t TAS5806MD_Amp_Get_Cur_Volume_Level(void) //Start count from Max(15)
{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rTAS5806MD_Amp_Get_Cur_Volume_Level() : volume =");
		_DBD(uCurrent_Vol_Level);
#endif

	return uCurrent_Vol_Level;
}

uint8_t TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse(void) //Start count from Min(0)
{
	uint8_t uInverse_Vol;
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rTAS5806MD_Amp_Get_Cur_Volume_Level_Inverse() : volume =");
		_DBD(uCurrent_Vol_Level);
#endif
	uInverse_Vol = (VOLUME_LEVEL_NUMER-1) -uCurrent_Vol_Level;

	return uInverse_Vol;
}

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
void TAS5806MD_Amp_Set_Default_Volume(void)
{
#ifndef ADC_VOLUME_STEP_ENABLE //BAP-01 volume will be set in ADC volume block instead of this line
	uint8_t uVol;

	//-10dB default
	uVol = 10; //Level 10

	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Control_Page();
	TAS5806MD_Amp_Volume_Register_Writing(uVol);
	
	BAmp_COM = FALSE;

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE) //2023-01-19_1 : We need to update Volume LED even though we use DSP volume under BSP-01/BSP-02
	LED_Display_Volume(uVol);
#endif
#endif //ADC_VOLUME_STEP_ENABLE
}

#else //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

void TAS5806MD_Amp_Set_Default_Volume(void)
{
	uint8_t uRead = 0, uVol = 0;
	int i;

	uRead = DAC_GAIN_DEFAULT_VALUE;

	for(i =0; i < VOLUME_LEVEL_NUMER; i++)
	{
		if(TAS5806MD_Volume_Table[i] == uRead)
		{
			uVol = i;
#ifdef TAS5806MD_DEBUG_MSG
			_DBG("\n\ruDefault CurVolLevel : 0x");
			_DBH(DAC_GAIN_DEFAULT_VALUE);
#endif
			break;
		}
	}
	
#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	LED_Display_Volume(uVol);
#elif defined(USEN_BAP) //Need to update volume level when we don't use LED_Display_Volume() //2022-10-27_1
	TAS5806MD_Amp_Set_Cur_Volume_Level(uVol); //Save current volume level
#endif

	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DAC_GAIN_CONTROL_REG,&uRead,1);

	BAmp_COM = FALSE;
}
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

void TAS5806MD_Amp_Move_to_Control_Page(void)
{
	uint8_t uRead = 0;

	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&uRead,1);
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);
}

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
void TAS5806MD_Amp_Move_to_Volume_Control_Page(void)
{
	uint8_t uRead;

	uRead = 0x00;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);

	uRead = 0x8c;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&uRead,1);

	uRead = 0x2a;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);
}
#endif //TI_AMP_DSP_VOLUME_CONTROL_ENABLE

#ifdef USEN_IT_AMP_EQ_ENABLE //2023-02-27_1
void TAS5806MD_Amp_Move_to_DSP_Control_Page(void)
{
	uint8_t uRead;

	uRead = 0x00;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);

	uRead = 0xaa;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&uRead,1);

	uRead = 0x24;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);
}

void TAS58066MD_Amp_Move_to_DRC_band3_Page(void) //2023-03-02_2
{
	uint8_t uRead;

	uRead = 0x00;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);

	uRead = 0x8c;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&uRead,1);

	uRead = 0x2d;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&uRead,1);
}

#endif


void TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_Power_Mode mode) //Power Control (DeepSleep/Sleep/Hi-Z/Play)
{
	uint8_t uRead = 0;
#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Set_PWR_Control_Mode() : ");//_DBH(mode);
#endif	
	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Control_Page();
		
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_PWR_CONTROL_REG,&uRead,1); //Read Current mute status
	
	switch(mode)
	{
		case TAS5806MD_PWR_Mode_DEEP_SLEEP:
			uRead &= TAS5806MD_PWR_REG_CLEAR;
			uRead |= TAS5806MD_PWR_DEEP_SLEEP;
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_PWR_CONTROL_REG,&uRead,1);
			break;
		case TAS5806MD_PWR_Mode_SLEEP:
			uRead &= TAS5806MD_PWR_REG_CLEAR;
			uRead |= TAS5806MD_PWR_SLEEP;
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_PWR_CONTROL_REG,&uRead,1);
			break;
		case TAS5806MD_PWR_Mode_HIZ:
			uRead &= TAS5806MD_PWR_REG_CLEAR;
			uRead |= TAS5806MD_PWR_HIZ;
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_PWR_CONTROL_REG,&uRead,1);
			break;
		case TAS5806MD_PWR_Mode_PLAY:
			uRead &= TAS5806MD_PWR_REG_CLEAR;
			uRead |= TAS5806MD_PWR_PLAY;
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_PWR_CONTROL_REG,&uRead,1);
			break;
		default:
			break;
	}

	BAmp_COM = FALSE;

#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rTAS5806MD_Amp_Set_PWR_Control_Mode() : ");//_DBH(uRead);
#endif

}

Bool TAS5806MD_Amp_Detect_FS(Bool BInit) //Detect Audio Sampling Feq //2022-10-05 //2022-10-17_2
{
	uint8_t uRead = 0;
	Bool bCheck = FALSE;

#ifdef I2C_ACCESS_ERROR_DEBUG
	if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE)
	{
		_DBG("\n\r+++ TAS5806MD_Amp_Detect_FS() don't execute this !!!!");

		return FALSE;
	}
#endif
	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Control_Page();
	
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_FS_DET_REG,&uRead,1);
#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rRead Data :");
	_DBH(uRead);
#endif
	uRead &= (~0xF0);

	switch(uRead) //Just get bit3-0(Audio Sampling Freq.) - 0000:Error, 0110:32KHz, 1001:48KHz, 1011:96KHz, 1000:Reserved
	{
		case I2S_FS_32KHZ:
			bCheck = TRUE;
#ifdef COMMON_DEBUG_MSG
			_DBG("\n\rI2S_FS_32KHZ");
#endif
		break;
		case I2S_FS_48KHZ:
			bCheck = TRUE;
#ifdef COMMON_DEBUG_MSG
			_DBG("\n\rI2S_FS_48KHZ");
#endif
		break;
		case I2S_FS_96KHZ:
			bCheck = TRUE;
#ifdef COMMON_DEBUG_MSG
			_DBG("\n\rI2S_FS_96KHZ");
#endif
		break;
		default:
			bCheck = FALSE;
		break;
	}
		
	if(bCheck) 
	{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\rRead Data 1 :");
		_DBH(uRead);
#endif
		if(BInit) //2022-10-17_2
		{
#ifdef TWS_MODE_ENABLE //2023-02-16_3 : To avoid pop noise on TWS Slave during init
#ifdef NOT_USE_POWER_DOWN_MUTE
			I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1); //2023-02-21_1 : After Factory Reset or Reboot, SPK Should keep power stage(PLAY) under PLAY MODE and it do not set uRead = 0
			//Set Mute On
			uRead |= TAS5806MD_MUTE_ON;
			I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_MUTE_CONTROL_REG,&uRead,1);
#else //NOT_USE_POWER_DOWN_MUTE
			HAL_GPIO_ClearPin(PF, _BIT(4)); //DAMP_PDN : ON
#endif //NOT_USE_POWER_DOWN_MUTE
#endif
			TAS5806MD_Amp_Init(TRUE);
		}

		TIMER20_mute_flag_Start(); //Added Mute Off(TIMER20_mute_flag_Start()) in TAS5806MD_Amp_Detect_FS() function. //2022-10-12_6
		BAmp_COM = FALSE;

		return TRUE;
	}
	else
	{
		BAmp_COM = FALSE;

		return FALSE;
	}
}


uint8_t TAS5806MD_Amp_Detect_Fault(Bool Return_Val_Only) //2022-10-25 : FAULT PIN //2022-11/10 : Changed return from Bool to uint8_t(B_Error_Flag = 1:HighTemp/2:other/0:None)
{	
	//Bool B_Is_Error = FALSE;
	uint8_t Ch_Fault = 0, Fault1 = 0, Fault2 = 0, Warning = 0,  B_Error_Flag = 0;
#ifdef USEN_BAP //2023-04-07_3
	uint8_t Volume_Level = 0;
#endif

	if(Is_BAmp_Init() || Is_I2C_Access_OK() == FALSE) //2023-03-10_7  //2023-02-21_5 : To avoid AMP access after boot on
	{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\r+++ Is_BAmp_Init is TRUE - 2");
#endif
		return 0xff; //2023-04-07_2 : To retry for fault detect
	}

	BAmp_COM = TRUE; //2023-03-10_7 //FALSE;

	TAS5806MD_Amp_Move_to_Control_Page();
	
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_CHAN_FAULT_REG,&Ch_Fault,1);	
#if 0
	if(Ch_Fault & 0xF)
		B_Is_Error = TRUE;
#endif	
	if(Ch_Fault & 0x8)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Left Channel DC fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	if(Ch_Fault & 0x4)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Right Channel DC fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	if(Ch_Fault & 0x2)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Left Channel over current fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	if(Ch_Fault & 0x1)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Right Channel over current fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_GLOBAL_FAULT1_REG,&Fault1,1);
#if 0
	if(Fault1 & 0xC7)
		B_Is_Error = TRUE;
#endif

	if(Fault1 & 0x80)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Indicate OTP CRC check error");
#endif
	}

	if(Fault1 & 0x40)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! The recent BQ is written failed");
#endif
	}

	if(Fault1 & 0x4)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Clock fault");
#endif

		if(TAS5806MD_CLK_Detect_Count() == 0xffffffff) //2023-04-07_1 : Added Clock Error Recovery after first AMP init.
		{
#ifdef COMMON_DEBUG_MSG
			_DBG("\n\r !!!!! Clock fault - Recovery");
#endif
			TAS5806MD_Init_After_Clk_Detect();
		}
	}

	if(Fault1 & 0x2)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! PVDD OV fault");
#endif
	}
	
	if(Fault1 & 0x1)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! PVDD UV fault");
#endif
	}

	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_GLOBAL_FAULT2_REG,&Fault2,1);

	if(Fault2 & 0x1)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Over temperature shut down fault");
#endif
		B_Error_Flag = 1;
		//B_Is_Error = TRUE;
	}
	
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_OT_WARNING_REG,&Warning,1);

	if(Warning & 0x4)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Over temperature warning ,135C");
#endif
		B_Error_Flag = 1;
		//B_Is_Error = TRUE;
	}

	if(B_Error_Flag == 1) //1:HighTemp
	{	
#if defined(TIMER21_LED_ENABLE) && defined(AMP_ERROR_ALARM)
		if(Get_Cur_Status_LED_Mode() != STATUS_AMP_ERROR_MODE)
			Set_Status_LED_Mode(STATUS_AMP_ERROR_MODE); //over-temperature or short-circuit condition
#endif

#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01
		if(!Return_Val_Only)
		{
#ifdef USEN_BAP
			//2023-04-07_3 : Need to execute Hi-Temp Error (1 : Related High-Temp / 2 : Clock)
			TAS5806MD_Fault_Clear_Reg(); //Added Fault Clear here to output audio w/o stopping
			Volume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
			Volume_Level += 10;
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
			TAS5806MD_Amp_Volume_Register_Writing(Volume_Level);
			TIMER20_Amp_error_flag_Start();
#endif
			//TAS5806MD_AGL_Value_Change(); //2023-04-07_3 : Disable
#else
#ifdef SWITCH_BUTTON_KEY_ENABLE
			Send_Remote_Key_Event(VOL_DOWN_KEY);
#endif
#endif
			TAS5806MD_Fault_Clear_Reg(); //Added Fault Clear here to output audio w/o stopping
			TIMER20_Amp_error_flag_Start();
		}
#endif
	}
	else
	{
#if 0 //2022-11-15_2 : This statement make Amp init error probably
		if(B_Is_Error)
		{
			B_Error_Flag = 2;
			TIMER20_Amp_error_flag_Start();
		}
		else
#endif
			B_Error_Flag = 0;
	}

	BAmp_COM = FALSE;

	return B_Error_Flag;
	//To Do !!! Need to Add Mute or Volume down functions here
	}

void TAS5806MD_Register_Read(void) //2022-10-25 : FAULT PIN
{
#ifdef TAS5806MD_DEBUG_MSG
	uint8_t uRead = 0;
	int i, j;
	
	_DBG("\n\r +++TAS5806MD_Register_Read()");
	
	for(j=0;j<4;j++)
	{
		switch(j)
		{
			case 0:
			TAS5806MD_Amp_Move_to_Control_Page();
			_DBG("\n\r ===TAS5806MD_Amp_Move_to_Control_Page()");
			break;

			case 1:
			TAS5806MD_Amp_Move_to_DSP_Control_Page();
			_DBG("\n\r ===TAS5806MD_Amp_Move_to_DSP_Control_Page()");
			break;

			case 2:
			TAS5806MD_Amp_Move_to_Volume_Control_Page();
			_DBG("\n\r ===TAS5806MD_Amp_Move_to_Volume_Control_Page()");
			break;

			case 3:
			TAS58066MD_Amp_Move_to_DRC_band3_Page();
			_DBG("\n\r ===TAS58066MD_Amp_Move_to_DRC_band3_Page()");
			break;
		}

		for(i=0;i<0xff;i++)
		{
			if(!(i%16))
			{
				switch(i/16)
				{
					case 0x0:
					_DBG("\n\r 0x00 ~ 0x0f :");
					break;
					case 0x1:
					_DBG("\n\r 0x10 ~ 0x1f :");
					break;
					case 0x2:
					_DBG("\n\r 0x20 ~ 0x2f :");
					break;
					case 0x3:
					_DBG("\n\r 0x30 ~ 0x3f :");
					break;
					case 0x4:
					_DBG("\n\r 0x40 ~ 0x4f :");
					break;
					case 0x5:
					_DBG("\n\r 0x50 ~ 0x5f :");
					break;
					case 0x6:
					_DBG("\n\r 0x60 ~ 0x6f :");
					break;
					case 0x7:
					_DBG("\n\r 0x70 ~ 0x7f :");
					break;
					case 0x8:
					_DBG("\n\r 0x80 ~ 0x8f :");
					break;
					case 0x9:
					_DBG("\n\r 0x90 ~ 0x9f :");
					break;
					case 0xa:
					_DBG("\n\r 0xa0 ~ 0xaf :");
					break;
					case 0xb:
					_DBG("\n\r 0xb0 ~ 0xbf :");
					break;
					case 0xc:
					_DBG("\n\r 0xc0 ~ 0xcf :");
					break;
					case 0xd:
					_DBG("\n\r 0xd0 ~ 0xdf :");
					break;
					case 0xe:
					_DBG("\n\r 0xe0 ~ 0xef :");
					break;
					case 0xf:
					_DBG("\n\r 0xf0 ~ 0xff :");
					break;
				}
			}
			
			I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, i,&uRead,1);
			_DBH(uRead);
		}
	}
#endif //TAS5806MD_DEBUG_MSG
}

void TAS5806MD_Fault_Clear_Reg(void) 
{
	uint8_t uWrite = 0;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Fault_Clear_Reg()");
#endif
	BAmp_COM = TRUE;

	uWrite = 0x80; //Bit 7 : Write Clear Bit
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_FAULT_CLEAR_REG,&uWrite,1);

	BAmp_COM = FALSE;
}

#ifdef USEN_BAP
void TAS5806MD_AGL_Value_Change(void)
{
	uint8_t uSize, i, Data;
	
	//Write TAS5806MD_Init	
	uSize = sizeof(TAS5806MD_AGL_Table)/2;

	for(i =0;i<uSize;i++)
	{
		Data = TAS5806MD_AGL_Table[i][1];
		I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_AGL_Table[i][0],&Data,1);
	}
}

void TAS5806MD_EQ_OnOff(Bool BEQ_On)
{
	uint8_t Data = 0;
	
	//EQ
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	
	if(BEQ_On)
		Data &= ~(TAS5806MD_EQ_BYPASS_CONTROL);
	else
		Data |= TAS5806MD_EQ_BYPASS_CONTROL;
	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);		
}

void TAS5806MD_DRC_OnOff(Bool BDRC_On)
{
	uint8_t Data = 0;
	
	//DRC
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	
	if(BDRC_On)
		Data &= ~(TAS5806MD_DRC_BYPASS_CONTROL);
	else
		Data |= TAS5806MD_DRC_BYPASS_CONTROL;
	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);		
}
#endif

#ifdef DRC_TOGGLE_TEST
void TAS5806MD_DRC_On(void)
{
	uint8_t Data = 0;

	//DRC
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	Data &= ~(TAS5806MD_DRC_BYPASS_CONTROL);
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	
	//AGL
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x00
	Data = 0x8C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&Data,1); //0x68:0x8c
	Data = 0x2C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x2c
	Data = 0xC0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x68,&Data,1); //0x68:0xc0
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x69,&Data,1); //0x69:0xc0
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6A,&Data,1); //0x6A:0xc0	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6B,&Data,1); //0x6B:0xc0
}

void TAS5806MD_DRC_Off(void)
{
	uint8_t Data = 0;

	//DRC
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	Data |= TAS5806MD_DRC_BYPASS_CONTROL;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data ,1);
	
	//AGL
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x00
	Data = 0x8C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&Data,1); //0x68:0x8c
	Data = 0x2C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x2c
	Data = 0x40;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x68,&Data,1); //0x68:0x40
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x69,&Data,1); //0x69:0xc0
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6A,&Data,1); //0x6A:0xc0	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6B,&Data,1); //0x6B:0xc0
}
#endif

#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
void TAS5806MD_Amp_Volume_Register_Writing(uint8_t uVolumeLevel)
{
	uint8_t uReg_Value[4] = {0,};
	uint8_t uArrayLevel = 0;
	int i;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Volume_Register_Writing() - Vol_Level = ");
	_DBD(uVolumeLevel);
#endif

	if(Is_BAmp_Init()) //2023-02-22_1
	{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\r+++ Is_BAmp_Init is TRUE - 3");
#endif
		return;
	}

	uArrayLevel = Find_Volume_Level[uVolumeLevel];

#ifdef USEN_IT_AMP_EQ_ENABLE //2023-04-28_1 : To apply BSP-01T EQ Setting to BAP-01 under EQ BSP Mode //#if !defined(USEN_BAP) && defined(USEN_IT_AMP_EQ_ENABLE) //2023-03-08_3 : Control volume level for each EQ Mode
#ifdef USEN_BAP //2023-04-28_1
	if(uVolumeLevel != 49)
#else
	if(uVolumeLevel != 15)
#endif
	{
		switch(Cur_EQ_Mode)
		{
			case EQ_POP_ROCK_MODE:
				uArrayLevel += 3;
			break;
			case EQ_JAZZ_MODE:
				uArrayLevel += 2;
			break;
			case EQ_VOCAL_MODE:
				uArrayLevel += 3;
			break;

			case EQ_NORMAL_MODE:
			case EQ_CLUB_MODE:
#ifdef USEN_BAP //2023-04-28_1 : To apply BSP-01T EQ Setting to BAP-01 under EQ BSP Mode
			case EQ_BAP_NORMAL_MODE:
#endif
			default:
			break;
		}
	}
#endif

#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\r 2. Vol_Level = ");
		_DBD(uVolumeLevel);
#endif

#if defined(USEN_BAP) && defined(ADC_INPUT_ENABLE)
#ifndef MASTER_MODE_ONLY
	if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode) //2023-01-09_3 : Under Slave Mode, Attenuator value is always +0dB and Volume is synchrinized with Master
#endif
		uArrayLevel += uAttenuator_Vol_Value();
#endif

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\ruArrayLevel = ");
	_DBD(uArrayLevel);
#endif

	if(uArrayLevel > 110)
		uArrayLevel = 110;
	
	for(i=0;i<4;i++)
		uReg_Value[i] = TAS5806MD_Volume_Table[uArrayLevel][i];

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\ruCurVol Reg value: ");

	for(i=0;i<4;i++)
	{
		_DBG(" 0x");
		_DBH(uReg_Value[i]);
	}
#endif

	BAmp_COM = TRUE;

	TAS5806MD_Amp_Move_to_Volume_Control_Page();
		
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_VOL_CONTROL_REG1,uReg_Value,4);
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_VOL_CONTROL_REG2,uReg_Value,4);

	TAS5806MD_Amp_Move_to_Control_Page(); //2023-02-21_3 : Fixed Mute Error after Volume Control. Need to Move General Page instead of DSP Page.

	BAmp_COM = FALSE;
}
#endif //#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE

#endif //TAS5806MD_ENABLE

