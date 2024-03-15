/**********************************************************************
* @file		PCM9211.c
* @brief	DIR
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"
#ifdef PCM9211_ENABLE
#include "AD85050.h"
#include "pcm9211.h"

static PCM92211_Status pcm9211_status;

uint16_t pcm9211_timer;

void PCM9211_10ms_timer(void)
{
	if(pcm9211_timer > df10msTimer0ms )
	{
		--pcm9211_timer;
	}
}

void PCM9211_Process(void)
{
	switch(pcm9211_status)
	{
		case PCM9211_POWER_UP:
			PCM9211_Init();
			break;

		case PCM9211_POWER_UP_COMPLTE:
			AD85050_PowerUp();
			pcm9211_status = PCM9211_RUN;
			break;

		case PCM9211_CHANGE_PATH_TO_AUXIN0:
			PCM9211_Set_Path_BT();
			pcm9211_status = PCM9211_RUN;
			break;

		case PCM9211_CHANGE_PATH_TO_ADC:
			PCM9211_Set_Path_AUX();
			pcm9211_status = PCM9211_RUN;
			break;

		case PCM9211_MUTE_WAITING:
			if(pcm9211_timer  == df10msTimer0ms)
			{
				AD85050_Amp_Mute(FALSE, FALSE);
				pcm9211_status = PCM9211_RUN;
		    }
			break;

		case PCM9211_RUN:
			break;

		default:
			break;
	}
}

void PCM9211_Set_Status(PCM92211_Status status)
{
	pcm9211_status = status;
}

void PCM9211_PowerUp(void)
{
	pcm9211_status = PCM9211_POWER_UP;
	HAL_GPIO_SetPin(PE, _BIT(2)); //reset
}

void PCM9211_PowerDown(void)
{
	pcm9211_status = PCM9211_POWER_DOWN;
}

void PCM9211_Init(void)
{
#ifdef I2C_1_ENABLE
	uint8_t uData = 0;

	/* Reset On */
	uData = PCM9211_RESET_CTL_REG_ON_VAL;
	I2C_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_RESET_CTL_REG,&uData,1);
	/* Reset Off : 0xC0 -> 0xC2 */
	uData = PCM9211_RESET_CTL_REG_OFF_VAL;
	I2C_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_RESET_CTL_REG,&uData,1);
	
	/* xti clock source : 12.288M 3.072Mhz, 48KHz (0x1A) */
	uData = PCM9211_XTI_SOURCE_CLOCK_12D288_48K;
	I2C_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_XTI_SOURCE_CLOCK,&uData,1);

    PCM9211_Set_Output(PCM9211_OUTPORT_PORT_CTL_REG_AUXIN1); //dummy
#endif
	pcm9211_status = PCM9211_POWER_UP_COMPLTE;
}

void PCM9211_Set_Output(uint8_t Port)
{
#ifdef I2C_1_ENABLE
	uint8_t output;

	output = Port;
	I2C_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_OUTPORT_PORT_CTL_REG,&output,1);
#endif
}

void PCM9211_Set_Path_Init(void)
{
	if(HAL_GPIO_ReadPin(PE) & (1<<0))
	{
		PCM9211_Set_Path_BT();
    }
	else
	{
		PCM9211_Set_Path_AUX();
	}
}

void PCM9211_Set_Path_BT(void)
{
	if(!Get_Is_Mute())
	{
		AD85050_Amp_Mute(TRUE, FALSE);
		pcm9211_status = PCM9211_MUTE_WAITING;
		pcm9211_timer = PCM9211_UNMUTE_TIMER;
  }

	PCM9211_Set_Output(PCM9211_OUTPORT_PORT_CTL_REG_AUXIN0);	
}

void PCM9211_Set_Path_AUX(void)
{
	if(!Get_Is_Mute())
	{
		AD85050_Amp_Mute(TRUE, FALSE);
		pcm9211_status = PCM9211_MUTE_WAITING;
		pcm9211_timer = PCM9211_UNMUTE_TIMER;
	}

	PCM9211_Set_Output(PCM9211_OUTPORT_PORT_CTL_REG_ADC);
}
#endif
