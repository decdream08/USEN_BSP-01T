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
#ifdef TIMER30_LED_PWM_ENABLE
#include "timer30.h"
#include "A31G21x_hal_timer30.h"
#endif
#ifdef TIMER1n_LED_PWM_ENABLE
#include "timer1n.h"
#include "A31G21x_hal_timer1n.h"
#endif
#ifdef GPIO_LED_ENABLE
#include "A31G21x_hal_pcu.h"
#endif
#ifdef TIMER21_LED_ENABLE
#include "timer21.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h" 
#endif
#ifdef AD85050_ENABLE
#include "AD85050.h" 
#endif
#if defined(FACTORY_RESET_LED_DISPLAY) || defined(FACTORY_MODE)
#include "timer20.h"
#endif

#include "led_display.h"
#include "remocon_action.h"

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/

#ifdef LED_PORT_HIGH_DISPLAY
#define Make_LED_ON(x, y) 				(HAL_GPIO_SetPin(x, _BIT(y)))
#define Make_LED_OFF(x, y) 				(HAL_GPIO_ClearPin(x, _BIT(y)))
#else //LED_PORT_HIGH_DISPLAY
#define Make_LED_ON(x, y) 				(HAL_GPIO_ClearPin(x, _BIT(y)))
#define Make_LED_OFF(x, y) 				(HAL_GPIO_SetPin(x, _BIT(y)))
#endif //LED_PORT_HIGH_DISPLAY

#ifdef TIMER21_LED_ENABLE //STATUS LED(WHITE & RED) : PC1 / PC2, L1 LED(WHITE & RED) : PD4 / PD5, L3 LED(Blue & White) : PD2 / PD3 
#ifdef USEN_BAP //Need to change LED port setting for USEN_BAP //2022-10-07_2
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
#elif defined(USEN_BAP2)
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
#else //USEN_BAP

//STATUS LED
#define STATUS_LED_WHITE_ON					(Make_LED_ON(PC,1))
#define STATUS_LED_WHITE_OFF					(Make_LED_OFF(PC,1))

#define STATUS_LED_RED_ON						(Make_LED_ON(PC,2))
#define STATUS_LED_RED_OFF						(Make_LED_OFF(PC,2))

//L1 LED
#define MUTE_LED_WHITE_ON						(Make_LED_ON(PD,4))
#define MUTE_LED_WHITE_OFF					(Make_LED_OFF(PD,4))

#define MUTE_LED_RED_ON						(Make_LED_ON(PD,5))
#define MUTE_LED_RED_OFF						(Make_LED_OFF(PD,5))

//L3 LED
#define BT_PAIRING_LED_BLUE_ON				(Make_LED_ON(PD,2))
#define BT_PAIRING_LED_BLUE_OFF				(Make_LED_OFF(PD,2))

#define BT_PAIRING_LED_WHITE_ON				(Make_LED_ON(PD,3))
#define BT_PAIRING_LED_WHITE_OFF				(Make_LED_OFF(PD,3))
#endif
#endif //USEN_BAP

//#define LED_DISPLAY_DEBUG					(1)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
/* Private define ----------------------------------------------------*/

#ifdef TIMER21_LED_ENABLE //PC1 / PC2 
Status_LED_Mode cur_status_led_mode = STATUS_POWER_OFF_MODE;
Status_LED_Mode return_status_led_mode = STATUS_POWER_OFF_MODE; //When user select un-mute, BT SPK should return latest status led mode. this variable must update status led mode excepting MUTE mode.
#ifdef AMP_ERROR_ALARM
Status_LED_Mode return_status_led_mode2 = STATUS_POWER_OFF_MODE; //When current state is Amp Error mode, Just display fast blinking. But we need to return latest status when Amp Error mode is ended.
#endif
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY
Status_LED_Mode return_background_status_led_mode = STATUS_POWER_OFF_MODE; //When cur mode is STATUS_BT_MASTER_SLAVE_PAIRING_MODE, we need to return original value. this variable must update status led mode excepting STATUS_BT_MASTER_SLAVE_PAIRING_MODE mode.
#endif

void Set_Status_LED_Mode(Status_LED_Mode mode)
{
#ifdef SOC_ERROR_ALARM
	if(Get_Cur_Status_LED_Mode() == STATUS_SOC_ERROR_MODE)
		return;
#endif
#if defined(USEN_BAP) || defined(USEN_BAP2) //2023-02-09_1 : BAP-01 do not have MUTE LED
	if(mode == STATUS_MUTE_ON_MODE)
		return;
#endif
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
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY //When cur mode is STATUS_BT_MASTER_SLAVE_PAIRING_MODE, we need to return original value. this variable must update status led mode excepting STATUS_BT_MASTER_SLAVE_PAIRING_MODE mode.
		&& mode != STATUS_BT_MASTER_SLAVE_PAIRING_MODE
#endif		
		) //When user select un-mute, BT SPK should return latest status led mode. this variable must update status led mode excepting MUTE mode.		
		return_status_led_mode = mode;

#ifdef AMP_ERROR_ALARM
	if(Get_Amp_error_flag())
	{
		return_status_led_mode2 = mode;
		
		return;
	}
	
	if(mode == STATUS_AMP_ERROR_MODE)
		return_status_led_mode2 = cur_status_led_mode;	
#endif

#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY //When cur mode is STATUS_BT_MASTER_SLAVE_PAIRING_MODE, we need to return original value. this variable must update status led mode excepting STATUS_BT_MASTER_SLAVE_PAIRING_MODE mode.
	if(mode != STATUS_BT_MASTER_SLAVE_PAIRING_MODE)
		return_background_status_led_mode = mode;

#ifdef LED_DISPLAY_CHANGE
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
#else
	if(Get_master_slave_grouping_flag() && mode != STATUS_BT_MASTER_SLAVE_PAIRING_MODE)
		return;
#endif
#endif

	cur_status_led_mode = mode;
	LED_Status_Display_WR_Color(mode);
}

#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY //When cur mode is STATUS_BT_MASTER_SLAVE_PAIRING_MODE, we need to return original value. this variable must update status led mode excepting STATUS_BT_MASTER_SLAVE_PAIRING_MODE mode.
Status_LED_Mode Get_Return_Background_Status_LED_Mode(void)
{
	return return_background_status_led_mode;
}
#endif

Status_LED_Mode Get_Return_Status_LED_Mode(void) //When user select un-mute, BT SPK should return latest status led mode. this variable must update status led mode excepting MUTE mode.
{
#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(MASTER_SLAVE_GROUPING_LED_DISPLAY) //2023-06-09_1 : When user executes Mute On-->Mute Off under Grouping mode on TWS Master SPK, Mute LED still keep on.
	if(Get_master_slave_grouping_flag() == TRUE && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
		return_status_led_mode = STATUS_BT_MASTER_SLAVE_PAIRING_MODE;
#endif
	return return_status_led_mode;
}

#ifdef AMP_ERROR_ALARM
Status_LED_Mode Get_Return_Status_LED_Mode2(void)
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\rGet_Return_Status_LED_Mode2()");
	_DBH(return_status_led_mode2);
#endif
	return return_status_led_mode2;
}
#endif

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
#if !defined(USEN_BAP) && !defined(USEN_BAP2) //2023-07-27_1
	Bool Mute_On; //Upon Mute On, this flag check whether need to set bliking mode or not. When mute is on, we don't need to run blinking mode except red led.
#endif
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode Master_Slave;

	Master_Slave = Get_Cur_Master_Slave_Mode();
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE) || defined(AD85050_ENABLE)
#if !defined(USEN_BAP) && !defined(USEN_BAP2) //2023-07-27_1
#ifdef AD82584F_USE_POWER_DOWN_MUTE
	Mute_On = IS_Display_Mute();
#else
	Mute_On = Is_Mute(); //AD82584F_Amp_Get_Cur_Mute_Status();
#endif
#endif
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)

#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Status_Display_WR_Color() : mode = ");
	_DBD(mode);	
	_DBG("\n\r Mute =");_DBH(Mute_On);
#endif

	if(!Power_State()) //Do not LED turn on when Power off
		return;

#ifdef FACTORY_RESET_LED_DISPLAY
	if(Get_factory_reset_led_display_flag() == TRUE) //Do not change LED statun when FACTORY RESET LED DISPLAY On.
		return;
#endif

#ifdef AUX_INPUT_DET_ENABLE
	if(Aux_In_Exist())
	{
		TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
	}
#endif

	switch(mode)
	{
		case STATUS_POWER_ON_MODE:
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
#ifdef USEN_BAP //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //2022-10-07_2			
			//Top L1 LED
			if(!Mute_On) //Fixed the MUTE LED is not turned on after power plug out/in under mute on state
			{
				MUTE_LED_WHITE_ON;
				MUTE_LED_RED_OFF;
			}
#endif
		}
			break;
			
		case STATUS_POWER_OFF_MODE: // White Off/Red Off
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
			
#if !defined(USEN_BAP) && !defined(USEN_BAP2) //2022-10-07_2 //2022-01-09_4
			//Front Status LED
			STATUS_LED_WHITE_OFF;
			STATUS_LED_RED_OFF;
#endif

			//Top L3 LED
			BT_PAIRING_LED_BLUE_OFF;
			BT_PAIRING_LED_WHITE_OFF;

#ifdef USEN_BAP //2022-01-09_4
			STATUS_LED_W_OFF;
#elif defined(USEN_BAP2)
			STATUS_LED_W_OFF;
#else //2022-10-07_2 

			MUTE_LED_WHITE_OFF;
			MUTE_LED_RED_OFF;
#endif
		}
			break;
			
		case STATUS_BT_PAIRED_MODE: // White On
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
			
#ifdef USEN_BAP //2023-07-27_1 : When user select mute on USEN Tablet App and then Power off-->on, BAP-01 doesn't have power LED ON. //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //2022-10-07_2
			//Front Status LED
			if(!Mute_On)
			{
				STATUS_LED_WHITE_ON;
				STATUS_LED_RED_OFF;

				//Top L1 LED
				MUTE_LED_WHITE_ON;
				MUTE_LED_RED_OFF;
			}
#endif
			//Top L3 LED
#ifdef LED_DISPLAY_CHANGE
			if(Get_master_slave_grouping_flag())
				break;
#endif
#ifndef MASTER_MODE_ONLY
			if(Master_Slave == Switch_Master_Mode) //Master Mode
#endif
			{
#ifdef AUX_INPUT_DET_ENABLE
				if(!Aux_In_Exist())
#endif
				{
					BT_PAIRING_LED_BLUE_OFF;
					BT_PAIRING_LED_WHITE_ON;
				}
			}
#ifndef MASTER_MODE_ONLY
			else //Slave Mode
			{
				BT_PAIRING_LED_BLUE_ON;
				BT_PAIRING_LED_WHITE_OFF;
			}
#endif
		}
			break;
			
		case STATUS_AUX_MODE: // White On //BT LED(BLUE/WHITE) OFF
		{
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off

#ifdef USEN_BAP //2023-07-27_1 //2022-10-07_2
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //2022-01-09_4
			//Front Status LED			
			if(!Mute_On)
			{
				STATUS_LED_WHITE_ON;
				STATUS_LED_RED_OFF;

				//Top L1 LED
				MUTE_LED_WHITE_ON;
				MUTE_LED_RED_OFF;
			}
#endif
			//Top L3 LED
			BT_PAIRING_LED_BLUE_OFF;
			BT_PAIRING_LED_WHITE_OFF;
		}
			break;
			
		case STATUS_BT_PAIRING_MODE: // White Fast Blinking
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
			
#ifdef USEN_BAP //2023-07-27_1 //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //2022-10-07_2 
			//Front Status LED //White Fast Blinking
			if(!Mute_On)
			{
				STATUS_LED_WHITE_ON;
				STATUS_LED_RED_OFF;

				//Top L1 LED
				MUTE_LED_WHITE_ON;
				MUTE_LED_RED_OFF;
			}
#endif
#ifdef LED_DISPLAY_CHANGE
			if(Get_master_slave_grouping_flag())
				break;
#endif

#ifndef MASTER_MODE_ONLY
			//Top L3 LED			
			if(Master_Slave == Switch_Master_Mode) //Master Mode //Fast Blinking
#endif
			{
#ifdef AUX_INPUT_DET_ENABLE
				if(!Aux_In_Exist())
#endif
				{
					BT_PAIRING_LED_BLUE_OFF;
					BT_PAIRING_LED_WHITE_ON;
				}
			}
#ifndef MASTER_MODE_ONLY
			else //Slave Mode
			{
				BT_PAIRING_LED_BLUE_ON;
				BT_PAIRING_LED_WHITE_OFF;
			}
#endif
		}
			break;

#ifdef AMP_ERROR_ALARM
				case STATUS_AMP_ERROR_MODE: // White/Blue Very Fast Blinking 	
				{
					TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
					
#ifndef USEN_BAP2 //2024-01-31_1 : BAP-02 display Power status LED blinking under AMP error(BAP-01 used BT status led White/Blue)
#ifdef LED_DISPLAY_CHANGE
					if(Get_master_slave_grouping_flag())
						break;
#endif
#endif
		
					//Front Status LED //White Fast Blinking
#ifdef USEN_BAP //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else
					if(!Mute_On)
					{
						STATUS_LED_WHITE_ON;
					}
#endif
#ifndef USEN_BAP2 //2024-01-31_1 : BAP-02 display Power status LED blinking under AMP error(BAP-01 used BT status led White/Blue)
					//Top L3 LED
					{
		
							BT_PAIRING_LED_BLUE_ON;
							BT_PAIRING_LED_WHITE_ON;
					}
#endif
				}
					break;
#endif

#ifdef GIA_MODE_LED_DISPLAY_ENABLE
		case STATUS_BT_GIA_PAIRING_MODE: // White Fast Blinking		
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
			
#ifdef LED_DISPLAY_CHANGE
			if(Get_master_slave_grouping_flag())
				break;
#endif

#ifdef USEN_BAP //2023-07-27_1 //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //2022-10-07_2 
			//Front Status LED //White Fast Blinking
			if(!Mute_On)
			{
				STATUS_LED_WHITE_ON;
				STATUS_LED_RED_OFF;

				//Top L1 LED
				MUTE_LED_WHITE_ON;
				MUTE_LED_RED_OFF;
			}
#endif

#ifndef MASTER_MODE_ONLY
			//Top L3 LED
			if(Master_Slave == Switch_Master_Mode) //Master Mode //Fast Blinking
#endif
			{
#ifdef AUX_INPUT_DET_ENABLE
				if(!Aux_In_Exist())
#endif
				{

					BT_PAIRING_LED_BLUE_OFF;
					BT_PAIRING_LED_WHITE_ON;
				}
			}
#ifndef MASTER_MODE_ONLY
			else //Slave Mode
			{
				BT_PAIRING_LED_BLUE_ON;
				BT_PAIRING_LED_WHITE_OFF;
			}
#endif
		}
			break;
#endif

#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY
		case STATUS_BT_MASTER_SLAVE_PAIRING_MODE: // White Fast Blinking		
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
			
#ifdef USEN_BAP //2023-07-27_1 //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)		
			STATUS_LED_W_ON;
#else //2022-10-07_2
			//Front Status LED //White Fast Blinking
			if(!Mute_On)
			{
				STATUS_LED_WHITE_ON;
				STATUS_LED_RED_OFF;

				//Top L1 LED
				MUTE_LED_WHITE_ON;
				MUTE_LED_RED_OFF;
			}
#endif

#ifndef MASTER_MODE_ONLY
			//Top L3 LED
			if(Master_Slave == Switch_Master_Mode) //Master Mode //Fast Blinking
#endif
			{
#if defined(AUX_INPUT_DET_ENABLE) && defined(LED_DISPLAY_CHANGE)
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY //2023-06-07_2 : Applied "2023-04-13_3" solution under BSP-01T //def USEN_BAP //2023-04-13_3 : Changed BAP-01 Spec which BAP-01 should be supported LED display for Master/Slave grouping under aux mode also.
				if(!Aux_In_Exist()|| Get_master_slave_grouping_flag()) //2023-04-18_1 : Added Grouping LED display condition under Aux Mode to avoid display LED under other case(BT connection disconnect from Peer Device and White LED is blinking).
#else //USEN_BAP
				if(!Aux_In_Exist())
#endif //USEN_BAP
#endif
				{
					BT_PAIRING_LED_BLUE_OFF;
					BT_PAIRING_LED_WHITE_ON;
				}
			}
#ifndef MASTER_MODE_ONLY
			else //Slave Mode
			{
				BT_PAIRING_LED_BLUE_ON;
				BT_PAIRING_LED_WHITE_OFF;
			}
#endif
		}
			break;
#endif //MASTER_SLAVE_GROUPING_LED_DISPLAY

		case STATUS_BT_FAIL_OR_DISCONNECTION_MODE: // White Slow Blinking
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On

#ifdef USEN_BAP //2023-07-27_1 //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //2022-10-07_2
			//Front Status LED //White Slow Blinking
			if(!Mute_On)
			{
				STATUS_LED_WHITE_ON;
				STATUS_LED_RED_OFF;

				//Top L1 LED
				MUTE_LED_WHITE_ON;
				MUTE_LED_RED_OFF;
			}
#endif

#ifdef LED_DISPLAY_CHANGE
			if(Get_master_slave_grouping_flag())
				break;
#endif

#ifndef MASTER_MODE_ONLY
			//Top L3 LED	
			if(Master_Slave == Switch_Master_Mode) //Master Mode //Slow Blinking
#endif
			{
#ifdef AUX_INPUT_DET_ENABLE
				if(!Aux_In_Exist())
#endif
				{
					BT_PAIRING_LED_BLUE_OFF;
					BT_PAIRING_LED_WHITE_ON;
				}
			}
#ifndef MASTER_MODE_ONLY
			else //Slave Mode
			{
				BT_PAIRING_LED_BLUE_ON;
				BT_PAIRING_LED_WHITE_OFF;
			}	
#endif
		}
			break;
			
		case STATUS_MUTE_ON_MODE: // Red On
		{
#ifndef LED_DISPLAY_CHANGE
			TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
#endif
#ifdef USEN_BAP //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //USEN_BAP //2022-10-07_2
			//Front Status LED
			STATUS_LED_WHITE_OFF;
			STATUS_LED_RED_ON;

			//Top L1 LED
			MUTE_LED_WHITE_OFF;
			MUTE_LED_RED_ON;
#endif
		}
			break;
			
		case STATUS_SOC_ERROR_MODE: // Red Blinking
		{
			TIMER21_Periodic_Mode_Run(TRUE); //Blinkiing Timer On
#ifdef LED_DISPLAY_CHANGE
			if(Get_master_slave_grouping_flag())
				break;
#endif

#ifdef USEN_BAP //2022-01-09_4
			STATUS_LED_W_ON;
#elif defined(USEN_BAP2)
			STATUS_LED_W_ON;
#else //USEN_BAP //2022-10-07_2 
			//Front Status LED //Red Fast Blinking
			STATUS_LED_WHITE_OFF;
			STATUS_LED_RED_ON;

			if(!Mute_On)
			{
				//Top L1 LED
				MUTE_LED_WHITE_OFF;
				MUTE_LED_RED_OFF;
			}
#endif
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

	if(!Power_State()) //Do not LED turn on when Power off
		return;

#ifdef FACTORY_RESET_LED_DISPLAY
	if(Get_factory_reset_led_display_flag() == TRUE) //Do not change LED statun when FACTORY RESET LED DISPLAY On.
		return;
#endif

	switch(Color)
	{
#ifdef USEN_BAP2 //2024-01-31_1 : BAP-02 display Power status LED blinking under AMP error(BAP-01 used BT status led White/Blue)
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
#endif
#if !defined(USEN_BAP) && !defined(USEN_BAP2) //2022-10-07_2
		case STATUS_LED_RED:
			if(On)
			{
				STATUS_LED_RED_ON;
			}
			else
			{
				STATUS_LED_RED_OFF;
			}
			break;
		//2022-01-09_4
		case STATUS_LED_WHITE:
#ifdef AUX_INPUT_DET_ENABLE //2023-03-14_3 : The white status LED should be turned on alway under AUX IN Mode.
			if(Aux_In_Exist())
			{
				STATUS_LED_WHITE_ON;
				break;
			}
#endif

			if(On)
			{
				STATUS_LED_WHITE_ON;
			}
			else
			{
				STATUS_LED_WHITE_OFF;
			}
			break;
#endif
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
#if defined(AUX_INPUT_DET_ENABLE) && defined(LED_DISPLAY_CHANGE) //Fixed Master L3 White LED ON after factory reset with Aux In
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY //2023-06-07_2 : Applied "2023-04-13_3" solution under BSP-01T //def USEN_BAP //2023-04-13_3 : Changed BAP-01 Spec which BAP-01 should be supported LED display for Master/Slave grouping under aux mode also.
			if(Aux_In_Exist() && !Get_master_slave_grouping_flag()) //2023-04-18_1 : Added Grouping LED display condition under Aux Mode to avoid display LED under other case(BT connection disconnect from Peer Device and White LED is blinking).
				break;
#else //USEN_BAP
			if(Aux_In_Exist())
				break;
#endif //USEN_BAP
#endif
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

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)

uint8_t Vol_LED[16] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,0};

#define VOL_LED_ALL_OFF				{HAL_TIMER3n_SetADuty(TIMER30,(TIMER30->PDR>>15)); \
									HAL_TIMER3n_SetBDuty(TIMER30,(TIMER30->PDR>>15)); \
									HAL_TIMER3n_SetCDuty(TIMER30,(TIMER30->PDR>>15)); \
									TIMER1n_Configure(Timer1n_10, 0); \
									TIMER1n_PWMRun(Timer1n_10); \
									TIMER1n_Configure(Timer1n_11, 0); \
									TIMER1n_PWMRun(Timer1n_11);} // Vol Level = 0

#define VOL_LED1_OFF				(HAL_TIMER3n_SetADuty(TIMER30,(TIMER30->PDR>>15)))
#define VOL_LED2_OFF				(HAL_TIMER3n_SetBDuty(TIMER30,(TIMER30->PDR>>15)))
#define VOL_LED3_OFF				(HAL_TIMER3n_SetCDuty(TIMER30,(TIMER30->PDR>>15)))
#define VOL_LED4_OFF				{TIMER1n_Configure(Timer1n_10, 0); \
									TIMER1n_PWMRun(Timer1n_10);}
#define VOL_LED5_OFF				{TIMER1n_Configure(Timer1n_11, 0); \
									TIMER1n_PWMRun(Timer1n_11);}

/***********************************************************************/
/********************  Total Period	/  High Period	/ Low Period *************/
/************ OFF			200us		0us			200us	************/
/************ LEVEL_1		200us		50us			150us	************/
/************ LEVEL_2		200us		100us		100us	************/
/************ FULL			200us		200us		0us		************/
/***********************************************************************/
#define VOL_LED1_LEVEL_1		(HAL_TIMER3n_SetADuty(TIMER30,(TIMER30->PDR>>3))) //PE1/PWM30AB, Vol Level = 1
#define VOL_LED1_LEVEL_2		(HAL_TIMER3n_SetADuty(TIMER30,(TIMER30->PDR>>1))) //PE1/PWM30AB, Vol Level = 2
#define VOL_LED1_FULL			(HAL_TIMER3n_SetADuty(TIMER30,(TIMER30->PDR>>0))) //PE1/PWM30AB, Vol Level = 3
#define VOL_LED2_LEVEL_1		(HAL_TIMER3n_SetBDuty(TIMER30,(TIMER30->PDR>>3))) //PE3/PWM30BB, Vol Level = 4
#define VOL_LED2_LEVEL_2		(HAL_TIMER3n_SetBDuty(TIMER30,(TIMER30->PDR>>1))) //PE3/PWM30BB, Vol Level = 5
#define VOL_LED2_FULL 			(HAL_TIMER3n_SetBDuty(TIMER30,(TIMER30->PDR>>0))) //PE3/PWM30BB, Vol Level = 6
#define VOL_LED3_LEVEL_1		(HAL_TIMER3n_SetCDuty(TIMER30,(TIMER30->PDR>>3))) //PE5/PWM30CB, Vol Level = 7
#define VOL_LED3_LEVEL_2		(HAL_TIMER3n_SetCDuty(TIMER30,(TIMER30->PDR>>1))) //PE5/PWM30CB, Vol Level = 8
#define VOL_LED3_FULL 			(HAL_TIMER3n_SetCDuty(TIMER30,(TIMER30->PDR>>0))) //PE5/PWM30CB, Vol Level = 9

#define VOL_LED4_LEVEL_1		{TIMER1n_Configure(Timer1n_10, 25); \
								TIMER1n_PWMRun(Timer1n_10);}//PE6/T10O, Vol Level = 10
#define VOL_LED4_LEVEL_2		{TIMER1n_Configure(Timer1n_10, 100); \
								TIMER1n_PWMRun(Timer1n_10);} //PE6/T10O, Vol Level = 11
#define VOL_LED4_FULL 			{TIMER1n_Configure(Timer1n_10, 200); \
								TIMER1n_PWMRun(Timer1n_10);} //PE6/T10O, Vol Level = 12
#define VOL_LED5_LEVEL_1		{TIMER1n_Configure(Timer1n_11, 25); \
								TIMER1n_PWMRun(Timer1n_11);} //PE7/T11O, Vol Level = 13
#define VOL_LED5_LEVEL_2		{TIMER1n_Configure(Timer1n_11, 100); \
								TIMER1n_PWMRun(Timer1n_11);} //PE7/T11O, Vol Level = 14
#define VOL_LED5_FULL 			{TIMER1n_Configure(Timer1n_11, 200); \
								TIMER1n_PWMRun(Timer1n_11);} //PE7/T11O, Vol Level = 15

/* Private function prototypes ---------------------------------------*/
/* Private variables ---------------------------------------------------*/
#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : LED display of EQ Mode
void LED_DIsplay_EQ_Mode(EQ_Mode_Setting EQ_mode)
{
	if(!Power_State())
		return;

	switch(EQ_mode)
		{
#ifdef USEN_EQ_ENABLE
		case EQ_NORMAL_MODE:
		VOL_LED1_OFF;
		VOL_LED2_OFF;
		VOL_LED3_OFF;
		VOL_LED4_OFF;
		VOL_LED5_FULL;
		break;
		
		case EQ_POP_ROCK_MODE:
		VOL_LED1_OFF;
		VOL_LED2_OFF;
		VOL_LED3_OFF;
		VOL_LED4_FULL;
		VOL_LED5_OFF;
		break;
		
		case EQ_CLUB_MODE:
		VOL_LED1_OFF;
		VOL_LED2_OFF;
		VOL_LED3_FULL;
		VOL_LED4_OFF;
		VOL_LED5_OFF;
		break;
		
		case EQ_JAZZ_MODE:
		VOL_LED1_OFF;
		VOL_LED2_FULL;
		VOL_LED3_OFF;
		VOL_LED4_OFF;
		VOL_LED5_OFF;
		break;
		
		case EQ_VOCAL_MODE:
		VOL_LED1_FULL;
		VOL_LED2_OFF;
		VOL_LED3_OFF;
		VOL_LED4_OFF;
		VOL_LED5_OFF;
		break;
#else //USEN_EQ_ENABLE
		case EQ_ROCK_MODE:
		VOL_LED1_FULL;
		VOL_LED2_OFF;
		VOL_LED3_OFF;
		VOL_LED4_OFF;
		VOL_LED5_OFF;
		break;
		
		case EQ_JAZZ_MODE:
		VOL_LED1_OFF;
		VOL_LED2_FULL;
		VOL_LED3_OFF;
		VOL_LED4_OFF;
		VOL_LED5_OFF;
		break;
		
		case EQ_CLASSIC_MODE:
		VOL_LED1_OFF;
		VOL_LED2_OFF;
		VOL_LED3_FULL;
		VOL_LED4_OFF;
		VOL_LED5_OFF;
		break;
		
		case EQ_POP_MODE:
		VOL_LED1_OFF;
		VOL_LED2_OFF;
		VOL_LED3_OFF;
		VOL_LED4_FULL;
		VOL_LED5_OFF;
		break;
#endif //USEN_EQ_ENABLE
	}

	TIMER20_eq_mode_check_flag_start(); //Start timer of EQ LED disply 
}
#endif //EQ_TOGGLE_ENABLE

void LED_Display_Volume_All_Off(void)
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Display_Volume_All_Off() ");
#endif
	VOL_LED_ALL_OFF;
}

void LED_Display_Volume(uint8_t Volume) // Value 0(Max) ~ 15(Min)
{
	uint8_t uVol_Level = 0, uNumber = 0, uRest = 0;

#ifdef LR_360_FACTORY_ENABLE //2023-04-06_3 : When user change the position of SW1 or SW2, Factory Resest is executed and LED Display All On() is called even though power off mode.
	if(!Power_State())
		return;
#endif

#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Display_Volume() : Volume = ");
	_DBD(Volume);
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE) || defined(AD85050_ENABLE)
#ifdef AD82584F_ENABLE
	AD82584F_Amp_Set_Cur_Volume_Level(Volume); //Save current volume level
#elif defined(AD85050_ENABLE)
#else //TAS5806MD_ENABLE
	TAS5806MD_Amp_Set_Cur_Volume_Level(Volume); //Save current volume level
#endif //AD82584F_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)

	uVol_Level = Vol_LED[Volume]; //Change Value(0(Max) ~ 15(Min)) into Vol_Level(0(Min) ~ 15(Max)) for calculation

	if(!uVol_Level)
	{
		VOL_LED_ALL_OFF; //All LEDs are turned off
	}
	else
	{
		uVol_Level -= 1; //For easy calculation
		uNumber = uVol_Level/3; // Each LED Level is 3 step
		uRest = uVol_Level%3;

#ifdef LED_DISPLAY_DEBUG
		_DBG("\n\ruVol_Level = ");
		_DBD(uVol_Level);
		_DBG("\n\ruNumber = ");
		_DBD(uNumber);
			_DBG("\n\ruRest = ");
		_DBD(uRest);
#endif

		switch(uNumber) // LED numbers are 5 and each LED has 3 steps.
		{
			case 0:
				VOL_LED1_OFF;
				VOL_LED2_OFF;
				VOL_LED3_OFF;
				VOL_LED4_OFF;
				
				//LED5 Control : Vol_Level(1 ~ 3)					
				if(uRest == 0) //Level 1
				{
					VOL_LED5_LEVEL_1;
				}
				else if(uRest == 1) //Level 2
				{
					VOL_LED5_LEVEL_2;
				}
				else //Full
				{
					VOL_LED5_FULL;
				}
				break;

			case 1:
				VOL_LED5_FULL;

				VOL_LED1_OFF;
				VOL_LED2_OFF;
				VOL_LED3_OFF;
				
				//LED4 Control : Vol_Level(4 ~ 6)
				if(uRest == 0) //Level 1
				{
					VOL_LED4_LEVEL_1;
				}
				else if(uRest == 1) //Level 2
				{
					VOL_LED4_LEVEL_2;
				}
				else //Full
				{
					VOL_LED4_FULL;
				}
				break;
				
			case 2:
				VOL_LED5_FULL;
				VOL_LED4_FULL;

				VOL_LED1_OFF;
				VOL_LED2_OFF;
				
				//LED3 Control : Vol_Level(7 ~ 9)
				if(uRest == 0) //Level 1
				{
					VOL_LED3_LEVEL_1;
				}
				else if(uRest == 1) //Level 2
				{
					VOL_LED3_LEVEL_2;
				}
				else //Full
				{
					VOL_LED3_FULL;
				}
				break;

			case 3:
				VOL_LED5_FULL;
				VOL_LED4_FULL;
				VOL_LED3_FULL;

				VOL_LED1_OFF;
				
				//LED2 Control : Vol_Level(10 ~ 12)
				if(uRest == 0) //Level 1
				{
					VOL_LED2_LEVEL_1;
				}
				else if(uRest == 1) //Level 2
				{
					VOL_LED2_LEVEL_2;
				}
				else //Full
				{
					VOL_LED2_FULL;
				}
				break;
				
			case 4:
				VOL_LED5_FULL;
				VOL_LED4_FULL;
				VOL_LED3_FULL;
				VOL_LED2_FULL;

				//LED1 Control : Vol_Level(13 ~ 15)
				if(uRest == 0) //Level 1
				{
					VOL_LED1_LEVEL_1;
				}
				else if(uRest == 1) //Level 2
				{
					VOL_LED1_LEVEL_2;
				}
				else //Full
				{
					VOL_LED1_FULL;
				}
				break;

			default:
				break;
		}
		
	}
}
#endif //TIMER30_LED_PWM_ENABLE

#ifdef FACTORY_RESET_LED_DISPLAY
void LED_Diplay_All_Off(void)
{
#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Diplay_All_Off() ");
#endif

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	//VOLUME LED ON
	VOL_LED_ALL_OFF;
#endif

#ifdef TIMER21_LED_ENABLE

#ifdef USEN_BAP //2022-01-09_4
	STATUS_LED_W_OFF;
#elif defined(USEN_BAP2)
	STATUS_LED_W_OFF;
#else //USEN_BAP //2022-10-07_2
	//STATUS LED ON
	STATUS_LED_WHITE_OFF;
	STATUS_LED_RED_OFF;

	//L1 LED
	MUTE_LED_WHITE_OFF;
	MUTE_LED_RED_OFF;
#endif

	//L3 LED
	BT_PAIRING_LED_BLUE_OFF;
	BT_PAIRING_LED_WHITE_OFF;
#endif
}

void LED_Display_All_On(void)
{
#ifdef LR_360_FACTORY_ENABLE //2023-04-06_3 : When user change the position of SW1 or SW2, Factory Resest is executed and LED Display All On() is called even though power off mode.
	if(!Power_State())
		return;
#endif

#ifdef LED_DISPLAY_DEBUG
	_DBG("\n\r+++LED_Display_All_On() ");
#endif

#if defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)
	//VOLUME LED ON
	VOL_LED5_FULL;
	VOL_LED4_FULL;
	VOL_LED3_FULL;
	VOL_LED2_FULL;
	VOL_LED1_FULL;
#endif

#ifdef TIMER21_LED_ENABLE
#ifdef USEN_BAP //2022-01-09_4
	STATUS_LED_W_ON,
#elif defined(USEN_BAP2)
	STATUS_LED_W_ON,
#else //USEN_BAP //2022-10-07_2
	//STATUS LED ON
	STATUS_LED_WHITE_ON;
	STATUS_LED_RED_ON;

	//L1 LED
	MUTE_LED_WHITE_ON;
	MUTE_LED_RED_ON;
#endif
	//L3 LED
	BT_PAIRING_LED_BLUE_ON;
	BT_PAIRING_LED_WHITE_ON;
#endif
}
#endif //FACTORY_RESET_LED_DISPLAY
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

