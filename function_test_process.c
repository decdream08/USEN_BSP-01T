/**********************************************************************
* @file		function_test_process.c
* @brief	power
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"
#include "power.h"
#include "AD85050.h"
#include "pcm9211.h"
#include "flash.h"
#include "led_display.h"
#include "bt_MB3021.h"
#include "timer20.h"
#include "protection.h"

#include "i2c.h"
#include "remocon_action.h"

#include "function_test_process.h"

uint16_t function_test_process_timer;
uint8_t function_test_process_step;

void FunctionTest_Process_10ms_timer(void)
{
	if(function_test_process_timer > df10msTimer0ms )
	{
		--function_test_process_timer;
	}
}

void FunctionTest_Process(void)
{
	switch(function_test_process_step)
	{
		case 0:
			if(function_test_process_timer == df10msTimer0ms )
			{
				if(Power_Get_Mode() == PWR_ON_NORMAL)
				{
					Power_Mode_Set(PWR_OFF_START);
					//Send_Remote_Key_Event(POWER_KEY);

					function_test_process_step = 1;
					function_test_process_timer = df10msTimer500ms;

				}
			}
			break;

		case 1:
			if(function_test_process_timer == df10msTimer0ms )
			{
				if(Power_Get_Mode() == PWR_STNDBY)
				{
					Power_Mode_Set(PWR_ON_START);
					//Send_Remote_Key_Event(POWER_KEY);
					
					function_test_process_step = 0;
					function_test_process_timer = df10msTimer500ms;
				}
			}
			break;

		default:
			break;
	}
}


