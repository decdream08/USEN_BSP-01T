/**********************************************************************
* @file		key.h
* @brief	key
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/
typedef enum {
	Volume_Up_In,
	Volume_Down_In
}Volume_Input;

void Key_10ms_timer(void);
void Key_Process(void);

void Factory_Mode_Setting(void);
void Power_SetState(Bool p_state);
Bool Power_State(void);
void Remocon_MUTE_Key_Action(void);
void Remocon_VOL_Key_Action(Volume_Input Volume_In);
void Remocon_EQ_Toggle_Key_Action(void);
void Remocon_EQ_Key_Action(EQ_Mode_Setting EQ_mode);
void Remocon_Mode_Key_Action(void);
void Remocon_Power_Key_Action_Toggle(void);
void Remocon_Power_Key_Action(Bool Power_on, Bool Slave_Sync, Bool Vol_Sync);
void Remocon_BT_Long_Key_Action(void);
void Remocon_BT_Short_Key_Action(void);

extern uint8_t keyCode;
extern uint8_t keyOk;

