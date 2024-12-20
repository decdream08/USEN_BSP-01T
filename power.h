/**********************************************************************
* @file		power.h
* @brief	power
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

typedef enum
{
  PWR_STNDBY,
  PWR_ON_START,
  PWR_ON_NORMAL,
  PWR_OFF_START,
  PWR_OFF_PROTECTION_START,
  PWR_OFF_AMP_FAULT_START,
  PWR_LOW_LEVEL_START,
  PWR_LOW_LEVEL_STNDBY,
} PowerModeDef;

void Power_10ms_timer(void);
void Power_Mode_Set(unsigned char mode);
PowerModeDef Power_Get_Mode(void);
void Power_Process(void);

void Power_SetState(Bool power_state);
Bool Power_State(void);

