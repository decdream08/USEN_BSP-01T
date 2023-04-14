
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

#ifdef AD82584F_ENABLE
#include "ad82584f.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h"
#endif

//Macro
#ifdef TIMER21_LED_ENABLE
typedef enum {
	STATUS_POWER_ON_MODE, //Mute : White On
	STATUS_POWER_OFF_MODE, //Mute : White Off/Red Off, Status : White Off/Red Off
	STATUS_BT_PAIRED_MODE, //Status : White On
	STATUS_AUX_MODE, //Status : White On / BT LED(BLUE/WHITE) OFF
	STATUS_BT_PAIRING_MODE, //Status : White Fast Blinking
#ifdef GIA_MODE_LED_DISPLAY_ENABLE
	STATUS_BT_GIA_PAIRING_MODE, //Status : White led Very Fast Blinking
#endif
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY
	STATUS_BT_MASTER_SLAVE_PAIRING_MODE, //Status : White/Blue led Fast 250ms Blinking
#endif
	STATUS_BT_FAIL_OR_DISCONNECTION_MODE, //Disconnection//Status : White Slow Blinking
	STATUS_MUTE_ON_MODE, //Mute : Red On/White Off, Status : Red On
#ifdef AMP_ERROR_ALARM
	STATUS_AMP_ERROR_MODE, //Status LED Very Fast
#endif
	STATUS_SOC_ERROR_MODE //Status : Red Blinking
} Status_LED_Mode;

typedef enum {
	STATUS_LED_RED,
	STATUS_LED_WHITE,
	L3_LED_BLUE,
	L3_LED_WHITE
} Status_LED_Color;
#endif //TIMER21_LED_ENABLE

//Function
#ifdef EQ_TOGGLE_ENABLE //2023-01-17
void LED_DIsplay_EQ_Mode(EQ_Mode_Setting EQ_mode);
#endif

#ifdef TIMER21_LED_ENABLE
void Set_Status_LED_Mode(Status_LED_Mode mode);
Status_LED_Mode Get_Cur_Status_LED_Mode(void);
Status_LED_Mode Get_Return_Status_LED_Mode(void);
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY //When cur mode is STATUS_BT_MASTER_SLAVE_PAIRING_MODE, we need to return original value. this variable must update status led mode excepting STATUS_BT_MASTER_SLAVE_PAIRING_MODE mode.
Status_LED_Mode Get_Return_Background_Status_LED_Mode(void);
#endif
void LED_Status_Display_WR_Color(Status_LED_Mode mode);
void LED_Status_Display_Blinking(Status_LED_Color Color, Bool On);
#endif
#ifdef AMP_ERROR_ALARM
Status_LED_Mode Get_Return_Status_LED_Mode2(void);
#endif

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
void LED_Display_Volume_All_Off(void);
void LED_Display_Volume(uint8_t Volume);
#endif

#ifdef GPIO_LED_ENABLE //Use GPIOs as LED output control - PC1 / PC2 / PD2 / PD3 / PD4 / PD5
#ifdef LED_TEST_PC4
void LED_Test(Bool bPower_on); //PC3
#endif
#endif //GPIO_LED_ENABLE

#ifdef FACTORY_RESET_LED_DISPLAY
void LED_Display_All_On(void);
void LED_Diplay_All_Off(void);
#endif

#endif //__LED_DISPLAY_H__

