/**********************************************************************
* @file		main.c
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library
* @version	1.00
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
**********************************************************************/
#include "main_conf.h"

#ifdef ADC_INTERRUPT_INPUT_ENABLE
#include "adc_interrupt.h"
#include "A31G21x_hal_adc.h"
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h"
#endif
#ifdef FLASH_SELF_WRITE_ERASE
#include "flash.h"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void ADC_IRQHandler_IT(void);
uint16_t ADC_INT_value_average(uint16_t *val);
uint32_t ADC_InterruptRun(uint8_t channel);
void ADC_INT_Error_Handler(void);
void ADC_Interrupt_Volume_Setting(void);

/* Private variables ---------------------------------------------------------*/
//volatile uint32_t fflag;
uint16_t adc_val[8];
ADC_CFG_Type AD_INT_config;


/**********************************************************************
 * @brief		ADC_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void ADC_IRQHandler_IT(void)
{
	uint16_t status;
	status = HAL_ADC_GetStatus(ADC);

	if (status & ADC_STAT_END)
	{
		//fflag=1;
		HAL_ADC_ClearStatus(ADC);	
		ADC_Interrupt_Volume_Setting();
	}
}

/**********************************************************************
 * @brief		ADC_INT_value_average
 * @param[in]	None
 * @return		None
 **********************************************************************/
uint16_t ADC_INT_value_average(uint16_t *val)
{
	uint32_t i;
	uint32_t adcavg;
	
	adcavg=0;			
	for (i=0;i<8;i++){	
		adcavg+=val[i];
	}
	adcavg>>=3;
#ifndef ADC_VOLUME_STEP_ENABLE
#ifdef ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG
	_DBH32(adcavg);
#endif
#endif

#ifdef ADC_VOLUME_STEP_ENABLE
	adcavg &= 0xFF0;
	adcavg>>=4;
#ifdef ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG
	_DBH32(adcavg);
#endif
#else
	adcavg &= 0xFF0;
	adcavg /= 256;
#endif
	return	(adcavg);
}

/**********************************************************************
 * @brief		ADC_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void ADC_Intterupt_Configure(void)
{
  	AD_INT_config.RefSel = ADC_REF_VDD;   //0:ADC_REF_VDD ,1:ADC_REF_AVREF
	AD_INT_config.TrgSel = ADC_TRIGGER_TIMER10;	
	AD_INT_config.InClkDiv = 14;
	
	if(HAL_ADC_Init(ADC, &AD_INT_config) != HAL_OK)
	{
		/* Initialization Error */
    	ADC_INT_Error_Handler();
	}
	
	HAL_ADC_ConfigInterrupt(ADC, ENABLE);
  
	/* Enable Interrupt for ADC channel */
	NVIC_SetPriority(ADC_IRQn, 3);
	NVIC_EnableIRQ(ADC_IRQn);
}

/**********************************************************************
 * @brief		ADC_InterruptRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
uint32_t ADC_InterruptRun(uint8_t channel)
{	
	uint32_t adcavg;
	uint32_t i;
	
	HAL_ADC_ChannelSel(ADC, channel); // select ch

	for (i=0;i<(8);i++)
	{	
		//fflag=0;
		HAL_ADC_ClearStatus(ADC);
		HAL_ADC_Start(ADC); // start 	
		//while(fflag==0){}			
		adc_val[i]=HAL_ADC_GetData(ADC);
	}
  
	adcavg=ADC_INT_value_average(adc_val);
	
#ifdef ADC_INPUT_DEBUG_MSG
	_DBG("AN0="); 

	_DBH16(adcavg); _DBG(" ");
	_DBG("\n\r");
#endif

	return adcavg;
}

/**********************************************************************
  * @brief  Reports the name of the source file and the source line number
  *   where the check_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: check_param error line source number
  * @retval : None
 **********************************************************************/
void ADC_INT_Error_Handler(void)
{
     /*User code*/
    while (1)
    {
    }
}

void ADC_Interrupt_Volume_Setting(void)
{
#ifdef ADC_INTERRUPT_INPUT_ENABLE
	uint8_t uCurVolLevel = 0;
	static uint32_t ADC3_Value = 0xffffffff, ADC3_Value_bk = 0xffffffff;
	Bool B_Update = TRUE; //2023-02-06_3 : To make ADC Gap
#endif
#ifdef ADC_VOLUME_STEP_ENABLE
	int i;
	uint8_t ADC_Level_Min, ADC_Level_Max;
	static uint8_t uCurVolLevel_bk = 0xff;
#endif

	ADC3_Value = ADC_InterruptRun(3); // ADC pin PA3 : VOL_CONT(Master Volume)

	if(ADC3_Value_bk != ADC3_Value)
	{
#ifdef ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG
		_DBG("\n\r === Master Volume = 0x");
		_DBH32(ADC3_Value);
#endif
#ifdef ADC_VOLUME_STEP_ENABLE
		for(i=1;i<65;i++)
		{
			ADC_Level_Min = (i-1)*4; //0 4 8
			ADC_Level_Max = (i*4)-2; //2 6 10 //ADC_Level_Max = (i*4)-1; //3 7 11 //2023-02-06_3 : To make ADC Gap
			
			if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value))
			{
#ifdef ADC_INPUT_DEBUG_MSG
				_DBG("\n\r === ADC Level = ");
				_DBD(i);
#endif

				//2023-02-06_3 : If cur volume level is not different with previous one, we need to update it
				uCurVolLevel = 64 - i;
#ifdef ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG
				_DBH(uCurVolLevel);
				_DBG(" - ");
				_DBH(uCurVolLevel_bk);
#endif									
				if(!(uCurVolLevel > uCurVolLevel_bk)) //2023-02-06_3 : Do not update cur volume level and current ADC value is not valid
				{
#ifdef ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG
					_DBG("\n\r === Wrong ADC Value & No Need Update !!!");
#endif
					B_Update = FALSE;
				}

				break;
			}
			else
				B_Update = FALSE;
		}

		if(B_Update)
		{
			uCurVolLevel = 64 - i; //0 ~ 63(64 Step / 0 - MAX)
#ifdef ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG
			_DBG("\n\r === Volume Level = ");
			_DBD(uCurVolLevel);
#endif								
#ifdef TAS5806MD_ENABLE
			if(uCurVolLevel_bk != uCurVolLevel)
			{
#ifdef ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG
				_DBG("\n\r ++++ Update Volume");
#endif
				uCurVolLevel_bk = uCurVolLevel;
				TAS5806MD_Amp_Volume_Set_with_Index(uCurVolLevel, FALSE, TRUE);
				//uCurVolLevel = 15 - ADC3_Value; //15 Step, Inverse Value, The integer value need to match with (VOLUME_LEVEL_NUMER)
#ifdef FLASH_SELF_WRITE_ERASE
				FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel);
#endif
			}
		}
#endif
#else //ADC_VOLUME_STEP_ENABLE
#ifdef TAS5806MD_ENABLE
		TAS5806MD_Amp_Volume_Set_with_Index(ADC3_Value, TRUE, TRUE);
		
		uCurVolLevel = 15 - ADC3_Value; //15 Step, Inverse Value, The integer value need to match with (VOLUME_LEVEL_NUMER)
#ifdef FLASH_SELF_WRITE_ERASE
		FlashSaveData(FLASH_SAVE_DATA_VOLUME, uCurVolLevel);
#endif
#endif
#endif //ADC_VOLUME_STEP_ENABLE
		ADC3_Value_bk = ADC3_Value;
	}
}

#endif //ADC_INTERRUPT_INPUT_ENABLE

