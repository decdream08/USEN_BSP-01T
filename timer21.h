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
#ifndef __TIMER21_H__
#define __TIMER21_H__
	
/* Includes ---------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

void TIMER21_Configure(void);
void TIMER21_Periodic_Mode_Run(Bool On);
int16_t TIMER21_500ms_Count_Value(void);

#endif /* __TIMER21_H__ */


