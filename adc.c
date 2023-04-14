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

#ifdef ADC_INPUT_ENABLE
#include "A31G21x_hal_adc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define ADC_DEBUG_MSG			(1)

/* Private function prototypes -----------------------------------------------*/
uint16_t adcaverage(uint16_t *val);
void ADC_Configure(void);
void Error_Handler(void);

/* Private variables ---------------------------------------------------------*/
uint16_t adcval[8];
ADC_CFG_Type AD_config;

/**********************************************************************
 * @brief		adcaverage
 * @param[in]	None
 * @return		None
 **********************************************************************/
uint16_t adcaverage(uint16_t *val)
{
	uint32_t i;
	uint32_t adcavg;
	
	adcavg=0;			
	for (i=0;i<8;i++){	
		adcavg+=val[i];
	}
	adcavg>>=3;
#ifndef ADC_VOLUME_STEP_ENABLE
#ifdef ADC_INPUT_DEBUG_MSG
	_DBH32(adcavg);
#endif
#endif

#ifdef ADC_VOLUME_STEP_ENABLE
	adcavg &= 0xFF0;
	adcavg>>=4;
#ifdef ADC_INPUT_DEBUG_MSG
	_DBH32(adcavg);
#endif
#else
	adcavg &= 0xFF0;
	adcavg /= 256;
#endif
	return (adcavg);
}

/**********************************************************************
 * @brief		ADC_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void ADC_Configure(void)
{
  	AD_config.RefSel = ADC_REF_VDD;   //0:ADC_REF_VDD ,1:ADC_REF_AVREF
	AD_config.TrgSel = ADC_TRIGGER_DISABLE;	
	AD_config.InClkDiv = 14;
	
	if(HAL_ADC_Init(ADC, &AD_config) != HAL_OK)
	{
		/* Initialization Error */
    	Error_Handler();
	}
}
	

/**********************************************************************
 * @brief		ADC_PollingRun
 * @param[in]	None
 * @return 	None
 **********************************************************************/
uint32_t ADC_PollingRun(uint8_t channel)
{	
	uint32_t adcavg;
	uint32_t i;
	
	HAL_ADC_ChannelSel(ADC, channel);
  
	for (i=0;i<(8);i++)
	{			
		HAL_ADC_ClearStatus(ADC);
		HAL_ADC_Start(ADC); // start 	
		while((HAL_ADC_GetStatus(ADC) & (ADC_STAT_END)) !=(ADC_STAT_END)){}				
		adcval[i]=HAL_ADC_GetData(ADC);
  }
#ifdef ADC_INPUT_DEBUG_MSG
	if(channel == 3)
		_DBG("\r\nVolume ADC = 0x");
	else if(channel == 2)
		_DBG("\r\nAttenuator ADC = 0x");
	else
		_DBG("\r\nOther ADC = 0x");
#endif
	adcavg=adcaverage(adcval);
#ifdef ADC_DEBUG_MSG
	_DBG("AN = "); _DBH16(adcavg); _DBG(" ");
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
void Error_Handler(void)
{
     /*User code*/
    while (1)
    {
    }
}
#endif //ADC_INPUT_ENABLE


