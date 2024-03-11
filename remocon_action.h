
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

enum KeyCode {
	NONE_KEY,
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

void Send_Remote_Key_Event(uint8_t IR_KEY);

#endif //_REMOCON_ACTION_H_


