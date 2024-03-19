/**********************************************************************
* @file		protection.c
* @brief	protection
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"
#include "protection.h"
#include "power.h"
#include "led_display.h"
#include "timer20.h"

uint16_t protection_timer;
uint8_t protection_check_flag;

Protect_Type protection_data;
ProtectionMode protection_mode;

void Protection_10ms_timer(void)
{
	if(protection_timer > df10msTimer0ms )
	{
		--protection_timer;
	}
}

void Protection_Process(void)
{
	if(protection_check_flag == ON)
	{
		if ( protection_data.mode == ProtectionNone )
		{
			if ( protection_check() != ProtectionNone )
			{
				protection_mode = protection_data.mode;
				Power_Mode_Set(PWR_OFF_PROTECTION_START);

				init_protect_values();
			}
		}
	}
}

ProtectionMode protection_check(void)
{
	if ( protection_data.mode == ProtectionNone )
	{
		if ( (HAL_GPIO_ReadPin(PC) & (1<<4)) ) // etc
		{
			protection_data.etc_count = 0;
		}
		else
		{
			++protection_data.etc_count;
			if ( protection_data.etc_count > PROTECTION_COUNT )
			{
				protection_data.mode = ProtectionETC;
				return protection_data.mode;;
			}
		}

		if ( (HAL_GPIO_ReadPin(PD) & (1<<0)) ) // amp
		{
			protection_data.amp_count = 0;
		}
		else
		{
			++protection_data.amp_count;
			if ( protection_data.amp_count > PROTECTION_COUNT )
			{
				protection_data.mode = ProtectionAMP;
				return protection_data.mode;;
			}

		}

		if ( (HAL_GPIO_ReadPin(PD) & (1<<1)) ) // led
		{
			protection_data.led_count = 0;
		}
		else
		{
			++protection_data.led_count;
			if ( protection_data.led_count > PROTECTION_COUNT )
			{
				protection_data.mode = ProtectionLED;
				return protection_data.mode;
			}

		}
	}

	return protection_data.mode;
}

uint8_t protection_status(void)
{
	if ( protection_data.mode == ProtectionNone )
	{
		return OFF;
	}
	else
	{
		return ON;
	}
}

void init_protect_values(void)
{
	protection_data.mode = ProtectionNone;
	protection_data.etc_count = 0;
	protection_data.amp_count = 0;
	protection_data.led_count = 0;
	
	protection_check_flag = OFF;
}

