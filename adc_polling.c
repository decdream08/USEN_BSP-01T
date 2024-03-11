/**********************************************************************
* @file		adc_polling.c
* @brief	ADC
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"

//#include "A31G21x_hal_adc.h"

#include "adc.h"
#include "adc_polling.h"
#include "AD85050.h"

uint32_t adc_polling_timer;
uint8_t adc_polling_step;

static uint32_t ADC3_Value = 0xffffffff, ADC3_Value_bk = 0xffffffff;
static uint32_t ADC4_Value = 0xffffffff, ADC4_Value_bk = 0xffffffff;
static uint32_t ADC2_Value = 0xffffffff, ADC2_Value_bk = 0xffffffff;

static uint8_t uCurVolLevel_ADC3_bk = 0xff;
static uint8_t uCurVolLevel_ADC4_bk = 0xff;
static uint8_t uCurVolLevel_ADC2_bk = 0xff;

void ADC_Polling_10ms_timer(void)
{
	if (adc_polling_timer != df10msTimer0ms)
		--adc_polling_timer;
}

void ADC_Polling_Process(void)
{
	uint8_t ADC_Level_Min, ADC_Level_Max;
	uint8_t uCurVolLevel = 0;
	Bool B_Update;
	int i;

	if(AD85050_GetStatus() < AD85050_POWER_UP_INIT)
		return;

	switch (adc_polling_step) {
		case 0:
		{
			ADC3_Value = ADC_PollingRun(3);

			if(ADC3_Value_bk != ADC3_Value)
			{
				for(i=1;i<52;i++)
				{
					B_Update = FALSE; //2023-02-06_3    

					if(i==1)
						ADC_Level_Min = 0;
					else
						ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246 // On case of 51 step, 251

					if(i==51)
						ADC_Level_Max = 255; //On case of 51 step, 255
					else
						ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253 // On case of 51 step, 250

					if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value))
					{
						if(i==1)
							ADC_Level_Min = 0;
						else
							ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

						if(i==51)
							ADC_Level_Max = 255;
						else
							ADC_Level_Max = (i*5)-1; //4 9 14 19 ... 244 249~253

						if((ADC3_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC3_Value)) //2023-02-08_3 : Added additional code for Volume GAP
						{
							uCurVolLevel = 51 - i;
							B_Update = TRUE; //2023-02-06_3 
						}
						else //2023-02-06_3 : Do not update cur volume level and current ADC value is not valid
						{
							if(uCurVolLevel > uCurVolLevel_ADC3_bk)
							{
								if((uCurVolLevel - uCurVolLevel_ADC3_bk) > 1)
									B_Update = TRUE;
								else
									B_Update = FALSE;
							}
							else
							{
								if((uCurVolLevel_ADC3_bk - uCurVolLevel) > 1)
									B_Update = TRUE;
								else
									B_Update = FALSE;
							}
						}

						break;
					}
				}

				if(B_Update) //2023-02-06_3
				{
					uint32_t l_CurVolLevel = 0;

					uCurVolLevel = 51 - i;

					if(uCurVolLevel_ADC3_bk != uCurVolLevel)
					{
						uCurVolLevel_ADC3_bk = uCurVolLevel;

						l_CurVolLevel = INVALID_VOLUME;
						l_CurVolLevel <<= 8;

						l_CurVolLevel |= uCurVolLevel;
						l_CurVolLevel <<= 8;

						l_CurVolLevel |= INVALID_VOLUME;

						AD85050_Amp_Volume_Set_with_Index(l_CurVolLevel, FALSE, TRUE);
					}

					B_Update = FALSE;
				}
				ADC3_Value_bk = ADC3_Value;
			}
		}
		++adc_polling_step;
		break;

		case 1:
		{
			ADC4_Value = ADC_PollingRun(4);

			if(ADC4_Value_bk != ADC4_Value)
			{
				for(i=1;i<52;i++)
				{
					B_Update = FALSE; //2023-02-06_3    

					if(i==1)
						ADC_Level_Min = 0;
					else
						ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246 // On case of 51 step, 251

					if(i==51)
						ADC_Level_Max = 255; //On case of 51 step, 255
					else
						ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253 // On case of 51 step, 250

					if((ADC4_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC4_Value))
					{
						if(i==1)
							ADC_Level_Min = 0;
						else
							ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

						if(i==51)
							ADC_Level_Max = 255;
						else
							ADC_Level_Max = (i*5)-1; //4 9 14 19 ... 244 249~253

						if((ADC4_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC4_Value)) //2023-02-08_3 : Added additional code for Volume GAP
						{
							uCurVolLevel = 51 - i;
							B_Update = TRUE; //2023-02-06_3 
						}
						else //2023-02-06_3 : Do not update cur volume level and current ADC value is not valid
						{
							if(uCurVolLevel > uCurVolLevel_ADC4_bk)
							{
								if((uCurVolLevel - uCurVolLevel_ADC4_bk) > 1)
									B_Update = TRUE;
								else
									B_Update = FALSE;
							}
							else
							{
								if((uCurVolLevel_ADC4_bk - uCurVolLevel) > 1)
									B_Update = TRUE;
								else
									B_Update = FALSE;
							}
						}

						break;
					}
				}

				if(B_Update) //2023-02-06_3
				{
					uint32_t l_CurVolLevel = 0;

					uCurVolLevel = 51 - i;

					if(uCurVolLevel_ADC4_bk != uCurVolLevel)
					{
						uCurVolLevel_ADC4_bk = uCurVolLevel;

						l_CurVolLevel = INVALID_VOLUME;
						l_CurVolLevel <<= 8;

						l_CurVolLevel |= uCurVolLevel;
						l_CurVolLevel <<= 8;

						l_CurVolLevel |= INVALID_VOLUME;

						AD85050_Amp_Volume_Set_with_Index(l_CurVolLevel, FALSE, TRUE);
					}

					B_Update = FALSE;
				}
				ADC4_Value_bk = ADC4_Value;
			}
		}

		++adc_polling_step;
		break;

		case 2:
		{
			ADC2_Value = ADC_PollingRun(2);

			if(ADC2_Value_bk != ADC2_Value)
			{
				for(i=1;i<52;i++)
				{
					B_Update = FALSE; //2023-02-06_3    

					if(i==1)
						ADC_Level_Min = 0;
					else
						ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246 // On case of 51 step, 251

					if(i==51)
						ADC_Level_Max = 255; //On case of 51 step, 255
					else
						ADC_Level_Max = (i*5); //5 10 15 20 ... 245 250~253 // On case of 51 step, 250

					if((ADC2_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC2_Value))
					{
						if(i==1)
							ADC_Level_Min = 0;
						else
							ADC_Level_Min = (i-1)*5+1; //0 6 11 16 ... 241 246

						if(i==51)
							ADC_Level_Max = 255;
						else
							ADC_Level_Max = (i*5)-1; //4 9 14 19 ... 244 249~253

						if((ADC2_Value >= ADC_Level_Min) && (ADC_Level_Max >= ADC2_Value)) //2023-02-08_3 : Added additional code for Volume GAP
						{
							uCurVolLevel = 51 - i;
							B_Update = TRUE; //2023-02-06_3 
						}
						else //2023-02-06_3 : Do not update cur volume level and current ADC value is not valid
						{
							if(uCurVolLevel > uCurVolLevel_ADC2_bk)
							{
								if((uCurVolLevel - uCurVolLevel_ADC2_bk) > 1)
									B_Update = TRUE;
								else
									B_Update = FALSE;
							}
							else
							{
								if((uCurVolLevel_ADC2_bk - uCurVolLevel) > 1)
									B_Update = TRUE;
								else
									B_Update = FALSE;
							}
						}

						break;
					}
				}

				if(B_Update) //2023-02-06_3
				{
					uint32_t l_CurVolLevel = 0;

					uCurVolLevel = 51 - i;

					if(uCurVolLevel_ADC2_bk != uCurVolLevel)
					{
						uCurVolLevel_ADC2_bk = uCurVolLevel;

						l_CurVolLevel = INVALID_VOLUME;
						l_CurVolLevel <<= 8;

						l_CurVolLevel |= uCurVolLevel;
						l_CurVolLevel <<= 8;

						l_CurVolLevel |= INVALID_VOLUME;

						AD85050_Amp_Volume_Set_with_Index(l_CurVolLevel, FALSE, TRUE);
					}

					B_Update = FALSE;
				}
				ADC2_Value_bk = ADC2_Value;
			}
		}

		adc_polling_step = 0;;

		break;	

		default:
		break;
	}
}

