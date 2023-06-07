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
#define USEN_BT_SPK_TI				(1) //BSP-02
//#define USEN_BT_SPK_ESMT		(1) //BSP-01
//#define USEN_BAP					(1) //BAP-01

// Custom Mode ****************************************************/
//#define PRIVATE_CUSTOM_MODE					(1) //2023-01-17

// Tests ****************************************************/
//#define USEN_FACTORY_MODE_TEST			(1)
//#define SIG_TEST				(1) //Need to Delete !!!
//#define ENABLE_TWS_MODE_AP_TEST				(1) //If TWS_MODE is enabled and then user wants to test TWS_MODE standalone with AP, it shuld be enabled.

#if defined(USEN_BT_SPK_TI) || defined(USEN_BT_SPK_ESMT)
#define USEN_BT_SPK				(1)
#define LR_360_FACTORY_ENABLE	(1) //2023-04-06_1 : Changed SPEC which LR/360 swich should work with Factory Reset
#endif

#ifdef USEN_BAP
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
#ifndef USEN_BAP
#define TWS_MODE_ENABLE							(1) //Support TWS MODE //2022-11-02
#endif
//#define NO_BROADCAST_MODE						(1)

#ifdef LGD_BT_SPK
#define ESTEC_BOARD			(1)
#endif

// Features ****************************************************************/
#ifdef LGD_SOUND_BAR
#define SOUND_BAR_GPIO_ENABLE				(1) //PC4(FAULT)/PD0(CLIP_OTW) : Interrupt Input, PC3(MUTE)/PD1(RESET) : Output
#endif

// IR Feature ****************/
#if defined(USING_REFERENCE_FLATFORM) || defined(LGD_BT_SPK) || defined(LGD_SOUND_BAR) //Under ESTec board, PC0 must be MODULE_RESET pin for BT Module
#define REMOCON_TIMER20_CAPTURE_ENABLE		(1) //Use TIMER20 CAPTURE for the IR function implement - PC0
#endif
//#define SUPPORT_LG_REMOCON	(1)

//ADC Fearue **************/
#ifdef USEN_BAP
#define ADC_INPUT_ENABLE				(1) //Implemented Master Volume & Attenuator Volume thru ADC feature. Please refer to ADC_INPUT_ENABLE //2022-10-12
//#define ADC_INTERRUPT_INPUT_ENABLE		(1) //Implemented Master Volume thru ADC Interrupt feature. Please refer to ADC_INTERRUPT_INPUT_ENABLE //2023-02-07
#endif

// UART Feature **************/ //When we use GUI Tool or Upgrade mode of MCS Logic, we need to disable UART 10.
#define UART_10_ENABLE							(1) //Use UART 10 for the communication with BT module - TX10 : PB0 / RX10 : PB1. If you don't use UART10, please make sure to disable this macro !!!

// I2C Feature **************/
#define I2C_0_ENABLE							(1) //Use I2C 0 for the communication with AD82584F - PF6:SCL0, PF7:SDA0. If you don't use I2C0, please make sure to disable this macro !!!

// SPI Feature **************/
#ifdef LGD_SOUND_BAR
#define SPI_11_ENABLE							(1) //Use USART 11 as SPI for the communication with ADAU1452(DSP) - MOSI:PD2/MISO:PD3/SCK:PD4/SS:PD5. If you don't use USART11 as SPI, please make sure to disable this macro !!!
#ifdef SPI_11_ENABLE
#define ADAU1452_ENABLE						(1) //Use ADAU1452 DSP using SPI control
#endif //SPI_11_ENABLE
#endif //LGD_SOUND_BAR

// GPIO Feature **************/
#ifdef LGD_BT_SPK
#define BT_SPK_GPIO_ENABLE							(1) //PC4 / PD0 : Interrupt Input, PD2 / PD3 : Output
#endif //LGD_BT_SPK
#ifdef USEN_BT_SPK
#define AUX_INPUT_DET_ENABLE						(1) // PC3 : interrupt Input for Aux In Detect.
#define USEN_GPIO_OTHERS_ENABLE					(1) //PF0(FACTORY RESET)/PF2(+3.3V_DAMP_SW_1)/PF3(+14V_DAMP_SW_1)/PF4(+14V_DAMP_PDN) /PF5(DAMP_ERROR)
#endif

// Button/Touch Key Feature **************/
#ifdef LGD_BT_SPK
#ifdef BT_SPK_GPIO_ENABLE //Touch Key Disable
#define BT_SPK_TACT_SWITCH						(1) //PE3 / PE4 / PE5 / PE6 / PE7 : TACT Switch input
#endif
#ifndef BT_SPK_TACT_SWITCH
//#define TOUCHKEY_ENABLE					(1) //Use Touchkey
#endif
#endif //LGD_BT_SPK

#ifdef USEN_BT_SPK
#ifdef USING_REFERENCE_FLATFORM
#define MODE_KEY_TOGGLE_ENABLE		(1)
#else
#define SWITCH_BUTTON_KEY_ENABLE		(1) //Use External INT for Switchs and Button Keys - PA0 / PA1 / PA2 / PA3 / PA4 / PA5 / PA6
#ifdef SWITCH_BUTTON_KEY_ENABLE
#ifndef USEN_BAP
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
#if defined(USEN_BT_SPK_TI) || defined(USEN_BAP) //BSP-02 //2023-02-10_2 : Fixed the problem BAP-01 can't connect with USEN Tablet
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
#ifdef AD82584F_ENABLE
#define USEN_EQ_ENABLE							(1) //Use USEN EQ Setting
#define AD82584F_USE_POWER_DOWN_MUTE 		(1) //To use PD pin instead of MUTE
#else
#define TAS5806MD_ENABLE					(1) //Use TAS5806DM
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
#ifdef USEN_BAP
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
#ifndef USEN_BAP
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
#if defined(USEN_BAP) || (defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE)) //To turning on all LED, when user press FACTORY RESET BUTTON //2023-01-05_4
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
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE) //To Do !!! - Need to check with TAS5806MD
#if defined(AUX_INPUT_DET_ENABLE) && !defined(USEN_BAP) //2023-05-22_2 : Delete AUTO_ONOFF_ENABLE for changing BAP-01 spec.
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
#ifndef USEN_BAP
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


#if defined(SPI_11_ENABLE) || defined(UART_10_ENABLE)
typedef enum{
	SERIAL_PORT10,
	SERIAL_PORT11,
	SERIAL_PORT_MAX	
}SerialPort_t;

typedef void (*Serial_Handle_t)(uint8_t *Data);
#endif //defined(SPI_11_ENABLE) || defined(UART_10_ENABLE)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef UART_10_ENABLE
#define UART10_Rx_Buffer_Size		255
extern uint8_t uBuffer_Count;
#endif
#ifdef BT_SPK_GPIO_ENABLE
extern Mute_Status Cur_Mute_Status;
#endif
#ifdef FACTORY_MODE
extern Bool bFACTORY_MODE;
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
extern Bool BBT_Pairing_Key_In;
extern Bool bFactory_Reset_Mode;
#endif

#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
extern Bool B_Master_Is_BAP; //2023-01-09_2
#endif

/* Private define ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

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
#ifdef UART_10_ENABLE
void UART10_IRQHandler_IT(void);
void Serial_Data_Clear(uint8_t length, uint8_t start_count);
void Serial_Data_Get(uint8_t *Buf, uint8_t length, uint8_t start_count);
#endif
#ifdef I2C_0_ENABLE /*I2C0_IRQHandler_IT */
void I2C0_IRQHandler_IT(void);
#endif
#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(TIMER20_COUNTER_ENABLE)
void TIMER20_IRQHandler_IT(void);
#endif
#ifdef SPI_11_ENABLE
void USART11_IRQHandler_IT(void);
#endif

#if defined(TIMER21_LED_ENABLE) || defined(TIMER12_13_LONG_KEY_ENABLE)
void All_Timer_Off(void); //TIMER12/TIMER13/TIMER21
#endif

#ifdef TIMER30_LED_PWM_ENABLE
void TIMER30_IRQHandler_IT(void);
#endif
#ifdef TIMER1n_LED_PWM_ENABLE
/* TIMER10/11 IRQHandler*/
void TIMER10_IRQHandler_IT(void);
void TIMER11_IRQHandler_IT(void);
#endif
#ifdef TIMER12_13_LONG_KEY_ENABLE
void TIMER13_IRQHandler_IT(void);
void TIMER12_IRQHandler_IT(void);
#endif
#ifdef TIMER21_LED_ENABLE
void TIMER21_IRQHandler_IT(void);
#endif
#ifdef BT_SPK_GPIO_ENABLE
void GPIOCD_IRQHandler_IT(void);
//PC4 / PD0 : Interrupt Input, PD2 / PD3 : Output //PD2 - DAC_MUTE_SW : High - Unmute / Low - Mute
void AMP_DAC_MUTE(Bool Mute);
void AMP_RESET(Bool Reset);
#ifdef BT_SPK_TACT_SWITCH
void GPIOE_IRQHandler_IT(void);
#endif
#else //BT_SPK_GPIO_ENABLE
#if defined(USEN_BAP) && defined(SWITCH_BUTTON_KEY_ENABLE)
void GPIOE_IRQHandler_IT(void);
#endif
#endif //BT_SPK_GPIO_ENABLE

#ifdef SOUND_BAR_GPIO_ENABLE
void GPIOCD_IRQHandler_IT1(void);
void DSP_Reset(Bool Reset_On);
#endif
#ifdef AUX_INPUT_DET_ENABLE
void GPIOCD_IRQHandler_IT2(void);
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
void GPIOAB_IRQHandler_IT(void);
#endif
#ifdef USEN_GPIO_OTHERS_ENABLE //Use External INT for Switchs and Button Keys - PF0 / PF5(AMP_ERROR input)
void GPIOF_IRQHandler_IT(void);
#endif
#if defined(USEN_BAP) && !(BT_SPK_TACT_SWITCH) //Implemented Interrupt Port E for BT Key(PE7) //2022-10-11_2 //2023-01-03
void EXIT_PortE_Disable(void); //2023-01-03_2 : Disable Interrupt Port E for BT Key(PE7)
void EXIT_PortE_Configure(void); //2023-01-03_2
#endif

#if defined(USEN_BAP) && defined(AUX_INPUT_DET_ENABLE) && defined(TIMER20_COUNTER_ENABLE) //2023-01-10_3
void Aux_Mode_Setting_After_Timer_Checking(Bool Aux_In);
void Set_Aux_Detection_flag(void); //2023-04-12_1
#endif
#ifdef AUX_INPUT_DET_ENABLE
Bool Aux_In_Exist(void);
#endif
#if defined(AMP_1_1CH_WORKAROUND) || defined(TAS5806MD_ENABLE) //2022-10-17_3
void TAS5806MD_Init_After_Clk_Detect(void);
#ifdef TAS5806MD_ENABLE
uint32_t TAS5806MD_CLK_Detect_Count(void); //2022-12-06 : Clk_detect_uCount = 0xffffffff(Complete CLK detection)
#endif
#endif
#ifdef ADC_INTERRUPT_INPUT_ENABLE
void ADC_IRQHandler_IT(void);
#endif //ADC_INTERRUPT_INPUT_ENABLE
#ifdef MASTER_MODE_ONLY
Switch_BAP_EQ_Mode Get_Cur_BAP_EQ_Mode(void);
#else //MASTER_MODE_ONLY
Switch_Master_Slave_Mode Get_Cur_Master_Slave_Mode(void);
#endif //MASTER_MODE_ONLY
Switch_LR_Stereo_Mode Get_Cur_LR_Stereo_Mode(void);

void Factory_Reset_Value_Setting(void);
#ifdef USEN_BT_SPK
#ifdef USEN_BAP //2023-04-06_4 : To recognize the place which call this function is whther SW start or Power On
void Init_Value_Setting(Bool B_bool);
#else
void Init_Value_Setting(void);
#endif
#endif
void SW_Reset(void);

#ifdef ADC_INPUT_ENABLE //2022-11-22_1
int8_t uAttenuator_Vol_Value(void);
#endif
#if defined(ADC_INPUT_ENABLE) && defined(USEN_BAP) && defined(TAS5806MD_ENABLE) //2023-03-02_3
uint8_t ADC_Volume_Attenuator_Value_Init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __A34M41x_CONF_H */


