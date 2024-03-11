
/**********************************************************************
* @file		led_display.h
* @brief	IR code
* @version	1.0
* @date		
* @author	MS Kim
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#ifndef __LED_DISPLAY_H__
#define __LED_DISPLAY_H__
#include "AD85050.h"

//Macro
typedef enum {
	STATUS_POWER_ON_MODE, //Mute : White On
	STATUS_POWER_OFF_MODE, //Mute : White Off/Red Off, Status : White Off/Red Off
	STATUS_BT_PAIRED_MODE, //Status : White On
	STATUS_AUX_MODE, //Status : White On / BT LED(BLUE/WHITE) OFF
	STATUS_BT_PAIRING_MODE, //Status : White Fast Blinking
	STATUS_BT_GIA_PAIRING_MODE, //Status : White led Very Fast Blinking
	STATUS_BT_MASTER_SLAVE_PAIRING_MODE, //Status : White/Blue led Fast 250ms Blinking
	STATUS_BT_FAIL_OR_DISCONNECTION_MODE, //Disconnection//Status : White Slow Blinking
	STATUS_MUTE_ON_MODE, //Mute : Red On/White Off, Status : Red On
	STATUS_AMP_ERROR_MODE, //Status LED Very Fast
	STATUS_SOC_ERROR_MODE //Status : Red Blinking
} Status_LED_Mode;

typedef enum {
	STATUS_LED_RED,
	STATUS_LED_WHITE,
	L3_LED_BLUE,
	L3_LED_WHITE
} Status_LED_Color;

//Function
void Set_Status_LED_Mode(Status_LED_Mode mode);
Status_LED_Mode Get_Cur_Status_LED_Mode(void);
Status_LED_Mode Get_Return_Status_LED_Mode(void);
Status_LED_Mode Get_Return_Background_Status_LED_Mode(void);

void LED_Status_Display_WR_Color(Status_LED_Mode mode);
void LED_Status_Display_Blinking(Status_LED_Color Color, Bool On);

Status_LED_Mode Get_Return_Status_LED_Mode2(void);

void LED_Display_All_On(void);
void LED_Diplay_All_Off(void);

#ifdef _DEBUG_MSG
void Debug_Test_Blue_LED_On(Bool On);
#endif

#endif //__LED_DISPLAY_H__

