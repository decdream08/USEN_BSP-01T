/**********************************************************************
* @file		ir.c
* @brief	IR code
* @version	1.0
* @date		
* @author	MS Kim
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/
#include "main_conf.h"
#ifdef REMOCON_TIMER20_CAPTURE_ENABLE
#include "ir.h"
#include "remocon_action.h"

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/
/* Private macro -----------------------------------------------------*/
#define time_9000_us	(9000UL)
#define time_4500_us	(4500UL)
#define time_2250_us	(2250UL)
#define time_1125_us	(1125UL)

#define REPEAT_DELAY	5
#define IR_MARGIN		(130UL)
//#define IR_CUSTOM_CODE (0x20df) //LG Remocon
#define IR_CUSTOM_CODE (0x50B8)

// State of the remocon protocol
#define IDLE		0
#define LEADER_ON	1
#define LEADER_OFF	2
#define CUSTOM		3
#define DATA1		4
#define DATA2		5

/* Private variables -------------------------------------------------*/
/* Private define ----------------------------------------------------*/
/* Private function prototypes ---------------------------------------*/
void IR_Error_Handler(void);
Bool Check_Valid_IR_Key(uint8_t key);

/* Private variables ---------------------------------------------------*/
TIMER2n_CAPTURECFG_Type TIMER2n_Config;

void IR_Interrupt_Service_Routine( void );
 
/**********************************************************************
 * @brief		TIMER20_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER20_IRQHandler_IT(void)
{
	if (HAL_TIMER2n_GetCaptureInterrupt(TIMER20) ==  1)
	{
		IR_Interrupt_Service_Routine();

		HAL_TIMER2n_ClearCaptureInterrupt(TIMER20);
		HAL_TIMER2n_ClearCounter(TIMER20);
	}

}

/**********************************************************************
 * @brief		TIMER2n_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void TIMER2n_Configure(void)
{	
	/*Timer2n clock source from PCLK*/
	HAL_SCU_Timer20_ClockConfig(TIMER20CLK_PCLK);
	TIMER2n_Config.CkSel = TIMER2n_MCCR2PCLK;    
	TIMER2n_Config.Prescaler = 32;    /* 32Mhz / 32 = 1Mhz ->1us*/
	TIMER2n_Config.ADR = (0xFFFF);
	TIMER2n_Config.ClrMode=TIMER2n_FALLING_EGDE; //The normal state is High and the remocon signal starts from falling edge.
	TIMER2n_Config.CAPCkSel=TIMER2n_CAP_EXTERNAL_CLK;     
	
	if(HAL_TIMER2n_Init(TIMER20, TIMER2n_CAPTURE_MODE, &TIMER2n_Config) != HAL_OK)
	{
		/* Initialization Error */
    		IR_Error_Handler();
	}
	HAL_TIMER2n_ConfigInterrupt(TIMER20, TIMER2n_CR_CAPTURE_INTR, ENABLE);
	HAL_TIMER2n_ClearCaptureInterrupt(TIMER20);
	
	/* Enable Interrupt for TIMERx channel */
	NVIC_SetPriority(TIMER20_IRQn, 3);
	NVIC_EnableIRQ(TIMER20_IRQn); 
		
	HAL_TIMER2n_ClearCounter(TIMER20);
	/* timer start & clear*/
	HAL_TIMER2n_Cmd(TIMER20, ENABLE);
}

/**********************************************************************
  * @brief  Reports the name of the source file and the source line number
  *   where the check_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: check_param error line source number
  * @retval : None
 **********************************************************************/
void IR_Error_Handler(void)
{
    while (1)
    {
    }
}

void IR_Interrupt_Service_Routine( void )
{
	static uint8_t key;							// Hold the key code
	static uint8_t 	repeat_delay = REPEAT_DELAY;	// Repeat code counter
	static uint16_t custom;							// Hold the custom (remote ID) code
	static uint8_t 	state = 0;						// State holder
	static uint8_t 	count;							// bits counter
	static uint8_t 	data1, data2;					// Temporary for holding the decoded data
	static uint8_t 	valid_repeat = 0;
	uint32_t t0;

	t0 = HAL_TIMER2n_GetCaptureData(TIMER20);

	switch (state) 
	{
		case IDLE:
			HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_RISING_EGDE);
			state = LEADER_ON;
		break;
		
		case LEADER_ON:
			HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_FALLING_EGDE);
			state = ((t0>(time_9000_us-8*IR_MARGIN)) && (t0<(time_9000_us+8*IR_MARGIN))) ? LEADER_OFF:IDLE;
		break;
		
		case LEADER_OFF:
			if(t0 > time_4500_us - (4*IR_MARGIN) && t0 < time_4500_us + (4*IR_MARGIN)) 
			{
				state = CUSTOM;
				custom = 0;
				count = 0;
				repeat_delay = REPEAT_DELAY;
			} 
			else
			{
				if(t0 > time_2250_us - (2*IR_MARGIN) && t0 < time_2250_us + (2*IR_MARGIN))
				{
					if(repeat_delay)
					{
						// Delay before sendnig the first repeat
						repeat_delay--;
					}
					else //repeat last key
					{
						if(valid_repeat)
						{
						
						}
					}
				}
				HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_FALLING_EGDE);
				state = IDLE;
			}
		break;
		
		case CUSTOM:
			if ( (t0 > time_1125_us - 3*IR_MARGIN) && (t0 < time_1125_us + 4*IR_MARGIN)) 
			{
				custom <<= 1;	/* a zero bit */
			} 
			else
			{
				if ( (t0 > time_2250_us - 2*IR_MARGIN) && (t0 < time_2250_us + 3*IR_MARGIN)) 
				{
					custom = (custom << 1) | 1;	/* a one bit */
				} 
				else 
				{
					//ignored garbage data
					HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_FALLING_EGDE);
					state = IDLE;
					valid_repeat = 0;
					break;
				}
			}

			/* count 16 'custom' bits */
			if (++count == 16) 
			{
				if (custom != IR_CUSTOM_CODE) 
				{
					//ignored garbage data
					HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_FALLING_EGDE);
					state = IDLE;
					valid_repeat = 0;

					break;
				}
				
				state = DATA1;
				count = 0;
				custom = 0xFFFF;
			}
		break;

		case DATA1:
			count++;
			if ( (t0 > time_1125_us - 3*IR_MARGIN) && (t0 <time_1125_us + 3*IR_MARGIN)) 
			{
				data1 <<= 1; //a zero bit value
			} 
			else 
			{
				if ((t0 > time_2250_us - 3*IR_MARGIN) && (t0 < time_2250_us + 4*IR_MARGIN)) 
				{
					data1 = (data1 << 1) | 1; //a one bit value
				} 
				else 
				{
					HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_FALLING_EGDE);
					state = IDLE;
					valid_repeat = 0;
					break;
				}
			}
			
			if (count == 8) // DATA1 is OK
			{
				state = DATA2;
				count = 0;
				data2 = 0;
			}
		break;
		
		case DATA2:
			count++;
			if ( (t0 > time_1125_us - 3*IR_MARGIN) && (t0 < time_1125_us + 3*IR_MARGIN)) 
			{
				data2 <<= 1;	//a zero bit value
			} 
			else 
			{ 
				if ((t0 > time_2250_us - 3*IR_MARGIN) && (t0 < time_2250_us + 4*IR_MARGIN)) 
				{
					data2 = (data2 << 1) | 1; //a one bit value
				} 
				else 
				{
					HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_FALLING_EGDE);
					state = IDLE;
					valid_repeat = 0;

					break;
				}
			}
			
			if (count == 8) // DATA2 is OK
			{      	
				HAL_TIMER2n_SetPolarity(TIMER20, TIMER2n_FALLING_EGDE);
				state = IDLE;
				if (data1 == (~data2 & 0xff))  //Check if data is valid - DATA2 is ~(DATA1) for LG Remocon
				{
					key = (0xFF&data1);
					valid_repeat = 1;  //allow key repeat 
#ifdef _TIMER20_CAPTURE_DEBUG_MSG
					_DBG("+KEY : ");
					_DBH(key);
					_DBG("\n\r");
#endif
					Send_Remote_Key_Event(key);
				}
			}
		break;
	}

#ifdef _TIMER20_CAPTURE_DEBUG_MSG
	//_DBG("+t0:");_DBD32(t0);_DBG("\n\r");
#endif
}
#endif //REMOCON_TIMER20_CAPTURE_ENABLE

