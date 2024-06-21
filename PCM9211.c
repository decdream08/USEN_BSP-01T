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
#include "timer20.h"
#include "bt_MB3021.h"

static PCM92211_Status pcm9211_status;
static PCM92211_PathStatus pcm9211_path_status;

uint16_t pcm9211_timer;
uint16_t pcm9211_path_check_timer;

extern Bool already_initialized;

void PCM9211_10ms_timer(void)
{
	if(pcm9211_timer > df10msTimer0ms )
	{
		--pcm9211_timer;
	}

	if(pcm9211_path_check_timer > df10msTimer0ms )
	{
		--pcm9211_path_check_timer;
	}
}

void PCM9211_Process(void)
{
	switch(pcm9211_status)
	{
		case PCM9211_POWER_UP:
			if(pcm9211_timer  == df10msTimer0ms)
			{
				PCM9211_Init();
			}
			break;

		case PCM9211_POWER_UP_COMPLTE:
			AD85050_PowerUp();
			pcm9211_status = PCM9211_RUN;
			break;

		case PCM9211_CHANGE_PATH_TO_AUXIN0:
			PCM9211_Set_Path_BT(TRUE);
			break;

		case PCM9211_CHANGE_PATH_TO_ADC:
			PCM9211_Set_Path_ADC(TRUE);
			break;

		case PCM9211_MUTE_WAITING:
		case PCM9211_MUTE_WAITING_WITH_SLAVE:
			if(pcm9211_timer  == df10msTimer0ms)
			{
				AD85050_Amp_Mute(FALSE, FALSE);

				if(pcm9211_status == PCM9211_MUTE_WAITING_WITH_SLAVE)
					MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x01); //MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Mute, 0x00);

				pcm9211_status = PCM9211_RUN;
		    }
			break;

		case PCM9211_INPUT_CHANGE:
			PCM9211_Set_Path_Init(FALSE);
			break;

		case PCM9211_RUN:
#if 0
			if(pcm9211_path_check_timer == df10msTimer0ms)
			{
				uint8_t uData = 0;

				I2C1_Interrupt_Read_Data(PCM9211_DEVICE_ADDR, PCM9211_OUTPORT_PORT_CTL_REG,&uData,1);

				if(uData == 0x00)
				{
					if(pcm9211_path_status == PCM9211_PATH_BT)
					{
						pcm9211_path_status = PCM9211_PATH_NULL;
						PCM9211_Set_Path_BT(TRUE);
					}
					else if(pcm9211_path_status == PCM9211_PATH_ADC)
					{
						pcm9211_path_status = PCM9211_PATH_NULL;
						PCM9211_Set_Path_ADC(TRUE);
					}
				}
#ifdef PCM9211_DEBUG_MSG
				_DBG("\n\rPCM9211 path = ");
				_DBD(uData);
#endif
				pcm9211_path_check_timer = df10msTimer1s;
			}
#endif
			break;

		default:
			break;
	}
}

void PCM9211_Set_Status(PCM92211_Status status)
{
	if((pcm9211_status == PCM9211_MUTE_WAITING_WITH_SLAVE || pcm9211_status == PCM9211_MUTE_WAITING) && Get_Is_Mute())
		Set_Is_Mute(FALSE);

	pcm9211_status = status;
}

void PCM9211_PowerUp(void)
{
	pcm9211_status = PCM9211_POWER_UP;
	HAL_GPIO_SetPin(PE, _BIT(2)); //reset

	if(already_initialized)
		;//MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, 0x00);

	pcm9211_timer = df10msTimer20ms;
}

void PCM9211_PowerDown(void)
{
	pcm9211_status = PCM9211_POWER_DOWN;
}

void PCM9211_Init(void)
{
#ifdef I2C_1_ENABLE
	uint8_t uData = 0;

#ifdef PCM9211_DEBUG_MSG
	_DBG("\n\rPCM9211_Init start");
#endif

#if 0
	/* Reset On */
	uData = PCM9211_RESET_CTL_REG_ON_VAL;
	I2C1_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_RESET_CTL_REG,&uData,1);
	/* Reset Off : 0xC0 -> 0xC2 */
	uData = PCM9211_RESET_CTL_REG_OFF_VAL;
	I2C1_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_RESET_CTL_REG,&uData,1);
#endif
	
	/* xti clock source : 12.288M 3.072Mhz, 48KHz (0x1A) */
	uData = PCM9211_XTI_SOURCE_CLOCK_12D288_48K;
	I2C1_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_XTI_SOURCE_CLOCK,&uData,1);

    PCM9211_Set_Output(PCM9211_OUTPORT_PORT_CTL_REG_AUXIN1); //dummy
#endif
	pcm9211_status = PCM9211_POWER_UP_COMPLTE;
	pcm9211_path_status = PCM9211_PATH_NULL;

#ifdef PCM9211_DEBUG_MSG
	_DBG("\n\rPCM9211_Init end");
#endif
}

void PCM9211_Set_Output(uint8_t Port)
{
#ifdef I2C_1_ENABLE
	uint8_t output;

	output = Port;
	I2C1_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_OUTPORT_PORT_CTL_REG,&output,1);
#endif
}

void PCM9211_Set_Path_Init(Bool mute_needed)
{
	if(HAL_GPIO_ReadPin(PE) & (1<<0)) //BT_OUT ON
	{
#if 1
		HAL_GPIO_ClearPin(PE, _BIT(6)); //BT_OUT1
		HAL_GPIO_ClearPin(PE, _BIT(5)); //BT_OUT2
		HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
		HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4
#else
		if(!(HAL_GPIO_ReadPin(PF) & (1<<1))) //area1
		{
			HAL_GPIO_ClearPin(PE, _BIT(6)); //BT_OUT1
			HAL_GPIO_SetPin(PE, _BIT(5)); //BT_OUT2
			HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
			HAL_GPIO_ClearPin(PE, _BIT(3)); //BT_OUT4

#ifdef PCM9211_DEBUG_MSG
		_DBG("\n\rPCM9211_Set_Path_Init - AREA1");
#endif
		}
		
		if(!(HAL_GPIO_ReadPin(PF) & (1<<2))) //area2
		{
			HAL_GPIO_SetPin(PE, _BIT(6)); //BT_OUT1
			HAL_GPIO_ClearPin(PE, _BIT(5)); //BT_OUT2
			HAL_GPIO_ClearPin(PE, _BIT(4)); //BT_OUT3
			HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4

#ifdef PCM9211_DEBUG_MSG
			_DBG("\n\rPCM9211_Set_Path_Init - AREA2");
#endif
		}
		
		if(!(HAL_GPIO_ReadPin(PF) & (1<<3))) //area1 + area2
		{
			HAL_GPIO_ClearPin(PE, _BIT(6)); //BT_OUT1
			HAL_GPIO_ClearPin(PE, _BIT(5)); //BT_OUT2
			HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
			HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4

#ifdef PCM9211_DEBUG_MSG
			_DBG("\n\rPCM9211_Set_Path_Init - AREA1 + AREA2");
#endif
		}
#endif
		
		PCM9211_Set_Path_BT(mute_needed);
	}
	else //BT_OUT OFF
	{
#ifdef PCM9211_DEBUG_MSG
		_DBG("\n\rPCM9211_Set_Path_Init - BT_OUT OFF");
#endif

		HAL_GPIO_SetPin(PE, _BIT(6)); //BT_OUT1
		HAL_GPIO_SetPin(PE, _BIT(5)); //BT_OUT2
		HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
		HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4
		
		if(!(HAL_GPIO_ReadPin(PC) & (1<<3)))
			PCM9211_Set_Path_BT(mute_needed);
		else
			PCM9211_Set_Path_ADC(mute_needed);
  }
}

void PCM9211_Set_Path_BT(Bool mute_needed)
{
	//uint32_t uCurVolLevel = 0;

#ifdef PCM9211_DEBUG_MSG
	_DBG("\n\rPCM9211_Set_Path_BT start");
#endif

	if(pcm9211_path_status == PCM9211_PATH_BT)
	{
		pcm9211_status = PCM9211_RUN;

#ifdef PCM9211_DEBUG_MSG
		_DBG("\n\rPCM9211_Set_Path_BT return");
#endif
		return;
	}

	if(mute_needed && !Get_Is_Mute())
	{
		AD85050_Amp_Mute(TRUE, FALSE);
		//MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Mute, 0x01);
		MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);

		pcm9211_status = PCM9211_MUTE_WAITING_WITH_SLAVE;
		pcm9211_timer  = df10msTimer600ms;
	}
	else
		pcm9211_status = PCM9211_RUN;

	delay_ms(10);

	HAL_GPIO_ClearPin(PE, _BIT(6)); //BT_OUT1
	HAL_GPIO_ClearPin(PE, _BIT(5)); //BT_OUT2
	HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
	HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4

	PCM9211_Set_Output(PCM9211_OUTPORT_PORT_CTL_REG_AUXIN0);
/*
	uCurVolLevel = AD85050_Amp_Get_Cur_Volume_Level();
    uCurVolLevel = uCurVolLevel >> 8;
	AD85050_Amp_Volume_Register_Writing((uint16_t)uCurVolLevel);
*/
	pcm9211_path_status = PCM9211_PATH_BT;

#ifdef PCM9211_DEBUG_MSG
	_DBG("\n\rPCM9211_Set_Path_BT end");
#endif
}

void PCM9211_Set_Path_ADC(Bool mute_needed)
{
	uint8_t uData = 0;
	//uint32_t uCurVolLevel = 0;

#ifdef PCM9211_DEBUG_MSG
	_DBG("\n\rPCM9211_Set_Path_ADC start");
#endif

	if(!(HAL_GPIO_ReadPin(PC) & (1<<3)))
	{
		pcm9211_status = PCM9211_RUN;
#ifdef PCM9211_DEBUG_MSG
		_DBG("\n\rPCM9211_Set_Path_ADC return");
#endif
		return;
	}

	if(pcm9211_path_status == PCM9211_PATH_ADC)
	{
		pcm9211_status = PCM9211_RUN;
		return;
	}

	if(mute_needed && !Get_Is_Mute())
	{
		AD85050_Amp_Mute(TRUE, FALSE);

		pcm9211_status = PCM9211_MUTE_WAITING;
		pcm9211_timer  = df10msTimer400ms;
	}
	else
		pcm9211_status = PCM9211_RUN;

	delay_ms(10);

	HAL_GPIO_SetPin(PE, _BIT(6)); //BT_OUT1
	HAL_GPIO_SetPin(PE, _BIT(5)); //BT_OUT2
	HAL_GPIO_SetPin(PE, _BIT(4)); //BT_OUT3
	HAL_GPIO_SetPin(PE, _BIT(3)); //BT_OUT4	

	uData = PCM9211_ADC_CH_CTL_REG_GAIN_5D5DB;
	I2C1_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_ADC_L_CH_CTL_REG,&uData,1);
	
	uData = PCM9211_ADC_CH_CTL_REG_GAIN_5D5DB;
	I2C1_Interrupt_Write_Data(PCM9211_DEVICE_ADDR, PCM9211_ADC_R_CH_CTL_REG,&uData,1);

	PCM9211_Set_Output(PCM9211_OUTPORT_PORT_CTL_REG_ADC);
/*
	uCurVolLevel = AD85050_Amp_Get_Cur_Volume_Level();
    uCurVolLevel = uCurVolLevel >> 8;
	AD85050_Amp_Volume_Register_Writing((uint16_t)uCurVolLevel);
*/
	pcm9211_path_status = PCM9211_PATH_ADC;

#ifdef PCM9211_DEBUG_MSG
	_DBG("\n\rPCM9211_Set_Path_ADC end");
#endif
}
#endif
