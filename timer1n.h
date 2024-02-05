/**********************************************************************
* @file		main_conf.h
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library on A31G21x
* @version	1.0
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
**********************************************************************/
#ifndef __TIMER1n_H__
#define __TIMER1n_H__

	
/* Includes ---------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

#ifdef TIMER1n_LED_PWM_ENABLE
typedef enum {
	Timer1n_10,
	Timer1n_11
} Timer1n_type;
#endif

typedef enum {
	Timer13_None_Key,
#if !defined(USEN_BAP) /*&& !defined(USEN_BAP_2)*/ //2022-10-07_3
	Timer13_Power_Key,
#endif
	Timer13_BT_Pairing_Key,
	Timer13_Factory_Reset_Key,
#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : Adding EQ Toggle Key using MUTE KEY(Long Key)
	Timer13_EQ_Toggle_Key,
#endif
} Timer13_Long_Key_Type;

#ifdef TIMER12_13_LONG_KEY_ENABLE
void TIMER12_Configure(void);
void TIMER12_Periodic_Mode_Run(Bool On, Bool Vol_Up);
int16_t TIMER12_50ms_Count_Value(void);

void TIMER13_Configure(void);
void TIMER13_Periodic_Mode_Run(Bool On, Timer13_Long_Key_Type Key_Type);
int16_t TIMER13_50ms_Count_Value(void);

#ifdef EQ_TOGGLE_ENABLE //2023-01-17
Bool Is_EQ_Long_Key(void); //To avoid, sending same key again
#endif

Bool Is_Power_Long_Key(void);
Bool Is_BT_Long_Key(void); //To avoid, sending same key again

Bool Is_Volume_Up_Long_Key(void);
Bool Is_Volume_Down_Long_Key(void);
#endif

#ifdef TIMER1n_LED_PWM_ENABLE
void TIMER1n_Configure(Timer1n_type timer_type, uint8_t utime);
void TIMER1n_PWMRun(Timer1n_type timer_type);
#endif

#endif /* __TIMER1n_H__ */


