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

#include "remocon_action.h"
#include "serial.h"
#include "AD85050.h"
#include "bt_MB3021.h"
#include "led_display.h"
#include "timer20.h"
#include "flash.h"
#include "power.h"
#include "pcm9211.h"
#include "key.h"

/* Private define ----------------------------------------------------*/
//#define REMOTE_CONTROL_ACTION_DBG							(1)

/* Private define ----------------------------------------------------*/
const uint8_t ir_key_map[] =
{
	POWER_KEY,
	BT_PAIRING_KEY,
	MUTE_KEY,
	VOL_UP_KEY,
	VOL_DOWN_KEY,
	SW1_KEY,
	SW2_KEY,
	FACTORY_RESET_KEY,
	BT_KEY,
	BT_UPDATE_KEY,
	INPUT_BT_KEY,
	INPUT_AUX_KEY,	
	BT_OUT_AREA_1_KEY,
	BT_OUT_AREA_2_KEY,
	BT_OUT_AREA_1_2_KEY,
	BT_OUT_OFF_KEY,
	BT_OUT_ON_KEY,
};

#define IR_KEY_MAP_COUNT (sizeof(ir_key_map))

void Send_Remote_Key_Event(uint8_t IR_KEY)
{
	uint8_t bCnt;
	Bool BValidKey = FALSE;

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

	if(!IS_BBT_Init_OK())
	{
#ifdef REMOTE_CONTROL_ACTION_DBG
		_DBG("\n\rNeed to wait until Power on init in BT is finshed!!!");
#endif
			return;
	}

	if(!Power_State())
	{
		if(IR_KEY != POWER_KEY && IR_KEY != FACTORY_RESET_KEY && IR_KEY != SW1_KEY && IR_KEY != SW2_KEY) //2023-04-06_3 : Need to allow SW1_KEY and SW2_KEY even though Power off mode.
		{
#if 1//def REMOTE_CONTROL_ACTION_DBG
			_DBG("\n\r5. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
			return;
		}
	}

	keyCode = IR_KEY;
	keyOk = ON;
}

