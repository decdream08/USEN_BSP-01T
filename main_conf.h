/**********************************************************************
* @file		main_conf.h
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library on A31G21x
* @version	1.0
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
**********************************************************************/
#ifndef __A31G21x_CONF_H
#define __A31G21x_CONF_H

	
/* Includes ---------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
#include "A31G21x_hal_pcu.h"
#include "A31G21x_hal_scu.h"
#include "A31G21x_hal_uartn.h"
#include "A31G21x_hal_debug_frmwrk.h"
#include "A31G21x_hal_usart1n.h"
#include "A31G21x_hal_i2c.h"
#include "A31G21x_hal_timer2n.h"

#ifdef __cplusplus
extern "C"
{
#endif	
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

// Define Model *************************************************************/
//#define LGD_BT_SPK				(1)
//#define LGD_SOUND_BAR			(1)
//#define USEN_BT_SPK_TI				(1) //BSP-02
//#define USEN_BT_SPK_ESMT		(1) //BSP-01
//#define USEN_BAP					(1) //BAP-01
#define USEN_BAP2					(1) //BAP-02 //2024-01-31

//#ifdef USEN_BAP2 //2024-01-31
//#define USEN_BAP					(1)
//#endif

// Custom Mode ****************************************************/
//#define PRIVATE_CUSTOM_MODE					(1) //2023-01-17

// Tests ****************************************************/
//#define USEN_FACTORY_MODE_TEST			(1)
//#define SIG_TEST				(1) //Need to Delete !!!
//#define ENABLE_TWS_MODE_AP_TEST				(1) //If TWS_MODE is enabled and then user wants to test TWS_MODE standalone with AP, it shuld be enabled.

#if defined(USEN_BAP) || defined(USEN_BAP2)
#define MASTER_MODE_ONLY		(1) //2023-03-27_1 : USEN BAP-01 is only supporting Master mode
#define USEN_BT_SPK				(1)
#define AMP_ERROR_ALARM			(1)
#define ADC_VOLUME_STEP_ENABLE			(1) //2023-01-04
#define ESD_ERROR_RECOVERY		(1) //2023-05-18_1 : For ESD Error Recovery under BAP-01
#ifdef ADC_VOLUME_STEP_ENABLE //2023-02-27 : Changed ADC volume step from 64 step to 50 step.
//#define ADC_VOLUME_64_STEP_ENABLE				(1)
#define ADC_VOLUME_50_STEP_ENABLE				(1)
#endif
#define AUX_DETECT_INTERRUPT_ENABLE						(1)
#else
#define AUX_DETECT_INTERRUPT_ENABLE						(1)
#endif

#ifdef USEN_BT_SPK
//#define USING_REFERENCE_FLATFORM				(1) //When we use Reference board for MCU/BT/Amp, we must enable this macro.
//#define OLD_BOARD								(1) //Proto Sample
//#define ES_1_BOARD				(1)
#define ES_2_BOARD				(1)
#endif

// Model Specific Features ****************************************************/
// Features ****************************************************************/
// IR Feature ****************/

//ADC Fearue **************/
#if defined(USEN_BAP) || defined(USEN_BAP2)
#define ADC_INPUT_ENABLE				(1) //Implemented Master Volume & Attenuator Volume thru ADC feature. Please refer to ADC_INPUT_ENABLE //2022-10-12
//#define ADC_INTERRUPT_INPUT_ENABLE		(1) //Implemented Master Volume thru ADC Interrupt feature. Please refer to ADC_INTERRUPT_INPUT_ENABLE //2023-02-07
#endif

// UART Feature **************/ //When we use GUI Tool or Upgrade mode of MCS Logic, we need to disable UART 10.
#define UART_10_ENABLE							(1) //Use UART 10 for the communication with BT module - TX10 : PB0 / RX10 : PB1. If you don't use UART10, please make sure to disable this macro !!!

// I2C Feature **************/
#define I2C_0_ENABLE							(1) //Use I2C 0 for the communication with AD82584F - PF6:SCL0, PF7:SDA0. If you don't use I2C0, please make sure to disable this macro !!!

#ifdef USEN_BAP2
#define PCM9211_ENABLE
#ifdef PCM9211_ENABLE
//#define I2C_1_ENABLE
#endif
#endif

// GPIO Feature **************/
#ifdef USEN_BT_SPK
#define AUX_INPUT_DET_ENABLE						(1) // PC3 : interrupt Input for Aux In Detect.
#define USEN_GPIO_OTHERS_ENABLE					(1) //PF0(FACTORY RESET)/PF2(+3.3V_DAMP_SW_1)/PF3(+14V_DAMP_SW_1)/PF4(+14V_DAMP_PDN) /PF5(DAMP_ERROR)
#endif

// Button/Touch Key Feature **************/
#ifdef USEN_BT_SPK
#ifdef USING_REFERENCE_FLATFORM
#define MODE_KEY_TOGGLE_ENABLE		(1)
#else
#define SWITCH_BUTTON_KEY_ENABLE		(1) //Use External INT for Switchs and Button Keys - PA0 / PA1 / PA2 / PA3 / PA4 / PA5 / PA6
#ifdef SWITCH_BUTTON_KEY_ENABLE
#if !defined(USEN_BAP) // && !defined(USEN_BAP_2)
#define POWER_KEY_TOGGLE_ENABLE				(1) //Use Power Key Toggle as USEN Spec
#endif
#define FACTORY_RESET_KEY_CAPACITOR_APPLY 	(1) //2023-05-04_4 : For factory reset key chattering uner BAP-01//For recovery, When we use capacitor on FACTORY RESET Line, the Rising Edge value is always 0x03(0x02 is correct) but Falling Edge is always 0x01

#endif //SWITCH_BUTTON_KEY_ENABLE
#endif
#define TIMER12_13_LONG_KEY_ENABLE		(1) //Use TIMER13(PA7) for Long Key control - Just Timer not Port(the PA7 can be used GPIO if you want)
#endif //USEN_BT_SPK

// BT Module //To Do !!! - Need to separate F1DQ3007(LGD BT SPK) and F1DQ3021(USEN BT SPK) **************/
#ifdef UART_10_ENABLE
#if defined(LGD_BT_SPK) || defined(USEN_BT_SPK) || defined(LGD_SOUND_BAR)
#define BT_MODULE_ENABLE			(1) //Use BT Module
#if defined(LGD_BT_SPK) || defined(LGD_SOUND_BAR)
#define F1DQ3007_ENABLE					(1) //Use F1DQ3007 BT Module
#define HIDE_CONNECTABLE_ENABLE			(1) //When we success LAST CONNECT, we should execute DISCOVERABLE to avoid other device.
#else //USEN_BT_SPK
#define MB3021_ENABLE					(1) //Use MB3021 BT Module
//#define F1DQ3021_ENABLE					(1) //Use F1DQ3021 BT Module
#endif
#endif
#ifdef MB3021_ENABLE
#define NEW_BT_FW_BUG_FIX					(1) //2023-02-20_1 : To fix BT FW(2302170) bug under TWS Mode(Slave Name is "MB3021BNU0")
#define BT_DISCONNECT_CONNECTABLE_ENABLE	(1) //When the BT module is disconnected, we need to set connectable mode to connect BT module from Peer Device
#define BT_GENERAL_MODE_KEEP_ENABLE			(1) //When user executes power plug out/in under general mode, BT SPK must keep general mode. If Last connection is failed, BT SPK should connect with other general device. //2022-12-23
#if defined(USEN_BT_SPK_TI) || defined(USEN_BAP) || defined(USEN_BAP2)//BSP-02 //2023-02-10_2 : Fixed the problem BAP-01 can't connect with USEN Tablet
#define BT_ALWAYS_GENERAL_MODE				(1) //Alway General mode and do not use USEN mode //2023-01-31_1
#endif //USEN_BT_SPK_TI
#endif //MB3021_ENABLE
#endif //UART_10_ENABLE

#ifdef TWS_MODE_ENABLE
#define NEW_TWS_MASTER_SLAVE_LINK			(1) //2023-04-26_1 : To make new spec which is new tws master slave link.
#define MCU_VERSION_INFO_DISPLAY_W_UART 	(1) //2023-06-07_3 : To send MCU version info using UART under TWS Mode
#endif

// Amp **************/
#ifdef I2C_0_ENABLE
#ifdef LGD_BT_SPK
#define TAS3251_ENABLE						(1) //Use TAS3251
#define AMP_1_1CH_WORKAROUND			(1) // 1.1ch setting error - need to init again upon receiving actual I2S input from BT Module
#endif
#ifdef LGD_SOUND_BAR
#define SABRE9006A_ENABLE					(1) //Use SABRE9006A DAC for LGD Soundbar
#endif
#ifdef USEN_BT_SPK
#ifdef USEN_BT_SPK_ESMT
#define AD82584F_ENABLE						(1) //Use AD82584F
#endif

//#define TAS5806MD_ENABLE					(1) //Use TAS5806DM
#define AD85050_ENABLE					(1) //Use TAS5806DM

#ifdef AD82584F_ENABLE
#define USEN_EQ_ENABLE							(1) //Use USEN EQ Setting
#define AD82584F_USE_POWER_DOWN_MUTE 		(1) //To use PD pin instead of MUTE
#elif defined(AD85050_ENABLE)
#define USEN_EQ_ENABLE							(1) //Use USEN EQ Setting
#define AD82584F_USE_POWER_DOWN_MUTE 		(1) //To use PD pin instead of MUTE
//#define NOT_USE_POWER_DOWN_MUTE				(1) //To use Deep Sleep mode instead of PD pin low under power off.  2022-10-04
#else
//#define TAS5806MD_ENABLE					(1) //Use TAS5806DM
#define USEN_EQ_ENABLE							(1) //Use USEN EQ Setting
//#ifdef USEN_BT_SPK_TI //2023-03-23_1
#define USEN_TI_AMP_EQ_ENABLE							(1) //Use New USEN EQ Setting using TI AMP //2023-02-27_1
//#endif
#define AD82584F_USE_POWER_DOWN_MUTE 		(1) //To use PD pin instead of MUTE
#define NOT_USE_POWER_DOWN_MUTE				(1) //To use Deep Sleep mode instead of PD pin low under power off.  2022-10-04
#define TI_AMP_DSP_VOLUME_CONTROL_ENABLE	(1) //To enable DSP Volume control instead of DAC Gain. 2023-01-04
#ifdef USEN_BAP
#define USE_TI_AMP_HI_Z_MUTE					(1) //To use HI_Z mode mute instead of mute register to improve the noise on DC power on and etc. 2023-05-22_1
#endif //USEN_BAP
#endif //AD82584F_ENABLE
#endif //USEN_BT_SPK
#endif //I2C_0_ENABLE

// WatchDog **************/
#if defined(USEN_BAP) || defined(USEN_BAP2)
#define WATCHDOG_TIMER_RESET					(1) //2023-05-16_1
#endif

// Model Feature (1/2)********************/
#ifdef USEN_BT_SPK //Need to Check !!!
//#define BT_NAME_EXTENSION						(1)
#ifdef I2C_0_ENABLE
//#define AUTO_VOLUME_LED_OFF						(1)
#endif
#define KEY_CHATTERING_ENABLE					(1)
#ifdef KEY_CHATTERING_ENABLE
#define KEY_CHATTERING_EXTENSION_ENABLE			(1) //For LR/360 Switch and Master/Slave Switch
#endif
#define AUX_CHATTERING_ENABLE					(1) //For Aux In Chattering
#define VOLUME_KEY_FILTERING					(1) //For Volue key filtering by user pressing Volume button
#define FACTORY_MODE							(1) //UART Disable for MCS Logic Tool(BT Tool)

#ifdef UART_10_ENABLE
#ifndef NO_BROADCAST_MODE
#define SPP_DATA_GET_FROM_PEER_DEVICE			(1)
#ifndef MASTER_MODE_ONLY
#define BLE_DATA_GET_FROM_MASTER_SPK			(1)
#endif
#ifdef I2C_0_ENABLE
#define INPUT_KEY_SYNC_WITH_SLAVE_ENABLE		(1) //When user push actual key instead of SPP(App), Slave SPK should be sync... Master need to send key value over BLE.
#endif
#ifdef TWS_MODE_ENABLE
#define SW1_KEY_TWS_MODE						(1) //LR/360 switch : LR Mode is TWS setting //2022-11-02
#define TWS_MASTER_SLAVE_COM_ENABLE				(1) //This is the commnucation thru AVRCP between MASTER and SLAVE
#endif
#define SPP_EXTENSION_ENABLE					(1) //Added USEN SPP protocol v3 spec
#define VERSION_INFORMATION_SUPPORT				(1) //Add date information instead of FW version information
#define PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE		(1)
#define FACTORY_RESET_LONG_KEY_SUPPORT			(1) //Changed FACTORY_RESET short key to long key(5-sec)
//#define PRODUCT_LINE_TEST_MASTER_ID2_FIXED		(1) //For Product line Test, Master is always fixed as ID2 for Master/Slave connection.
#ifndef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
#define MASTER_SLAVE_GROUPING					(1) //Support specific Master can connect with speicific Slave
#endif //PRODUCT_LINE_TEST_MASTER_ID2_FIXED
#ifdef SPP_EXTENSION_ENABLE
#define SPP_EXTENSION_V42_ENABLE				(1) //Added USEN SPP protocol v4.2 spec
#ifdef MASTER_SLAVE_GROUPING
#ifdef TWS_MODE_ENABLE
#define TWS_MASTER_SLAVE_GROUPING				(1) //Added Master Slave Grouping feature under TWS Mode
#endif
#define SPP_EXTENSION_V44_ENABLE				(1) //Added USEN SPP protocol v4.4 spec(Added BT short key)
//#define KEY_IS_INVALID_UNDER_MASTER_SLAVE_GROUPING				(1) //To make invalid key action excepting FACTORY_RESET_KEY/SW1_KEY/SW2_KEY under Master/Slave Grouping mode. To avoid minor issues(LED Display)
//#define MASTER_SLAVE_GROUPING_RECOVERY			(1) //Added BT module reset under slave mode when slave finish master/slave grouping
#endif //MASTER_SLAVE_GROUPING
#if defined(SPP_EXTENSION_V42_ENABLE) && defined(VERSION_INFORMATION_SUPPORT)
#define SPP_EXTENSION_V50_ENABLE				(1) //Added USEN SPP protocol v5.0 spec
#endif
#endif //SPP_EXTENSION_ENABLE
#endif //NO_BROADCAST_MODE
#define BT_CLEAR_CONNECTABLE_MODE				(1) //Need to disable connectable mode after finishing BT connection
#define DEVICE_NAME_CHECK_PAIRING				(1) //To pairing with PeerDevice, We check if device name is valid.
#ifndef DEVICE_NAME_CHECK_PAIRING
#define INQUIRY_ACCESS_CODE_SUPPORT				(1) //Inquiry Access Code Support : General Inquiry Access Code mode / Limitted Inquiry Access Code mode
#endif
#endif //UART_10_ENABLE

// LED Feature **************/
#ifdef USEN_BT_SPK
#if !defined(USEN_BAP) && !defined(USEN_BAP2)
#define TIMER30_LED_PWM_ENABLE			(1) //Use TIMER30 for PWM control for LED - PE1 / PE3 / PE 5
#define TIMER1n_LED_PWM_ENABLE			(1) //Use TIMER10/TIMER11 for PWM control for LED - PE6 / PE7
#endif
#define TIMER21_LED_ENABLE				(1) //Use TIMER21 for STATUS_LED - PC1(STATUS_LED_W) / PC2(STATUS_LED_R)
#define GPIO_LED_ENABLE					(1) //Use GPIOs as LED output control - PC1 / PC2 / PD2 / PD3 / PD4 / PD5
#define GIA_MODE_LED_DISPLAY_ENABLE		(1) //To recognize General Inquiry Access mode with LED Display(White/Blue Led slow bliniking). This is available in Master mode only.
#ifdef MASTER_SLAVE_GROUPING
#define MASTER_SLAVE_GROUPING_LED_DISPLAY			(1)
#define LED_DISPLAY_CHANGE							(1) //To fix mute on/power off LED display under Master/Slave grouping
#endif //MASTER_SLAVE_GROUPING
#if (defined(USEN_BAP) || defined(USEN_BAP2)) || (defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)) //To turning on all LED, when user press FACTORY RESET BUTTON //2023-01-05_4
#define FACTORY_RESET_LED_DISPLAY				(1) //When Factory Reset, display all LED ON
#endif
#ifdef ES_2_BOARD
#define LED_POWER_CONTROL_ENABLE	(1) //To avoid blinkering when power on
#else //ES_2_BOARD
#define LED_PORT_PUSH_PULL			(1) // Make LED port as Push-pull otherwise works LED port as Open-Drain
#endif //ES_2_BOARD
#ifdef USING_REFERENCE_FLATFORM
#define LED_PORT_HIGH_DISPLAY			(1) //The Reference platform turns on LEDs when the LED port is high. But USEN board works in the opposite way.
#endif
#ifndef REMOCON_TIMER20_CAPTURE_ENABLE
#define TIMER20_COUNTER_ENABLE				(1) //Use TIMER20 for Counter(Always On)
#endif
#endif //USEN_BT_SPK

// Model Feature (2/2)********************/
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
#define SPP_CMD_AND_USER_INPUT_KEY_SYNC			(1) //Need to sync SPP CMD and USER_INPUT_KEY
#define SPP_CMD_AND_MASTER_INFO_SEND			(1) //Send current Master SPK to Tablet
#endif

#if defined(INPUT_KEY_SYNC_WITH_SLAVE_ENABLE) && defined(I2C_0_ENABLE)
#define SLAVE_ADD_MUTE_DELAY_ENABLE			(1) //For Slave Pop noise
#define SLAVE_VOLUME_FORCED_SYNC			(1) //Need to update volume info even though invalid value becasue Slave can get different volume level.
#endif
#define FIVE_USER_EQ_ENABLE						(1)
#ifdef TIMER20_COUNTER_ENABLE
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE) || defined(AD85050_ENABLE) //To Do !!! - Need to check with TAS5806MD
#if defined(AUX_INPUT_DET_ENABLE) && (!defined(USEN_BAP) && !defined(USEN_BAP2)) //2023-05-22_2 : Delete AUTO_ONOFF_ENABLE for changing BAP-01 spec.
#define AUTO_ONOFF_ENABLE						(1) //To implemet Auto OnOff(5min) feature //Only valid this feature under master mode
#endif
#endif
#ifdef UART_10_ENABLE
#ifndef MASTER_MODE_ONLY
#define SLAVE_AUTO_OFF_ENABLE					(1) //When Slave lost Master SPK, Slave wait 10min and then goes power off mode
#endif
#if defined(AUTO_ONOFF_ENABLE) && defined(DEVICE_NAME_CHECK_PAIRING) //When user selects power off button under USEN Tablet, we need to enable auto power on.
#define USEN_TABLET_AUTO_ON						(1) //When user selects power off button under USEN Tablet, we need to enable auto power on. Auto power on works when Tablet plays streaming
#endif
#endif
#if !defined(USEN_BAP) && !defined(USEN_BAP2)
#if defined(I2C_0_ENABLE) && defined(UART_10_ENABLE)
#define SOC_ERROR_ALARM							(1) //SoC Error Alarm(I2C Error or UART Error)
#endif //#if defined(I2C_0_ENABLE) && defined(UART_10_ENABLE)
#endif
#endif
#define FLASH_SELF_WRITE_ERASE					(1) //To save volume, PDL and Slave last connection information, Use Flash write and erase
#ifdef FLASH_SELF_WRITE_ERASE
#define FLASH_SELF_WRITE_ERASE_EXTENSION		(1) //To save Power On/Off, Mute On/Off and EQ Mode information, Use Flash write and erase
#ifndef PRIVATE_CUSTOM_MODE
#define FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ		(1) //To deleted EQ mode in flash save data
#endif
#endif
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
#define FLASH_NOT_SAVE_POWER_MODE_SLAVE				(1) //To avoid power sync with Master SP, we don't update Power mode on Slave
#endif

// Special Feature for Test ********************/
#ifdef UART_10_ENABLE
#ifndef NO_BROADCAST_MODE
#ifdef MASTER_SLAVE_GROUPING
//#define MASTER_SLAVE_GROUPING_SLAVE_EMPTY		(1) //Support Master / Slave grouping when slave doesn't have last connection information.
#endif
#endif //NO_BROADCAST_MODE

//#define AVRCP_ENABLE							(1) //Support AVRCP for only
#ifdef AVRCP_ENABLE
//#define AVRCP_CONNECTION_CONTROL_ENABLE			(1)
#endif
#endif //UART_10_ENABLE

#ifdef PRIVATE_CUSTOM_MODE
#define EQ_TOGGLE_ENABLE						(1)
#endif
//#define DRC_TOGGLE_TEST							(1) //DRC On/Off Toggle using MUTE KEY(RED LED ON : DRC Off/WHITE LED ON : DRC On)

#ifdef AD82584F_ENABLE
//#define SRS_FILTER_ENABLE						(1) //Use SRS Filter on AD82584F
#endif

#endif //USEN_BT_SPK

// DEBUG MSG **************************************************/
#ifdef _DEBUG_MSG
//#define COMMON_DEBUG_MSG		(1)
#ifdef UART_10_ENABLE
//#define _UART_DEBUG_MSG		(1) //Debug message for UART especially F1MEDIA
#ifdef BT_MODULE_ENABLE
//#define BT_DEBUG_MSG			(1) //Debug message for F1MEDIA
//#define F1DQ3007_DEBUG_MSG			(1) //Debug message for F1MEDIA
#endif
#endif
#ifdef I2C_0_ENABLE
//#define _I2C_DEBUG_MSG		(1) //Debug message for I2C especially TAS3251
#ifdef TAS5806MD_ENABLE
//#define TAS5806MD_DEBUG_MSG		(1) //Debug message for TAS5806MD AMP
//#define I2C_ACCESS_ERROR_DEBUG 		(1) //I2C AMP Init error check
#endif
#ifdef AD85050_ENABLE
//#define AD85050_DEBUG_MSG		(1) //Debug message for AD85050 AMP
//#define I2C_ACCESS_ERROR_DEBUG 		(1) //I2C AMP Init error check
#endif
#endif
#ifdef REMOCON_TIMER20_CAPTURE_ENABLE
//#define _TIMER20_CAPTURE_DEBUG_MSG		(1) //Debug message for TIMER20 CAPTURE especially IR Key
#endif

#ifdef BT_SPK_GPIO_ENABLE
//#define GPIO_DEBUG_MSG							(1) 
#endif
#ifdef TOUCHKEY_ENABLE
//#define _DBG_MSG_EN 						1
#endif
#ifdef FLASH_SELF_WRITE_ERASE
//#define _DBG_FLASH_WRITE_ERASE							(1)
#endif //FLASH_SELF_WRITE_ERASE
#ifdef AUX_INPUT_DET_ENABLE
//#define AUX_INPUT_DET_DEBUG						(1)
#endif
#ifdef SLAVE_AUTO_OFF_ENABLE
//#define SLAVE_AUTO_OFF_DEBUG_MSG				(1)
#endif
#ifdef AUTO_ONOFF_ENABLE
//#define AUTO_ONOFF_DEBUG_MSG					(1)
#endif
#ifdef MASTER_SLAVE_GROUPING
//#define MASTER_SLAVE_GROUPING_DEBUG_MSG			(1)
#endif
#ifdef BT_GENERAL_MODE_KEEP_ENABLE
//#define BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG		(1)
#endif
#ifdef DEVICE_NAME_CHECK_PAIRING
//#define DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG			(1)
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
//#define SWITCH_BUTTON_KEY_ENABLE_DEBUG_MSG			(1)
#endif
//#define MUTE_CHECK_DEBUG_MSG					(1)
#ifdef ADC_INPUT_ENABLE
//#define ADC_INPUT_DEBUG_MSG						(1)
#endif
#ifdef ADC_INTERRUPT_INPUT_ENABLE
//#define ADC_INTERRUPT_INPUT_ENABLE_DEBUG_MSG		(1)
#endif //ADC_INTERRUPT_INPUT_ENABLE
#ifdef SOC_ERROR_ALARM
//#define SOC_ERROR_ALARM_DEBUG_MSG				(1)
#endif
#ifdef MASTER_MODE_ONLY
//#define MASTER_MODE_ONLY_DEBUG_MSG				(1)
#endif
#ifdef WATCHDOG_TIMER_RESET
//#define WATCHDOG_TIMER_RESET_DEBUG_MSG			(1)
#endif
#ifdef ESD_ERROR_RECOVERY
//#define ESD_ERROR_RECOVERY_DEBUG_MSG				(1)
#endif
#endif //_DEBUG_MSG

// End Define *************************************************/

#ifdef ESD_ERROR_RECOVERY
#define ESD_ERROR_RECOVERY_TIME           (500000)	//1sec // (5000000/2) // 5000000 = 10sec (100000 = 200ms)
#endif

typedef enum {
	Mute_Status_Mute,
	Mute_Status_Unmute
}Mute_Status;

#ifdef MASTER_MODE_ONLY
typedef enum {
	Switch_EQ_BSP_Mode,
	Switch_EQ_NORMAL_Mode
}Switch_BAP_EQ_Mode;
#else //MASTER_MODE_ONLY
typedef enum {
	Switch_Slave_Mode,
	Switch_Master_Mode
}Switch_Master_Slave_Mode;
#endif //MASTER_MODE_ONLY

typedef enum {
	Switch_LR_Mode,
	Switch_Stereo_Mode
}Switch_LR_Stereo_Mode;
	
#ifdef USEN_EQ_ENABLE
typedef enum {
	EQ_NORMAL_MODE,
	EQ_POP_ROCK_MODE,
	EQ_CLUB_MODE,
	EQ_JAZZ_MODE,
	EQ_VOCAL_MODE,
#if defined(USEN_BAP) && defined(USEN_TI_AMP_EQ_ENABLE)
	EQ_BAP_NORMAL_MODE, //2023-03-28_6 : Added EQ NORMAL switch mode from EJT
#endif
#ifdef EQ_TOGGLE_ENABLE //2023-01-17
	EQ_NONE_MODE = 0xff
#endif
}EQ_Mode_Setting;
#else 
typedef enum {
	EQ_ROCK_MODE,
	EQ_JAZZ_MODE,
	EQ_CLASSIC_MODE,
	EQ_POP_MODE
}EQ_Mode_Setting;
#endif

typedef enum{
	SERIAL_PORT10,
	SERIAL_PORT11,
	SERIAL_PORT_MAX	
}SerialPort_t;

typedef enum{
    SLAVE_BT_VOLUME,
    AREA1_VOLUME,
    AREA2_VOLUME
}Attenuator_Type;

#define INVALID_VOLUME 0xff

#define AREA2_VOLUME_MASK 0xff0000
#define AREA1_VOLUME_MASK 0x00ff00
#define BT_VOLUME_MASK    0x0000ff

#define OFF 0
#define ON	1

enum enTimer1msStatus {
	df1msTimer0ms		= 0,
	df1msTimer1ms		= 1,
	df1msTimer2ms		= 2,
	df1msTimer3ms		= 3,
	df1msTimer5ms		= 5,
	df1msTimer10ms		= 10,
	df1msTimer15ms		= 15,
	df1msTimer20ms		= 20,
	df1msTimer30ms		= 30,
	df1msTimer40ms		= 40,
	df1msTimer50ms		= 50,
	df1msTimer60ms		= 60,
	df1msTimer70ms		= 70,
	df1msTimer80ms		= 80,
	df1msTimer90ms		= 90,
	df1msTimer100ms		= 100,
	df1msTimer120ms		= 120,
	df1msTimer150ms		= 150,
	df1msTimer200ms		= 200,
	df1msTimer400ms		= 400,	
	df1msTimer500ms		= 500,
	df1msTimer1s			= 1000,
	df1msTimer60s			= 60000,
};

enum enTimer10msStatus {
	df10msTimer0ms		= 0,
	df10msTimer10ms		= 1,
	df10msTimer20ms		= 2,
	df10msTimer30ms		= 3,
	df10msTimer40ms		= 4,
	df10msTimer50ms		= 5,
	df10msTimer60ms		= 6,
	df10msTimer70ms		= 7,
	df10msTimer80ms		= 8,
	df10msTimer90ms		= 9,
	df10msTimer100ms	= 10,
	df10msTimer110ms  = 11,
	df10msTimer150ms	= 15,
	df10msTimer170ms	= 17,	
	df10msTimer200ms	= 20,
	df10msTimer210ms	= 21,
	df10msTimer250ms   = 25,
	df10msTimer300ms	= 30,
	df10msTimer400ms	= 40,
	df10msTimer450ms	= 45,
	df10msTimer500ms	= 50,
	df10msTimer600ms	= 60,
	df10msTimer700ms	= 70,
	df10msTimer800ms	= 80,
	df10msTimer900ms	= 90,
	df10msTimer1s		= 100,
	df10msTimer1s100ms	= 110,
	df10msTimer1s200ms	= 120,
	df10msTimer1s500ms	= 150,
	df10msTimer1s900ms	= 190,	
	df10msTimer2s		= 200,
	df10msTimer2s200ms		= 220,
	df10msTimer2s250ms		= 225,
	df10msTimer2s500ms = 250,
	df10msTimer3s		= 300,
	df10msTimer3s500ms		= 350,	
	df10msTimer3s800ms	= 380,
	df10msTimer4s		= 400,
	df10msTimer4s500ms	= 450,
	df10msTimer5s		= 500,
	df10msTimer5s500ms	= 550,	
	df10msTimer6s		= 600,
	df10msTimer7s		= 700,
	df10msTimer8s		= 800,
	df10msTimer9s		= 900,
	df10msTimer10s		= 1000,
	df10msTimer10s500ms		= 1050,
	df10msTimer11s		= 1100,	
	df10msTimer12s		= 1200,
	df10msTimer15s		= 1500,
	df10msTimer30s		= 3000,
	df10msTimer32s		= 3200,
	df10msTimer60s		= 6000,
	df10msTimer1min15sec	= 7500,
	df10msTimer2min		= 12000,
	df10msTimer2min30sec	= 14000,
	df10msTimer3min		= 18000,
	df10msTimer15min	= 90000,
};

enum enTimer50msStatus {
	df50msTimer0ms		= 0,
	df50msTimer50ms		= 1,
	df50msTimer100ms		= 2,
	df50msTimer150ms		= 3,
	df50msTimer200ms		= 4,
	df50msTimer250ms		= 5,
	df50msTimer300ms		= 6,
	df50msTimer350ms		= 7,
	df50msTimer400ms		= 8,
	df50msTimer450ms		= 9,
	df50msTimer500ms	= 10,
	df50msTimer1s		= 20,
	df50msTimer1s500ms		= 30,
	df50msTimer2s	= 40,
	df50msTimer3s	= 60,
	df50msTimer5s	= 100,
	df50msTimer6s	= 120,
};

enum enTimer100msStatus {
	df100msTimer0ms		= 0,
	df100msTimer100ms	= 1,
	df100msTimer200ms	= 2,
	df100msTimer300ms	= 3,
	df100msTimer400ms	= 4,
	df100msTimer500ms	= 5,
	df100msTimer700ms	= 7,	
	df100msTimer1s		= 10,
	df100msTimer1d1s	= 11,
	df100msTimer1d5s	= 15,
	df100msTimer1d6s	= 16,
	df100msTimer1d9s	= 19,	
	df100msTimer2s		= 20,
	df100msTimer2d5s		= 25,
	df100msTimer2d6s		= 26,
	df100msTimer3s		= 30,
	df100msTimer3d5s		= 35,
	df100msTimer4s		= 40,
	df100msTimer4d5s		= 45,
	df100msTimer5s		= 50,
	df100msTimer5d5s		= 55,
	df100msTimer6s		= 60,
	df100msTimer6d5s		= 65,	
	df100msTimer7s		= 70,
	df100msTimer7d5s		= 75,	
	df100msTimer8s		= 80,
	df100msTimer8d5s		= 85,	
	df100msTimer10s		= 100,
	df100msTimer12s		= 120,
	df100msTimer13s		= 130,
	df100msTimer15s		= 150,	
	df100msTimer20s		= 200,
	df100msTimer30s		= 300,
	df100msTimer30d1s	= 301,
	df100msTimer1min	= 1 * 60 * 10,	
	df100msTimer2min	= 2* 60 * 10,	
	df100msTimer3min	= 3 * 60 * 10,
	df100msTimer5min	= 5 * 60 * 10,
	df100msTimer10min	= 10 * 60 * 10,
	df100msTimer20min	= 20 * 60 * 10,	
	df100msTimer30min	= 30 * 60 * 10,
	df100msTimer40min	= 40 * 60 * 10,
	df100msTimer50min	= 50 * 60 * 10,	
	df100msTimer60min	= 60 * 60 * 10,
	df100msTimer70min	= 70 * 60 * 10,
	df100msTimer80min	= 80 * 60 * 10,	
	df100msTimer90min	= 90 * 60 * 10, 
 
};


void main_10ms_timer(void);
void main_timer_function(void);

typedef void (*Serial_Handle_t)(uint8_t *Data);

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define UART10_Rx_Buffer_Size		255
extern uint8_t uBuffer_Count;

extern Bool bFACTORY_MODE;

extern Bool BBT_Pairing_Key_In;
extern Bool bFactory_Reset_Mode;

extern Bool B_Master_Is_BAP; //2023-01-09_2

/* Private define ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
uint32_t ADC_Value_Update_to_send_Slave(void);

#ifdef WATCHDOG_TIMER_RESET
void WDT_ReloadTimeRun(void);
void WDT_Configure(void);
void WDT_ResetRun(void);
void SysTick_Handler_IT(void);
#endif

/* Initialize all port */
void Port_Init(void); 
/* Configure the system clock to 32 MHz */
void SystemClock_Config(void);
void DEBUG_MenuPrint(void);
void DEBUG_Init(void);
void delay_ms(uint32_t m_ms);
void UART10_IRQHandler_IT(void);
void Serial_Data_Clear(uint8_t length, uint8_t start_count);
void Serial_Data_Get(uint8_t *Buf, uint8_t length, uint8_t start_count);
void I2C0_IRQHandler_IT(void);
#ifdef I2C_1_ENABLE /*I2C1_IRQHandler_IT */
void I2C1_IRQHandler_IT(void);
#endif
void TIMER20_IRQHandler_IT(void);

void All_Timer_Off(void); //TIMER12/TIMER13/TIMER21

void TIMER13_IRQHandler_IT(void);
void TIMER12_IRQHandler_IT(void);

void TIMER21_IRQHandler_IT(void);
void GPIOE_IRQHandler_IT(void);
void GPIOCD_IRQHandler_IT2(void);
void GPIOAB_IRQHandler_IT(void);

void GPIOF_IRQHandler_IT(void);

void EXIT_PortE_Disable(void); //2023-01-03_2 : Disable Interrupt Port E for BT Key(PE7)
void EXIT_PortE_Configure(void); //2023-01-03_2

void Aux_Mode_Setting_After_Timer_Checking(Bool Aux_In);
void Set_Aux_Detection_flag(void); //2023-04-12_1

Bool Aux_In_Exist(void);

Switch_BAP_EQ_Mode Get_Cur_BAP_EQ_Mode(void);
Switch_LR_Stereo_Mode Get_Cur_LR_Stereo_Mode(void);

void Factory_Reset_Value_Setting(void);
void Init_Value_Setting(Bool B_bool);
void SW_Reset(void);

int8_t uAttenuator_Vol_Value(void);
uint8_t ADC_Volume_Attenuator_Value_Init(Attenuator_Type attenuator_type);

#ifdef __cplusplus
}
#endif

#endif /* __A34M41x_CONF_H */


