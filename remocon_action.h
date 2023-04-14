
/**
 *
 * @file remocon_action.h
 *
 * @author   MS KIM
 *
 * @brief
 */

#ifndef _REMOCON_ACTION_H_
#define _REMOCON_ACTION_H_

//Macro
#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
#ifdef AD82584F_ENABLE
#include "ad82584f.h" //due to "EQ_Mode_Setting"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h"
#endif

// LG KEY MAP - this key map is based on LG TV remocon //Need to change ....???
#ifdef SUPPORT_LG_REMOCON
#define POWER_KEY			0x10
#define NUM_0_KEY			0x08
#define NUM_1_KEY			0x88
#define NUM_2_KEY			0x48
#define NUM_3_KEY			0xc8
#define NUM_4_KEY			0x28
#define NUM_5_KEY			0xa8
#define NUM_6_KEY			0x68
#define NUM_7_KEY			0xe8
#define NUM_8_KEY			0x18
#define NUM_9_KEY			0x98
#define VOL_UP_KEY			0x40
#define VOL_DOWN_KEY			0xc0
#define MUTE_KEY			0x90
#define CH_UP_KEY			0x00
#define CH_DOWN_KEY			0x80
#define PLAY_KEY			0x0d
#define PAUSE_KEY			0x5d
#else //SUPPORT_LG_REMOCON
#define NONE_KEY			0x00
#define POWER_KEY			0x80
#define BT_PAIRING_KEY		0x40
#define MUTE_KEY			0xc0

#define EXIT_KEY				0x20
#define DIRECTION_KEY		0xa0
#define SPEAK_KEY			0x60

#define VOL_UP_KEY			0x10
#define VOL_DOWN_KEY		0x50

#define SUB_VOL_UP_KEY			0xe0
#define SUB_VOL_DOWN_KEY		0x90

#define BASS_BOOST_KEY			0xb8
#define PRESET_KEY				0xc8

#define NUM_1_KEY			0xd0
#define NUM_2_KEY			0x30
#define NUM_3_KEY			0xb0

#define MOVIE_KEY			0x70
#define GAME_KEY			0xf0
#define MUSIC_KEY			0x08

#ifdef SWITCH_BUTTON_KEY_ENABLE
#define SW1_KEY							(SUB_VOL_UP_KEY)		//0xe0
#define SW2_KEY							(BASS_BOOST_KEY)		//0xb8
#define FACTORY_RESET_KEY				(SPEAK_KEY)				//0x60
#define BT_KEY							(NUM_1_KEY)				//0xd0
#ifndef POWER_KEY_TOGGLE_ENABLE
#define POWER_ON_KEY					(POWER_KEY)				//0x80
#define POWER_OFF_KEY					(SUB_VOL_DOWN_KEY)		//0x90
#endif //POWER_KEY_TOGGLE_ENABLE
#ifdef USEN_BAP //2022-10-12_4
#define BT_UPDATE_KEY					(MOVIE_KEY)				//0x70
#endif //USEN_BAP
#ifdef EQ_TOGGLE_ENABLE //2023-01-17
#define EQ_KEY							(MUSIC_KEY)				//0x08
#endif //EQ_TOGGLE_ENABLE
#endif //SWITCH_BUTTON_KEY_ENABLE
#endif //SUPPORT_LG_REMOCON
#endif //REMOCON_TIMER20_CAPTURE_ENABLE

#ifdef AUTO_ONOFF_ENABLE
extern Bool auto_power_off;
#endif
#ifdef USEN_TABLET_AUTO_ON //When user selects power off button under USEN Tablet, we need to enable auto power on.
extern Bool USEN_Tablet_auto_power_on;
#endif

//Function
Bool Power_State(void);
void Send_Remote_Key_Event(uint8_t IR_KEY);
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
void Remocon_EQ_Toggle_Key_Action(void);
void Remocon_EQ_Key_Action(EQ_Mode_Setting EQ_mode);
void Remocon_Mode_Key_Action(void);
#endif
#ifdef MASTER_MODE_ONLY //2023-03-27_4 : Under BAP-01 NORMAL mode, BAP-01 can get only NORMAL MODE.
void Remocon_BSP_NORMAL_Mode_Switch_Action(void);
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
void Remocon_Power_Key_Action(Bool Power_on, Bool Slave_Sync, Bool Vol_Sync); //Slave_Sync - TRUE: Need to sync with Slave SPK  /FALSE: Do not need to sync with Slave SPK \n
//Vol_Sync - TRUE : Need Vol Sync /FALSE : Do not need Vol Sync and Volume Level will be set in other place
#endif
#ifdef MASTER_SLAVE_GROUPING
void Remocon_BT_Short_Key_Action(void);
#endif

#endif //_REMOCON_ACTION_H_


