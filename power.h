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
} PowerModeDef;

void Power_10ms_timer(void);
void Power_Mode_Set(unsigned char mode);
void Power_Process(void);

Bool Power_State(void);

