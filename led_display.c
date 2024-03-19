/**********************************************************************
* @file		led_display.c
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

#if defined(TIMER30_LED_PWM_ENABLE) || defined(TIMER1n_LED_PWM_ENABLE) || defined(GPIO_LED_ENABLE) || defined(TIMER21_LED_ENABLE)
#include "A31G21x_hal_pcu.h"
#include "timer21.h"
#include "AD85050.h" 
#include "timer20.h"
#include "led_display.h"
#include "remocon_action.h"
#include "key.h"
#include "power.h"
#include "protection.h"

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/

#define Make_LED_ON(x, y) 				(HAL_GPIO_ClearPin(x, _BIT(y)))
#define Make_LED_OFF(x, y) 				(HAL_GPIO_SetPin(x, _BIT(y)))

//STATUS LED
//L1 LED  //Need to change name to STATUS_LED_W(MUTE_LED_WHITE) //2022-01-09_4
#define STATUS_LED_W_ON						(Make_LED_ON(PC,1)) 
#define STATUS_LED_W_OFF					(Make_LED_OFF(PC,1))

//SLAVE BT IND.
#define BT_PAIRING_LED_BLUE_ON				(Make_LED_ON(PD,2))
#define BT_PAIRING_LED_BLUE_OFF				(Make_LED_OFF(PD,2))

//Master BT IND.
#define BT_PAIRING_LED_WHITE_ON				(Make_LED_ON(PD,3))
#define BT_PAIRING_LED_WHITE_OFF			(Make_LED_OFF(PD,3))

//#define LED_DISPLAY_DEBUG					(1)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
/* Private define ----------------------------------------------------*/

#ifdef TIMER21_LED_ENABLE //PC1 / PC2 
Status_LED_Mode cur_status_led_mode = STATUS_POWER_OFF_MODE;
Status_LED_Mode return_status_led_mode = STATUS_POWER_OFF_MODE; //When user select un-mute, BT SPK should return latest status led mode. this variable must update status led mode excepting MUTE mode.
Status_LED_Mode return_status_led_mode2 = STATUS_POWER_OFF_MODE; //When current state is Amp Error mode, Just display fast blinking. But we need to return latest status when Amp Error mode is ended.
Status_LED_Mode return_background_status_led_mode = STATUS_POWER_OFF_MODE; //When cur mode is STATUS_BT_MASTER_SLAVE_PAIRING_MODE, we need to return original value. this variable must update status led mode excepting STATUS_BT_MASTER_SLAVE_PAIRING_MODE mode.

void Set_Status_LED_Mode(Status_LED_Mode mode)
{
	if(mode == STATUS_MUTE_ON_MODE)
		return;

#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++Set_Status_LED_Mode() : mode = ");
	_DBD(mode);
	_DBG("\n\rreturn_status_led_mode = ");
	_DBD(return_status_led_mode);
	_DBG("\n\rcur_status_led_mode = ");
	_DBD(cur_status_led_mode);
#ifdef AMP_ERROR_ALARM
	_DBG("\n\rcur_status_led_mode2 = ");
	_DBD(return_status_led_mode2);
#endif
#endif
	//After Mute On --> Power Off--> On --> Mute Off, Front LED is not updated and just turned off.
	if(mode != STATUS_MUTE_ON_MODE && mode != STATUS_AUX_MODE && mode != STATUS_POWER_OFF_MODE
		&& mode != STATUS_BT_MASTER_SLAVE_PAIRING_MODE
		&& mode != STATUS_PROTECTION_MODE
		) //When user select un-mute, BT SPK should return latest status led mode. this variable must update status led mode excepting MUTE mode.		
		return_status_led_mode = mode;

	if(Get_Amp_error_flag())
	{
		return_status_led_mode2 = mode;
		
		return;
	}
	
	if(mode == STATUS_AMP_ERROR_MODE)
		return_status_led_mode2 = cur_status_led_mode;	

	if(mode != STATUS_BT_MASTER_SLAVE_PAIRING_MODE)
		return_background_status_led_mode = mode;

	if(Get_master_slave_grouping_flag())
	{
		LED_Status_Display_WR_Color(mode);
	 	mode = STATUS_BT_MASTER_SLAVE_PAIRING_MODE;
		cur_status_led_mode = mode;
		
		return;
	}

	if(mode == STATUS_MUTE_ON_MODE)
	{
		LED_Status_Display_WR_Color(mode);
		LED_Status_Display_WR_Color(return_status_led_mode);

		return;
	}

	cur_status_led_mode = mode;
	LED_Status_Display_WR_Color(mode);
}

Status_LED_Mode Get_Return_Background_Status_LED_Mode(void)
{
	return return_background_status_led_mode;
}

Status_LED_Mode Get_Return_Status_LED_Mode(void) //When user select un-mute, BT SPK should return latest status led mode. this variable must update status led mode excepting MUTE mode.
{
	return return_status_led_mode;
}

Status_LED_Mode Get_Return_Status_LED_Mode2(void)
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\rGet_Return_Status_LED_Mode2()");
	_DBH(return_status_led_mode2);
#endif
	return return_status_led_mode2;
}

Status_LED_Mode Get_Cur_Status_LED_Mode(void)
{
	return cur_status_led_mode;
}

/**********************************************************************
 * @brief		STATUS LED Display with White LED & Red LED
 * @param[in]	
	STATUS_POWER_ON_MODE, //Mute : White On
	STATUS_POWER_OFF_MODE, //Mute : White Off/Red Off, Status : White Off/Red Off
	STATUS_BT_PAIRED_MODE, //Status : White On
	STATUS_AUX_MODE, //Status : White On
	STATUS_BT_PAIRING_MODE, //Status : White Fast Blinking
	STATUS_BT_FAIL_OR_DISCONNECTION_MODE, //Status : White Slow Blinking
	STATUS_MUTE_ON_MODE, //Mute : Red On/White Off, Status : Red On
	STATUS_SOC_ERROR_MODE //Status : Red Blinking
 * @return 	None
 **********************************************************************/
void LED_Status_Display_WR_Color(Status_LED_Mode mode) //L1/L3 LED
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Status_Display_WR_Color() : mode = ");
	_DBD(mode);	
	_DBG("\n\r Mute =");_DBH(Mute_On);
#endif

	if(mode == STATUS_PROTECTION_MODE)
	{
		TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On

		BT_PAIRING_LED_BLUE_OFF;
		BT_PAIRING_LED_WHITE_OFF;

		//Front Status LED //White Fast Blinking
		STATUS_LED_W_ON;

		return;
	}

	if(!Power_State()) //Do not LED turn on when Power off
		return;

	if(Get_factory_reset_led_display_flag() == TRUE) //Do not change LED statun when FACTORY RESET LED DISPLAY On.
		return;

	if(Aux_In_Exist())
	{
		TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
	}

	switch(mode)
	{
		case STATUS_POWER_ON_MODE:
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
			STATUS_LED_W_ON;
		}
			break;
			
		case STATUS_POWER_OFF_MODE: // White Off/Red Off
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
			
			//Top L3 LED
			BT_PAIRING_LED_BLUE_OFF;
			BT_PAIRING_LED_WHITE_OFF;

			STATUS_LED_W_OFF;

		}
			break;
			
		case STATUS_BT_PAIRED_MODE: // White On
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
		
			STATUS_LED_W_ON;

			//Top L3 LED
			if(Get_master_slave_grouping_flag())
				break;

			if(!Aux_In_Exist())
			{
				BT_PAIRING_LED_BLUE_OFF;
				BT_PAIRING_LED_WHITE_ON;
			}
		}
			break;
			
		case STATUS_AUX_MODE: // White On //BT LED(BLUE/WHITE) OFF
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off

			STATUS_LED_W_ON;

			//Top L3 LED
			BT_PAIRING_LED_BLUE_OFF;
			BT_PAIRING_LED_WHITE_OFF;
		}
			break;
			
		case STATUS_BT_PAIRING_MODE: // White Fast Blinking
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
			
			STATUS_LED_W_ON;

			if(Get_master_slave_grouping_flag())
				break;

			if(!Aux_In_Exist())
			{
				BT_PAIRING_LED_BLUE_OFF;
				BT_PAIRING_LED_WHITE_ON;
			}
		}
			break;

		case STATUS_AMP_ERROR_MODE: // White/Blue Very Fast Blinking 	
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
					
			//Front Status LED //White Fast Blinking
			STATUS_LED_W_ON;
		}
			break;

		case STATUS_BT_GIA_PAIRING_MODE: // White Fast Blinking		
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
			
			if(Get_master_slave_grouping_flag())
				break;

			STATUS_LED_W_ON;

			if(!Aux_In_Exist())
			{
				BT_PAIRING_LED_BLUE_OFF;
				BT_PAIRING_LED_WHITE_ON;
			}
		}
		break;

		case STATUS_BT_MASTER_SLAVE_PAIRING_MODE: // White Fast Blinking		
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On

			STATUS_LED_W_ON;

			if(!Aux_In_Exist()|| Get_master_slave_grouping_flag()) //2023-04-18_1 : Added Grouping LED display condition under Aux Mode to avoid display LED under other case(BT connection disconnect from Peer Device and White LED is blinking).
			{
				BT_PAIRING_LED_BLUE_OFF;
				BT_PAIRING_LED_WHITE_ON;
			}
		}
			break;

		case STATUS_BT_FAIL_OR_DISCONNECTION_MODE: // White Slow Blinking
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On

			STATUS_LED_W_ON;

			if(Get_master_slave_grouping_flag())
				break;

			if(!Aux_In_Exist())
			{
				BT_PAIRING_LED_BLUE_OFF;
				BT_PAIRING_LED_WHITE_ON;
			}
		}
			break;
			
		case STATUS_MUTE_ON_MODE: // Red On
		{
			STATUS_LED_W_ON;
		}
			break;
			
		case STATUS_SOC_ERROR_MODE: // Red Blinking
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On

			if(Get_master_slave_grouping_flag())
				break;

			STATUS_LED_W_ON;

		}
			break;
			
		default:
			break;
	}
}

void LED_Status_Display_Blinking(Status_LED_Color Color, Bool On)
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Status_Display_Blinking() : Color = ");
	_DBD(Color);
	_DBG("\n\rOn = ");
	_DBD(On);
#endif

	if(Color == STATUS_PROTECTION_LED_WHITE)
	{
		if(Get_Amp_error_flag())
		{
			if(On)
			{
				STATUS_LED_W_ON;
			}
			else
			{
				STATUS_LED_W_OFF;
			}
		}

		return;
	}

	if(!Power_State()) //Do not LED turn on when Power off
		return;

	if(Get_factory_reset_led_display_flag() == TRUE) //Do not change LED statun when FACTORY RESET LED DISPLAY On.
		return;

	switch(Color)
	{
		case STATUS_LED_WHITE:
			if(Get_Amp_error_flag())
			{
				if(On)
				{
					STATUS_LED_W_ON;
				}
				else
				{
					STATUS_LED_W_OFF;
				}
			}
			break;

		case L3_LED_BLUE:
			if(On)
			{
				BT_PAIRING_LED_BLUE_ON;
			}
			else
			{
				BT_PAIRING_LED_BLUE_OFF;
			}
			break;

		case L3_LED_WHITE:
			if(Aux_In_Exist() && !Get_master_slave_grouping_flag()) //2023-04-18_1 : Added Grouping LED display condition under Aux Mode to avoid display LED under other case(BT connection disconnect from Peer Device and White LED is blinking).
				break;

			if(On)
			{
				BT_PAIRING_LED_WHITE_ON;;
			}
			else
			{
				BT_PAIRING_LED_WHITE_OFF;
			}
			break;
	}		
}
#endif

void LED_Diplay_All_Off(void)
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Diplay_All_Off() ");
#endif

	STATUS_LED_W_OFF;

	//L3 LED
	BT_PAIRING_LED_BLUE_OFF;
	BT_PAIRING_LED_WHITE_OFF;
}

void LED_Display_All_On(void)
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Display_All_On() ");
#endif

	STATUS_LED_W_ON,

	//L3 LED
	BT_PAIRING_LED_BLUE_ON;
	BT_PAIRING_LED_WHITE_ON;
}
#endif //#if defined(TIMER30_LED_PWM_ENABLE) || defined(TIMER1n_LED_PWM_ENABLE) || defined(GPIO_LED_ENABLE) || defined(TIMER21_LED_ENABLE)

#ifdef _DEBUG_MSG
void Debug_Test_Blue_LED_On(Bool On)
{
	if(On)
		BT_PAIRING_LED_BLUE_ON;
	else
		BT_PAIRING_LED_BLUE_OFF;
}
#endif

