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
#ifdef SABRE9006A_ENABLE
#include "i2c.h"
#include "audio_dac.h"

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/

#define MAX_VOLUME_LEVEL		(0)
#define VOLUME_LEVEL_NUMER	(sizeof(Audio_Dac_Volume_Table))
#define MIN_VOLUME_LEVEL		(VOLUME_LEVEL_NUMER - 1)

#define AUDIO_DAC_MUTE_ON						(0x01)
#define AUDIO_DAC_MUTE_OFF					(~(0x01))

/****************************************************
*	Volume in dBs = -REG_VALUE/2
* 	(defualt = 8'd0)
*****************************************************/
#define REG_0_DAC0_VOL_ADDR			(0x00)
#define REG_1_DAC1_VOL_ADDR			(0x01)
#define REG_2_DAC2_VOL_ADDR			(0x02)
#define REG_3_DAC3_VOL_ADDR			(0x03)
#define REG_4_DAC4_VOL_ADDR			(0x04)
#define REG_5_DAC5_VOL_ADDR			(0x05)
#define REG_6_DAC6_VOL_ADDR			(0x06)
#define REG_7_DAC7_VOL_ADDR			(0x07)

#define REG_DAC_VOL_VALUE				(0x00)  //default value : 00

/****************************************************
*	[7]: SPDIF_ENABLE
*	 1'b0 = Use either I2S or DSD input
*	 1'b1 = Use SPIF input
*	[6:0]: Auto-mute trigger point in dB = -REG_VALUE
* 	(default = 1'b0, 7'd104)
*****************************************************/
#define REG_8_AUTOMUTE_LEV_ADDR		(0x08)

#define REG_8_AUTOMUTE_LEV_VALUE		(0x68) //default value

/****************************************************
*	Larger REG_VALUE = less time
*	Smaller REG_VALUE = longer time
*	Time in Seconds = 2096896/(REG_VALUE x DATA_CLK)
*****************************************************/
#define REG_9_AUTOMUTE_TIME_ADDR		(0x09)

#define REG_9_AUTOMUTE_TIME_VALUE			(0x04) //default value

/******************************************************************************************
*	Default is 24-bit, I2S, NO-DEEMP, UNMUTE (default = 8'b00000110)
*	[7:6]: 24-/20-/16-Bit for Serial Data Modes(00-24bit/01-20bit/10-16bit/11-24bit)
*	[5:4]: LJ/I2S/RJ Serial Data Modes (00-I2S/01-LJ/10-RJ/11-I2S)
*	[3]: Reserved
*	[2]: JITTER_REDUCTION_ENABLE(0-Bypass and stop JITTER_REDUCTION/1-Use JITTER_REDUCTION)
* 	[1]: BYPASS_DEEMPHASIS FILTER(0-Use De-emphasize Filter/1-Bypass De-emphasis Filter)
* 	[0]: MUTE DACS(0-Unmute All DACs/1-Mute All DACs)
*******************************************************************************************/
#define REG_10_MODE_CONTROL1_ADDR		(0x0A)

#define REG_10_MODE_CONTROL1_VALUE			(0x06) //default value

/******************************************************************************************
*	[7:5]: RESERVED
*	[4:2]: DPLL BANDWIDTH(BW)
*	000-No BW/001-Lowest BW/011-Med-Low BW/100-Medium BW/101-Med-High BW/110-High BW/111-Highest BW
*	[1:0]: DE-EMPHASIS DELECT(00-32kHz/01-44.1kHz/10-48kHz/11-RESERVED)
* 	(default = 8'b00000101)
*******************************************************************************************/
#define REG_11_MODE_CONTROL1_ADDR		(0x0B)

#define REG_11_MODE_CONTROL2_VALUE			(0x02) //00000010 : default value(0x05)

/******************************************************************************************
*	[7:1]: RESERVED
*	[0]: FIR ROLLOFF SPEED(0-Slow Rolloff/1-Fast Rolloff)
* 	(default = 8'b00000001)
*******************************************************************************************/
#define REG_14_FIR_ROLLOFF_ADDR			(0x0E)

#define REG_14_FIR_ROLLOFF_VALUE			(0x01) //default

/******************************************************************************************
*	[7]: CHANNEL MAPPING(0-8ch/1-2ch)
*	[6:4]: RESERVED
*	[3]: AUTOMUTE LOOPBACK(0-Auto-mute condition will not reduce volume to -infinity
*							1-Auto-mute condition will reduce volume to -infinity
*	[2:0]: VOL_RAMP_RATE(Time=8192/(2vrr x fs)
*	(000-Time=8192/(fs)/001-Time=4096/(fs)/....
* 	(default = 8'b01111000)
*******************************************************************************************/
#define REG_16_VOL_CONFIG_ADDR			(0x10)

#define REG_16_VOL_CONFIG_VALUE			(0x78) //default

/******************************************************************************************
*	[7:4]: RESERVED
*	[3:2]: Input_select(00-serial/01-DSD/10-auto detect/11-auto detect(default)
*	[1]: dpll_bw_128x(1-Multiply the DPLL BANDWIDTH setting by 128/0-Use the DPLL BANDWIDTH setting)
*	[0]: spdif_auto_deemph(1-Automatically enable the de-emhasis filters depending on the channel status bits
*		/0-Do not automatically enable the de-emphasis filters depending on the channel status bits)
* 	(default = 8'b00001110)
*******************************************************************************************/
#define REG_17_MODE_CONTROL4_ADDR			(0x11)

#define REG_17_MODE_CONTROL4_VALUE			(0x02) //00000010 : default value(0x0E)

//Register #19: Programmable Filter Address (Defuault 0x00)
//Register #22-20: Programmable Filter Coefficient (Defualt 0x000000)
//Register #23: Programmable Filter Control (Defuault 0x00)

//Regoster #27-24: Master Trim(Defualt = 32'h 7FFFFFFF)


/******************************************************************************************
*	READ_ONLY !!!!
*
*	[7:5]: Chip_id(Returns 3'd4(SABRE9006A))
*	[3:2]: Reserved
*	[1]: Automute_status(1-Auto-mute condition is active/0-Auto-mute condition is inactive)
*	[0]: Lock_status(1-The Jitter Eliminator is locked to an incoming signal
*								/0-The jitter Eliminator is not locked to an incoming signal)
*******************************************************************************************/
#define REG_64_DAC_STATUS_ADDR				(0x40)

enum {
	REG_0_ADDR = REG_0_DAC0_VOL_ADDR,
	REG_1_ADDR,
	REG_2_ADDR,
	REG_3_ADDR,
	REG_4_ADDR,
	REG_5_ADDR,
	REG_6_ADDR,
	REG_7_ADDR,
	REG_8_ADDR,
	REG_9_ADDR,
	REG_10_ADDR,
	REG_11_ADDR,
	REG_14_ADDR = REG_14_FIR_ROLLOFF_ADDR,
	REG_16_ADDR = REG_16_VOL_CONFIG_ADDR,
	REG_17_ADDR,
	REG_60_ADDR = REG_64_DAC_STATUS_ADDR,
};

//#define MAX_VOLUME_LEVEL		(0)
//#define VOLUME_LEVEL_NUMER	(sizeof(TAS3251_Volume_Table))
//#define MIN_VOLUME_LEVEL		(VOLUME_LEVEL_NUMER - 1)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
/* Private define ----------------------------------------------------*/
/* Private function prototypes ---------------------------------------*/
/* Private variables ---------------------------------------------------*/


//SABRE9006A_ADDR_DEVICE_ADDR_L					(0x48), //8bit - 0x90 /7bit - 100 1000
const uint8_t SABRE9006A_Init[][2] = {
	{ REG_0_ADDR, REG_DAC_VOL_VALUE },
	{ REG_1_ADDR, REG_DAC_VOL_VALUE },
	{ REG_2_ADDR, REG_DAC_VOL_VALUE },
	{ REG_3_ADDR, REG_DAC_VOL_VALUE },
	{ REG_4_ADDR, REG_DAC_VOL_VALUE },
	{ REG_5_ADDR, REG_DAC_VOL_VALUE },
	{ REG_6_ADDR, REG_DAC_VOL_VALUE },
	{ REG_7_ADDR, REG_DAC_VOL_VALUE },
	{ REG_8_ADDR, REG_8_AUTOMUTE_LEV_VALUE},
	{ REG_9_ADDR, REG_9_AUTOMUTE_TIME_VALUE },
	{ REG_10_ADDR, REG_10_MODE_CONTROL1_VALUE },
	{ REG_11_ADDR, REG_11_MODE_CONTROL2_VALUE },
	{ REG_14_ADDR, REG_14_FIR_ROLLOFF_VALUE},
	{ REG_16_ADDR, REG_16_VOL_CONFIG_VALUE},
	{ REG_17_ADDR, REG_17_MODE_CONTROL4_VALUE},
};

uint8_t Audio_Dac_Volume_Table[] = {
	0x00,
	0x10,
	0x20,
	0x30,
	0x40, 	
	0x50,
	0x60,
	0x70,
	0x80,
	0x90, 
	0xff	// Mute
};

void Audio_Dac_Init(void)
{
	uint32_t uSize = 0, i;
	uint8_t Data = 0;
	
#ifdef _I2C_DEBUG_MSG
	_DBG("\n\rAudio_Dac_Init");
#endif

	//Write SABRE9006A_Init
	//SABRE9006A_DEVICE_ADDR_L = (0x48), //8bit - 0x90 /7bit - 100 1000
	uSize = sizeof(SABRE9006A_Init)/2;

	for(i =0;i<uSize;i++)
	{
		Data = SABRE9006A_Init[i][1];
		I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, SABRE9006A_Init[i][0],&Data,1);
	}

#ifdef _I2C_DEBUG_MSG
	_DBG("\n\rTAS3251_DSP_Amp_Init - OK !!!");
#endif
}

void Audio_Dac_Mute(void)
{
	uint8_t uRead = 0;

#ifdef _I2C_DEBUG_MSG	
	_DBG("\n\rTAS3251_DSP_Amp_Mute!!!");
#endif

	I2C_Interrupt_Read_Data(SABRE9006A_DEVICE_ADDR_L, REG_10_ADDR,&uRead,1); //Read Current mute status

	if(uRead & AUDIO_DAC_MUTE_ON) // Current mode - Mute On
	{ //Set Mute Off
		uRead &= AUDIO_DAC_MUTE_OFF;
		I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_10_ADDR,&uRead,1);
#ifdef _I2C_DEBUG_MSG
		_DBG("\n\rMute Off");
#endif
	}		
	else // Mute On
	{ //Set Mute On
		uRead |= AUDIO_DAC_MUTE_ON;
		I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_10_ADDR,&uRead,1);
#ifdef _I2C_DEBUG_MSG
		_DBG("\n\rMute On");
#endif
	}
}

// First of all, You need to make sure if current page is Book 00 & Page00
void Audio_Dac_Volume_Control(Vol_Setting Vol_mode)
{
	uint8_t uRead = 0, uCurVolLevel = 0, i;
	
#ifdef _I2C_DEBUG_MSG	
	_DBG("\n\rTAS3251_DSP_Amp_Volume_Control!!! --- BVolumeUp =");
#endif

	//We suppose the right level and the left level is same. Read Current mute status
	I2C_Interrupt_Read_Data(SABRE9006A_DEVICE_ADDR_L, REG_0_ADDR,&uRead,1);
	
	// Check whether current volume level is Max/Min or not
	if(((uRead >= Audio_Dac_Volume_Table[MIN_VOLUME_LEVEL]) && (Vol_mode == Volume_Down))	\
		|| ((uRead <= Audio_Dac_Volume_Table[MAX_VOLUME_LEVEL]) && (Vol_mode == Volume_Up)))
	{
#ifdef _I2C_DEBUG_MSG
		_DBG("\n\rCan't change current volume level becuase it's a MAX or MIN !!! ");
		_DBH(uRead);
#endif
		return;
	}

	for(i =0; i < VOLUME_LEVEL_NUMER; i++)
	{
		if(Audio_Dac_Volume_Table[i] == uRead)
		{
			uCurVolLevel = i;
#ifdef _I2C_DEBUG_MSG
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
			
	uRead = Audio_Dac_Volume_Table[uCurVolLevel];

	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_0_ADDR,&uRead,1);
	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_1_ADDR,&uRead,1);
	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_2_ADDR,&uRead,1);
	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_3_ADDR,&uRead,1);
	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_4_ADDR,&uRead,1);
	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_5_ADDR,&uRead,1);
	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_6_ADDR,&uRead,1);
	I2C_Interrupt_Write_Data(SABRE9006A_DEVICE_ADDR_L, REG_7_ADDR,&uRead,1);
}

#ifdef _I2C_DEBUG_MSG
void Audio_Dac_Read_Register(void)
{
	uint32_t uSize = 0, i;
	uint8_t Data = 0;

	_DBG("\n\rSABRE9006A Read All Reg = ");
	
	uSize = sizeof(SABRE9006A_Init)/2;
	
	for(i =0;i<uSize;i++)
	{
		I2C_Interrupt_Read_Data(SABRE9006A_DEVICE_ADDR_L, SABRE9006A_Init[i][0],&Data,1);
		_DBH(Data);
	}

	//We suppose the right level and the left level is same. Read Current mute status
	I2C_Interrupt_Read_Data(SABRE9006A_DEVICE_ADDR_L, REG_60_ADDR,&Data,1);
	_DBG("\n\rSABRE9006A Read Status = ");
	_DBH(Data);
}
#endif
#endif //SABRE9006A_ENABLE


