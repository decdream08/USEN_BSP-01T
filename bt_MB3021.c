/**
 ****************************************************************************************
 *
 * @file bt_module.c
 *
 * @brief
 *
 * @author MS Kim
 *
 * Copyright ESTec
 *
 ****************************************************************************************
 */

/**
****************************************************************************************

Hardware requirement: MCS Logic - MB3021\n
Port Configuration: 

 ****************************************************************************************
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "main_conf.h"

#ifdef TWS_MASTER_SLAVE_GROUPING
#include <stdlib.h>
#endif
#ifdef MB3021_ENABLE
#ifdef TIMER21_LED_ENABLE
#include "led_display.h"
#include "timer21.h"
#endif
#ifdef UART_10_ENABLE
#include "serial.h"
#include "bt_MB3021.h"

#if defined(REMOCON_TIMER20_CAPTURE_ENABLE) || defined(SWITCH_BUTTON_KEY_ENABLE)
#include "remocon_action.h"
#endif
#ifdef TIMER20_COUNTER_ENABLE
#include "timer20.h"
#endif
#ifdef AD82584F_ENABLE
#include "ad82584f.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h" 
#endif
#if defined(FLASH_SELF_WRITE_ERASE) && (defined(GIA_MODE_LED_DISPLAY_ENABLE) || defined(MASTER_SLAVE_GROUPING)) //Save whether Paired Device exist in Flash
#include "flash.h"
#endif
#ifdef DEVICE_NAME_CHECK_PAIRING
#include <string.h>
#endif

//Macro
#define PACKET_CMD_SHIFT_BIT			16

#define SYNC_BYTE			0x42 // 'B'

//Packet Type - 4 Types
//Command Packet Define - SYNC_BYTE(0x42) + CMD_PACKET(0x43) + MAJOR_ID + MINOR_ID + DATA_LEN + DATA.... + CHK_SUM(XOR)
#define PACKET_CMD			0x43 // 'C'

//Indication Packet Define - SYNC_BYTE(0x42) + IND_PACKET(0x49) + MAJOR_ID + MINOR_ID + DATA_LEN + DATA.... + CHK_SUM(XOR)
#define PACKET_IND			0x49 // 'I'

//Response Packet Define - SYNC_BYTE(0x42) + RESP_PACKET(0x52) + MAJOR_ID + MINOR_ID + DATA_LEN + ERROR_CODE + DATA.... + CHK_SUM(XOR)
#define PACKET_RESP			0x52 // 'R'

//Data Packet Define - SYNC_BYTE(0x42) + DATA_PACKET(0x44) + DATA_SOURCE_TYPE + DATA_CH + DATA_LEN(High) + DATA_LEN(Low) + DATA.... + CHK_SUM(XOR)
#define PACKET_DATA			0x44 // 'D'

//Data Source Type - DATA_SOURCE_TYPE
#define DATA_SOURCE_TYPE_SPP				0x70 //SPP Data
#define DATA_SOURCE_TYPE_OPP				0x71 //OPP Data
#define DATA_SOURCE_TYPE_UART_UPGRADE		0x72 //UART Upgrade Data Data
#define DATA_SOURCE_TYPE_BLE_DATA			0x73 //BLE Data

//Data Channel - DATA_CH
#define DATA_CH_DEFAULT			0x00 // Default
#define DATA_CH_ANDROID1		0x01 // Android 1
#define DATA_CH_ANDROID2		0x02 // Android 2
#define DATA_CH_IAP1			0x03 // IAP1
#define DATA_CH_IAP2			0x04 // IAP2

//Major id
#define MAJOR_ID_GENERAL_CONTROL				0x00 //Control Common Function
#define MAJOR_ID_QUALIFICATION_COMMANDS			0x01
#define MAJOR_ID_A2DP_CONTROL					0x10 //Control A2DP Profile Function
#define MAJOR_ID_AVRCP_CONTROL					0x11 //Control AVRCP Profile Function
#define MAJOR_ID_HFP_CONTROL					0x12 //Control HFP Profile Function
#define MAJOR_ID_SPP_CONTROL					0x13 //Control SPP Profile Function
												//0x14
#define MAJOR_ID_TWS_CONTROL					0x15 //Control TWS Function
#define MAJOR_ID_BA_CONTROL		0x16 //Control Braodcast Audio Control
#define MAJOR_ID_BLE_CONTROL					0x17 //Control BLE Function
#define MAJOR_ID_PBAP_CONTROL					0x18 //Control PBAP Function
#define MAJOR_ID_UPGRADE_CONTROL				0x1B //Control BLE Function
#define MAJOR_ID_SPP_DATA_TRANSFER				0x70
#define MAJOR_ID_UPGRADE_DATA_TRANSFER				0x72
#define MAJOR_ID_BLE_DATA_TRANSFER				0x73
#define MAJOR_ID_PBAP_DATA_TRANSFER				0x74
#define MAJOR_ID_MANUALFACTURE_CONTROL			0xF0
#define MAJOR_ID_DEBUG							0xF1


//Minor id 32 is composed of MAJOR_ID + MINOR_ID + REQ_LEN BYTE VALUE + RSP_LEN BYTE VALUE
//MAJOR_ID_GENERAL_CONTROL/MINOR ID & CMD
#define MINOR_ID_WATCHDOG						0x00
#define CMD_WATCHDOG_32							(0x0001UL|((MINOR_ID_WATCHDOG|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_SET_BLUETOOTH_STANDBY_MODE		0x01
#define CMD_SET_BLUETOOTH_STANDBY_MODE_32		(0x0101UL|((MINOR_ID_SET_BLUETOOTH_STANDBY_MODE|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_SET_MODEL_NAME					0x02
//To Do !!! - Need to check with customer about Name. Temperally the length is 0byte
#ifdef BT_NAME_EXTENSION // Add 7 Byte //But there is size issue and only available for additinal 1 Byte
//#define CMD_SET_MODEL_NAME_32					(0x1601UL|((MINOR_ID_SET_MODEL_NAME|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT)) //7Byte
#define CMD_SET_MODEL_NAME_32					(0x1001UL|((MINOR_ID_SET_MODEL_NAME|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT)) //5Byte
#else
#define CMD_SET_MODEL_NAME_32					(0x0F01UL|((MINOR_ID_SET_MODEL_NAME|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#endif
#define MINOR_ID_SET_DEVICE_ID					0x03
#ifdef VERSION_INFORMATION_SUPPORT //This size shall be 0x06 only!!!
#define CMD_SET_DEVICE_ID_32					(0x0601UL|((MINOR_ID_SET_DEVICE_ID|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#else
#define CMD_SET_DEVICE_ID_32					(0x0601UL|((MINOR_ID_SET_DEVICE_ID|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#endif
//#define MINOR_ID_INIT_SOURCE_MODE				0x04
//#define CMD_INIT_SOURCE_MODE_32				T.B.D
#define MINOR_ID_INIT_SINK_MODE					0x05
#define CMD_INIT_SINK_MODE_32					(0x0E01UL|((MINOR_ID_INIT_SINK_MODE|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_SET_DISCOVERABLE_MODE			0x07
#define CMD_SET_DISCOVERABLE_MODE_32			(0x0101UL|((MINOR_ID_SET_DISCOVERABLE_MODE|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_SET_CONNECTABLE_MODE			0x08
#define CMD_SET_CONNECTABLE_MODE_32				(0x0101UL|((MINOR_ID_SET_CONNECTABLE_MODE|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_GET_PAIRED_DEVICE_LIST			0x0A
#define CMD_GET_PAIRED_DEVICE_LIST_32			(0x01FFUL|((MINOR_ID_GET_PAIRED_DEVICE_LIST|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_DELETE_PAIRED_DEVICE_LIST		0x0B //Delete All
#define CMD_DELETE_PAIRED_DEVICE_LIST_32		(0x0101UL|((MINOR_ID_DELETE_PAIRED_DEVICE_LIST|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_CONTROL_ACL_CONNECTION			0x0D
#define CMD_CONTROL_ACL_CONNECTION_32			(0x0701UL|((MINOR_ID_CONTROL_ACL_CONNECTION|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_READ_FW_VERSION				0x0E
#define CMD_READ_FW_VERSION_32					(0x00FFUL|((MINOR_ID_READ_FW_VERSION|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_MODULE_POWER_CONTROL			0x11
#define CMD_MODULE_POWER_CONTROL_32				(0x0101UL|((MINOR_ID_MODULE_POWER_CONTROL|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_INFORM_HOST_MODE				0x17
#define CMD_INFORM_HOST_MODE_32					(0x0101UL|((MINOR_ID_INFORM_HOST_MODE|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#ifdef DEVICE_NAME_CHECK_PAIRING
#define MINOR_ID_SET_IOCAPABILITY_MODE			0x1F
#define CMD_SET_IOCAPABILITY_MODE_32			(0x0801UL|((MINOR_ID_SET_IOCAPABILITY_MODE|(MAJOR_ID_GENERAL_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#endif

#ifdef SIG_TEST
//MAJOR_ID_QUALIFICATION_COMMANDS(0x01/MINOR ID & CMD
#define MINOR_ID_ENTER_PTS_TEST_MODE			0x01
#define CMD_A2DP_ENTER_PTS_TEST_MODE			(0x0001UL|((MINOR_ID_ENTER_PTS_TEST_MODE|(MAJOR_ID_QUALIFICATION_COMMANDS << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_A2DP_PTS_EVENT					0x02
#define CMD_A2DP_PTS_EVENT					    (0x0101UL|((MINOR_ID_A2DP_PTS_EVENT|(MAJOR_ID_QUALIFICATION_COMMANDS << 8)) << PACKET_CMD_SHIFT_BIT))
#endif

//MAJOR_ID_A2DP_CONTROL(0x10)/MINOR ID & CMD
#define MINOR_ID_A2DP_CONNECTION_CONTROL		0x00
#define CMD_A2DP_CONNECTION_CONTROL_32			(0x0701UL|((MINOR_ID_A2DP_CONNECTION_CONTROL|(MAJOR_ID_A2DP_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_A2DP_ROUTING_CONTROL			0x02
#define CMD_A2DP_ROUTING_CONTROL_32				(0x0701UL|((MINOR_ID_A2DP_ROUTING_CONTROL|(MAJOR_ID_A2DP_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_A2DP_USER_EQ_CONTROL			0x03
#define CMD_A2DP_USER_EQ_CONTROL_32				(0x0101UL|((MINOR_ID_A2DP_USER_EQ_CONTROL|(MAJOR_ID_A2DP_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#ifdef AVRCP_ENABLE
//MAJOR_ID_AVRCP_CONTROL(0x11)/MINOR ID & CMD
#define MINOR_ID_AVRCP_CONNECTION_CONTROL		0x00
#define CMD_AVRCP_CONNECTION_CONTROL_32			(0x0701UL|((MINOR_ID_AVRCP_CONNECTION_CONTROL|(MAJOR_ID_AVRCP_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
#define MINOR_ID_SEND_SYNC_DATA					0x13
#define CMD_SEND_SYNC_DATA_32					(0x0F01UL|((MINOR_ID_SEND_SYNC_DATA|(MAJOR_ID_AVRCP_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#endif

#ifdef TWS_MODE_ENABLE
//MAJOR_ID_TWS_CONTROL(0x15)/MINOR ID & CMD
#define MINOR_ID_TWS_MODE_CONTROL				0x00
#define CMD_ID_TWS_MODE_CONTROL_32				(0x0101UL|((MINOR_ID_TWS_MODE_CONTROL|(MAJOR_ID_TWS_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_TWS_ROUTING_MODE_CONTROL		0x01
#define CMD_ID_TWS_ROUTING_MODE_CONTROL_32		(0x0101UL|((MINOR_ID_TWS_ROUTING_MODE_CONTROL|(MAJOR_ID_TWS_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#endif

//MAJOR_ID_BA_CONTROL(0x16)/MINOR ID & CMD
#define MINOR_ID_BA_MODE_CONTROL			0x00
#define CMD_BA_MODE_CONTROL_32				(0x0101UL|((MINOR_ID_BA_MODE_CONTROL|(MAJOR_ID_BA_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

//MAJOR_ID_BLE_CONTROL(0x17)/MINOR ID & CMD
#define MINOR_ID_ADVERTISING_CONTROL			0x02
#define CMD_ADVERTISING_CONTROL_32				(0x0101UL|((MINOR_ID_ADVERTISING_CONTROL|(MAJOR_ID_BLE_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_SET_BLE_MANUFACTURE_DATA		0x03
#define CMD_SET_BLE_MANUFACTURE_DATA_32			(0x0701UL|((MINOR_ID_SET_BLE_MANUFACTURE_DATA|(MAJOR_ID_BLE_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#if defined(VERSION_INFORMATION_SUPPORT) && defined(TWS_MASTER_SLAVE_COM_ENABLE) && defined(MCU_VERSION_INFO_DISPLAY_W_UART) //2023-06-07_3 : To send MCU Version informaiton under BSP-01T
#define MAJOR_ID_MCU_VERSION					0x20
#define CMD_SEND_MCU_VERSION_INFO_32			(0x0601UL|((MAJOR_ID_MCU_VERSION << 8) << PACKET_CMD_SHIFT_BIT))
#endif

//INDIGATOR
//MINOR ID - MAJOR_ID_GENERAL_CONTROL
#define MINOR_ID_MODULE_STATE_CHANGED_IND		0x00 //Data[0] : 0x0 - initialising / 0x1 - Ready
#ifdef DEVICE_NAME_CHECK_PAIRING
#define MINOR_ID_REMOTE_DEVICE_NAME_IND			0x0C //Data[0] : Result code / Data[1-6] : Bluetooth Device Address / Data[7~N] : BT Device Name
#endif
#define MINOR_ID_FIRMWARE_VERSION_IND			0x0E //7Byte(Data[0~1]:Year/Data[2~3]:Month/Data[4~5]:Day/Data[6]:Index)
#define MINOR_ID_LOCAL_ADDRESS_NAME_IND			0x0F //Data[0-5]:Bluetooth Device Address / Data[6-N]:Bluetooth Device Name

#define MINOR_ID_ROUTING_STATE_IND				0x22 //Data[0] - 0x0:Disconnect, 0x1:Connect/Data[1] - 0x0:Disconnect Audio Routing, 0x2:Analog, 0x6:A2DP_0
#define MINOR_ID_ACL_OPENED_IND					0x23 //Data[0-5]:Bluetooth Device Address / / Data[6]: status: 0x0 -OK, 0x4 - No device, 0xb - Peerdevice busy /Data[7],Data[8]
#define MINOR_ID_ACL_CLOSED_IND					0x24 //Data[0-5]:Bluetooth Device Address / Data[6]: Disconnect : 0x13/BT Delete : 0x16/PeerDevice Off : 0x08 

//MINOR ID - MAJOR_ID_A2DP_CONTROL - 0x10
#define MINOR_ID_A2DP_PROFIL_STATE_CHANGED_IND		0x00 //Data[0-5] : BDA/Data[6]:Status/Data[7]:Connection Direction/Data[8]:Address Type
#define MINOR_ID_A2DP_MEDIA_STATE_CHANGED_IND		0x01 //Data[0-5] : BT Address/Data[6] : 0x01 - Closed, 0x02  - Opened, 0x03 - Streaming, 0x04 - Suspended
#define MINOR_ID_A2DP_STREAM_ROUTING_CHANGED_IND		0x02 //Data[0] : Routing state(0x0:UnRouted,0x1:Routed)/Data[1~6]:Routed Device Address
#if defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE) //2022-11-03
#define MINOR_ID_A2DP_SELECTED_CODEC_IND			0x05 //Data[0-5] : BT Address/Data[6-7]: Selected Codec Flags
#endif
#define MINOR_ID_A2DP_CONNECT_FAIL_ERRORCODE_IND	0x70 //Data[0-5] : BDA/Data[6]:Status

//MINOR ID -MAJOR_ID_AVRCP_CONTROL - 0x11
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
#define MINOR_ID_AVRCP_PROFILE_STATE_CHANGED_IND		0x00 //Data[0-5] : BT Address / Data[6] : Profile State
#endif
#define MINOR_ID_AVRCP_PLAYBACK_STATUS_CHANGED_IND	0x01 //Data[0-5] : BT Address / Data[6] : Profile State
#define MINOR_ID_AVRCP_VOLUME_CHANGED_IND		0x02 //Data[0-5] : BT Address / Data[6] : Volume Value(0-127)
#define MINOR_ID_AVRCP_CAPABILITY_LIST_IND			0x03 //Data[0-5] : BT Address/Data[6-7]: Capability List //Need to check !!! 0x11 : 0x03
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
#define MINOR_ID_AVRCP_SEND_SYNC_DATA_IND						0x13
#endif

//MINOR ID - MAJOR_ID_SPP_CONTROL - 0x13
#define MINOR_ID_SPP_PROFILE_STATE_CHANGED_IND		0x00 //Data[0-5] : BDA/Data[6]:Profile Statue/Data[7]:Connection Channel Index(0x1 / Android SPP)

//MINOR ID - MAJOR_ID_BA_CONTROL - 0x16
#define MINOR_ID_BA_MODE_CONTROL_IND 			0x00
#define MINOR_ID_RECEIVER_CONNECTION_STATUS_IND 			0x02
#define MINOR_ID_BA_MANUFACTURE_DATA_IND		0x05 //Size : 0<N<12/Data[0 ~11] : User Data //BLE DATA for Receiver

//BT_CMD_RESP_FLAG
#define BCRF_INIT_SINK_MODE						0x01
#define BCRF_SET_DEVICE_ID_CONTROL				0x02
#define BCRF_SET_MODEL_NAME_CONTROL				0x04
#define BCRF_MODULE_POWER_CONTROL				0x08
#define BCRF_SET_DISCOVERABLE_MODE				0x10
#define BCRF_SET_CONNECTABLE_MODE				0x20
#define BCRF_ADVERTISING_CONTROL				0x40
#define BCRF_DISCONNECT_ROUTING					0x80
#define BCRF_ROUTE_TO_AUDIO_SOURCE				0x100
#define BCRF_SET_BLE_MANUFACTURE_DATA			0x200
#define BCRF_BA_MODE_CONTROL		0x400
#define BCRF_INFORM_HOST_MODE					0x800
#define BCRF_GET_PAIRED_DEVICE_LIST				0x1000
#define BCRF_SET_LAST_CONNECTION				0x2000
#define BCRF_DELETE_PAIRED_DEVICE_LIST			0x4000
#define BCRF_SEND_SPP_RECEIVE_DATA_OK			0x8000
#define BCRF_SEND_SPP_RECEIVE_DATA_NG			0x10000
#ifdef SPP_EXTENSION_ENABLE
#define BCRF_SEND_SPP_DATA_RESP					0x20000
#endif
#ifdef AVRCP_CONNECTION_CONTROL_ENABLE
#define BCRF_SET_AVRCP_CONNECTION_CONTROL		0x40000
#endif
#ifdef DEVICE_NAME_CHECK_PAIRING
#define BCRF_SET_IO_CAPABILITY_MODE				0x80000
#endif
#ifdef BT_CLEAR_CONNECTABLE_MODE
#define BCRF_CLEAR_CONNECTABLE_MODE				0x100000
#endif
#ifdef TWS_MODE_ENABLE
#define BCRF_TWS_MODE_CONTROL					0x200000
#define BCRF_TWS_MODE_CONTROL_RETRY				0x400000
#define BCRF_TWS_SET_DISCOVERABLE_MODE			0x800000
#define BCRF_TWS_ROUTING_MODE_CONTROL			0x1000000
#ifdef TWS_MASTER_SLAVE_GROUPING
#define BCRF_TWS_SET_DEVICE_ID_SAVE				0x2000000
#endif
#endif
#if 0//2023-01-06_1 : Changed SW which send 64 step volume to BAP //defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-05_6 : To send 64 Step volume level under BAP
#define BCRF_SEND_BLE_EXTRA_DATA				0x4000000
#endif

//BT Profile State
#define BT_PROFILE_STATE_INITIALISING			0x01
#define BT_PROFILE_STATE_READY					0x02
#define BT_PROFILE_STATE_CONNECTING				0x04
#define BT_PROFILE_STATE_CONNECTED				0x08
#define BT_PROFILE_STATE_DISCONNECTING			0x10

#ifdef MASTER_SLAVE_GROUPING
#define A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE 		0x01
#endif

//Result Code
#define ERROR_CODE_SUCCESS						0x00
#define ERROR_CODE_FAIL							0x01
#define ERROR_CODE_PARAM_LENGTH_ERROR			0x02
#define ERROR_CODE_INVALID_PARAM				0x03
#define ERROR_CODE_WRONG_STATE					0x04
#define ERROR_CODE_NOT_SUPPORTED				0x05
#define ERROR_CODE_FAIL_UNKNOWN					0xFF

#ifdef SPP_EXTENSION_ENABLE
#define SPP_ERROR_CHECKSUM_ERROR				0xF1
#define SPP_ERROR_INVALID_DATA					0xF2
#define SPP_ERROR_POWER_OFF_STATE				0xF3
#define SPP_ERROR_LENGTH_ERROR					0xF4
#define SPP_ERROR_START_CODE_NG					0xF5
#define SPP_ERROR_INVALID_DATA_UNDER_CUR_STATE				0xF6 //2022-09-22
#define SPP_ERROR_BT_SPK_BUSY					0xF7 //To make invalid key action excepting under Master/Slave Grouping mode. To avoid minor issues(LED Display)
#endif //SPP_EXTENSION_ENABLE

//REMOTE BLE DATA
#define BLE_DATA_0								0x42
#define BLE_DATA_1								0x41
#define BLE_DATA_VENDOR_ID_HIGH_BYTE			0x03
#define BLE_DATA_VENDOR_ID_LOW_BYTE				0x07
#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
#define BLE_DATA_PRODUCT_ID_HIGH_BYTE			0x10
#define BLE_DATA_PRODUCT_ID_LOW_BYTE			0x10
#else
#define BLE_DATA_PRODUCT_ID_HIGH_BYTE			0x00
#define BLE_DATA_PRODUCT_ID_LOW_BYTE			0x00
#endif

#ifdef MASTER_SLAVE_GROUPING
#define BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE			0x10
#define BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE			0x10
#define BLE_DATA_GROUPING_PRODUCT0_ID_HIGH_BYTE			0x20
#define BLE_DATA_GROUPING_PRODUCT0_ID_LOW_BYTE			0x20
#endif

#ifdef SPP_EXTENSION_V50_ENABLE
#define MCU_FW_VERSION_INDICATOR				0xAF
#define BT_FW_VERSION_INDICATOR					0xBF
#endif

typedef enum {
	BLE_POWER_KEY,
	BLE_MUTE_KEY,
	BLE_VOLUME_KEY,
	BLE_EQ_KEY,
	BLE_REBOOT_KEY,
	BLE_FACTORY_RESET_KEY,
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //can't use this with BLE_BT_SHORT_KEY for BLE Com
	BLE_MUTE_OFF_DELAY, //0x00 : Invalid, 0x01 : Mute Off delay, 0x02 : Mute On
#endif
#ifdef SPP_EXTENSION_V44_ENABLE //can't use this with BLE_MUTE_OFF_DELAY for SPP
	BLE_BT_SHORT_KEY = (BLE_FACTORY_RESET_KEY+1), //Master Mode Only - 0x00 : BT short key off, 0x01 : BT short key on
#endif
}BLE_Remocon_Key;

#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-05_6
typedef enum {
	BLE_EXT_VOLUME_DATA,
	//BLE_EXT_ATTENUATOR_DATA,
	BLE_EXT_DATA_END
}BLE_Extra_Data;
#endif

typedef enum {
	REMOTE_POWER_NONE_ACTION,
	REMOTE_POWER_ON_ACTION,
	REMOTE_POWER_OFF_ACTION
}Remote_Power_Key_Action;

//Variable
#ifdef VERSION_INFORMATION_SUPPORT
char MCU_Version[6] = "230609"; //MCU Version Info
#ifdef SPP_EXTENSION_V50_ENABLE
char BT_Version[7]; //MCU Version Info
#endif
#endif

Bool BBT_Init_OK = FALSE;
Bool BSPP_Ready_OK = FALSE;
Bool BBT_Is_Connected = FALSE; //To check whether A2DP(Peer Device) is connected or not
Bool BMaster_Send_BLE_Remote_Data = FALSE; //To avoid init sequence action when BLE data is sent
Bool B_SSP_REBOOT_KEY_In = FALSE; //To send Reboot BLE DATA and execute Reboot
Bool B_SSP_FACTORY_RESET_KEY_In = FALSE; //To send FACTORY RESET KEY and execute it.
Bool BBT_Is_Routed = FALSE; //To check whether BT(excepting Aux) has music steam or not but TWS Mode has some different/wrong value.

#ifdef AVRCP_ENABLE
Bool BBT_Is_Last_Connection_for_AVRCP = FALSE;
Bool BBT_Need_Sequence = FALSE;
#endif
#if defined(BT_DISCONNECT_CONNECTABLE_ENABLE) || defined(BT_CLEAR_CONNECTABLE_MODE)
Bool BKeep_Connectable = FALSE; //To avoid BA_MODE_CONTROL execution by sequnece. When this flag is set, it's disconnected with peerdevice.
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
//Bool BTWS_Master_Slave_Connect = FALSE; //2023-02-21_2 //Just check whether current status is TWS connection or not
TWS_Connect_Status BTWS_Master_Slave_Connect = TWS_Get_Information_Ready;
#endif
#ifdef TWS_MASTER_SLAVE_GROUPING
Bool BTWS_Master_Slave_Grouping = FALSE; //Just check whether current status is TWS master salve grouping or not
#endif
#if defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE)
Bool BSlave_Need_Recovery_Init = FALSE; //Just check whether Slave need Recovery Init(Only first one time, it has I2S Clock) or not //2022-12-20_3
#endif
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
Bool B_BLE_Extra_Data = FALSE; //2023-01-09_1 : Just check wether Slave has been recevied 64 step volume or not. If it has been received 64 step volume, it means slave is BAP-01 and save it to flash.
Bool B_Master_Is_BAP = FALSE; //2023-01-09_2 : To disable BLE_VOLUME_KEY using BLE DATA from Master under BAP slave when Master & Slave are BAP
#endif

#ifdef TWS_MODE_ENABLE
TWS_Status BTWS_LIAC = TWS_Status_Master_Ready;
#endif

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_15 : To make current status of Peer device(A2DP) and TWS Slave device
Peer_Device_Connection_Status Peer_Device_Status = PEER_DEVICE_NONE;
TWS_Slave_Connection_Status TWS_Slave_Status = TWS_SLAVE_NONE_CONNECTION;
#endif

uint8_t uPaired_Device_Count = 0; //First time, we use this to chech if last conection is available and second time, we use this to check if current connection is last connection.
uint8_t uAuto_receive_buf32[32] = {0,};
#ifdef SPP_EXTENSION_V44_ENABLE
uint8_t uSPP_receive_buf8[9] = {0xff,};
#else
uint8_t uSPP_receive_buf8[8] = {0xff,};
#endif

#ifdef SPP_EXTENSION_ENABLE
#ifdef SPP_EXTENSION_V42_ENABLE
uint8_t uCurrent_Status_buf8[15] = {0xff,};
#else //SPP_EXTENSION_V42_ENABLE
uint8_t uCurrent_Status_buf8[8] = {0xff,};
#endif
#endif //SPP_EXTENSION_ENABLE
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
uint8_t uInput_Key_Sync_buf8[9] = {0xff,}; //For Master
#endif
uint8_t uBLE_Remocon_Data[9] = {0xff,}; //For Slave //Added mute off delay for SLAVE SPK
#else //SLAVE_ADD_MUTE_DELAY_ENABLE
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
uint8_t uInput_Key_Sync_buf8[8] = {0xff,};
#endif
uint8_t uBLE_Remocon_Data[8] = {0xff,};
#endif //SLAVE_ADD_MUTE_DELAY_ENABLE
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) 
uint8_t uBLE_Extra_Data[BLE_EXT_DATA_END+1] = {0xff,}; //For Slave //To get/send 64 Step volume level under BAP //2023-01-05_6
#endif

#ifndef MASTER_MODE_ONLY
uint8_t uSlave_BA_MODE_CONTROL_count = 0; //To use 200ms poilling way
#endif

char uBT_Local_Address[6] = {0,}; //BT Local_Addres(USEN Address) : 6Byte
char uBT_Cur_A2DP_Device_Address[6] = {0,}; //Current Connected Device Address from A2DP
char uCur_SPP_Device_Address[6] = {0,}; //Current Connected SPP protocol Device Address
char uLast_Connected_Device_Address[6] = {0,}; //Last connection Device Address from MINOR_ID_GET_PAIRED_DEVICE_LIST
#ifdef TWS_MODE_ENABLE //2022-11-03
char uLast_Connected_TWS_Device_Address[6] = {0,}; //Last connection Device Address from MINOR_ID_GET_PAIRED_DEVICE_LIST
#endif

#ifdef DEVICE_NAME_CHECK_PAIRING
char strUSEN_Device_Name[] = "Lenovo Tab M8";
char strUSEN_Device_Name_1[] = "USEN MUSIC DEVICE";
#ifdef TWS_MODE_ENABLE
char strUSEN_Device_Name_2[] = "USEN MUSIC LINK";
#ifdef NEW_BT_FW_BUG_FIX //2023-02-20_1
char strUSEN_Device_Name_3[] = "MB3021BNU0";
#endif
#endif

char uBT_Remote_Device_Name[32] = {0,}; //BT Remote Device Name : Before connection //Can check if we already get Device name
char uBT_Remote_Device_Address[6] = {0,}; //BT Remote Device Address : Before connection
#ifdef TWS_MODE_ENABLE
char uBT_Cur_TWS_Device_Address[6] = {0,}; //Current Connected Devicde Address from TWS
char uBT_TWS_Remote_Device_Name[32] = {0,}; //BT Remote Device Name : Before connection //Can check if we already get Device name
char uBT_TWS_Remote_Device_Address[6] = {0,}; //TWS Remote Device Address : Before connection
#endif
uint8_t uBT_Remote_Device_Name_Size = 0; //BT Remote Device Name Size : Before connection

Bool bDevice_Paring_Accept = FALSE; //For checking, if Device Pairing Accept is OK : Before connection
#endif

#ifdef USEN_TABLET_AUTO_ON
Bool bIs_USEN_Device = TRUE; //Check if current connected device is USEN MUSIC LINK?
#endif

uint8_t uMode_Change = 0; //To avoid, BT has no output sometime when user alternates Aux mode. This variable just saved previous mode(Aux/BT)

#if defined(FLASH_SELF_WRITE_ERASE) && (defined(GIA_MODE_LED_DISPLAY_ENABLE) || defined(MASTER_SLAVE_GROUPING)) //Save whether Paired Device exist in Flash
uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END];
#endif

uint8_t uLast_Connection_Retry_Count; //For last connection retry - 2 times

//uint8_t uBT_Local_Name[6] = {0,}; //BT  Name : NByte. Need too much space.
static uint32_t bPolling_Get_Data = 0;
static uint32_t bPolling_Get_Data_backup = 0; //Sometimes, Slave can't link with Master because Slave's MCU send BA_SWITCH_MODE (Receiver) to BT Moduel but BT Module can't recognize it. (USEN#43)
static uint16_t bPolling_Get_BT_Profile_State = 0;
#ifdef MASTER_SLAVE_GROUPING
static uint16_t bPolling_Set_Action = 0; //To avoid, INT message missing because of MB3021_BT_Module_Input_Key_Sync_With_Slave()
#endif
#ifdef SPP_EXTENSION_ENABLE														
static uint8_t uSPP_RECEIVE_DATA_ERROR = 0;
#endif
static uint8_t uEQ_Mode = 0;

#ifdef MASTER_SLAVE_GROUPING
static uint8_t uNext_Grouping_State = 0;
static uint8_t uPrev_Grouping_State = 0;
#endif

#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
Bool B_Delete_PDL_by_Factory_Reset = FALSE;
#endif

#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED //2023-04-03_1 : For master mode checking of BAP-01 on factory line, we need to make BSP-01 Slave and it should be worked auto factory reset on disconnection with Master.
static Bool B_Auto_FactoryRST_On = FALSE;
#endif

//Function

//Bool do_recovery - to send same data again which we receive error message
Bool MB3021_BT_Module_CMD_Execute(uint8_t major_id, uint8_t minor_id, uint8_t *data, uint16_t data_length, Bool do_recovery);

Bool MB3021_BT_Module_Check_Valid_Major_Response(uint8_t check_Data);
Bool MB3021_BT_Module_Check_Valid_Minor_Response(uint8_t major_id, uint8_t minor_id);
Bool MB3021_BT_Module_Check_Valid_Major_Indication(uint8_t check_Data);
Bool MB3021_BT_Module_Check_Valid_Minor_Indication(uint8_t major_id, uint8_t minor_id);

static uint16_t SPP_BLE_COM_Calculate_Checksum(uint8_t *data, uint16_t data_length); //Checksum for SPP com and BLE com
static uint8_t MB3021_BT_Module_Calculate_Checksum(uint8_t *data, uint16_t data_length);

static void MB3021_BT_Module_RESP_Error_Code(uint8_t major_id, uint8_t minor_id, uint8_t error_code);

static void MB3021_BT_Module_Receive_Data_RESP(uint8_t major_id, uint8_t minor_id, uint8_t *data, uint16_t data_length);
static void MB3021_BT_Module_Receive_Data_IND(uint8_t major_id, uint8_t minor_id, uint8_t *data, uint16_t data_length);
static void MB3021_BT_Module_Remote_Data_Receive(uint8_t source_type, uint8_t data_channel, uint8_t *data, uint16_t data_length);

uint8_t MB3021_BT_Module_Send_cmd(uint32_t code32); //Just in case of Request Length = 0
uint8_t MB3021_BT_Module_Send_cmd_param(uint32_t code32, uint8_t *param);

void MB3021_BT_Module_Send_Data_Packcet(uint8_t *param, uint16_t size); //size / param - without checksum

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_15 : To make current status of Peer device(A2DP)
void Set_Peer_Device_Status(Peer_Device_Connection_Status Status)
{
	Peer_Device_Status = Status;
}

#ifdef FLASH_SELF_WRITE_ERASE //2023-04-26_8 : To check whether SPK has the history of TWS connection or not
Bool Read_TWS_Connection_From_Flash(void)
{	
	Bool Ret_Val;
#ifdef TWS_MASTER_SLAVE_GROUPING
	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
	
	if(uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff)
		Ret_Val = TRUE; //SPK has TWS connection.
	else
#endif
		Ret_Val = FALSE; //SPK doesn't have TWS connection.

	return Ret_Val;
}
#endif
#endif

#ifdef BT_ALWAYS_GENERAL_MODE //2023-03-13_1 : Just check whether input is BT Long Key or Factory Reset Key
Bool Is_Delete_PDL_by_Factory_Reset(void)
{
	return B_Delete_PDL_by_Factory_Reset;
}
#endif

#ifdef TWS_MASTER_SLAVE_GROUPING //2023-02-20_2
TWS_Grouping_Master_Status Cur_TWS_Grouping_Status = TWS_Grouping_Master_Ready;

void Set_Cur_TWS_Grouping_Status(TWS_Grouping_Master_Status set_status)
{
	Cur_TWS_Grouping_Status = set_status;
}

TWS_Grouping_Master_Status Get_Cur_TWS_Grouping_Status(void)
{
	return Cur_TWS_Grouping_Status;
}

void MASTER_SLAVE_Grouping_Send_SET_DEVICE_ID(Bool B_Send_Again) //2023-02-20_2 : Send SET DEVICE ID two times during Master Slave Grouping
{
	uint8_t random, random1;
	int8_t m,n;
	static uint8_t uBuf[32] = {0,};

#ifdef BT_DEBUG_MSG
	_DBG("\n\rMASTER_SLAVE_Grouping_Send_SET_DEVICE_ID()");
#endif

	/* SEND Data is total 15 byte from BYTE[0] ~ BYTE[14] */
	//BYTE[0] - START CODE of SET_DEVICE_ID
	uBuf[0] = 0xEE;
	
	//BYTE[1] ~ BYTE[6] - Make random value 6 BYTE for SET_DEVICE_ID : Send Two Times because Slave can't receive this data some times
	if(B_Send_Again == FALSE) //First Time
	{	
		random1 = rand()%6;
		
		for(m=1; m<7; m++)
		{
			do {
				if(m!=random1)
					random = rand();
				else
					random = TIMER20_100ms_Count_Value()&0xff;
#ifdef BT_DEBUG_MSG
				_DBG("\n\r random = 0x");
				_DBH(random);
#endif
				if(random == 0x00 || random == 0xFF)
					n = 1;
				else
					n = 0;
			} while(n);

			uBuf[m] = random; //Save random value from buf[0] ~ buf[5] - Total 6EA

			Set_Cur_TWS_Grouping_Status(TWS_Grouping_Master_Send_DeviceID1);
		}
	}
	else //Second Time
	{
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);	

		for(m=0;m<6;m++)
		{
			uBuf[m+1] = uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0+m];
		}
	}
	
	//BYTE[7] - Null(0x00)
	uBuf[7] = 0x00;

	//Save SET_DEVICE_ID(6 BYTE) to flash : BYTE[1] ~ BYTE[6]
	if(B_Send_Again == FALSE)
	{
		for(m=0; m<6; m++)
			FlashSave_SET_DEVICE_ID(FLASH_SAVE_SET_DEVICE_ID_0+m, uBuf[m+1]); //Excepting uBuf[0](Start Code of SET_DEVICE_ID)

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2 : To save current TWS mode under Master(Master - 0x01 or Slave - 0x02)
			FlashSave_SET_DEVICE_ID(FLASH_TWS_MASTER_SLAVE_ID, 0x01);
#endif
	}

	//BYTE[8] - Checksum
	uBuf[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uBuf, 8); //Check sum
	
	//Send CMD SET_DEVICE_ID to Slave
	strncpy((char *)(&uBuf[9]), uBT_Cur_TWS_Device_Address, 6);

#ifdef BT_DEBUG_MSG
	_DBG("\n\r +++ TWS Grouping : Slave Address = ");

	for(m=0; m<6; m++)
		_DBH(uBT_Cur_TWS_Device_Address[m]);
#endif

	if(BTWS_Master_Slave_Connect)
	{
#ifndef TWS_MODE_AP_TEST
		MB3021_BT_Module_Send_cmd_param(CMD_SEND_SYNC_DATA_32, uBuf); //AVRCP COM : Send SET_DEVICE_ID to Slave SPK thru AVRCP - without checksum
#endif
		if(B_Send_Again == TRUE)
		{
			bPolling_Get_Data &= (~BCRF_TWS_SET_DEVICE_ID_SAVE); //Clear flag
		}
	}

#ifdef MASTER_SLAVE_GROUPING
	//TIMER20_Master_Slave_Grouping_flag_Stop(FALSE);
#endif
}
#endif

#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#ifndef MASTER_MODE_ONLY //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
void MB3021_BT_Module_Send_Extra_Data(void) //2023-01-06_1 : Changed SW which send 64 step volume to BAP
{
	uint8_t uExtraData[9];
	
	uExtraData[0] = BLE_DATA_0;
	uExtraData[1] = BLE_DATA_1;
	//To Do !!! Need to check with customer
	uExtraData[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
	uExtraData[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 

#ifdef MASTER_SLAVE_GROUPING
	if(Get_master_slave_grouping_flag() == TRUE) //2023-01-06_3 : To sync volume with BAP-01 Slave under MASTER_SLAVE_GROUPING 
	{
		uExtraData[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
		uExtraData[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
	}
	else
#endif
	{
		uExtraData[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
		uExtraData[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;
	}

	//Making Actual BLE Extra Data : Param1 (64 Step Volume)
	uExtraData[6] = 0xCC; //Start Code
#ifdef TAS5806MD_ENABLE
	uExtraData[7] = TAS5806MD_Amp_Get_Cur_Volume_Level(); //Parameter1
#else
	uExtraData[7] = 0x05;;
#endif
	uExtraData[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uExtraData[6], 2); //Check sum

	BMaster_Send_BLE_Remote_Data = TRUE;
	
	MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0200, uExtraData); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
}
#endif //MASTER_MODE_ONLY

#ifdef ADC_VOLUME_64_STEP_ENABLE
uint8_t Convert_64Step_to_16Step(uint8_t uVol) //To set from 64 Step volume level to 16 step under BAP //2023-01-05_6
{
	uint8_t uMatching_Vol = 0;
	
	if(uVol >= 57)
		uMatching_Vol = 15;
	else if(uVol >= 52)
		uMatching_Vol = 14;
	else if(uVol >= 49)
		uMatching_Vol = 13;
	else if(uVol >= 45)
		uMatching_Vol = 12;
	else if(uVol >= 42)
		uMatching_Vol = 11;
	else if(uVol >= 37)
		uMatching_Vol = 10;
	else if(uVol >= 34)
		uMatching_Vol = 9;
	else if(uVol >= 29)
		uMatching_Vol = 8;
	else if(uVol >= 25)
		uMatching_Vol = 7;
	else if(uVol >= 21)
		uMatching_Vol = 6;
	else if(uVol >= 16)
		uMatching_Vol = 5;
	else if(uVol >= 13)
		uMatching_Vol = 4;
	else if(uVol >= 9)
		uMatching_Vol = 3;
	else if(uVol >= 5)
		uMatching_Vol = 2;
	else
		uMatching_Vol = 1;

	return uMatching_Vol;
}

uint8_t Convert_16Step_to_64Step(uint8_t uVol) //To set from 16 Step volume level to 64 step under BAP //2023-01-09_1
{
	uint8_t uMatching_Vol = 0;
	
	if(uVol >= 15)
		uMatching_Vol = 57;
	else if(uVol >= 14)
		uMatching_Vol = 52;
	else if(uVol >= 13)
		uMatching_Vol = 49;
	else if(uVol >= 12)
		uMatching_Vol = 45;
	else if(uVol >= 11)
		uMatching_Vol = 42;
	else if(uVol >= 10)
		uMatching_Vol = 37;
	else if(uVol >= 9)
		uMatching_Vol = 34;
	else if(uVol >= 8)
		uMatching_Vol = 29;
	else if(uVol >= 7)
		uMatching_Vol = 25;
	else if(uVol >= 6)
		uMatching_Vol = 21;
	else if(uVol >= 5)
		uMatching_Vol = 16;
	else if(uVol >= 4)
		uMatching_Vol = 13;
	else if(uVol >= 3)
		uMatching_Vol = 9;
	else if(uVol >= 2)
		uMatching_Vol = 5;
	else
		uMatching_Vol = 1;

	return uMatching_Vol;
}
#else //ADC_VOLUME_50_STEP_ENABLE //2023-02-27_3
uint8_t Convert_50Step_to_16Step(uint8_t uVol) //2023-02-28_1 : Changed volume table //2023-02-27_3 : To set from 64 Step volume level to 16 step under BAP
{
	uint8_t uMatching_Vol = 0;
	
#ifdef BT_DEBUG_MSG
	_DBG("\n\rConvert_In_Vol = ");_DBD(uVol);
#endif

	if(uVol >= 49)
		uMatching_Vol = 15;
	else if(uVol >= 48)
		uMatching_Vol = 14;
	else if(uVol >= 47)
		uMatching_Vol = 13;
	else if(uVol >= 44)
		uMatching_Vol = 12;
	else if(uVol >= 41)
		uMatching_Vol = 11;
	else if(uVol >= 38)
		uMatching_Vol = 10;
	else if(uVol >= 35)
		uMatching_Vol = 9;
	else if(uVol >= 31)
		uMatching_Vol = 8;
	else if(uVol >= 27)
		uMatching_Vol = 7;
	else if(uVol >= 23)
		uMatching_Vol = 6;
	else if(uVol >= 20)
		uMatching_Vol = 5;
	else if(uVol >= 17)
		uMatching_Vol = 4;
	else if(uVol >= 14)
		uMatching_Vol = 3;
	else if(uVol >= 6)
		uMatching_Vol = 2;
	else 
		uMatching_Vol = 1;

#ifdef BT_DEBUG_MSG
	_DBG("\n\ruMatching_Vol = ");_DBD(uMatching_Vol);
#endif

	return uMatching_Vol;
}

uint8_t Convert_16Step_to_50Step(uint8_t uVol) //2023-02-28_1 : Changed volume table //2023-02-27_3 : To set from 16 Step volume level to 50 step under BAP
{
	uint8_t uMatching_Vol = 0;
	
	if(uVol >= 15)
		uMatching_Vol = 49;
	else if(uVol >= 14)
		uMatching_Vol = 48;
	else if(uVol >= 13)
		uMatching_Vol = 47;
	else if(uVol >= 12)
		uMatching_Vol = 44;
	else if(uVol >= 11)
		uMatching_Vol = 41;
	else if(uVol >= 10)
		uMatching_Vol = 38;
	else if(uVol >= 9)
		uMatching_Vol = 35;
	else if(uVol >= 8)
		uMatching_Vol = 31;
	else if(uVol >= 7)
		uMatching_Vol = 27;
	else if(uVol >= 6)
		uMatching_Vol = 23;
	else if(uVol >= 5)
		uMatching_Vol = 20;
	else if(uVol >= 4)
		uMatching_Vol = 17;
	else if(uVol >= 3)
		uMatching_Vol = 14;
	else if(uVol >= 2)
		uMatching_Vol = 6;
	else
		uMatching_Vol = 1;

#ifdef BT_DEBUG_MSG
	_DBG("\n\rMatching_Vol = ");_DBD(uMatching_Vol);
#endif

	return uMatching_Vol;
}

#endif //ADC_VOLUME_64_STEP_ENABLE
#endif		

void MB3021_BT_Module_Set_Discoverable_Mode_by_Param(Set_Discoverable_Mode_Param uParam) //2023-05-30_2 : To make Limitted Access Code Mode
{
	uint8_t uBuf;
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_Set_Discoverable_Mode_by_Param()");
#endif
	uBuf = uParam;

	switch(uBuf)
	{
		case SET_DISABLE_DISCOVERABLE_MODE:
			uBuf = 0x00; //Disable Discoverable Mode
		break;
		
		case SET_ENABLE_LIMITED_DISCOVERABLE_MODE:
			uBuf = 0x01; //Enable Limitted Access Code Mode
		break;

		case SET_ENABLE_GENERAL_DISCOVERABLE_MODE:
			uBuf = 0x02; //Enable General Discoverable Mode
		break;
		
		case SET_ENABLE_DUAL_DISCOVERABLE_MODE:
			uBuf = 0x03; //Enable Dual Discoverable Mode
		break;
	}

	MB3021_BT_Module_Send_cmd_param(CMD_SET_DISCOVERABLE_MODE_32, &uBuf);
}

#ifdef TWS_MASTER_SLAVE_COM_ENABLE
TWS_Connect_Status Is_TWS_Master_Slave_Connect(void) //TRUE : TWS Mode / FALSE : Not TWS Mode(But it's not Broadcast Mode and may just TWS Mode Ready)
{
	return BTWS_Master_Slave_Connect; //Just check whether current status is TWS connection or not
}
#endif

#ifdef TWS_MODE_ENABLE
#ifdef TWS_MASTER_SLAVE_GROUPING //2022-12-15 //TWS : Send TWS CMD
void MB3021_BT_Module_TWS_Start_Master_Slave_Grouping(void)
{
	bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL;
#ifndef MASTER_MODE_ONLY
	if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
		BTWS_LIAC = TWS_Status_Master_Mode_Control;
}
#endif
void MB3021_BT_Module_Set_Discoverable_Mode(void)
{
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_Set_Discoverable_Mode()");
#endif

	bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE;
}

void MB3021_BT_Module_TWS_Set_Discoverable_Mode(void)
{
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_TWS_Set_Discoverable_Mode()");
#endif

	bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE;
}

void MB3021_BT_Module_TWS_Mode_Exit(void)
{
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_TWS_Mode_Exit()");
#endif	

	bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL_RETRY; 
}

void MB3021_BT_Module_TWS_Mode_Set_Again(void)
{
#ifdef BT_DEBUG_MSG
		_DBG("\n\rMB3021_BT_Module_TWS_Mode_Set_Again()");
#endif	

	bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL; 
}
#endif

void MB3021_BT_Module_Forced_Input_Audio_Path_Setting(void)
{
	bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; 
}

#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && defined(TAS5806MD_ENABLE)
uint8_t Get_Current_EQ_Mode(void)
{
#ifdef MASTER_MODE_ONLY //2023-03-27_4 : Under BAP-01 NORMAL mode, BAP-01 can get only NORMAL MODE.
	if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
	{
		uEQ_Mode = EQ_NORMAL_MODE;
	}
#endif
	return uEQ_Mode;
}
#endif

void Init_uBLE_Remocon_Data(void) //Under Slave and Power off mode, Slave work all CMD thru BLE
{
	int i;
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
	for(i=0;i<9;i++)
	uBLE_Remocon_Data[i] = 0xff;
#else //SLAVE_ADD_MUTE_DELAY_ENABLE
	for(i=0;i<8;i++)
	uBLE_Remocon_Data[i] = 0xff;
#endif //SLAVE_ADD_MUTE_DELAY_ENABLE
}

#ifdef DEVICE_NAME_CHECK_PAIRING
uint16_t SUM_of_Array(char *array)
{
	int i, sum = 0;
	
	for(i=0; i<6; i++)
	{
		sum += array[i];
	}

#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
	_DBG("\n\rSUM_of_Array() :");
	_DBH(sum);
#endif

	return sum;
}
#endif

#ifdef MASTER_SLAVE_GROUPING
#ifdef TWS_MASTER_SLAVE_GROUPING
void MB3021_BT_TWS_Master_Slave_Grouping_Start(void) //2022-12-15 //TWS : TWS Start - TWS Grouping Start
{
	Switch_Master_Slave_Mode mode;

	mode = Get_Cur_Master_Slave_Mode();

#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
	if(Get_Cur_LR_Stereo_Mode() == Switch_Stereo_Mode)
		return;
#endif

#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
	_DBG("\n\rMB3021_BT_TWS_Master_Slave_Grouping_Start()");
#endif
#ifdef FLASH_SELF_WRITE_ERASE
	if(mode == Switch_Slave_Mode)
	{
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

		if(uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff) //we don't to execute this when SET_DEVICE_ID is 0xffffffffffff(6Byte)
		{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rTWS Mode connection is already done. return without execution MB3021_BT_TWS_Master_Slave_Grouping_Start() !!!");
#endif
			return;
		}
	}
#endif
#ifdef TWS_MASTER_SLAVE_GROUPING
	BTWS_Master_Slave_Grouping = TRUE;
#endif

	MB3021_BT_Module_TWS_Start_Master_Slave_Grouping();//MB3021_BT_Module_HW_Reset(); //HW Reset
	
	TIMER20_Master_Slave_Grouping_flag_Start();
}

void MB3021_BT_TWS_Master_Slave_Grouping_Stop(void)
{
#if (defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)) && defined(MASTER_SLAVE_GROUPING_LED_DISPLAY) //To execute MB3021_BT_TWS_Master_Slave_Grouping_Stop under TAS5806MD_ENABLE //2022-10-11_4
	Bool Mute_On;
#endif

#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
	_DBG("\n\rMB3021_BT_TWS_Master_Slave_Grouping_Stop()");
#endif

#ifdef TWS_MASTER_SLAVE_GROUPING
	BTWS_Master_Slave_Grouping = FALSE;
#endif

#if (defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)) && defined(MASTER_SLAVE_GROUPING_LED_DISPLAY) //To execute MB3021_BT_TWS_Master_Slave_Grouping_Stop under TAS5806MD_ENABLE //2022-10-11_4
#ifdef AD82584F_USE_POWER_DOWN_MUTE
	Mute_On = IS_Display_Mute();
#else
#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE)
	Mute_On = Is_Mute(); //AD82584F_Amp_Get_Cur_Mute_Status();
#else //#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE)
	Mute_On = FALSE;
#endif //#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE)
#endif //AD82584F_USE_POWER_DOWN_MUTE

#ifdef LED_DISPLAY_CHANGE
	if(Power_State()) //When SPK is Power Off, we don't need to recovery into previous mode
#endif
	{
		if(Mute_On)
		{
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
		}
		else
		{
			//TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
			Set_Status_LED_Mode(Get_Return_Background_Status_LED_Mode());
		}
	}
#ifdef LED_DISPLAY_CHANGE
	else //When SPK is Power Off, we don't need to recovery into previous mode
	{
		TIMER21_Periodic_Mode_Run(FALSE); //Blinking Timer Off
	}
#endif

	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);	
	
	if(uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff) //In case of valid SET_DEVICE_IDE
	{//TWS Mode Success
		//MB3021_BT_Module_HW_Reset(); //HW Reset
		if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode) //2023-04-04_2 : When SW Reset after TWS connection, some statement should be worked at Master mode only. Because Slave can't reboot sometimes.
		{
			MB3021_BT_Disconnect_All_ACL(); //To avoid last connection fail after reboot //Reboot recovery solution - 1
			delay_ms(1000); //delay for send BLE Data to Slave SPK(Receiver)
		}
		
		SW_Reset();
	}
	else
	{//TWS Mode Fail
#ifdef TWS_MODE_ENABLE
		BTWS_LIAC = TWS_Status_Master_Ready;
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
		BTWS_Master_Slave_Connect = TWS_Get_Information_Ready;
#endif
		MB3021_BT_Module_TWS_Mode_Exit();
	}
#endif //#if defined(AD82584F_ENABLE) && defined(MASTER_SLAVE_GROUPING_LED_DISPLAY)
}

#endif

void MB3021_BT_Master_Slave_Grouping_CMD_Set(uint8_t cmd)
{
	uNext_Grouping_State = cmd;
}

void MB3021_BT_Master_Slave_Grouping_Start(void)
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;

	mode = Get_Cur_Master_Slave_Mode();
#endif

#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
	if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
		return;
#endif

#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
	_DBG("\n\r2. MB3021_BT_Master_Slave_Grouping_Start()");
#endif	
#if defined(FLASH_SELF_WRITE_ERASE) && !defined(MASTER_MODE_ONLY)
	if(mode == Switch_Slave_Mode)
	{
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
#ifdef MASTER_SLAVE_GROUPING_SLAVE_EMPTY
		if(uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x01
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_1
			&& uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x02
#endif
			) //we don't need to execute this when slave doesn't have last connection information. because we already execute this with other way.
		{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rMB3021_BT_Master_Slave_Grouping_Start() : Just return without execution !!!");
#endif
			return;
		}
#else //MASTER_SLAVE_GROUPING_SLAVE_EMPTY
		if(uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] == 0x01
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_1
			|| uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] == 0x02
#endif
			) //we don't to execute this when slave already have last connection information.
		{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
			_DBG("\n\rMB3021_BT_Master_Slave_Grouping_Start() : Just return without execution !!!");
#endif
			return;
		}
#endif //MASTER_SLAVE_GROUPING_SLAVE_EMPTY
	}
#endif

	TIMER20_Master_Slave_Grouping_flag_Start();

#ifdef MASTER_MODE_ONLY
	uNext_Grouping_State = GROUPING_MASTER_NORMAL_MODE;
#else
	if(mode == Switch_Master_Mode)
		uNext_Grouping_State = GROUPING_MASTER_NORMAL_MODE;
	else
		uNext_Grouping_State = GROUPING_SLAVE_NORMAL_MODE;
#endif
}

void MB3021_BT_Master_Slave_Grouping_Stop(void)
{
#if (defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)) && defined(MASTER_SLAVE_GROUPING_LED_DISPLAY) //To execute MB3021_BT_Master_Slave_Grouping_Stop under TAS5806MD_ENABLE //2022-10-11_4
	Bool Mute_On;
#endif
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;

	mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
		_DBG("\n\r99. MB3021_BT_Master_Slave_Grouping_Stop()");
#endif

#ifdef MASTER_MODE_ONLY
	uNext_Grouping_State = GROUPING_MASTER_NORMAL_MODE;
#else
	if(mode == Switch_Master_Mode)
		uNext_Grouping_State = GROUPING_MASTER_NORMAL_MODE;
	else
		uNext_Grouping_State = GROUPING_SLAVE_NORMAL_MODE;
#endif

#if (defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)) && defined(MASTER_SLAVE_GROUPING_LED_DISPLAY) //To execute MB3021_BT_Master_Slave_Grouping_Stop under TAS5806MD_ENABLE //2022-10-11_4
#ifdef AD82584F_USE_POWER_DOWN_MUTE
	Mute_On = IS_Display_Mute();
#else
#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE)
	Mute_On = Is_Mute(); //AD82584F_Amp_Get_Cur_Mute_Status();
#else //#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE)
		Mute_On = FALSE;
#endif //#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE)
#endif //AD82584F_USE_POWER_DOWN_MUTE

#ifdef LED_DISPLAY_CHANGE
	if(Power_State()) //When SPK is Power Off, we don't need to recovery into previous mode
#endif
	{
		if(Mute_On)
		{
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
		}
		else
		{
			//TIMER21_Periodic_Mode_Run(FALSE); //Blinkiing Timer Off
			Set_Status_LED_Mode(Get_Return_Background_Status_LED_Mode());			
		}
	}
#ifdef LED_DISPLAY_CHANGE
	else //When SPK is Power Off, we don't need to recovery into previous mode
	{
		TIMER21_Periodic_Mode_Run(FALSE); //Blinking Timer Off
	}
#endif

#endif //#if defined(AD82584F_ENABLE) && defined(MASTER_SLAVE_GROUPING_LED_DISPLAY)
}
#endif //MASTER_SLAVE_GROUPING

Bool IS_BBT_Init_OK(void) //To avoid key input before BT_init
{
	return BBT_Init_OK;
}

Bool BT_Is_Routed(void)
{
	return BBT_Is_Routed;
}

void MB3021_BT_Module_Value_Init(void)
{
	int i;

	for(i=0;i<32;i++)
	{
		uAuto_receive_buf32[i] = 0;
#ifdef DEVICE_NAME_CHECK_PAIRING
		uBT_Remote_Device_Name[i] =0;
#endif
#ifdef TWS_MODE_ENABLE
		uBT_TWS_Remote_Device_Name[i] = 0;
#endif

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
		if(i < 9)
		{
			uBLE_Remocon_Data[i] = 0xff;
#ifdef SPP_EXTENSION_V44_ENABLE
			if(i == (BLE_EQ_KEY+1)) //2023-02-03_2 : To Delete Mute On/Off action when USEN Tablet send first data
				uSPP_receive_buf8[i] = 0x00;
			else
				uSPP_receive_buf8[i] = 0xff;
#endif
		}
#endif
		if(i< 8)
		{
#ifndef SPP_EXTENSION_V44_ENABLE
			uSPP_receive_buf8[i] = 0xff;
#endif
#ifndef SLAVE_ADD_MUTE_DELAY_ENABLE
			uBLE_Remocon_Data[i] = 0xff;
#endif
			if(i < 6)
			{
				uBT_Local_Address[i] = 0; //BT Local_Addres : 6Byte
				uBT_Cur_A2DP_Device_Address[i] = 0; //Current Connected Device Address
				uLast_Connected_Device_Address[i] = 0;
				uCur_SPP_Device_Address[i] = 0;
#ifdef TWS_MODE_ENABLE
				uBT_TWS_Remote_Device_Address[i] = 0;
				uLast_Connected_TWS_Device_Address[i] = 0;
				uBT_Cur_TWS_Device_Address[i] = 0;
#endif
			}
		}
	}
#ifdef AVRCP_ENABLE
	BBT_Is_Last_Connection_for_AVRCP = FALSE;
	BBT_Need_Sequence = FALSE;
#endif
	BBT_Init_OK = FALSE;
	BSPP_Ready_OK = FALSE;
	BMaster_Send_BLE_Remote_Data = FALSE;
	BBT_Is_Connected = FALSE;
#if defined(BT_DISCONNECT_CONNECTABLE_ENABLE) || defined(BT_CLEAR_CONNECTABLE_MODE)
	BKeep_Connectable = FALSE;
#endif
#if defined(SPP_EXTENSION_ENABLE) && defined(TAS5806MD_ENABLE)
	uEQ_Mode = EQ_NORMAL_MODE; //Normal Mode
#endif
#ifdef MASTER_SLAVE_GROUPING
	uNext_Grouping_State = 0;
	uPrev_Grouping_State = 0;
#endif
	bPolling_Get_Data = 0;
	bPolling_Get_Data_backup = 0;

	bPolling_Get_BT_Profile_State = 0;
	uPaired_Device_Count = 0;
#ifdef MASTER_SLAVE_GROUPING
	bPolling_Set_Action =0;
#endif
#ifdef DEVICE_NAME_CHECK_PAIRING
	uBT_Remote_Device_Name_Size = 0;
#endif
#ifdef USEN_TABLET_AUTO_ON
	bIs_USEN_Device = FALSE;
#endif
#ifdef TWS_MODE_ENABLE
	BTWS_LIAC = TWS_Status_Master_Ready;
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
	BTWS_Master_Slave_Connect = TWS_Get_Information_Ready;
#endif
	uLast_Connection_Retry_Count = 2;
#ifndef MASTER_MODE_ONLY
	uSlave_BA_MODE_CONTROL_count = 0;
#endif
#ifdef TWS_MASTER_SLAVE_GROUPING
	BTWS_Master_Slave_Grouping = FALSE;
#endif
#if defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE)
	BSlave_Need_Recovery_Init = FALSE; //2022-12-20_3
#endif
#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 
	B_Delete_PDL_by_Factory_Reset = FALSE;
#endif
#ifdef USEN_BAP
	B_Master_Is_BAP = FALSE; //2023-02-28_2 : Need to init to check whether BAP-01 Slave is connected with BAP-01 Master
#endif
#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
	B_Auto_FactoryRST_On = FALSE; //2023-04-03_1
#endif
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_15 : To make current status of Peer device(A2DP) and TWS Slave device
	Peer_Device_Status = PEER_DEVICE_NONE;
	TWS_Slave_Status = TWS_SLAVE_NONE_CONNECTION;
#endif

}

void MB3021_BT_Module_HW_Reset(void)
{
#ifdef USING_REFERENCE_FLATFORM
	/* PE0 Output - MODULE_RESET */
	HAL_GPIO_ClearPin(PE, _BIT(0));
	delay_ms(500);
	HAL_GPIO_SetPin(PE, _BIT(0));
	delay_ms(500);
#else //USING_REFERENCE_FLATFORM
	/* PC0 Output - MODULE_RESET */
	HAL_GPIO_ClearPin(PC, _BIT(0));
	delay_ms(500);
	HAL_GPIO_SetPin(PC, _BIT(0));
	//delay_ms(500);
#endif //USING_REFERENCE_FLATFORM
}

#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
void MB3021_BT_Module_Input_Key_Init(void) //To Do!!! - Need to change later using Flash data
{
#ifdef BT_DEBUG_MSG
	int i;

	_DBG("\n\rMB3021_BT_Module_Input_Key_Init()");
#endif

#ifdef FLASH_SELF_WRITE_ERASE //Set EQ Setting after Power On
	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);					
#endif
	uInput_Key_Sync_buf8[0] = 0xAA; //Start Code
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uInput_Key_Sync_buf8[1] = uFlash_Read_Buf[FLASH_SAVE_DATA_POWER]; //Power On/Off Button

	if(uInput_Key_Sync_buf8[1] == 0xff)
		uInput_Key_Sync_buf8[1] = 1; //Power On
		
	uInput_Key_Sync_buf8[2] = uFlash_Read_Buf[FLASH_SAVE_DATA_MUTE]; //Mute On/Off Button

	if(uInput_Key_Sync_buf8[2] == 0xff)
		uInput_Key_Sync_buf8[2] = 0; //Mute Off
#else
	uInput_Key_Sync_buf8[1] = 0x01; //Power On
	uInput_Key_Sync_buf8[2] = 0x00; //Mute On/Off Button
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
	uInput_Key_Sync_buf8[3] = AD82584F_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level
#else //AD82584F_ENABLE
#ifdef USEN_BAP //Save is working in inverse when BAP-01 & Slave is changed "Power ON" //2023-01-02_1
	uInput_Key_Sync_buf8[3] = TAS5806MD_Amp_Get_Cur_Volume_Level(); //Volume Level
#else
	uInput_Key_Sync_buf8[3] = TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level
#endif
#endif //TAS5806MD_ENABLE
#else //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
	uInput_Key_Sync_buf8[3] = 0x05;
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
	uInput_Key_Sync_buf8[4] = uFlash_Read_Buf[FLASH_SAVE_DATA_EQ]; //EQ
	
	if(uInput_Key_Sync_buf8[4] == 0xff)
		uInput_Key_Sync_buf8[4] = 0; //EQ Normal
#else
	uInput_Key_Sync_buf8[4] = 0x00; //EQ
#endif
	uInput_Key_Sync_buf8[5] = 0x00; //Reboot On/Off Button
	uInput_Key_Sync_buf8[6] = 0x00; //Factory Reset On/Off Button
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
	uInput_Key_Sync_buf8[7] = 0x00; //Slave mute off delay - Off
	uInput_Key_Sync_buf8[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uInput_Key_Sync_buf8, 8); //Check sum
#else
	uInput_Key_Sync_buf8[7] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uInput_Key_Sync_buf8, 7); //Check sum
#endif

#ifdef BT_DEBUG_MSG
	_DBG("\n\r");
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
	for(i=0; i<9;i++)
#else
	for(i=0; i<8;i++)
#endif
	{
		//_DBH(uInput_Key_Sync_buf8[i]);
	}
#endif
	//MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, uInput_Key_Sync_buf8[3]); //Call afater init finishing
}

#ifdef SPP_CMD_AND_MASTER_INFO_SEND
void Send_Cur_Master_Info_To_Tablet(void)
{
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //To set 64 Step volume level under BAP //2023-01-05_6
	uint8_t uVol_Level = 0;
#endif

	uCurrent_Status_buf8[0] = 0xBB; 
	
	if(Power_State() == TRUE)
		uCurrent_Status_buf8[1] = 0x01; //Power On/Off : Power On mode
	else
		uCurrent_Status_buf8[1] = 0x00; //Power On/Off : Power Off mode

#ifdef AD82584F_USE_POWER_DOWN_MUTE
	if(IS_Display_Mute() == TRUE) //When Mute On status, we don't need to mute off. This function is for LED Display
		uCurrent_Status_buf8[2] = 0x01; //Mute On/Off : Mute On mode
	else
	{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION //When Power On thru App after Power Plug Off/On, IS_Display_Mute() is always FALSE even though Mute On. So, we need to use Flash data to keep Mute On. 2022-09-20
		if(!Power_State())
		{
			uint8_t uRead_Buf[FLASH_SAVE_DATA_END];

			Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
		
			if(uRead_Buf[FLASH_SAVE_DATA_MUTE])
				uCurrent_Status_buf8[2] = 0x01; //Mute On/Off : Mute On mode
			else
				uCurrent_Status_buf8[2] = 0x00; //Mute On/Off : Mute Off mode
		}
		else
#endif
			uCurrent_Status_buf8[2] = 0x00; //Mute On/Off : Mute Off mode
	}
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-06_2 : Need to send 16 step volume to Tablet under BAP
	uVol_Level = TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-27_3
	uCurrent_Status_buf8[3] = Convert_64Step_to_16Step(uVol_Level);
#else //ADC_VOLUME_50_STEP_ENABLE
	uCurrent_Status_buf8[3] = Convert_50Step_to_16Step(uVol_Level);
#endif //ADC_VOLUME_64_STEP_ENABLE
#else //defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#ifdef AD82584F_ENABLE
	uCurrent_Status_buf8[3] = AD82584F_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level
#else //AD82584F_ENABLE
	uCurrent_Status_buf8[3] = TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level
#endif //TAS5806MD_ENABLE
#endif //defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#else //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
	uCurrent_Status_buf8[3] = 0x05; //Volume Level
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef MASTER_MODE_ONLY //2023-03-28_3 : Send current information to USEN Tablet when user changes SW2_KEY(EQ NORMAL/EQ BSP) due to changed spec
	if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
	{
		uEQ_Mode = EQ_NORMAL_MODE;
	}
#endif
	uCurrent_Status_buf8[4] = uEQ_Mode; //Sound EQ mode
	uCurrent_Status_buf8[5] = 0x00; //Reboot Off
	uCurrent_Status_buf8[6] = 0x00; //Factory Reset Off
#ifdef SPP_EXTENSION_V42_ENABLE
#ifdef USEN_BAP //2023-03-02_1 : Send BAP-01 Name to USEN Tablet
	uCurrent_Status_buf8[7] = 0x42;
	uCurrent_Status_buf8[8] = 0x41;
	uCurrent_Status_buf8[9] = 0x50;
	uCurrent_Status_buf8[10] = 0x2D;
	uCurrent_Status_buf8[11] = 0x30;
	uCurrent_Status_buf8[12] = 0x31;
#ifdef MASTER_MODE_ONLY //2023-03-28_4 : Send current SW2_KEY status whether it's EQ NORMAL or EQ BSP due to chagned spec under BAP-01
	if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
		uCurrent_Status_buf8[13] = 0x01; //EQ NORMAL
	else
		uCurrent_Status_buf8[13] = 0x02; //EQ BSP
#else
	uCurrent_Status_buf8[13] = 0x20;
#endif //#ifdef MASTER_MODE_ONLY
#else //USEN_BAP
	uCurrent_Status_buf8[7] = 0x42;
	uCurrent_Status_buf8[8] = 0x53;
	uCurrent_Status_buf8[9] = 0x50;
	uCurrent_Status_buf8[10] = 0x2D;
	uCurrent_Status_buf8[11] = 0x30;
	uCurrent_Status_buf8[12] = 0x31;
	uCurrent_Status_buf8[13] = 0x42;
#endif //USEN_BAP
	uCurrent_Status_buf8[14] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 14);
#else //SPP_EXTENSION_V42_ENABLE
	uCurrent_Status_buf8[7] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 7);
#endif //SPP_EXTENSION_V42_ENABLE

	bPolling_Get_Data |= BCRF_SEND_SPP_DATA_RESP; //Send SPP Response NG
}
#endif

void MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_Key_Sync_With_Slave Input_Key, uint8_t uValue) //Only Available under Master Mode
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;
#endif
	static uint8_t uBuf[32] = {0,};
	int i;

#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_Input_Key_Sync_With_Slave() - Start");
#endif
#ifndef MASTER_MODE_ONLY
	mode = Get_Cur_Master_Slave_Mode();
#endif

#ifndef MASTER_MODE_ONLY
	if(mode != Switch_Master_Mode)
		return;
#endif

#if defined(MASTER_SLAVE_GROUPING) && defined(SLAVE_ADD_MUTE_DELAY_ENABLE)
	if(uNext_Grouping_State > GROUPING_EVENT_WAIT_STATE && uNext_Grouping_State != GROUPING_MASTER_SET_MANUFACTURE_DATA && Input_Key == Input_key_Sync_Slave_Mute_Off) //To avoid Slave pop-up upon Maste/Slave pairing
		return;
#endif

#if defined(MB3021_ENABLE) && defined(USEN_BAP) //2023-02-28_3(BAP-01 Issue #14) : Under BAP-01 Master Mode, if user changed rotary volume position very fast many times before BT init during Power on, Last connection is almost failed.
	if(!IS_BBT_Init_OK())
		return;
#endif

#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
		_DBG("\n\r");_DBH(uPrev_Grouping_State); _DBG("_");_DBH(uNext_Grouping_State);
		if(Input_Key == Input_key_Sync_Slave_Mute_Off)
		{
			_DBG(":");_DBH(uValue); 
		}
#endif
#ifdef BT_DEBUG_MSG	
	_DBG("\n\rMB3021_BT_Module_Input_Key_Sync_With_Slave() - End");
#endif

#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //To set 64 Step volume level under BAP //2023-01-05_6
	if(Input_Key == input_key_Sync_Volume)
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-27_3
	uInput_Key_Sync_buf8[Input_Key+1] = Convert_64Step_to_16Step(uValue);
#else //ADC_VOLUME_50_STEP_ENABLE
	uInput_Key_Sync_buf8[Input_Key+1] = Convert_50Step_to_16Step(uValue);
#endif //ADC_VOLUME_64_STEP_ENABLE
	else
#endif		
	uInput_Key_Sync_buf8[Input_Key+1] = uValue; //+1 for Start Code(0xAA)
	
	//When Mute Off, we need to set Input_key_Sync_Slave_Mute_Off value into 0x01 or 0x00.
	//For example, unless this if statement, When user press BT Key(for grouping) on just master Slave always mute state even though use press mute off.
	if(Input_Key == input_key_Sync_Mute && uValue == 0x00)
	{
		uInput_Key_Sync_buf8[Input_key_Sync_Slave_Mute_Off+1] = 0x01;
	}
	
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
	uInput_Key_Sync_buf8[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uInput_Key_Sync_buf8, 8); //Check sum
#else //INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	uInput_Key_Sync_buf8[7] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uInput_Key_Sync_buf8, 7); //Check sum
#endif //INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
	
	//Configure Send Data
#ifndef TWS_MODE_AP_TEST	
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) && defined(TWS_MASTER_SLAVE_COM_ENABLE)//2022-11-07
	if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
	{
		if(BTWS_Master_Slave_Connect)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rBCRF_SEND_SLAVE_DATA !!! TWS Address 1 : ");
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
			for(i=0; i<9; i++)
#else
			for(i=0; i<8; i++)
#endif
			{
				uBuf[i] = uInput_Key_Sync_buf8[i];
#ifdef SPP_CMD_AND_USER_INPUT_KEY_SYNC
				if(i<7)
					uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i];
#endif
#if 0//def BT_DEBUG_MSG
				_DBH(uInput_Key_Sync_buf8[i]);
#endif
			}

			strncpy((char *)(&uBuf[i]), uBT_Cur_TWS_Device_Address, 6);
			
			MB3021_BT_Module_Send_cmd_param(CMD_SEND_SYNC_DATA_32, uBuf); //AVRCP COM : Send SPP data to Slave SPK thru AVRCP - without checksum
		}
	}
	else
#endif
	{
		uBuf[0] = BLE_DATA_0;
		uBuf[1] = BLE_DATA_1;
		//To Do !!! Need to check with customer
		uBuf[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
		uBuf[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 
#ifdef MASTER_SLAVE_GROUPING
		if(Get_master_slave_grouping_flag() == TRUE)
		{
			uBuf[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
			uBuf[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
		}
		else
#endif
		{
			uBuf[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
			uBuf[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;
		}

#ifdef BT_DEBUG_MSG
		_DBG("\n\r");
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
		for(i=0; i<9; i++)
#else
		for(i=0; i<8; i++)
#endif
		{
			uBuf[i+6] = uInput_Key_Sync_buf8[i];
#ifdef SPP_CMD_AND_USER_INPUT_KEY_SYNC
			if(i<7)
				uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i];
#endif
#ifdef BT_DEBUG_MSG
			_DBH(uInput_Key_Sync_buf8[i]);
#endif
		}

		BMaster_Send_BLE_Remote_Data = TRUE;

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
		MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#else
		MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0700, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#endif
#ifndef MASTER_MODE_ONLY  //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-05_6 : To set 64 Step volume level under BAP when BAP receives SPP data from peerdevice
		//bPolling_Get_Data |= BCRF_SEND_BLE_EXTRA_DATA; //defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-05_6 : To set 64 Step volume level under BAP when BAP receives SPP data from peerdevice
		if(uNext_Grouping_State == GROUPING_NONE_MODE) //Excepting GROUPING //2023-01-06_1
			TIMER20_BT_send_extra_data_flag_start(); //2023-01-05_6 : Changed SW which send 64 step volume to BAP
#endif
#endif
	}
#endif //TWS_MODE_AP_TEST
#ifdef SPP_CMD_AND_MASTER_INFO_SEND
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //When Master sends Input_key_Sync_Slave_Mute_off to Slave, Master do not need send it to Tablet.
	if(Input_Key != Input_key_Sync_Slave_Mute_Off)
#endif
	Send_Cur_Master_Info_To_Tablet();
#endif
}
#endif

void MB3021_BT_Disconnect_All_ACL(void)
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;
#endif
	uint8_t uBuf[7] = {0,};
	int i;
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Disconnect_All_ACL()");
#endif
#ifndef MASTER_MODE_ONLY
	mode = Get_Cur_Master_Slave_Mode();
	
	if(mode == Switch_Master_Mode)
#endif
	{	
		uBuf[0] = 0xff; //Data[0] - 0x0 : Disconnect ACL, 0x1 : Connect ACL, 0xff : Disconnect All ACL
		
		for(i=0;i<6;i++)
			uBuf[i+1] = uLast_Connected_Device_Address[i]; //Data[0~5] - Peer Device Address			
		
		//uBuf[6] = 0xff; //Data[0] - 0x0 : Disconnect ACL, 0x1 : Connect ACL, 0xff : Disconnect All ACL
#if 0//def BT_DEBUG_MSG	
		_DBH(uBuf[0] );
#endif

		MB3021_BT_Module_Send_cmd_param(CMD_CONTROL_ACL_CONNECTION_32, uBuf);		
	}
}

void MB3021_BT_Disconnect_ACL(uint8_t *Addr) //2023-03-09_2 : Disconnect with specific peer device but it's not all
{
	uint8_t uBuf[7] = {0,};
	int i;
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Disconnect_ACL()");
#endif

	uBuf[0] = 0x00; //Data[0] - 0x0 : Disconnect ACL, 0x1 : Connect ACL, 0xff : Disconnect All ACL
	
	for(i=0;i<6;i++)
		uBuf[i+1] = Addr[i]; //Data[0~5] - Peer Device Address			
	
#ifdef BT_DEBUG_MSG
	for(i=0;i<7;i++)
	_DBH(uBuf[i]);
#endif

	MB3021_BT_Module_Send_cmd_param(CMD_CONTROL_ACL_CONNECTION_32, uBuf);		
}


#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
void MB3021_BT_Delete_Paired_List_All(Bool Factory_Reset)
#else //BT_ALWAYS_GENERAL_MODE
void MB3021_BT_Delete_Paired_List_All(void)
#endif //BT_ALWAYS_GENERAL_MODE
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;
#endif
	uint8_t uBuf;
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Delete_Paired_List_All()");
#endif
#ifndef MASTER_MODE_ONLY
	mode = Get_Cur_Master_Slave_Mode();
	
	if(mode == Switch_Master_Mode)
		uBuf = 0xFF; //Delete All Device In Paired Device List
	else
		uBuf = 0xFD; //Added new spec under slave mode(Delete Broadcast Device Address)
#else
	uBuf = 0xFF; //Delete All Device In Paired Device List
#endif
#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
	B_Delete_PDL_by_Factory_Reset = Factory_Reset;
#endif

#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2
	//To recovery Factory Reset CMD
	TIMER20_factory_reset_cmd_recovery_flag_start();
#else
	//To recovery Factory Reset CMD
	TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_delete_paired_list);
#endif
	
	MB3021_BT_Module_Send_cmd_param(CMD_DELETE_PAIRED_DEVICE_LIST_32, &uBuf);		
}

void MB3021_BT_Read_FW_Version(void)
{
	MB3021_BT_Module_Send_cmd(CMD_READ_FW_VERSION_32);	
}

void MB3021_BT_A2DP_Connection_Control(Bool Connect) //TRUE : connect / FALSE : disconnect
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;
#endif
	uint8_t uBuf[7] = {0,};
	int i;

#ifdef BT_DEBUG_MSG	
	_DBG("\n\rMB3021_BT_A2DP_Connection_Control() : connect =");
	_DBH(Connect);
#endif
#ifndef MASTER_MODE_ONLY
	mode = Get_Cur_Master_Slave_Mode();

	if(mode == Switch_Master_Mode)
#endif
	{
		if(Connect) //Data[0] - 0x0 : Disconnect A2DP, 0x1 : Connect A2DP, 0x2 : Connect A2DP Connection
			uBuf[0] = 0x01; 
		else
			uBuf[0] = 0x00; 
		
		for(i=0;i<6;i++)
			uBuf[i+1] = uBT_Cur_A2DP_Device_Address[i]; //Data[1~6] - Peer Device Address 			

#if 0//def BT_DEBUG_MSG	
		_DBH(uBuf[0] );
#endif
		MB3021_BT_Module_Send_cmd_param(CMD_A2DP_CONNECTION_CONTROL_32, uBuf);
	}
}

void MB3021_BT_A2DP_Routing_Control(Bool Route) //TRUE : Route / FALSE : Unroute
{
#ifndef MASTER_MODE_ONLY
	Switch_Master_Slave_Mode mode;
#endif
	uint8_t uBuf[7] = {0,};
	int i;

#ifdef BT_DEBUG_MSG	
	_DBG("\n\rMB3021_BT_A2DP_Routing_Control() : Route =");
	_DBH(Route);
#endif

#ifndef MASTER_MODE_ONLY
	mode = Get_Cur_Master_Slave_Mode();

	if(mode == Switch_Master_Mode)
#endif
	{
		if(Route) //Data[0] - 0x0 : Disconnect A2DP, 0x1 : Connect A2DP, 0x2 : Connect A2DP Connection
			uBuf[0] = 0x01; 
		else
			uBuf[0] = 0x00; 
		
		for(i=0;i<6;i++)
			uBuf[i+1] = uBT_Cur_A2DP_Device_Address[i]; //Data[1~6] - Peer Device Address 			

#if 0//def BT_DEBUG_MSG
		_DBH(uBuf[0] );
#endif
		MB3021_BT_Module_Send_cmd_param(CMD_A2DP_ROUTING_CONTROL_32, uBuf);
	}
}

void MB3021_BT_Module_Init(Bool Factory_Reset) //No need BT module Init. Just check ID
{
	int i;
	
#ifdef TWS_MODE_ENABLE
	TIMER20_tws_mode_recovery_flag_Stop();
#endif
#ifndef LR_360_FACTORY_ENABLE //2023-04-06_2 : When we use this macro, the factory reset clear function is moved to other place which gets MINOR_ID_DELETE_PAIRED_DEVICE_LIST.
	TIMER20_factory_reset_cmd_recovery_flag_stop(); //To Clear Factory Reset recovery for factory_reset_delete_paired_list  becasuse SPK get response here.
#endif
#ifdef FIVE_USER_EQ_ENABLE
	TIMER20_user_eq_mute_flag_stop(); //To avoid noise when user tries eq mode change - 300ms mute
#endif
	TIMER20_Forced_Input_Audio_Path_Setting_flag_stop();

	MB3021_BT_Module_Value_Init(); //Variable Init	

	if(Factory_Reset) // To clear Factory Reset Value, when user select factory reset over Remote App : Start
	{
#ifdef FLASH_SELF_WRITE_ERASE //A Factory Reset already clears the Flash Data which are FLASH_SAVE_SLAVE_LAST_CONNECTION and FLASH_SAVE_DATA_PDL_NUM in "case MINOR_ID_DELETE_PAIRED_DEVICE_LIST: //0x00 : 0x0B"
		FlashEraseOnly();
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
		AD82584F_Amp_Set_Default_Volume(); //Set Default Volume //This function should be called before "uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i]" and MB3021_BT_Module_Input_Key_Init() because Master must send correct vol_level after factory reset
#else //AD82584F_ENABLE
		TAS5806MD_Amp_Set_Default_Volume(); //Set Default Volume //This function should be called before "uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i]" and MB3021_BT_Module_Input_Key_Init() because Master must send correct vol_level after factory reset
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
		MB3021_BT_Module_Input_Key_Init();
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
		for(i=0; i<9; i++)
#else
		for(i=0; i<8; i++)
#endif
		{
#ifdef SPP_CMD_AND_USER_INPUT_KEY_SYNC
			if(i<7)
				uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i];
#endif
		}
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
		AD82584F_Amp_Mute(FALSE, TRUE); //Mute release after init
		AD82584F_Amp_Mute(TRUE, FALSE); //Mute release after init
#else //AD82584F_ENABLE
#ifdef AD82584F_USE_POWER_DOWN_MUTE //2023-03-08_4
		Set_Display_Mute(FALSE);
#else
		TAS5806MD_Amp_Mute(FALSE, TRUE); //Mute release after init
		TAS5806MD_Amp_Mute(TRUE, FALSE); //Mute release after init
#endif
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)

#ifdef _DBG_FLASH_WRITE_ERASE
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

		for(i=0;i<FLASH_SAVE_DATA_END;i++)
			_DBH(uFlash_Read_Buf[i]);
#endif
		TIMER20_auto_power_flag_Stop();
	}
	
#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2 : To separate factory reset and BT HW reset which is action by Factory Reset. 
	TIMER20_BT_hw_reset_cmd_recovery_flag_start(); //To recover, Firmware version is missed
#else
	TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_firmware_version); //To recover, Firmware version is missed
#endif
	TIMER21_Periodic_Mode_Run(FALSE); //Blinking Timer Off
	TIMER20_auto_power_flag_Start();
#ifdef SLAVE_AUTO_OFF_ENABLE
	TIMER20_Slave_auto_power_off_flag_Start();
#endif
	MB3021_BT_Module_HW_Reset(); //HW Reset
#ifdef SOC_ERROR_ALARM
	TIMER20_uart_error_flag_Start();
#endif
#ifdef MASTER_SLAVE_GROUPING 
	TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop();
	TIMER20_Master_Slave_Grouping_flag_Stop(TRUE);
#endif
}

#if defined(USEN_BAP) && defined(MASTER_MODE_ONLY) //2023-05-09_1
void Set_MB3021_BT_Module_Source_Change_Direct(void)
{
	static uint8_t uBuf[2] = {0,};
#if 0//def BT_DEBUG_MSG	
	_DBG("\n\rSet_MB3021_BT_Module_Source_Change_Direct()");
#endif
	if(BBT_Init_OK) //The Source change is only available when BT Init is finished
	{
#ifdef AUX_INPUT_DET_ENABLE
		if(Aux_In_Exist())
		{
			uBuf[0] = 0x50; //Aux Mode
#ifdef TIMER21_LED_ENABLE
			Set_Status_LED_Mode(STATUS_AUX_MODE);
#endif
#if 0//def BT_DEBUG_MSG	
			_DBG("AUX Mode");
#endif
		}
		else
#endif //AUX_INPUT_DET_ENABLE
		{
#ifdef TIMER21_LED_ENABLE
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
#endif
			uBuf[0] = 0x07; //Bluetooth Mode
#if 0//def BT_DEBUG_MSG	
			_DBG("BT Mode");
#endif
		}

#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE) //To avoid, BT has no output sometime when user alternates Aux mode / BT mode repeately
		if(uMode_Change == uBuf[0] 
#ifdef AD82584F_USE_POWER_DOWN_MUTE
			&& !IS_Display_Mute()
#else
			&& Is_Mute()
#endif
			)
		{
#ifdef AD82584F_ENABLE
			if(AD82584F_Amp_Get_Cur_CLK_Status())
#else //TAS5806MD_ENABLE
			if(TAS5806MD_Amp_Detect_FS(FALSE)) //2022-10-17_2
#endif //AD82584F_ENABLE
			{
				if(uMode_Change == 0x07 || uMode_Change == 0x50) //BT Mode or Aux Mode
				{
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r Mute Off : To avoid, BT has no output sometime when user alternates Aux mode / BT mode repeately");
#endif
#ifdef AUTO_ONOFF_ENABLE
					TIMER20_auto_power_flag_Stop();
#endif
#ifndef USEN_BAP //2023-05-09_1 : Reduced the checking time from 5.3s to 3s and no need mute under BAP-01
				    TIMER20_mute_flag_Start();
#endif
				}
			}
		}
		else
			uMode_Change = uBuf[0];
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)

		MB3021_BT_Module_Send_cmd_param(CMD_INFORM_HOST_MODE_32, uBuf);
#ifdef SWITCH_BUTTON_KEY_ENABLE
		bFactory_Reset_Mode = FALSE;
#endif
	}
}
#endif

void Set_MB3021_BT_Module_Source_Change(void)
{
	bPolling_Get_Data |= BCRF_INFORM_HOST_MODE;
}

static uint8_t MB3021_BT_Module_Calculate_Checksum(uint8_t *data, uint16_t data_length) //Checksum for MCS Logic Protocol
{
	uint8_t checksum = 0;
	uint32_t i = 0;

	for(i=0;i<data_length;i++)
		checksum ^= data[i];
	
#if 0//def BT_DEBUG_MSG
	_DBG("\n\rChecksum : ");
	_DBH(checksum);
#endif

	return checksum;
}

static uint16_t SPP_BLE_COM_Calculate_Checksum(uint8_t *data, uint16_t data_length) //Checksum for SPP com and BLE com
{
	uint8_t checksum = 0x00;
	uint32_t i = 0;

	for(i=0;i<data_length;i++)
		checksum += data[i];

	checksum = 0x100 - (checksum & 0xff);

	return checksum;
}


Bool Get_Connection_State(void) //220217
{
	return BBT_Is_Connected;
}

Bool Is_SSP_REBOOT_KEY_In(void)
{
	return B_SSP_REBOOT_KEY_In;
}

Bool Is_SSP_FACTORY_RESET_KEY_In(void)
{
	return B_SSP_FACTORY_RESET_KEY_In;
}

Bool MB3021_BT_Module_Check_Valid_Major_Response(uint8_t check_Data)
{
	Bool ret = TRUE;
	
	switch(check_Data)
	{
		case MAJOR_ID_GENERAL_CONTROL: //0x00
		case MAJOR_ID_QUALIFICATION_COMMANDS: //0x01
		case MAJOR_ID_A2DP_CONTROL: //0x10
		case MAJOR_ID_AVRCP_CONTROL: //0x11
		case MAJOR_ID_HFP_CONTROL: //0x12
		case MAJOR_ID_SPP_CONTROL: //0x13
		case MAJOR_ID_TWS_CONTROL: //0x15
		case MAJOR_ID_BA_CONTROL: //0x16
		case MAJOR_ID_BLE_CONTROL: //0x17
		case MAJOR_ID_UPGRADE_CONTROL: //0x1B
		break;

		default:
			ret = FALSE;
			break;
	}

	return ret;
}

Bool MB3021_BT_Module_Check_Valid_Minor_Response(uint8_t major_id, uint8_t minor_id)
{
	Bool ret = TRUE;
	
	switch(major_id)
	{
		case MAJOR_ID_GENERAL_CONTROL: //0x00
		{
			switch(minor_id)
			{
				case MINOR_ID_SET_MODEL_NAME: //0x00 : 0x02
				case MINOR_ID_SET_DEVICE_ID: //0x00 : 0x03			
				case MINOR_ID_INIT_SINK_MODE: //0x00 : 0x05				
				case MINOR_ID_SET_DISCOVERABLE_MODE: //0x00 : 0x07
				case MINOR_ID_SET_CONNECTABLE_MODE: //0x00 : 0x08	
				case MINOR_ID_GET_PAIRED_DEVICE_LIST: //0x00 : 0x0A
				case MINOR_ID_DELETE_PAIRED_DEVICE_LIST: //0x00 : 0x0B
				case MINOR_ID_MODULE_POWER_CONTROL: //0x00 : 0x11
				case MINOR_ID_INFORM_HOST_MODE: //0x00 : 0x17
#ifdef DEVICE_NAME_CHECK_PAIRING
				case MINOR_ID_SET_IOCAPABILITY_MODE: //0x00 : 0x1F
#endif
					break;
				
				default:
					ret = FALSE;
					break;
			}
		}
		break;

		case MAJOR_ID_A2DP_CONTROL: //0x10
		{
			switch(minor_id)
			{
				case MINOR_ID_A2DP_CONNECTION_CONTROL: //0x10 : 0x00
					break;
				case MINOR_ID_A2DP_ROUTING_CONTROL: //0x10 : 0x02
					break;
				default:
					ret = FALSE;
					break;
			}
		}
		break;
		

		case MAJOR_ID_AVRCP_CONTROL: //0x11
		{
#if defined(AVRCP_ENABLE) || defined(TWS_MASTER_SLAVE_COM_ENABLE)
			switch(minor_id)
			{
#ifdef AVRCP_ENABLE
				case MINOR_ID_AVRCP_CONNECTION_CONTROL: //0x11 : 0x00
					break;
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
				case MINOR_ID_SEND_SYNC_DATA: //0x11 : 0x13
					break;
#endif
				default:
					ret = FALSE;
					break;
			}
#endif
		}
		break;

#ifdef TWS_MODE_ENABLE
		case MAJOR_ID_TWS_CONTROL: //0x15
		{
			switch(minor_id)
			{
				case MINOR_ID_TWS_MODE_CONTROL: //0x15 : 0x00
					break;
				
				default:
					ret = FALSE;
					break;
			}
		}
		break;
#endif

		case MAJOR_ID_BA_CONTROL: //0x16
		{
			switch(minor_id)
			{
				case MINOR_ID_BA_MODE_CONTROL: //0x16 : 0x00
					break;
				
				default:
					ret = FALSE;
					break;
			}
		}
		break;
		
		case MAJOR_ID_BLE_CONTROL: //0x17
		{
			switch(minor_id)
			{
				case MINOR_ID_ADVERTISING_CONTROL: //0x17 : 0x02
				case MINOR_ID_SET_BLE_MANUFACTURE_DATA: //0x17 : 0x03
					break;
				
				default:
					ret = FALSE;
					break;
			}
		}
		break;
		
		case MAJOR_ID_QUALIFICATION_COMMANDS: //0x01
		case MAJOR_ID_HFP_CONTROL: //0x12
		case MAJOR_ID_SPP_CONTROL: //0x13
		case MAJOR_ID_UPGRADE_CONTROL: //0x1B
		break;

		default:
			ret = FALSE;
			break;
	}

	return ret;
}


Bool MB3021_BT_Module_Check_Valid_Major_Indication(uint8_t check_Data)
{
	Bool ret = TRUE;
	
	switch(check_Data)
	{
		case MAJOR_ID_GENERAL_CONTROL: //0x00
		case MAJOR_ID_A2DP_CONTROL: //0x10
		case MAJOR_ID_AVRCP_CONTROL: //0x11
		case MAJOR_ID_HFP_CONTROL: //0x12
		case MAJOR_ID_SPP_CONTROL: //0x13
		case MAJOR_ID_TWS_CONTROL: //0x15
		case MAJOR_ID_BA_CONTROL: //0x16
		case MAJOR_ID_BLE_CONTROL: //0x17
		case MAJOR_ID_PBAP_CONTROL: //0x18
		case MAJOR_ID_UPGRADE_CONTROL: //0x1B
		break;

		default:
			ret = FALSE;
			break;
	}

	return ret;
}

Bool MB3021_BT_Module_Check_Valid_Minor_Indication(uint8_t major_id, uint8_t minor_id)
{
	Bool ret = TRUE;
	
	switch(major_id)
	{		
		case MAJOR_ID_GENERAL_CONTROL: //0x00
			switch(minor_id)
			{
				case MINOR_ID_MODULE_STATE_CHANGED_IND: //0x00 : 0x00
#ifdef DEVICE_NAME_CHECK_PAIRING
				case MINOR_ID_REMOTE_DEVICE_NAME_IND: //0x00 : 0x0C
#endif
				case MINOR_ID_FIRMWARE_VERSION_IND: //0x00 : 0x0E
				case MINOR_ID_LOCAL_ADDRESS_NAME_IND: //0x00 : 0x0F
				case MINOR_ID_ROUTING_STATE_IND: //0x00 : 0x22
				case MINOR_ID_ACL_OPENED_IND: //0x00 : 0x23
				case MINOR_ID_ACL_CLOSED_IND: //0x00 : 0x24
					break;
			
				default:
					ret = FALSE;
					break;
			}
			break;
			
		case MAJOR_ID_A2DP_CONTROL: //0x10
			switch(minor_id)
			{
				case MINOR_ID_A2DP_PROFIL_STATE_CHANGED_IND: //0x10 : 0x00
					break;
#if defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE) //2022-11-03
				case MINOR_ID_A2DP_MEDIA_STATE_CHANGED_IND: //0x10 : 0x01
					break;
				case MINOR_ID_A2DP_SELECTED_CODEC_IND: //0x10 : 0x05
					break;
#endif
				case MINOR_ID_A2DP_STREAM_ROUTING_CHANGED_IND://0x10 : 0x02
					break;
				case MINOR_ID_A2DP_CONNECT_FAIL_ERRORCODE_IND://0x10 : 0x70
					break;
				default:
					ret = FALSE;
					break;
			}
			break;

		case MAJOR_ID_AVRCP_CONTROL: //0x11
#if defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE)
			switch(minor_id)
			{
				case MINOR_ID_AVRCP_CAPABILITY_LIST_IND: //0x11 : 0x03
					break;
				case MINOR_ID_AVRCP_VOLUME_CHANGED_IND: //0x11 : 0x02
					break;
				case MINOR_ID_AVRCP_PLAYBACK_STATUS_CHANGED_IND: //0x11 : 0x01
					break;
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
				case MINOR_ID_AVRCP_PROFILE_STATE_CHANGED_IND: //0x11 : 0x00
					break;
				case MINOR_ID_AVRCP_SEND_SYNC_DATA_IND: //0x11 : 0x13
					break;
#endif
				default:
					ret = FALSE;
					break;
			}
#endif
			break;

		case MAJOR_ID_BA_CONTROL: //0x16
			switch(minor_id)
			{
				case MINOR_ID_BA_MODE_CONTROL_IND: //0x16 : 0x00
					break;

				case MINOR_ID_RECEIVER_CONNECTION_STATUS_IND: //0x16 : 0x02
					break;

				case MINOR_ID_BA_MANUFACTURE_DATA_IND: //0x16 : 0x05
					break;

				default:
					ret = FALSE;
					break;
			}
			break;
		
		case MAJOR_ID_BLE_CONTROL: //0x17
			switch(minor_id)
			{
				case MINOR_ID_ADVERTISING_CONTROL: //0x17 : 0x02
				case MINOR_ID_SET_BLE_MANUFACTURE_DATA: //0x17 : 0x03
					break;
				
				default:
					ret = FALSE;
					break;
			}
			break;

		case MAJOR_ID_SPP_CONTROL: //0x13
			switch(minor_id)
			{
				case MINOR_ID_SPP_PROFILE_STATE_CHANGED_IND: //0x13 : 0x00
					break;
				
				default:
					ret = FALSE;
					break;
			}
			break;
		
		case MAJOR_ID_HFP_CONTROL: //0x12
		case MAJOR_ID_PBAP_CONTROL: //0x18
		case MAJOR_ID_UPGRADE_CONTROL: //0x1B
			break;		

		default:
			ret = FALSE;
			break;
	}

	return ret;
}

Bool MB3021_BT_Module_Check_Vailid_BLE_Remote_Data(uint8_t *data, uint16_t data_length)
{
	int i;
	Bool BRet = TRUE;

	for(i =0; i< data_length; i++)
	{
		switch(i)
		{
			case BLE_POWER_KEY: //Power On/Off - Power On : 0x01/Power Off : 0x00
			{
				if(data[BLE_POWER_KEY] > 0x02)
					return FALSE;
			}
			break;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			case BLE_MUTE_KEY: //Mute On/Off - Mute On : 0x01/Mute Off : 0x00
			{
				if(data[BLE_MUTE_KEY] > 0x02)
					return FALSE;
			}
			break;

			case BLE_VOLUME_KEY: //Volume Setting - Level 0(0x00) ~ Level 15(0x0f)
			{
				if(data[BLE_VOLUME_KEY] > 0x0f)
					return FALSE;
			}
			break;

			case BLE_EQ_KEY: //Sound Effect(EQ) - Normal : 0x00/POP&ROCK : 0x01/CLUB : 0x02/JAZZ : 0x03/VOCAL : 0x04
			{
				if(data[BLE_EQ_KEY] > 0x04)
					return FALSE;
			}
			break;
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			case BLE_REBOOT_KEY: //Reboot On - Reboot On : 0x01/Reboot Off : 0x00
			{
				if(data[BLE_REBOOT_KEY] > 0x01)
					return FALSE;
			}
			break;

			case BLE_FACTORY_RESET_KEY: //Factory Reset Key - Factory Reset On : 0x01/Factory Reset Off : 0x00
			{
				if(data[BLE_FACTORY_RESET_KEY] > 0x01)
					return FALSE;
			}
			break;
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
			case BLE_MUTE_OFF_DELAY: //mute off delay - Noraml : 0x00 / mute off delay : 0x01 /mute On : 0x02
			{
#ifdef SLAVE_VOLUME_FORCED_SYNC
				if(data[BLE_MUTE_OFF_DELAY] > 0x04) //0x03 ~ 0x04 : Dummy for forced volume setting
#else
				if(data[BLE_MUTE_OFF_DELAY] > 0x02)
#endif
					return FALSE;
			}
			break;
#endif
			default:
				BRet = FALSE;
			break;
		}
	}

	return BRet;
}

Bool MB3021_BT_Module_Check_Vailid_SSP_Remote_Data(uint8_t *data, uint16_t data_length)
{
	int i;
	Bool BRet = TRUE;

	for(i =0; i< data_length; i++)
	{
		switch(i)
		{
			case BLE_POWER_KEY: //Power On/Off - Power On : 0x01/Power Off : 0x00
			{
				if(data[BLE_POWER_KEY] > 0x02)
					return FALSE;
			}
			break;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			case BLE_MUTE_KEY: //Mute On/Off - Mute On : 0x01/Mute Off : 0x00
			{
				if(data[BLE_MUTE_KEY] > 0x02)
					return FALSE;
			}
			break;

			case BLE_VOLUME_KEY: //Volume Setting - Level 0(0x00) ~ Level 15(0x0f)
			{
				if(data[BLE_VOLUME_KEY] > 0x0f)
					return FALSE;
			}
			break;

			case BLE_EQ_KEY: //Sound Effect(EQ) - Normal : 0x00/POP&ROCK : 0x01/CLUB : 0x02/JAZZ : 0x03/VOCAL : 0x04
			{
				if(data[BLE_EQ_KEY] > 0x04)
					return FALSE;
			}
			break;
#endif //AD82584F_ENABLE
			case BLE_REBOOT_KEY: //Reboot On - Reboot On : 0x01/Reboot Off : 0x00
			{
				if(data[BLE_REBOOT_KEY] > 0x01)
					return FALSE;
			}
			break;

			case BLE_FACTORY_RESET_KEY: //Factory Reset Key - Factory Reset On : 0x01/Factory Reset Off : 0x00
			{
				if(data[BLE_FACTORY_RESET_KEY] > 0x01)
					return FALSE;
			}
			break;
#ifdef SPP_EXTENSION_V44_ENABLE
			case BLE_BT_SHORT_KEY: //BT Short Key(Master Only) - Noraml : 0x00 / BT Short Key : 0x01
			{
#if defined(USEN_BAP) || defined(USEN_BT_SPK_TI) //2023-03-17_1 : Added BT Long Key Action from USEN Tablet using SPP. This key is only valid under Master.
				if(data[BLE_BT_SHORT_KEY] > 0x03)
					return FALSE;
#else
				if(data[BLE_BT_SHORT_KEY] > 0x02)
					return FALSE;
#endif
			}
			break;
#endif

			default:
				BRet = FALSE;
			break;
		}
	}

	return BRet;
}

static void MB3021_BT_Module_RESP_Error_Code(uint8_t major_id, uint8_t minor_id, uint8_t error_code)
{
	switch(error_code)
	{
		case ERROR_CODE_SUCCESS:
#if 0//def BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE_SUCCESS : ");
#endif
			break;

		case ERROR_CODE_FAIL:
#ifdef BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE_FAIL : ");
#endif
			break;

		case ERROR_CODE_PARAM_LENGTH_ERROR:
#ifdef BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE_PARAM_LENGTH_ERROR : ");
#endif
			break;

		case ERROR_CODE_INVALID_PARAM:
#ifdef BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE_INVALID_PARAM : ");
#endif
			break;

		case ERROR_CODE_WRONG_STATE:
#ifdef BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE_WRONG_STATE : ");
#endif
			break;

		case ERROR_CODE_NOT_SUPPORTED:
#ifdef BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE_NOT_SUPPORTED : ");
#endif
			break;

		case ERROR_CODE_FAIL_UNKNOWN:
#ifdef BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE_FAIL_UNKNOWN : ");
#endif
			break;

		default:
#ifdef BT_DEBUG_MSG
			_DBG("\n\rERROR_CODE NOT DEFINE : ");
#endif
			break;
	}

#if 0//def BT_DEBUG_MSG
	_DBH(major_id);
	_DBG("/");
	_DBH(minor_id);
#endif
}

static void MB3021_BT_Module_Remote_Data_Receive(uint8_t source_type, uint8_t data_channel, uint8_t *data, uint16_t data_length) //SPP COM : Receive SPP Data/BLE Data
{
	Bool BRet = TRUE;
	Remote_Power_Key_Action Power_Key_Action = REMOTE_POWER_NONE_ACTION;
	int i = 0;
	uint8_t bChecksum = 0;
#if defined(FIVE_USER_EQ_ENABLE) && !defined(SPP_EXTENSION_ENABLE)
	uint8_t uBuf = 0;
#endif
#ifdef AUTO_VOLUME_LED_OFF
	uint8_t uVolume_Level = 0;
	uint8_t bMute = FALSE;
#endif
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-10_1 : When we send volume info to Slave, we need to keep original value to avoid wrong volume level of slave
	uint8_t uVol_buf = 0;
#endif
#ifdef SPP_EXTENSION_V44_ENABLE
	uint8_t uSPP_receive_buf8_bk[7]; //To recover when parameter is NG. If parameter is all OK, we save the data value thru SPP to uSPP_receive_buf8 buffer.
#else
	uint8_t uSPP_receive_buf8_bk[6]; //To recover when parameter is NG. If parameter is all OK, we save the data value thru SPP to uSPP_receive_buf8 buffer.
#endif
	
	//To Do !!! The data length shuld be less than 8
	switch(source_type)
	{
		case DATA_SOURCE_TYPE_SPP: //SPP COM : Receive SPP Data from Peer Device
		{
#ifdef SPP_DATA_GET_FROM_PEER_DEVICE
#ifndef MASTER_MODE_ONLY
			Switch_Master_Slave_Mode mode;
				
			mode = Get_Cur_Master_Slave_Mode();

			if(mode == Switch_Master_Mode) //SPP Data from Tablet
#endif
			{						
				if(data[0] == 0xAA)
				{
#ifdef SPP_EXTENSION_V44_ENABLE
					if(data_length == 0x09) //Remocon Data thru SPP
#else
					if(data_length == 0x08) //Remocon Data thru SPP
#endif
					{
#ifdef SPP_EXTENSION_V44_ENABLE
						//Data[0] = Header, Data[1~7] = Remocon Data, Data[8] = Checksum
						bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 8); //using checksum Data check whether it's valid or invalid

						if(bChecksum != data[8])
#else //SPP_EXTENSION_V44_ENABLE
						//Data[0] = Header, Data[1~6] = Remocon Data, Data[7] = Checksum
						bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 7); //using checksum Data check whether it's valid or invalid

						if(bChecksum != data[7])
#endif //SPP_EXTENSION_V44_ENABLE
						{
#ifdef BT_DEBUG_MSG
							_DBG("\n\rReceive data thru SPP communication is NG because check sum is NG");
							_DBH(bChecksum);
							_DBG("/");
#ifdef SPP_EXTENSION_V44_ENABLE
							_DBH(data[8]);
#else
							_DBH(data[7]);	
#endif
#endif
							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
#ifdef SPP_EXTENSION_ENABLE							
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_CHECKSUM_ERROR;
#endif
							BRet = FALSE;

							break;
						}
#ifdef SPP_EXTENSION_V44_ENABLE
						if(!MB3021_BT_Module_Check_Vailid_SSP_Remote_Data(data+1, 7)) //Check whether SSP remote Data is valid or not
#else
						if(!MB3021_BT_Module_Check_Vailid_SSP_Remote_Data(data+1, 6)) //Check whether SSP remote Data is valid or not
#endif
						{
#ifdef BT_DEBUG_MSG
							_DBG("\n\rReceive data thru SPP communication is NG because some Data have invalid value");
#endif							
							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
#ifdef SPP_EXTENSION_ENABLE							
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA;
#endif

							BRet = FALSE;
							break;
						}
						
#ifdef KEY_IS_INVALID_UNDER_MASTER_SLAVE_GROUPING
						if(Get_master_slave_grouping_flag()) //To make invalid key action under Master/Slave Grouping mode. To avoid minor issues(LED Display)
						{
#ifdef BT_DEBUG_MSG
							_DBG("\n\rReceive data thru SPP communication is Invalid under Master/Slave Grouping mode");
#endif							
							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
#ifdef SPP_EXTENSION_ENABLE							
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_BT_SPK_BUSY;
#endif
							BRet = FALSE;
							break;
						}
#endif

#ifdef SPP_EXTENSION_V44_ENABLE
						for(i = 1; i<8; i++) //Actual Data is data[1] ~ data[7]
#else
						for(i = 1; i<7; i++) //Actual Data is data[1] ~ data[6]
#endif
						{
							uSPP_receive_buf8_bk[i-1] = uSPP_receive_buf8[i];

							if(!Power_State()) //When Power is off, all keys execepting Power Key are ignored.
							{
								if(data[1] != 0x01) //POWER_KEY
								{
#ifdef BT_DEBUG_MSG
									_DBG("\n\r1. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
									uSPP_receive_buf8[1] = data[1];

									bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG

#ifdef SPP_EXTENSION_ENABLE														
									uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_POWER_OFF_STATE;
#endif								

									BRet = FALSE;

									break;
								}
							}
#ifdef TAS5806MD_ENABLE
							if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE) //2023-02-27_2 //2023-02-22_1 : TWS Slave BT SPK executes Amp init again. Sometimes, BT SPK get this interrupt during Amp Init and Amp Init has wrong data.
							{
#ifdef TAS5806MD_DEBUG_MSG
								_DBG("\n\r+++ Is_BAmp_Init is TRUE - 22");
#endif
#ifdef SPP_EXTENSION_ENABLE														
								uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_BT_SPK_BUSY; //Temparary
#endif								

								BRet = FALSE;

								break;
							}
#endif
							if(uSPP_receive_buf8_bk[i-1] != data[i])
							{
								switch(i-1)
								{
									case BLE_POWER_KEY: //##SPP ## Power On/Off - Power On : 0x01/Power Off : 0x00 //Power Key Action should be executed here because when Power off, we need to save amp value before Power Off
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 1.Power On/Off");
#endif
										if(data[1] == 0x01)
											Power_Key_Action = REMOTE_POWER_ON_ACTION;// Remocon_Power_Key_Action(TRUE);
										else if(data[1] == 0x00)
										{
											Power_Key_Action = REMOTE_POWER_OFF_ACTION; //Remocon_Power_Key_Action(FALSE);
											//break; //When the key is Power_Off_Key, We just set Power_Off_Key only excepting other Keys !!! 
										}
										else
										{
											Power_Key_Action = REMOTE_POWER_NONE_ACTION; //Remocon_Power_Key_Action(FALSE);
											BRet = FALSE;
										}
									}
									break;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
									case BLE_MUTE_KEY: //##SPP ## Mute On/Off - Mute On : 0x01/Mute Off : 0x00
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 2.Mute On/Off : ");
#endif
										if(data[2] == 0x01)
										{
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Mute(TRUE, TRUE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Mute(TRUE, TRUE);
#endif //TAS5806MD_ENABLE				
#ifdef AUTO_VOLUME_LED_OFF
											bMute = TRUE;
#endif
										}
										else if(data[2] == 0x00)
										{
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Mute(FALSE, TRUE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Mute(FALSE, TRUE);
#endif //TAS5806MD_ENABLE				
										}
										else
											BRet = FALSE;
									}
									break;

									case BLE_VOLUME_KEY: //##SPP ## Volume Setting - Level 0(0x00) ~ Level 15(0x0f)
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 3.Volume Setting");
#endif
										if(data[3] <= 0x0f)
										{
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_3 : To Fit SPP volume data from PeerDevice under BAP //2023-01-10_1
#ifdef ADC_VOLUME_64_STEP_ENABLE
											uVol_buf = Convert_16Step_to_64Step(data[3]); //2023-01-09_1 : To convert from 16-step to 64-step
#else //ADC_VOLUME_50_STEP_ENABLE
											uVol_buf = Convert_16Step_to_50Step(data[3]); //2023-02-27_3 : To convert from 16-step to 50-step
#endif //ADC_VOLUME_64_STEP_ENABLE

#ifdef AD82584F_ENABLE
											AD82584F_Amp_Volume_Set_with_Index(uVol_buf, TRUE, FALSE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Volume_Set_with_Index(uVol_buf, TRUE, FALSE);
#endif //TAS5806MD_ENABLE

#else //#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Volume_Set_with_Index(data[3], TRUE, FALSE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Volume_Set_with_Index(data[3], TRUE, FALSE);
#endif //TAS5806MD_ENABLE
#endif //#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
										}
										else
											BRet = FALSE;
									}
									break;

									case BLE_EQ_KEY: //##SPP ## Sound Effect(EQ) - Normal : 0x00/POP&ROCK : 0x01/CLUB : 0x02/JAZZ : 0x03/VOCAL : 0x04
									{
#ifdef BT_DEBUG_MSG					
										_DBG("\n\r+++ 4.Sound Effect(EQ)");
#endif

#if defined(SPP_EXTENSION_ENABLE) && defined(MASTER_MODE_ONLY) //2023-03-27_3 : When BAP-01 is NORMAL Mode, we need to ignore EQ_KEY w/o error from USEN Tablet.
										if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
										{
											uEQ_Mode = EQ_NORMAL_MODE;
											
											break;
										}
										else
#endif
										//To Do !!! EQ control function
										if(data[4] <= 0x04)
										{
#ifdef FIVE_USER_EQ_ENABLE
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#else //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
											TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#endif											
#endif //TAS5806MD_ENABLE
											TIMER20_user_eq_mute_flag_start();
#ifdef USEN_TI_AMP_EQ_ENABLE //2023-02-27_1
											uEQ_Mode = data[4];
											TAS5806MD_Amp_EQ_DRC_Control((EQ_Mode_Setting)uEQ_Mode);
#else //USEN_TI_AMP_EQ_ENABLE
#ifdef SPP_EXTENSION_ENABLE
											uEQ_Mode = data[4];
											MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uEQ_Mode);
#else //SPP_EXTENSION_ENABLE
											uBuf = data[4];

											MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uBuf);
#endif //SPP_EXTENSION_ENABLE
#endif
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
											FlashSaveData(FLASH_SAVE_DATA_EQ, uEQ_Mode);
#endif
#endif //FIVE_USER_EQ_ENABLE
#ifdef AUTO_VOLUME_LED_OFF
											TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
#ifdef AD82584F_ENABLE
											uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
											AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#else //AD82584F_ENABLE
											uVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
											TAS5806MD_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#endif //TAS5806MD_ENABLE								
#endif

										}
										else
											BRet = FALSE;
									}
									break;
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
									case BLE_REBOOT_KEY: //##SPP ## Reboot On - Reboot On : 0x01/Reboot Off : 0x00
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 5.Reboot");
#endif
										if(data[5] == 0x01)
										{
											B_SSP_REBOOT_KEY_In = TRUE;
										}
										else if(data[5] == 0x00)
										{
											//B_SSP_REBOOT_KEY_In = FALSE; //No need clear because this needs reboot action
										}
										else
											BRet = FALSE;
									}
									break;

									case BLE_FACTORY_RESET_KEY: //##SPP ## Factory Reset Key - Factory Reset On : 0x01/Factory Reset Off : 0x00
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 6.Factory");
#endif
										if(data[6] == 0x01)
										{
											B_SSP_FACTORY_RESET_KEY_In = TRUE;
#ifdef AUTO_VOLUME_LED_OFF
											TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
											uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
											AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#else //AD82584F_ENABLE
											uVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
											TAS5806MD_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#endif

#if 0										
#ifdef SWITCH_BUTTON_KEY_ENABLE
											bFactory_Reset_Mode = TRUE;
#endif
											Factory_Reset_Value_Setting();
#endif
										}								
										else if(data[6] == 0x00)
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rFactory Reset Off 1 - To Do !!!");
#endif
										}
										else
											BRet = FALSE;
									}
									break;
#ifdef SPP_EXTENSION_V44_ENABLE
									case BLE_BT_SHORT_KEY: //##SPP ## Factory Reset Key - Factory Reset On : 0x01/Factory Reset Off : 0x00
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 7.BT Short Key");
#endif
										if(data[7] == 0x01)
										{
#ifdef AUX_INPUT_DET_ENABLE //Need to ignore BT Short key thru SPP under Aux mode. 2022-09-22
											if(Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
											{
												BRet = FALSE;											
												bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
#ifdef SPP_EXTENSION_ENABLE														
												uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA_UNDER_CUR_STATE;
#endif
												return;
											}
#endif
#ifdef MASTER_SLAVE_GROUPING
#if defined(TWS_MASTER_SLAVE_GROUPING) && defined(SW1_KEY_TWS_MODE) //2022-12-15 //TWS : Tablet BT Short Key Action is Invalid Under TWS Mode
											if(Get_Cur_LR_Stereo_Mode() != Switch_LR_Mode)
#endif
											MB3021_BT_Master_Slave_Grouping_Start();
#endif										
										}
										else if(data[7] == 0x00)
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rBT Short/Long Key Off - To Do !!!");
#endif
										}
#if defined(USEN_BAP) || defined(USEN_BT_SPK_TI) //2023-03-17_1 : Added BT Long Key Action from USEN Tablet using SPP. This key is only valid under Master.
										else if(data[7] == 0x02)
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\r+++ 7-1. BT Long Key");
#endif
#ifdef AUX_INPUT_DET_ENABLE //Need to ignore BT Short key thru SPP under Aux mode. 2022-09-22
											if(Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
											{
												BRet = FALSE;											
												bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
#ifdef SPP_EXTENSION_ENABLE														
												uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA_UNDER_CUR_STATE;
#endif
												return;
											}
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
											Send_Remote_Key_Event(BT_PAIRING_KEY);
#endif
										}
#endif
										else
											BRet = FALSE;
									}
									break;
#endif

									default:
										BRet = FALSE;
									break;
								}
							}
						}

						if(BRet)
						{
							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_OK; //Send SPP Response OK
							
#ifdef SPP_EXTENSION_V44_ENABLE
							for(i = 0; i<9; i++) //including checksum - data[8]
#else
							for(i = 0; i<8; i++) //including checksum - data[7]
#endif
							{
#ifdef SPP_EXTENSION_V44_ENABLE
								if(i == 7)
									uSPP_receive_buf8[i] = 0x00; //To make valid value for next BT Shortk
								else
#endif
								{
#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE) //2023-03-27_4 : Under BAP-01 NORMAL mode, BAP-01 can get only NORMAL MODE.
									if(i == (BLE_EQ_KEY+1) && Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
									{
										uSPP_receive_buf8[i] = EQ_NORMAL_MODE;
									}
									else
#endif
									uSPP_receive_buf8[i] = data[i]; //Save SPP Data finally !!!
								}
#ifdef SPP_CMD_AND_USER_INPUT_KEY_SYNC
								if(i<7) //No need checksum
									uInput_Key_Sync_buf8[i] = uSPP_receive_buf8[i];
#endif
							}
#ifdef AUTO_VOLUME_LED_OFF
							if(bMute == FALSE)
								TIMER20_auto_volume_led_off_flag_Start();
							else
								bMute = TRUE;
#endif

							switch(Power_Key_Action) //Power Key Action should be executed here because when Power off, we need to save amp value before Power Off
							{
								case REMOTE_POWER_ON_ACTION:
#ifdef SWITCH_BUTTON_KEY_ENABLE
									Remocon_Power_Key_Action(TRUE, FALSE, TRUE);
#endif
								break;
								case REMOTE_POWER_OFF_ACTION:
#ifdef SWITCH_BUTTON_KEY_ENABLE
									Remocon_Power_Key_Action(FALSE, FALSE, TRUE);
#endif
								break;

								default:
								break;
							}
						}
						else
						{
#ifdef BT_DEBUG_MSG
							_DBG("\n\rBCRF_SEND_SPP_RECEIVE_DATA_NG");
#endif

							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
#ifdef SPP_EXTENSION_ENABLE							
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA;
#endif
						}
#ifdef BT_DEBUG_MSG
						_DBG("\n\rSPP Data :");
#ifdef SPP_EXTENSION_V44_ENABLE
						for(i = 0; i<9; i++)
#else
						for(i = 0; i<8; i++)
#endif
						{
							_DBH(uSPP_receive_buf8[i]);
						}
#endif
					}
					else
					{						
						bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
#ifdef SPP_EXTENSION_ENABLE
						uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_LENGTH_ERROR;
#endif
#ifdef BT_DEBUG_MSG
						_DBG("\n\rSPP_ERROR_LENGTH_ERROR");
#endif
					}
				}
#ifdef SPP_EXTENSION_ENABLE
				else if(data[0] == 0xBB) //##SPP ## 
				{
					if(data_length == 3)
					{
						if(data[1] == 0x01) //Master SPK need to send current status information to Tablet over SPP
						{
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 2);
							
							if(data[2] == bChecksum)
							{
								uCurrent_Status_buf8[0] = 0xBB; 
								
								if(Power_State() == TRUE)
									uCurrent_Status_buf8[1] = 0x01; //Power On/Off : Power On mode
								else
									uCurrent_Status_buf8[1] = 0x00; //Power On/Off : Power Off mode

#ifdef AD82584F_USE_POWER_DOWN_MUTE
								if(IS_Display_Mute() == TRUE) //When Mute On status, we don't need to mute off. This function is for LED Display
									uCurrent_Status_buf8[2] = 0x01; //Mute On/Off : Mute On mode
								else
								{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION //When Power On thru App after Power Plug Off/On, IS_Display_Mute() is always FALSE even though Mute On. So, we need to use Flash data to keep Mute On. 2022-09-20
									if(!Power_State())
									{
										uint8_t uRead_Buf[FLASH_SAVE_DATA_END];

										Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
									
										if(uRead_Buf[FLASH_SAVE_DATA_MUTE])
											uCurrent_Status_buf8[2] = 0x01; //Mute On/Off : Mute On mode
										else
											uCurrent_Status_buf8[2] = 0x00; //Mute On/Off : Mute Off mode
									}
									else
#endif
										uCurrent_Status_buf8[2] = 0x00; //Mute On/Off : Mute Off mode
								}
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-10_2 :  When BAP-01 is turned on using Power plug-in and USEN Tablet asks BAP-01 status using 0xBB, it sends wrong volume data
								uVol_buf = TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level //2023-01-06_2
#ifdef ADC_VOLUME_64_STEP_ENABLE //2023-02-27_3
								uCurrent_Status_buf8[3] = Convert_64Step_to_16Step(uVol_buf);
#else //ADC_VOLUME_50_STEP_ENABLE
								uCurrent_Status_buf8[3] = Convert_50Step_to_16Step(uVol_buf);
#endif //ADC_VOLUME_64_STEP_ENABLE

#else //defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#ifdef AD82584F_ENABLE
								uCurrent_Status_buf8[3] = AD82584F_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level
#else //AD82584F_ENABLE
								uCurrent_Status_buf8[3] = TAS5806MD_Amp_Get_Cur_Volume_Level_Inverse(); //Volume Level
#endif //TAS5806MD_ENABLE
#endif //defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#else //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
								uCurrent_Status_buf8[3] = 0x05; //Volume Level
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)								
#if defined(MASTER_MODE_ONLY) && defined(TAS5806MD_ENABLE) //2023-03-28_3 : Send current information to USEN Tablet when user changes SW2_KEY(EQ NORMAL/EQ BSP) due to changed spec
								if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
								{
									uEQ_Mode = EQ_NORMAL_MODE;
								}
#endif
								uCurrent_Status_buf8[4] = uEQ_Mode; //Sound EQ mode
								
								uCurrent_Status_buf8[5] = 0x00; //Reboot Off
								uCurrent_Status_buf8[6] = 0x00; //Factory Reset Off
#ifdef SPP_EXTENSION_V42_ENABLE
#ifdef USEN_BAP //2023-03-02_1 : Send BAP-01 Name to USEN Tablet
								uCurrent_Status_buf8[7] = 0x42;
								uCurrent_Status_buf8[8] = 0x41;
								uCurrent_Status_buf8[9] = 0x50;
								uCurrent_Status_buf8[10] = 0x2D;
								uCurrent_Status_buf8[11] = 0x30;
								uCurrent_Status_buf8[12] = 0x31;
#ifdef MASTER_MODE_ONLY //2023-03-28_4 : Send current SW2_KEY status whether it's EQ NORMAL or EQ BSP due to chagned spec under BAP-01
								if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
									uCurrent_Status_buf8[13] = 0x01; //EQ NORMAL
								else
									uCurrent_Status_buf8[13] = 0x02; //EQ BSP
#else
								uCurrent_Status_buf8[13] = 0x20;
#endif //#ifdef MASTER_MODE_ONLY
#else //USEN_BAP
								uCurrent_Status_buf8[7] = 0x42;
								uCurrent_Status_buf8[8] = 0x53;
								uCurrent_Status_buf8[9] = 0x50;
								uCurrent_Status_buf8[10] = 0x2D;
								uCurrent_Status_buf8[11] = 0x30;
								uCurrent_Status_buf8[12] = 0x31;
								uCurrent_Status_buf8[13] = 0x42;
#endif //USEN_BAP
								uCurrent_Status_buf8[14] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 14);
#else //SPP_EXTENSION_V42_ENABLE
								uCurrent_Status_buf8[7] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 7);
#endif //SPP_EXTENSION_V42_ENABLE

								bPolling_Get_Data |= BCRF_SEND_SPP_DATA_RESP; //Send SPP Response
							}
							else
							{
								uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_CHECKSUM_ERROR;
							}
						}
#ifdef SPP_EXTENSION_V50_ENABLE
						else if(data[1] == 0x02) //Master SPK need to send MCU FW Version
						{
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 2);
							
							if(data[2] == bChecksum)
							{
								uCurrent_Status_buf8[0] = 0xBB; 
								uCurrent_Status_buf8[1] = MCU_FW_VERSION_INDICATOR;

								//MCU FW uCurrent_Status_buf8[2] ~ [7]
								strncpy((char *)(uCurrent_Status_buf8+2), MCU_Version, 6);
								uCurrent_Status_buf8[8] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 8);
							
								bPolling_Get_Data |= BCRF_SEND_SPP_DATA_RESP; //Send SPP Response
							}
							else
							{
								uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_CHECKSUM_ERROR;
							}
						}
						else if(data[1] == 0x03) //Master SPK need to send BT FW Version
						{
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 2);
							
							if(data[2] == bChecksum)
							{
								uCurrent_Status_buf8[0] = 0xBB; 
								uCurrent_Status_buf8[1] = BT_FW_VERSION_INDICATOR;

								//BT FW uCurrent_Status_buf8[2] ~ [8]
								strncpy((char *)(uCurrent_Status_buf8+2), BT_Version, 7);
								uCurrent_Status_buf8[9] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 9);
							
								bPolling_Get_Data |= BCRF_SEND_SPP_DATA_RESP; //Send SPP Response
							}
							else
							{
								uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_CHECKSUM_ERROR;
							}
						}
#endif
						else
						{
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA;
						}
					}
					else
					{
						uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_LENGTH_ERROR;
					}
				}
				else
				{				
					bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
					
					uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_START_CODE_NG;
#ifdef BT_DEBUG_MSG
					_DBG("\n\rSPP_ERROR_START_CODE_NG");
#endif

				}
#endif
			}
#endif //SPP_DATA_GET_FROM_PEER_DEVICE
		} //##SPP ## 
		break;

		case DATA_SOURCE_TYPE_BLE_DATA: //BLE COM
		//To Do !!! ???
		break;

		default:
		break;
	}
}

static void MB3021_BT_Module_Receive_Data_RESP(uint8_t major_id, uint8_t minor_id, uint8_t *data, uint16_t data_length)
{
	Bool ret;
	uint8_t uError_code = 0;

	uError_code = data[0];

#if 0//def BT_DEBUG_MSG
		_DBG("\n\rMB3021_BT_Module_Receive_Data_RESP\n\r");
		_DBH(major_id);
		_DBG("/");
		_DBH(minor_id);
#endif

	ret = MB3021_BT_Module_Check_Valid_Minor_Response(major_id, minor_id); //Already checked effectiveness but need to double checck

	if(ret)
	{
		MB3021_BT_Module_RESP_Error_Code(major_id, minor_id, uError_code);
#ifdef MASTER_SLAVE_GROUPING //If we want to command recovery, just use below statement.
		MB3021_BT_Module_CMD_Execute(major_id, minor_id, data, data_length, FALSE);	
#else
		if(uError_code == ERROR_CODE_SUCCESS)
			MB3021_BT_Module_CMD_Execute(major_id, minor_id, data, data_length, FALSE);	
		else //Recovery : send CMD again
		{
#ifndef MASTER_MODE_ONLY
			Switch_Master_Slave_Mode mode;
			
			mode = Get_Cur_Master_Slave_Mode();
#endif
			//When Factory reset under Slave mode, we don't need to check ERROR_CODE_SUCCESS becasue If Slave does not have last connection, it will return error code.
			//or the BA_MODE_CONTROL is not valid, it's already set same mode.
			if(
#ifndef MASTER_MODE_ONLY
				(major_id == MAJOR_ID_GENERAL_CONTROL && minor_id == MINOR_ID_DELETE_PAIRED_DEVICE_LIST && mode == Switch_Slave_Mode) ||
#endif
			(major_id == MAJOR_ID_BA_CONTROL && minor_id == MINOR_ID_BA_MODE_CONTROL && uError_code == ERROR_CODE_FAIL)
			)
			{
				MB3021_BT_Module_CMD_Execute(major_id, minor_id, data, data_length, FALSE);
			}
			else
				MB3021_BT_Module_CMD_Execute(major_id, minor_id, data, data_length, TRUE);
		}
#endif
	}
}


static void MB3021_BT_Module_Receive_Data_IND(uint8_t major_id, uint8_t minor_id, uint8_t *data, uint16_t data_length)
{	
	int i = 0;
#ifdef BLE_DATA_GET_FROM_MASTER_SPK
	Bool BRet = TRUE;
	Remote_Power_Key_Action Power_Key_Action = REMOTE_POWER_NONE_ACTION;
#ifdef FIVE_USER_EQ_ENABLE
	uint8_t uBuf = 0;
#endif
	uint8_t bChecksum = 0;
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
	uint8_t uBLE_Remocon_Data_bk[7]; //Add mute off delay for Slave SPK
#else
	uint8_t uBLE_Remocon_Data_bk[6]; //To recover when parameter is NG. If parameter is all OK, we save the data value thru BLE to uBLE_Remocon_Data buffer.
#endif
#endif
#ifdef AUTO_VOLUME_LED_OFF
	uint8_t uVolume_Level = 0;
	Bool bMute = FALSE;
#endif
#ifdef AVRCP_ENABLE
	uint8_t uVolume = 0;
#endif
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2022-12-27 : To use Device Name Check Mode under General Mode
	char Address_buf[6];
#endif
	
#if 0//def BT_DEBUG_MSG	
	_DBG("\n\r++Receive_Data_IND : ");
	_DBH(major_id);
	_DBG("/");
	_DBH(minor_id);
#endif

	switch(major_id)
	{
		case MAJOR_ID_GENERAL_CONTROL: //0x00
			switch(minor_id)
			{
				case MINOR_ID_MODULE_STATE_CHANGED_IND: //0x00 : 0x00 //2022-11-04 : Move Here !!!
				{
#ifdef MASTER_MODE_ONLY
					bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //For init sequence (Init Sequnece : Broadcaster -0) //Last -5
#else //MASTER_MODE_ONLY
#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE) //2023-04-26_2 : To make New TWS Connection, we need to skip GET_PAIRED_DEVICE_LIST(for Last connection) and execute SET_CONNECTABLE_MODE when first TWS connection under TWS master mode.
					if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
					{
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
						{
#ifdef TWS_MASTER_SLAVE_GROUPING
							Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
					
							if(uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff) //we don't to execute this when SET_DEVICE_ID is 0xffffffffffff(6Byte)
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rTry to get GET_PAIRED_DEVICE_LIST for last connection !!!");
#endif
								bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //For init sequence (Init Sequnece : Broadcaster -0) //Last -5
							}
							else
#endif
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rTry to do New TWS Connection - 1 instead of GET_PAIRED_DEVICE_LIST");
#endif
								bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device
							}
						}
						else
							bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //For init sequence (Init Sequnece : Broadcaster -0) //Last -5
					}
#else //#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE)
					if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
						bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //For init sequence (Init Sequnece : Broadcaster -0) //Last -5
#endif //#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE)
					else //Slave Mode
					{
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
						{									
							bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; 								
						}
						else
#endif
						bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Receiver -1)
					}
#endif //MASTER_MODE_ONLY
					BBT_Init_OK = TRUE;
				}
				break;
				
#ifdef DEVICE_NAME_CHECK_PAIRING
				case MINOR_ID_REMOTE_DEVICE_NAME_IND: //Init //0x00 : 0x0C
				{
#ifdef USEN_TABLET_AUTO_ON
					char name_check[17] ={0,};
#endif					
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
					_DBG("\n\r++Ind : MINOR_ID_REMOTE_DEVICE_NAME_IND");
					_DBG(" : ");
					_DBH(uPaired_Device_Count);
#endif
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
					//Add cnodition BTWS_Master_Slave_Connect to avoid Master recognize peer device as slave after android device reboot when Master device updates tws slave address //2022-12-22
					if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_LIAC == TWS_Status_Master_Mode_Control 
#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(FLASH_SELF_WRITE_ERASE) //2023-04-26_8 : To get DEVICE NAME over MINOR_ID_REMOTE_DEVICE_NAME_IND, Added TWS condition.
						&& ((!Read_TWS_Connection_From_Flash() && BTWS_Master_Slave_Connect < TWS_Get_Slave_Name) //2023-04-26_8 : During TWS Grouping
						|| (Read_TWS_Connection_From_Flash())) //2023-04-26_8 : After TWS Grouping
#else
						&& BTWS_Master_Slave_Connect < TWS_Get_Slave_Name
#endif
					&& (strncmp(strUSEN_Device_Name_2, (char *)data+7, 15) == 0) //2023-04-03_2 : TWS Device is only availible
					) //2023-02-21_2 : Change Condition for BTWS_Master_Slave_Connect
					{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
						_DBG("\n\r=== break : Switch_LR_Mode");
						_DBG("\n\rUpdated TWS Name Address = ");
#endif
						bPolling_Get_Data |= BCRF_SET_IO_CAPABILITY_MODE; //For init sequence (Init Sequnece : 1) //Need to Check !!!???
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
						BTWS_Master_Slave_Connect = TWS_Get_Slave_Name;
#endif
						for(i=0;i<6;i++) //Save Remote BT Device Name //6 Byte
						{
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_11 : Changed IO_CAPABILITY_MODE condition. so, we need to update uBT_Remote_Device_Address in here.
							uBT_Remote_Device_Address[i] = uBT_TWS_Remote_Device_Address[i] = data[i+1]; //Save Remote BT Device Address
#else //NEW_TWS_MASTER_SLAVE_LINK
							uBT_TWS_Remote_Device_Address[i] = data[i+1]; //Save Remote BT Device Address
#endif //NEW_TWS_MASTER_SLAVE_LINK
							
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
							_DBH(uBT_TWS_Remote_Device_Address[i]);
#endif
						}
							
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
						_DBG("\n\r");
#endif							
						uBT_Remote_Device_Name_Size = data_length-7 ;
	
						for(i=0;i<uBT_Remote_Device_Name_Size;i++) //Save Remote BT Device Name //N Byte
						{
							if(i >= 31)
							{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
								_DBG("\n\r++uBT_Remote_Device_Name is bigger than buffer size = 32");
#endif
								break;
							}
	
							uBT_TWS_Remote_Device_Name[i] = data[i+7]; //Save BT Device Name
						}
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
						_DBG("\n\rTWS Name Size = ");_DBD(uBT_Remote_Device_Name_Size);
						_DBG("\n\rTWS Name = 0x");
						
						for(i=0;i<uBT_Remote_Device_Name_Size;i++)
						{
							_DBH(uBT_TWS_Remote_Device_Name[i]);
						}
#endif						
						break;
					}
#endif

#ifdef USEN_TABLET_AUTO_ON
					bIs_USEN_Device = FALSE;
#endif					

#if defined(SWITCH_BUTTON_KEY_ENABLE) && !defined(BT_GENERAL_MODE_KEEP_ENABLE) //2022-12-27 : To use Device Name Check Mode under General Mode
					if(BBT_Pairing_Key_In == TRUE) //In GIA_PAIRING Mode, we don't need this case statement
					{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
						_DBG("\n\r=== break : GIA_PAIRING Mode");
#endif
						break;
					}
#endif
#ifdef USEN_TABLET_AUTO_ON
					uBT_Remote_Device_Name_Size = data_length-7 ;

					for(i=0;i<uBT_Remote_Device_Name_Size;i++) //Save Remote BT Device Name //6 Byte
					{
						if(i >= 17)
						{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
							_DBG("\n\r++uBT_Remote_Device_Name is bigger than buffer size = 17");
#endif
							break;
						}
						name_check[i] = data[i+7]; //Save BT Device Name
					}
					//Check if current connected device is USEN MUSIC DEVICE?
					if(strncmp(strUSEN_Device_Name_1, name_check, 17) == 0)
					{
						bIs_USEN_Device = TRUE;
					}
#endif										
					//if(uPaired_Device_Count && SUM_of_Array(uBT_Cur_A2DP_Device_Address)) //Under Last Connection, we don't need this case statement
					if((strncmp(uLast_Connected_Device_Address, uBT_Cur_A2DP_Device_Address, 6) == 0) && SUM_of_Array(uBT_Cur_A2DP_Device_Address)) //Under Last Connection, we don't need this case statement
					{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
						_DBG("\n\r=== break : Last Connection Mode");
#endif
						break;
					}
#ifdef TWS_MODE_ENABLE //2023-04-04_1
					if(SUM_of_Array(uBT_Remote_Device_Address) && strncmp((char *)data+1, uBT_Cur_A2DP_Device_Address, 6) == 0) //Get MINOR_ID_REMOTE_DEVICE_NAME_IND twice so we need to ignore Second one
#else
					if(SUM_of_Array(uBT_Remote_Device_Address) && strncmp(uBT_Remote_Device_Address, uBT_Cur_A2DP_Device_Address, 6) == 0) //Get MINOR_ID_REMOTE_DEVICE_NAME_IND twice so we need to ignore Second one
#endif
					{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
						_DBG("\n\r#### Already Remote Device is connected with Master SPK. Just return Here~!!!");
#endif
						break;
					}
					
					bPolling_Get_Data |= BCRF_SET_IO_CAPABILITY_MODE; //For init sequence (Init Sequnece : 1) //Need to Check !!!???
				
					for(i=0;i<6;i++) //Save Remote BT Device Name //6 Byte
					{
						uBT_Remote_Device_Address[i] = data[i+1]; //Save Remote BT Device Address
					}
						
					uBT_Remote_Device_Name_Size = data_length-7 ;

					for(i=0;i<uBT_Remote_Device_Name_Size;i++) //Save Remote BT Device Name //N Byte
					{
						if(i >= 31)
						{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
							_DBG("\n\r++uBT_Remote_Device_Name is bigger than buffer size = 32");
#endif
							break;
						}

						uBT_Remote_Device_Name[i] = data[i+7]; //Save BT Device Name
					}
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
					_DBG("\n\rBT Name Size = ");_DBD(uBT_Remote_Device_Name_Size);
					_DBG("\n\rBT Name = 0x");
					
					for(i=0;i<uBT_Remote_Device_Name_Size;i++)
					{
						_DBH(uBT_Remote_Device_Name[i]);
					}
#endif						
				}	
					break;
#endif
				case MINOR_ID_FIRMWARE_VERSION_IND: //Init //0x00 : 0x0E
#if defined(VERSION_INFORMATION_SUPPORT) && defined(TWS_MASTER_SLAVE_COM_ENABLE) && defined(MCU_VERSION_INFO_DISPLAY_W_UART) //2023-06-07_3 : To send MCU Version informaiton under BSP-01T
					if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
					{
					   MB3021_BT_Module_Send_cmd_param(CMD_SEND_MCU_VERSION_INFO_32, (uint8_t *)MCU_Version);
					}
#endif
#ifdef SOC_ERROR_ALARM
					TIMER20_uart_error_flag_Stop();
#endif
#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2
					TIMER20_BT_hw_reset_cmd_recovery_flag_stop();
#else
					TIMER20_factory_reset_cmd_recovery_flag_stop(); //To Clear Factory Reset recovery for factory_reset_firmware_version becasuse SPK get response here.
#endif
					bPolling_Get_Data |= BCRF_INIT_SINK_MODE; //For init sequence (Init Sequnece : 1) //Last - 1

#ifdef SPP_EXTENSION_V50_ENABLE
					strncpy(BT_Version, (char *)data, 7); //Save BT Version information - 7 Byte
#endif
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r++Ind : Firmware Version : ");

					for(i=0;i<data_length;i++)
						_DBC(data[i]);
#endif
					break;

				case MINOR_ID_LOCAL_ADDRESS_NAME_IND: //0x00 : 0x0F
#if 0//def BT_DEBUG_MSG	
					_DBG("\n\r++Ind : MINOR_ID_LOCAL_ADDRESS_NAME_IND : ");
#endif
					for(i=0;i<6;i++) //Just save Local Address becasue Name is too big(upto N)
					{
						uBT_Local_Address[i] = data[i];
#if 0//def BT_DEBUG_MSG	
						_DBH(data[i]);
#endif
					}
					bPolling_Get_Data |= BCRF_SET_DEVICE_ID_CONTROL; //For init sequence (Init Sequnece : 2) //Last - 2
					break;

				case MINOR_ID_ROUTING_STATE_IND: //0x00 : 0x22
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r++Ind : MINOR_ID_ROUTING_STATE_IND : ");_DBH(data[0]);
#endif
					switch(data[0])
					{
						case 0x00: //0x00 : 0x22 : 0x00 //Routing Disconnect //Mute On
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef AD82584F_ENABLE
							AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
							TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUTO_ONOFF_ENABLE)
							TIMER20_auto_power_flag_Start();
#endif
#endif
							break;
							
						case 0x01: //0x00 : 0x22 : 0x01 // Routing Connect //Mute Off
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
							switch(data[1])
							{
								case 0x02: //0x00 : 0x22 : 0x01 : 0x02 //Analog
#ifdef TIMER20_COUNTER_ENABLE
#ifdef AUTO_ONOFF_ENABLE
								TIMER20_auto_power_flag_Stop();

								if(auto_power_off)
									Remocon_Power_Key_Action(TRUE, TRUE, TRUE);
#endif
#ifdef AD82584F_USE_POWER_DOWN_MUTE
								if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
#endif
								TIMER20_mute_flag_Start();
#endif
								break;

								case 0x06: //0x00 : 0x22 : 0x01 : 0x06 //A2DP
#ifdef USEN_TABLET_AUTO_ON //When user selects power off button under USEN Tablet, we need to enable auto power on.							
								if(USEN_Tablet_auto_power_on)
									Remocon_Power_Key_Action(TRUE, TRUE, TRUE);
#endif
								break;

								case 0x09: //0x00 : 0x22 : 0x01 : 0x09 //Broadcaster
#ifdef TIMER20_COUNTER_ENABLE
#ifdef AD82584F_USE_POWER_DOWN_MUTE
								if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
#endif
								TIMER20_mute_flag_Start();
#endif

								break;

								default:
								break;
							}							
#endif
							break;
							
						default:
							break;
					}

					break;

				case MINOR_ID_ACL_OPENED_IND: //0x00 : 0x23
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r++Ind : MINOR_ID_ACL_OPENED_IND : "); // Data[6]: status: 0x0 -OK, 0x4 - No device, 0xb - Peerdevice busy
#endif
					switch(data[6])
					{
						case 0x0b: //Peerdevice busy
							if(uLast_Connection_Retry_Count) //retry two times
							{
								uLast_Connection_Retry_Count--;
								bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //Retry Last connecton //Reboot recovery solution - 2
							}
						break;

						case 0x00: //Success
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) //This position make a problem that Factory Reset pariing with USEN Tablet
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_LIAC == TWS_Status_Master_Mode_Control)
						{									
							TIMER20_tws_mode_recovery_flag_Stop();
						}
#endif
						break;
						case 0x04: //No Device
#ifdef TWS_MODE_ENABLE
						BTWS_LIAC = TWS_Status_Master_Ready;
#endif
						default:
							uLast_Connection_Retry_Count = 0; //Need to clear in case of other(excepting 0x0b)
						break;
					}
				break;

				case MINOR_ID_ACL_CLOSED_IND: //0x00 : 0x24
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r++Ind : MINOR_ID_ACL_CLOSED_IND : "); //Data[6]: Disconnect : 0x13/BT Delete : 0x16/TWS Disconnection : 0x08(Timeout)
#endif
#ifdef TWS_MODE_ENABLE //2023-05-15_1 : When uBT_TWS_Remote_Device_Address is all 0(0x00 00 00 00 00 00), we need to make recovery here using uBT_Cur_TWS_Device_Address.
					if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
					{
						for(i=0;i<6;i++) //Save Remote BT Device Name //6 Byte
						{	
							if(uBT_TWS_Remote_Device_Address[i] != 0x00)
							{
								i = 0;
								break;
							}
						}

						if(i) //If this is true, it means that uBT_TWS_Remote_Device_Address is all 0.(0x00 00 00 00 00 00). So, we need to make recovery here.
						{
							for(i=0;i<6;i++) //Save Remote BT Device Name //6 Byte
							{						
								uBT_TWS_Remote_Device_Address[i] = uBT_Cur_TWS_Device_Address[i];
							}
						}
					}
#endif
#ifdef TWS_MODE_ENABLE //2023-03-09_3 : When TWS mode is not ACL_CLOSED under TWS Slave mode, we don't need to make Mute On.
					if(!(strncmp(uBT_TWS_Remote_Device_Address, (char *)data, 6) != 0 && Is_TWS_Master_Slave_Connect() != TWS_Get_Information_Ready && Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode))
#endif
					{
#if defined(I2C_0_ENABLE) && defined(TWS_MODE_ENABLE)
						if(!(Aux_In_Exist() && Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
							&& (strncmp(uBT_Cur_A2DP_Device_Address, Address_buf, 6) == 0) //2023-04-04_1 : When ACL_CLOSE and the closed device is A2DP only, we need to make mute.
							) //2023-03-15_3 : Added condition because if Master is Aux mode under TWS mode, we don't need make mute on also .
#ifdef AD82584F_ENABLE //2023-02-03_1 : Need to make Mute ON when BT SPK disconnect with PeerDevice
						AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#else //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
						TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#endif											
#endif //TAS5806MD_ENABLE
#endif //I2C_0_ENABLE
					}

					BBT_Is_Routed = FALSE;

#ifdef TWS_MODE_ENABLE
					if(strncmp(uBT_TWS_Remote_Device_Address, (char *)data, 6) == 0) //TWS device Closed//Compare saved TWS address and current BT address
					{
						BTWS_Master_Slave_Connect = TWS_Get_Information_Ready; //TWS Device is disconnected
#if defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE)
						BSlave_Need_Recovery_Init = FALSE; //2022-12-20_3
#endif
#ifdef BT_DEBUG_MSG
						_DBG("\n\r+++ Dis-connected with TWS Device");
#endif
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
						_DBG("\n\rTWS Name Address = ");
						for(i=0;i<6;i++) //Save Remote BT Device Name //6 Byte
						{
							_DBH(uBT_TWS_Remote_Device_Address[i]);
						}
#endif
						for(i=0;i<6;i++)
						{
#ifdef TWS_MODE_ENABLE //2022-11-07
							uBT_TWS_Remote_Device_Address[i] = 0;
							uLast_Connected_TWS_Device_Address[i] = 0;
							uBT_Cur_TWS_Device_Address[i] = 0;
#endif
						}
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) //Move to here to only work TWS on disconnection case //2022-12-22
#ifdef NEW_TWS_MASTER_SLAVE_LINK
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //2023-04-26_18 : Changed conditon, to recover disconnection for TWS Slave //2023-03-30_2 : Add ConnTermLocalHost(0x16)//TimeOut/OectUser //2022-11-07
#else //NEW_TWS_MASTER_SLAVE_LINK
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_LIAC == TWS_Status_Master_Mode_Control /*&& (data[6] == 0x08 || data[6] == 0x13 || data[6] == 0x16)*/) //2023-03-30_2 : Add ConnTermLocalHost(0x16)//TimeOut/OectUser //2022-11-07
#endif //NEW_TWS_MASTER_SLAVE_LINK
						{	
#ifdef BT_DEBUG_MSG
							_DBG("\n\r+++ Dis-connected with TWS Device Recovery");
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
#ifdef NEW_TWS_MASTER_SLAVE_LINK
							BTWS_Master_Slave_Connect = TWS_Get_Slave_Disconnection; //TWS Device is disconnected
#else //NEW_TWS_MASTER_SLAVE_LINK
							BTWS_Master_Slave_Connect = TWS_Get_Information_Ready;
#endif //NEW_TWS_MASTER_SLAVE_LINK
#endif
							TIMER20_tws_mode_recovery_flag_Start(); //To Recovery TWS Mode Connection

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_19 : When TWS Slave is disconnected with TWS Master, it should display disconnection status using BT Status Blue LED.
							if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
								Set_Status_LED_Mode(STATUS_BT_FAIL_OR_DISCONNECTION_MODE);
#endif
						}
#endif
					}
					else //A2DP or other device Closed //2023-03-08_1
#endif
#ifdef NEW_TWS_MASTER_SLAVE_LINK
					{
#endif
#ifndef MASTER_MODE_ONLY
						if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode) //2023-03-08_1 : Sometimes, Some peerdevice send only MINOR_ID_ACL_CLOSED_IND on disconnection. so we need to add recovery code here.
#endif
						{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
							_DBG("\n\rA2DP Device is disconnected(MINOR_ID_ACL_CLOSED_IND) !!!");
							_DBG("\n\rCur Addr : ");
							for(i=0;i<6;i++)
							_DBH(data[i]);

							_DBG("\n\rA2DP Addr : ");
							for(i=0;i<6;i++)
							_DBH(uBT_Cur_A2DP_Device_Address[i]);
#endif

#ifdef TWS_MODE_ENABLE							
							if(!(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BBT_Is_Connected && strncmp(uBT_Cur_A2DP_Device_Address, (char *)data, 6))) //2023-04-03_2: When TWS slave is conected, we don't need to display BT STATUS LED on TWS Master.
#endif
							{
								if(strncmp(uBT_Cur_A2DP_Device_Address, (char *)data, 6) == 0) //2023-05-30_3 : Under Broadcast mode, when Other A2DP source try to connect SPK even though current A2DP source is connected with SPK, we don't need to display BT STAUS LED(Blinking for disconnection).
								{
									BBT_Is_Connected = FALSE; //2023-03-29_1 : When user disconnects BSP-01 in BT Menu on Peerdevice, If user executes power off->on over power button, Master has BT LED On(It should be blinking).
									bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_READY;
								}
							}

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_13 : If the address of closed device is A2DP, we need to make "BBT_Is_Connected = FALSE"
							if(!strncmp(uBT_Cur_A2DP_Device_Address, (char *)data, 6))
							{
								BBT_Is_Connected = FALSE;
								Peer_Device_Status = PEER_DEVICE_DISCONNECTED;
							}
#endif

						}
#ifdef DEVICE_NAME_CHECK_PAIRING //Disconnect BT
						//uPaired_Device_Count = 0;
						uBT_Remote_Device_Name_Size = 0;
			
						for(i=0; i<6; i++)
						{
							uBT_Remote_Device_Address[i] = 0;
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_13 : When A2DP device is disconnected, we need to clear the adress of cur A2DP device address and Last connection device address
							if(!BBT_Is_Connected)
#endif
							{
								uBT_Cur_A2DP_Device_Address[i] = 0;
								uLast_Connected_Device_Address[i] = 0;
							}
						}
#else
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_13 : When A2DP device is disconnected, we need to clear the adress of cur A2DP device address and Last connection device address
						if(!BBT_Is_Connected)
#endif
						{
							for(i=0; i<6; i++) //Clear variable upon Disconnecting
							{
								uBT_Cur_A2DP_Device_Address[i] = 0;
								uLast_Connected_Device_Address[i] = 0;
							}
						}
#endif

#if 0 //defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) //Move to upside //2022-12-22
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_LIAC == TWS_Status_Master_Mode_Control && (data[6] == 0x08 || data[6] == 0x13)) //TimeOut/OectUser //2022-11-07
						{	
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
							BTWS_Master_Slave_Connect = FALSE;
#endif
							TIMER20_tws_mode_recovery_flag_Start(); //To Recovery TWS Mode Connection
						}
#endif
#ifdef NEW_TWS_MASTER_SLAVE_LINK
					}
#endif
				break;

				default:
					break;
			}
			break;			
			
		case MAJOR_ID_A2DP_CONTROL: //0x10
			switch(minor_id)
			{
				case MINOR_ID_A2DP_PROFIL_STATE_CHANGED_IND: //0x10 : 0x00
#if 0//def BT_DEBUG_MSG
				_DBG("\n\r++MINOR_ID_A2DP_PROFIL_STATE_CHANGED_IND : ");
#endif
				//L3 LED Display Setting - MASTER BT SPK : All Status / SLAVE BT SPK : Initialising Status Only
#if 0 //2023-01-26_1 : Need to save the address of A2DP Device only. So, this statement is moved down.
				for(i=0;i<6;i++) //Just save Local Address becasue Name is too big(upto N)
				{
					uBT_Cur_A2DP_Device_Address[i] = data[i];
				}
#endif
				if(data[6] ==0x00) //0x10 : 0x00 //Initialising
				{
#if 0//def BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Initialising ");
#endif
					bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_INITIALISING;
					BBT_Is_Routed = FALSE;

				}
				else if(data[6] == 0x01) //0x10 : 0x00 //Ready
				{
#if 0//def BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Ready ");
#endif
#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(TWS_MODE_ENABLE) //2022-12-27 When BT SPK disconnected with A2DP peer device only, it executes retry action.
//2023-02-10_1 : Fixed the problem BAP-01 can't reconnect with peerdevice after disconnecting with peerdevice
					for(i=0; i<6; i++)
					{
						Address_buf[i] = data[i];
					}
					//Check if current connected device is A2DP device?
					if(strncmp(uBT_Cur_A2DP_Device_Address, Address_buf, 6) == 0 || (Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)) //2023-02-16_1 : Under TWS slave mode, slave keeps Blue LED On when user executes factory reset of Master. Accroding to spec, but Blue LED should be blinking.
#endif
					{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
						_DBG("\n\rA2DP Device is disconnected(READY) !!!");
#endif
						bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_READY;
						BBT_Is_Routed = FALSE;

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-30_1 : current A2DP device is only available until user select BT Long Key to connect other device.
						Peer_Device_Status = PEER_DEVICE_DISCONNECTED;
#endif
					}
#ifndef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_13 : To clear BT related address in ACL_CLOSE
#ifdef DEVICE_NAME_CHECK_PAIRING //Disconnect BT
					//uPaired_Device_Count = 0;
					uBT_Remote_Device_Name_Size = 0;
		
					for(i=0; i<6; i++)
					{
						uBT_Remote_Device_Address[i] = 0;
						uBT_Cur_A2DP_Device_Address[i] = 0;
					}
#else
					for(i=0; i<6; i++) //Clear variable upon Disconnecting
					{
						uBT_Cur_A2DP_Device_Address[i] = 0;
					}
#endif
#endif //NEW_TWS_MASTER_SLAVE_LINK
				}
				else if(data[6] == 0x02) //0x10 : 0x00 //Connecting
				{
#if 0//def BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Connecting");
#endif
					bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_CONNECTING;
				}
				else if(data[6] == 0x03) //0x10 : 0x00 //Connected
				{
#if 0//def BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Connected ");
#endif
					uLast_Connection_Retry_Count = 0;
#ifdef TWS_MODE_ENABLE //2023-04-03_2 : When TWS slave is conected, we don't need to display BT STATUS LED on TWS Master.
					if(!(((data[7] & 0x03) == 0x03) && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Master_Mode))
#endif
					{
						bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_CONNECTED;
#if defined(TIMER20_COUNTER_ENABLE) && defined(AUTO_ONOFF_ENABLE)
						TIMER20_auto_power_flag_Start();
#endif				
					}
				}
				else //0x10 : 0x00 (data[6] == 0x04) //Disconnecting
				{
#if 0//def BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Disconnecting");
#endif
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2022-12-27 When BT SPK disconnected with A2DP peer device only, it executes retry action.
					for(i=0; i<6; i++)
					{
						Address_buf[i] = data[i];
					}

					//Check if current connected device is A2DP device?
					if(strncmp(uBT_Cur_A2DP_Device_Address, Address_buf, 6) == 0)
#endif
					{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
						_DBG("\n\rA2DP Device is disconnected !!!");
#endif
						bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_DISCONNECTING;
						BBT_Is_Routed = FALSE;
					}
#ifndef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_13 : To clear BT related address in ACL_CLOSE
#ifdef DEVICE_NAME_CHECK_PAIRING
					uBT_Remote_Device_Name_Size = 0;

					for(i=0; i<6; i++)
					{
						uBT_Remote_Device_Address[i] = 0;
						uBT_Cur_A2DP_Device_Address[i] = 0;
					}
#else
					for(i=0; i<6; i++) //Clear variable upon Disconnecting
					{
						uBT_Cur_A2DP_Device_Address[i] = 0;
					}
#endif
#endif //NEW_TWS_MASTER_SLAVE_LINK
				}

#ifdef TWS_MODE_ENABLE //2022-11-17_3 : the BBT_Is_Connected should be TRUE Under A2DP but not Under TWS.
				if(data[6] == 0x03 && data_length == 8) //0x10 : 0x00
				{
					if((data[7] & 0x03) == 0x01) //Normal A2DP
					{
						BBT_Is_Connected = TRUE;

						//2023-01-26_1 : Need to save the address of A2DP Device only. So, this statement is moved down.
						for(i=0;i<6;i++) //Just save Local Address becasue Name is too big(upto N)
						{
							uBT_Cur_A2DP_Device_Address[i] = data[i];
						}
					}
#if defined(SW1_KEY_TWS_MODE) && defined(TWS_MASTER_SLAVE_GROUPING) //2022-12-15 //TWS : SET BCRF_TWS_SET_DEVICE_ID_SAVE to send 
					else
					{
						if((data[7] & 0x03) == 0x03) //TWS Mode
						{
							if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
							{
								if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
								{
#ifdef BT_DEBUG_MSG
									_DBG("\n\rSET BCRF_TWS_SET_DEVICE_ID_SAVE");
#endif
#ifdef FLASH_SELF_WRITE_ERASE
									Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
							
									if(uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff) //we don't to execute this when SET_DEVICE_ID is 0xffffffffffff(6Byte)
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\rTWS Mode connection is already done. return without execution MB3021_BT_TWS_Master_Slave_Grouping_Start() !!!");
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE //2023-02-22_3 : After reboot, TWS Master sends data but TWS Slave ignore it due to AMP initializing. So, we need to send it again for recovery.
										TWS_Power_Init_Master_Send_Data_Start();
#endif
#ifdef NEW_TWS_MASTER_SLAVE_LINK
										BTWS_Master_Slave_Connect = TWS_Get_Slave_Connected; //2023-04-26_10 : Moved to upper side and When SPK get the information of TWS connection, we set "BTWS_Master_Slave_Connect = TWS_Get_Slave_Connected" for after TWS Grouping.
#endif
									}
									else //First TWS Connection
									{
#ifdef NEW_TWS_MASTER_SLAVE_LINK
										BTWS_Master_Slave_Connect = TWS_Get_Slave_Address; //2023-04-26_10 : Moved to upper side and When SPK get the information of TWS connection, we set "BTWS_Master_Slave_Connect = TWS_Get_Slave_Address" during TWS Grouping
#endif
										bPolling_Get_Data |= BCRF_TWS_SET_DEVICE_ID_SAVE;
									}
#endif
									//2023-02-21_2 : To recovery, SPK can't get TWS Address.
									strncpy(uBT_Cur_TWS_Device_Address, (char *)data, 6); //Compare saved TWS address and current BT address
#ifndef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_10
									BTWS_Master_Slave_Connect = TWS_Get_Slave_Address; //2023-04-26_10 : Moved to upper side and devided to two types //2023-02-21_2
#endif

#ifdef TWS_MASTER_SLAVE_GROUPING //2023-01-18_1 : To avoid click noise when Master is connected with Slave under TWS mode
#ifdef AD82584F_USE_POWER_DOWN_MUTE
									if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
#endif
									{
#ifdef AD82584F_ENABLE
										AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
										TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif
#endif //TAS5806MD_ENABLE
										TIMER20_mute_flag_Start();
									}
#endif //TWS_MASTER_SLAVE_GROUPING

								}
#if defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE) //2022-12-20_3
								else
								{
									BSlave_Need_Recovery_Init = TRUE; //2022-12-20_3
									Auto_addtime_for_master_slave_grouping(); //2023-02-21_2 : TWS Slave need to add more time to grouping with Master
#ifdef BT_DEBUG_MSG
									_DBG("\n\rBSlave_Need_Recovery_Init is TRUE !!!");
#endif
								}
#endif
							}
						}
					}
#endif
				}
#else //TWS_MODE_ENABLE
				if(data[6] == 0x03) //0x10 : 0x00
				{
					if((data[7] & 0x03) == 0x01) //NORMAL A2DP
					{
						BBT_Is_Connected = TRUE;
					}
					else
					{
						BBT_Is_Connected = FALSE;
					}
				}

#endif //TWS_MODE_ENABLE
					break;

#if defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE) //2022-11-03
				case MINOR_ID_A2DP_MEDIA_STATE_CHANGED_IND: //0x10 : 0x01
#ifdef BT_DEBUG_MSG	
				_DBG("\n\r+++Ind : MINOR_ID_A2DP_MEDIA_STATE_CHANGED_IND");
#endif
					switch(data[6])
					{
						case 0x3: //0x10 : 0x01 //Streaming
#ifdef TWS_MODE_ENABLE
						if(!Aux_In_Exist() && BTWS_LIAC == TWS_Status_Master_LIAC && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //2023-01-20 : To turn on audio when Master SPK works as standalone under TWS(Last connection case)
						{
#ifdef BT_DEBUG_MSG	
							_DBG("\n\rSend INFORM_HOST_MODE !!!");
#endif								
							bPolling_Get_Data |= BCRF_INFORM_HOST_MODE;
						}
#endif
						BBT_Is_Routed = TRUE;
#ifdef TIMER20_COUNTER_ENABLE
#ifdef AUTO_ONOFF_ENABLE
						TIMER20_auto_power_flag_Stop();

						if(auto_power_off
#ifdef USEN_TABLET_AUTO_ON
 						|| USEN_Tablet_auto_power_on
#endif
						)
							Remocon_Power_Key_Action(TRUE, TRUE, TRUE);
#endif
#ifdef AD82584F_USE_POWER_DOWN_MUTE
						if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
#endif
							TIMER20_mute_flag_Start();
#endif
						break;
						
						case 0x4: //0x10 : 0x01 //Suspended
						BBT_Is_Routed = FALSE;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
						if(!Aux_In_Exist()) //2023-01-18_2 : Added condition. To avoid mute when Master has audio from Aux but BT is changed from "streaming" to "suspend".
						{

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef AD82584F_ENABLE
							AD82584F_Amp_Mute(TRUE, FALSE); //Mute On
#else //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
							TAS5806MD_Amp_Mute(TRUE, FALSE); //Mute On
#endif
#endif //TAS5806MD_ENABLE
						}
#ifdef AUTO_ONOFF_ENABLE
						if(!Aux_In_Exist())
							TIMER20_auto_power_flag_Start();
#endif
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
						break;
						case 0x2: //0x10 : 0x01 //Opened
#ifdef TWS_MODE_ENABLE
#ifdef SW1_KEY_TWS_MODE //2022-12-06_1 : Under connection with PC, TWS master mode working should be use this line when last connection
						if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_LIAC == TWS_Status_Master_GIAC)
						{
							bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; 
						}
#endif
#ifdef TAS5806MD_ENABLE //Need to Amp Init for Opened under Slave / TWS Mode //2022-11-14_2
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && BSlave_Need_Recovery_Init)
						{
							//TWS_Slave_Amp_Init_Start(); //2023-02-22_2
						}

						BSlave_Need_Recovery_Init = FALSE; //2022-12-20_3
						
						break;
#endif
#endif //TWS_MODE_ENABLE
						case 0x1: //0x10 : 0x01 //Closed
#if 0 //2023-03-15_5 : Under TWS Mode, If user repeat connection/disconnection using BT List on Peer Device, sometimes, Slave LED is bliking and then Slave LED is on.
//def TWS_MODE_ENABLE //2023-03-14_1 : When user remove Power plug from Slave under TWS mode, Master sends sync data to slave for a while. it's fixed.
						if(strncmp(uBT_TWS_Remote_Device_Address, (char *)data, 6) == 0) //OK //Compare saved TWS address and current BT address
						{
							BTWS_Master_Slave_Connect = TWS_Get_Information_Ready;
						}
#endif
						default:
						
							if(data[6] == 0x01 && Aux_In_Exist()
#ifndef MASTER_MODE_ONLY
								&& Get_Cur_Master_Slave_Mode() == Switch_Master_Mode
#endif
								) //2023-03-15_4
							{
#ifdef AD82584F_USE_POWER_DOWN_MUTE
								if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
#endif
								TIMER20_mute_flag_Start();
							}
							else
							{
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#if defined(SLAVE_ADD_MUTE_DELAY_ENABLE) && !defined(AVRCP_ENABLE)
								MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef AD82584F_ENABLE
								AD82584F_Amp_Mute(TRUE, FALSE); //Mute On
#else //AD82584F_ENABLE
								TAS5806MD_Amp_Mute(TRUE, FALSE); //Mute On
#endif //TAS5806MD_ENABLE
#ifdef AUTO_ONOFF_ENABLE
								if(!Aux_In_Exist())
									TIMER20_auto_power_flag_Start();
#endif
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
							}
						break;
					}
				break;
#endif
				case MINOR_ID_A2DP_STREAM_ROUTING_CHANGED_IND: //0x10 : 0x02
					switch(data[0])
					{
						case 0x0: //0x10 : 0x02 //UnRouted
						{							
							BBT_Is_Routed = FALSE;

#ifdef AUTO_ONOFF_ENABLE
							if(!Aux_In_Exist())
								TIMER20_auto_power_flag_Start();
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef MASTER_SLAVE_GROUPING 
							if(uNext_Grouping_State > GROUPING_NONE_MODE) //To get MINOR_ID_BA_MODE_CONTROL response under master slave grouping mode //To avoid missing interrupt
								bPolling_Set_Action |= A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE;
							else
#endif
							{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
								MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef USEN_BAP //2023-05-19_1 : Under BAP-01 Aux mode, customer wants to output audio at once when user change BT to Aux.
								if(!Aux_In_Exist())
#endif
								{
#ifdef AD82584F_ENABLE
									AD82584F_Amp_Mute(TRUE, FALSE); //Mute On	
#else //AD82584F_ENABLE
									TAS5806MD_Amp_Mute(TRUE, FALSE); //Mute On
#endif //TAS5806MD_ENABLE
								}
							}
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
						}
						break;
						
						case 0x1: //0x10 : 0x02 //Routed
						BBT_Is_Routed = TRUE;
#ifdef TIMER20_COUNTER_ENABLE
#ifdef AUTO_ONOFF_ENABLE
						TIMER20_auto_power_flag_Stop();

						if(auto_power_off)
							Remocon_Power_Key_Action(TRUE, TRUE, TRUE);
#endif
#ifdef USEN_TABLET_AUTO_ON
						if(USEN_Tablet_auto_power_on)
						{
							if(!Aux_In_Exist())
								Remocon_Power_Key_Action(TRUE, TRUE, TRUE);

							USEN_Tablet_auto_power_on = FALSE;
						}
#endif
#ifdef AD82584F_USE_POWER_DOWN_MUTE
						if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
#endif
						TIMER20_mute_flag_Start();
#endif
						break;
						
						default:
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
						MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
#endif
#ifdef AD82584F_ENABLE
						AD82584F_Amp_Mute(TRUE, FALSE); //Mute On
#else //AD82584F_ENABLE
						TAS5806MD_Amp_Mute(TRUE, FALSE); //Mute On
#endif //TAS5806MD_ENABLE
#ifdef AUTO_ONOFF_ENABLE
						if(!Aux_In_Exist())
							TIMER20_auto_power_flag_Start();
#endif
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
						break;
					}
					break;
					
#if defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE) //2022-11-03
				case MINOR_ID_A2DP_SELECTED_CODEC_IND: //0x10 : 0x05
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\r###MINOR_ID_A2DP_SELECTED_CODEC_IND");
#endif
#ifdef AVRCP_ENABLE
					if(BBT_Is_Last_Connection_for_AVRCP == TRUE)
					{
						BBT_Is_Last_Connection_for_AVRCP = FALSE;
						
						if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
						{
							bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)// Last connection is success(Last -7) !!! //To Do !!!
						}
					}
#endif
				}
				break;
#endif

				case MINOR_ID_A2DP_CONNECT_FAIL_ERRORCODE_IND: //0x10 : 0x70 //LAST CONNECTION FAIL
#ifdef BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Disconnecting");
					_DBG("\n\r###LAST CONNECTION FAIL");
#endif
					//Need to delete below because Aux is NG when Power Off --> Plug out --> Plug In
					//bPolling_Get_Data = 0;
					//bPolling_Get_BT_Profile_State = 0;
#ifdef MASTER_SLAVE_GROUPING
					bPolling_Set_Action =0;
#endif
#ifdef TWS_MODE_ENABLE
					for(i=0; i<6; i++)
					{
						Address_buf[i] = data[i];
					}
					
					if(!(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && BBT_Is_Connected && strncmp(uBT_Cur_A2DP_Device_Address, Address_buf, 6))) //2023-04-03_2 : When TWS slave is conected, we don't need to display BT STATUS LED on TWS Master.
#endif
					{
						bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_DISCONNECTING;
						BBT_Is_Connected = FALSE;
					}

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_15 : To make current status of Peer device(A2DP) and TWS Slave device
					Peer_Device_Status = PEER_DEVICE_NONE;
					TWS_Slave_Status = TWS_SLAVE_NONE_CONNECTION;
#endif

					break;					
					
				default:
					break;
			}
			break;

#if defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE)
		case MAJOR_ID_AVRCP_CONTROL: //0x11
			switch(minor_id)
			{
#if 1//def AVRCP_ENABLE //To avoid missing MINOR_ID_A2DP_SELECTED_CODEC_IND interrupt //Need to Check !!!
				case MINOR_ID_AVRCP_CAPABILITY_LIST_IND: //0x11 : 0x03
				{
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
					if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_LIAC == TWS_Status_Master_GIAC)
					{
						bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; 
					}
#endif
#ifdef BT_DEBUG_MSG
					_DBG("\n\r###MINOR_ID_A2DP_SELECTED_CODEC_IND");
#endif
#ifdef AVRCP_ENABLE
					if(BBT_Is_Last_Connection_for_AVRCP == TRUE)
					{
						BBT_Is_Last_Connection_for_AVRCP = FALSE;
						
						if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
						{
							bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)// Last connection is success(Last -7) !!! //To Do !!!
						}
					}
#endif
				}
				break;
#endif //AVRCP_ENABLE			
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
				case MINOR_ID_AVRCP_PROFILE_STATE_CHANGED_IND: //0x11 : 0x00 //Data[0 ~ 5] : BT Address/Data[6] : Profile State
				{
					if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && (data[6] == 0x03)) //Connected
					{
						if(strncmp(uBT_TWS_Remote_Device_Address, (char *)data, 6) == 0) //OK //Compare saved TWS address and current BT address
						{
							strncpy(uBT_Cur_TWS_Device_Address, uBT_TWS_Remote_Device_Address, 6); //Saved current TWS address
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_10							
							BTWS_Master_Slave_Connect = TWS_Get_Slave_Connected;
#else //NEW_TWS_MASTER_SLAVE_LINK
							BTWS_Master_Slave_Connect = TWS_Get_Slave_Information_Done;
#endif //NEW_TWS_MASTER_SLAVE_LINK
#ifdef BT_DEBUG_MSG
							_DBG("\n\r+++ Connected with TWS Device");
#endif
#ifndef TWS_MODE_AP_TEST
							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
							{
#ifdef TAS5806MD_ENABLE
								//TAS5806MD_Amp_Init(FALSE); //2023-02-22_2 : Disable AMP Init that is for AMP recovery //2022-11-14_4 : After Master/Slave connection, Sync Master status with Slave
								//TAS5806MD_Amp_Volume_Set_with_Index(TAS5806MD_Amp_Get_Cur_Volume_Level(), FALSE, TRUE); //2022-11-14 : After Master/Slave connection, Sync Master status with Slave
#else
#ifdef AD82584F_ENABLE
								AD82584F_Amp_Volume_Set_with_Index(AD82584F_Amp_Get_Cur_Volume_Level(), FALSE, TRUE);
#endif
#endif
								bPolling_Get_Data |= BCRF_TWS_ROUTING_MODE_CONTROL; //2022-11-17_1 : To set Master SPK as left channel(Slave should be Right channel after this command)
							}
#endif //TWS_MODE_AP_TEST
						}
						else
						{
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_17 : To save TWS Master address under TWS Slave mode.
							if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
							{
								strncpy(uBT_TWS_Remote_Device_Address, (char *)data, 6);
#ifdef BT_DEBUG_MSG
								_DBG("\n\r+++ Slave is connected with TWS Master");
#endif
							}
#endif
							//BTWS_Master_Slave_Connect = FALSE; //Can't set this variable to FALSE here !!! Because A2DP come here also.
						}
					}
					else
					{
						//BTWS_Master_Slave_Connect = FALSE; //Can't set this variable to FALSE here !!! Because A2DP come here also.
					}
				}
				break;

				case MINOR_ID_AVRCP_SEND_SYNC_DATA_IND: //0x11 : 0x13 //Data[0-8] : User Data/Data[9-14] : Address of Remote Device
				{
					if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
					{
#ifdef BLE_DATA_GET_FROM_MASTER_SPK
						static Bool BAlready_Set_Vol1; //To set volume level from Master's information when power on and do not set volume level in power on function.
						static Bool BNeed_Mute_Off1; //To set mute off from Master's information when power on and do not set mute off in power on function.
							
						BAlready_Set_Vol1 = FALSE;
						BNeed_Mute_Off1 = FALSE;
	
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
						if(Get_Cur_LR_Stereo_Mode() != Switch_LR_Mode)
							break;
#ifdef TAS5806MD_ENABLE
						if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE) //2023-02-27_2 //2023-02-22_1 : TWS Slave BT SPK executes Amp init again. Sometimes, BT SPK get this interrupt during Amp Init and Amp Init has wrong data.
						{
#ifdef TAS5806MD_DEBUG_MSG
							_DBG("\n\r+++ Is_BAmp_Init is TRUE - 0");
#endif
							break;
						}
#endif
#endif
#ifdef BT_DEBUG_MSG
						_DBG("\n\r+++Ind : MINOR_ID_AVRCP_SEND_SYNC_DATA_IND");
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
						if(data[0+6] == 0xAA && data_length == (0x09+6)) //Remocon Data thru AVRCP
#else
						if(data[0+6] == 0xAA && data_length == (0x08+6)) //Remocon Data thru AVRCP
#endif
						{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							//Data[0] = Header, Data[1~7] = Remocon Data, Data[8] = Checksum
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data+6, 8); //using checksum Data check whether it's valid or invalid
#else
							//Data[0] = Header, Data[1~6] = Remocon Data, Data[7] = Checksum
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data+6, 7); //using checksum Data check whether it's valid or invalid
#endif

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							if(bChecksum != data[8+6])
#else
							if(bChecksum != data[7+6])
#endif
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rReceive data thru AVRCP communication is NG because check sum is NG");
								_DBH(bChecksum);
								_DBG("/");
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
								_DBH(data[8+6]);
#else
								_DBH(data[7+6]);	
#endif
#endif
								BRet = FALSE;
								
								break;
							}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							if(!MB3021_BT_Module_Check_Vailid_BLE_Remote_Data(data+1+6, 7)) //Check whether AVRCP remote Data is valid or not
#else
							if(!MB3021_BT_Module_Check_Vailid_BLE_Remote_Data(data+1+6, 6)) //Check whether AVRCP remote Data is valid or not
#endif
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rReceive data thru AVRCP communication is NG because some Data have invalid value");
#endif
								BRet = FALSE;
								break;
							}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							for(i = 1; i<8; i++)
#else					
							for(i = 1; i<7; i++)
#endif
							{
#ifdef SLAVE_VOLUME_FORCED_SYNC
								if((i-1) == BLE_VOLUME_KEY) //Need to update volume info even though invalid value becasue Slave can get different volume level.
									uBLE_Remocon_Data_bk[i-1] = 0xff;
								else
#endif
								uBLE_Remocon_Data_bk[i-1] = uBLE_Remocon_Data[i];

								if(!Power_State()) //When Power is off, all keys execepting Power Key are ignored.
								{
									if(data[1+6] != 0x01) //POWER_KEY
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r2. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
										uBLE_Remocon_Data[1] = data[1+6]; //Need to save current power key information. if user turn off/on Master under Slave power off mode, Slave can't turn on.
										uBLE_Remocon_Data_bk[1] = data[1+6];
										BRet = FALSE;
										
										break;
									}
								}

								if(uBLE_Remocon_Data_bk[i-1] != data[i+6])
								{
									switch(i-1)
									{
										case BLE_POWER_KEY: //##AVRCP## Power On/Off - Power On : 0x01/Power Off : 0x00 //Power Key Action should be executed here because when Power off, we need to save amp value before Power Off
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 1.Power On/Off : For Slave");
#endif
											if(data[1+6] == 0x01)
												Power_Key_Action = REMOTE_POWER_ON_ACTION;// Remocon_Power_Key_Action(TRUE);
											else if(data[1+6] == 0x00)
											{
												Power_Key_Action = REMOTE_POWER_OFF_ACTION; //Remocon_Power_Key_Action(FALSE);
												//break; //When the key is Power_Off_Key, We just set Power_Off_Key only excepting other Keys !!! 
											}
											else
											{
												Power_Key_Action = REMOTE_POWER_NONE_ACTION; //Remocon_Power_Key_Action(FALSE);
												BRet = FALSE;
											}
										}
										break;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
										case BLE_MUTE_KEY: //##AVRCP## Mute On/Off - Mute On : 0x01/Mute Off : 0x00
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 2.Mute On/Off : For Slave");
#endif
											if(data[2+6] == 0x01)
											{
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(TRUE, TRUE);
#else
												TAS5806MD_Amp_Mute(TRUE, TRUE);
#endif
#ifdef AUTO_VOLUME_LED_OFF
												bMute = TRUE;
#endif
											}
											else if(data[2+6] == 0x00)
											{
#if 0//def SLAVE_ADD_MUTE_DELAY_ENABLE //Don't use this statement because this makes mute on state even though it must be mute off
												if(Is_TIMER20_mute_flag_set() == 0)
#endif
												BNeed_Mute_Off1 = TRUE; //Power state is power off on booting so, Amp mute function do not work and need this flag to mute off.

#if defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE) //2022-12-06 : To delete audio noise(Slave abnormal volume) under TWS Mode, we keep audio mute untill CLK is stable
												// if(TAS5806MD_CLK_Detect_Count() == 0xffffffff || Get_Cur_LR_Stereo_Mode() != Switch_LR_Mode)
												if(TAS5806MD_CLK_Detect_Count() != 0xffffffff && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //2023-03-08_4 : Changed condition because Slave can't mute off after TWS pairing with Master and then rebooting even though user push mute off on Master after rebooting.
												{
#ifdef AD82584F_USE_POWER_DOWN_MUTE
													Set_Display_Mute(FALSE);
#endif
													TIMER20_mute_flag_Start();
												}
												else
#endif
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(FALSE, TRUE);
#else
												TAS5806MD_Amp_Mute(FALSE, TRUE);
#endif
											}
											else
												BRet = FALSE;
										}
										break;

										case BLE_VOLUME_KEY: //##AVRCP## Volume Setting - Level 0(0x00) ~ Level 15(0x0f)
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 3.Volume Setting : For Slave");
#endif
											if(data[3+6] <= 0x0f)
											{
#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(LR_360_FACTORY_ENABLE) //2023-04-26_22 : When Power Off and Power on under Slave mode, Slave can't display Volume Level LED. This is side effect of //2023-04-06_3
												if(Power_State())
#endif
												{
#ifdef AD82584F_ENABLE
													AD82584F_Amp_Volume_Set_with_Index(data[3+6], TRUE, FALSE);
#else //AD82584F_ENABLE
													TAS5806MD_Amp_Volume_Set_with_Index(data[3+6], TRUE, FALSE);
#endif //TAS5806MD_ENABLE
													BAlready_Set_Vol1 = TRUE;
												}
											}
											else
												BRet = FALSE;
										}
										break;

										case BLE_EQ_KEY: //##AVRCP## Sound Effect(EQ) - Normal : 0x00/POP&ROCK : 0x01/CLUB : 0x02/JAZZ : 0x03/VOCAL : 0x04
										{
#ifdef BT_DEBUG_MSG					
											_DBG("\n\rxxx 4.Sound Effect(EQ) : For Slave");
#endif
											//To Do !!! EQ control function
											if(data[4+6] <= 0x04)
											{
#ifdef FIVE_USER_EQ_ENABLE
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#else //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
												TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#endif
#endif //TAS5806MD_ENABLE
												TIMER20_user_eq_mute_flag_start();	

												uBuf = data[4+6];
#ifdef USEN_TI_AMP_EQ_ENABLE //2023-02-27_1
												TAS5806MD_Amp_EQ_DRC_Control((EQ_Mode_Setting)uBuf);
#else
												MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uBuf);
#endif
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
												FlashSaveData(FLASH_SAVE_DATA_EQ, uBuf);
#endif
#endif
#ifdef AUTO_VOLUME_LED_OFF
												TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
#ifdef AD82584F_ENABLE
												uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();

												AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#else //AD82584F_ENABLE
												uVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
											
												TAS5806MD_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#endif //TAS5806MD_ENABLE
#endif
											}
											else
												BRet = FALSE;
										}
										break;
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)

										case BLE_REBOOT_KEY: //##AVRCP## Reboot On - Reboot On : 0x01/Reboot Off : 0x00
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 5.Reboot : For Slave");
#endif
											if(data[5+6] == 0x01)
												SW_Reset();
											else if(data[5+6] == 0x00)
											{
												NOP();
#ifdef BT_DEBUG_MSG
												_DBG("\n\rReboot Off - To Do !!!");
#endif
											}
											else
												BRet = FALSE;
										}
										break;

										case BLE_FACTORY_RESET_KEY: //##AVRCP## Factory Reset Key - Factory Reset On : 0x01/Factory Reset Off : 0x00
#ifdef PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE
										break; //Just ignore Factory Reset over AVRCP Data. Don't set BRet = FALSE because Power Action is not worked
#else //PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE										
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 6.Factory : For Slave");
#endif
											if(data[6+6] == 0x01)
											{
#ifdef SWITCH_BUTTON_KEY_ENABLE
												bFactory_Reset_Mode = TRUE;
#endif
#ifdef AUTO_VOLUME_LED_OFF
												TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
												uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
												AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#endif
												Factory_Reset_Value_Setting();
											}								
											else if(data[6+6] == 0x00)
											{
												NOP();
#ifdef BT_DEBUG_MSG
												_DBG("\n\rFactory Reset Off 2 - To Do !!!");
#endif
											}
											else
												BRet = FALSE;
										}
										break;
#endif //PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
										case BLE_MUTE_OFF_DELAY: //##AVRCP## 
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 7.Mute Off Delay : For Slave");
#endif						
											if(data[7+6] == 0x01)
												TIMER20_mute_flag_Start();
											else if(data[7+6] == 0x02)
											{
												TIMER20_mute_flag_Stop();
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //AD82584F_ENABLE
												TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
											}
											else
											{
												TIMER20_mute_flag_Stop();
#ifdef BT_DEBUG_MSG
												_DBG("\n\rBLE_MUTE_OFF_DELAY - No Effect !!!");
#endif
											}

											data[7+6] = 0; //To clear Mute off delay
										}
										break;
#endif
										default:
										break;
									}
								}
							}

							if(BRet)
							{
								for(i = 0; i<8; i++)
								{
									uBLE_Remocon_Data[i] = data[i+6]; //Save AVRCP Data finally !!!
								}
#ifdef AUTO_VOLUME_LED_OFF
								if(bMute == FALSE)
									TIMER20_auto_volume_led_off_flag_Start();
								else
									bMute = TRUE;
#endif
								switch(Power_Key_Action) //Power Key Action should be executed here because when Power off, we need to save amp value before Power Off
								{
									case REMOTE_POWER_ON_ACTION:
#ifdef SWITCH_BUTTON_KEY_ENABLE
										if(BAlready_Set_Vol1)
											Remocon_Power_Key_Action(TRUE, FALSE, FALSE);
										else
											Remocon_Power_Key_Action(TRUE, FALSE, TRUE);

										if(data[2+6] == 0x01) //Mute On. When Power off state under Slave mode, we need to set mute on here.
										{
											TIMER20_mute_flag_Stop();
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Mute(TRUE, TRUE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Mute(TRUE, TRUE);
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
											
#if defined(UART_10_ENABLE) && defined(MB3021_ENABLE) //When Power on/off under slave mute on, slave miss BLE commnucation from Master very first one time.
											/*UART USART10 Configure*/
											Serial_Init(SERIAL_PORT10, 115200);
#endif
#ifdef AUTO_VOLUME_LED_OFF
											bMute = TRUE;
#endif
										}

										if(BNeed_Mute_Off1) //Mute off. When Power off state under Slave mode, we need to set mute off here.
										{
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#if defined(TWS_MODE_ENABLE) && defined(TAS5806MD_ENABLE) //2022-12-06 : To delete audio noise(Slave abnormal volume) under TWS Mode, we keep audio mute untill CLK is stable
											if(TAS5806MD_CLK_Detect_Count() == 0xffffffff || Get_Cur_LR_Stereo_Mode() != Switch_LR_Mode)
#endif
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Mute(FALSE, TRUE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Mute(FALSE, TRUE);
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
											BNeed_Mute_Off1 = FALSE;
										}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Fixed Slave SPK has pop-noise when Power Plug-in
										//TIMER20_mute_flag_Start(); 
#endif //SLAVE_ADD_MUTE_DELAY_ENABLE
#endif
									break;
									case REMOTE_POWER_OFF_ACTION:
#ifdef SWITCH_BUTTON_KEY_ENABLE
										Remocon_Power_Key_Action(FALSE, FALSE, TRUE);
#endif
									break;

									default:
									break;
								}
							}
							/*else
								bPolling_Get_Data |= BCRF_SEND_BLE_REMOTE_DATA_NG; //Send BLE Response OK*/
#ifdef BT_DEBUG_MSG
							_DBG("\n\rAVRCP Data :");

							for(i = 0; i<8; i++)
							{
								_DBH(uBLE_Remocon_Data[i]);
							}
#endif
						}
#ifdef TWS_MASTER_SLAVE_GROUPING //2022-12-15 //TWS : AVRCP Data Receive under TWS Slave Mode
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
						else if(data[0+6] == 0xee && data_length == (0x09+6)) //Remocon Data thru AVRCP
#else
						else if(data[0+6] == 0xee && data_length == (0x08+6)) //Remocon Data thru AVRCP
#endif
						{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							//Data[0] = Header, Data[1~7] = Remocon Data, Data[8] = Checksum
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data+6, 8); //using checksum Data check whether it's valid or invalid
#else
							//Data[0] = Header, Data[1~6] = Remocon Data, Data[7] = Checksum
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data+6, 7); //using checksum Data check whether it's valid or invalid
#endif

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							if(bChecksum != data[8+6])
#else
							if(bChecksum != data[7+6])
#endif
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rReceive data thru AVRCP communication is NG because check sum is NG");
								_DBH(bChecksum);
								_DBG("/");
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
								_DBH(data[8+6]);
#else
								_DBH(data[7+6]);	
#endif
#endif								
								break;
							}

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							for(i = 1; i<8; i++)
#else					
							for(i = 1; i<7; i++)
#endif
							{
								if(data[i+6] == 0x00 || data[i+6] == 0xFF) //the Data should be 0x01 ~ 0xFE
									break;
							}

							//2022-12-15 //TWS : Save SET_DEVICE_ID(6 Byte) to Flash under TWS Slave
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							for(i = 1; i<7; i++)
#else					
							for(i = 1; i<6; i++)
#endif
							{
								FlashSave_SET_DEVICE_ID((FLASH_SAVE_SET_DEVICE_ID_0+i-1), data[i+6]);
							}
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2 : To save current TWS mode under Slave(Master - 0x01 or Slave - 0x02)								
								FlashSave_SET_DEVICE_ID(FLASH_TWS_MASTER_SLAVE_ID, 0x02);
#endif
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_7 : Slave connection To make New TWS Connection, we need to reset after 5sec since TWS Master sent SET_DEVICE_IDE to TWS Slave
							TIMER20_TWS_Grouping_send_flag_start();
#endif
#ifdef BT_DEBUG_MSG
							_DBG("\n\rFlash Save Data :");

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							for(i = 1; i<7; i++)
#else					
							for(i = 1; i<6; i++)
#endif
							{
								_DBH(data[i+6]);
								_DBG(", ");
							}
#endif
						}
#endif //TWS_MASTER_SLAVE_GROUPING
#endif //BLE_DATA_GET_FROM_MASTER_SPK
					}
				}
				break;
#endif

#ifndef TWS_MODE_ENABLE
				case MINOR_ID_AVRCP_PLAYBACK_STATUS_CHANGED_IND: //0x11 : 0x01
				break;
				
				case MINOR_ID_AVRCP_VOLUME_CHANGED_IND: //0x11 : 0x02
				{
#if 0//def BT_DEBUG_MSG
					_DBG("\n\r###AVRCP :");
#endif

					if(data[6] > 123)
						uVolume = 15;
					else if(data[6] > 114)
						uVolume = 14;
					else if(data[6] > 106)
						uVolume = 13;
					else if(data[6] > 97)
						uVolume = 12;
					else if(data[6] > 89)
						uVolume = 11;
					else if(data[6] > 80)
						uVolume = 10;
					else if(data[6] > 72)
						uVolume = 9;
					else if(data[6] > 63)
						uVolume = 8;
					else if(data[6] > 55)
						uVolume = 7;
					else if(data[6] > 46)
						uVolume = 6;
					else if(data[6] > 38)
						uVolume = 5;
					else if(data[6] > 29)
						uVolume = 4;
					else if(data[6] > 21)
						uVolume = 3;
					else if(data[6] > 12)
						uVolume = 2;
					else if(data[6] > 4)
						uVolume = 1;
					else
						uVolume = 0;

					uVolume = AD82584F_Amp_Volume_Set_with_Index(uVolume, TRUE, FALSE); //Set Volume and Save Volume
					AD82584F_Amp_Volume_Set_with_Index(uVolume, FALSE, TRUE); //Send Volume information to Slave
				}
				break;
#endif //TWS_MODE_ENABLE
				default:
					break;
			}
			break;

#endif //defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE)

		case MAJOR_ID_SPP_CONTROL: //0x13
			switch(minor_id)
			{
				case MINOR_ID_SPP_PROFILE_STATE_CHANGED_IND: //0x13 : 0x00
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r+++Ind : MINOR_ID_SPP_PROFILE_STATE_CHANGED_IND");
#endif
					{//Data[0-5] : BDA/Data[6]:Profile Statue/Data[7]:Connection Channel Index(0x1 / Android SPP)
#ifndef MASTER_MODE_ONLY
						Switch_Master_Slave_Mode mode;

						mode = Get_Cur_Master_Slave_Mode();

						if(mode == Switch_Master_Mode)
#endif
						{					
							for (i=0;i<6;i++)
								uCur_SPP_Device_Address[i] = data[i]; //For SPP Communication

							if(data[6] == 0x03)
								BSPP_Ready_OK = TRUE;
							else
								BSPP_Ready_OK = FALSE;
						}
					}
					break;
				
				default:
					break;
			}
			break;

		case MAJOR_ID_BA_CONTROL: //0x16
			switch(minor_id)
			{
				case MINOR_ID_BA_MODE_CONTROL_IND: //0x16 : 0x00
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r+++Ind : MINOR_ID_BA_MODE_CONTROL");
#endif
#ifdef MASTER_SLAVE_GROUPING
					if(uNext_Grouping_State > GROUPING_NONE_MODE) //Groping mode don't need next step here !!!
						break;
#endif
					{
#ifndef MASTER_MODE_ONLY
						Switch_Master_Slave_Mode mode;

						mode = Get_Cur_Master_Slave_Mode();

						if(mode == Switch_Master_Mode)
#endif
						{
							bPolling_Get_Data |= BCRF_SET_BLE_MANUFACTURE_DATA; //For init sequence (Init Sequnece : Broadcaster -4) //For init sequence (Init Sequnece : Receiver -2)
#ifdef AVRCP_ENABLE
							BBT_Need_Sequence = TRUE;
#endif
							TIMER20_Forced_Input_Audio_Path_Setting_flag_start(); //To avoid, Audio audio output NG
						}
#ifndef MASTER_MODE_ONLY
						else
						{
							if(data[0] == 0x00) //Normal Mode - Try Receiver Re-Connection
							{
								bPolling_Get_Data |= BCRF_BA_MODE_CONTROL;
							}
							else
								bPolling_Get_Data |= BCRF_SET_BLE_MANUFACTURE_DATA; //For init sequence (Init Sequnece : Receiver -2)
#if 0//def BT_DEBUG_MSG								
							_DBG("\n\r+++Ind : Receiver Mode Done");
#endif
						}
#endif
					}
					break;

				case MINOR_ID_RECEIVER_CONNECTION_STATUS_IND: //0x16 : 0x02
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\r+++Ind : MINOR_ID_RECEIVER_CONNECTION_STATUS_IND");
#endif
					{
#ifndef MASTER_MODE_ONLY
						Switch_Master_Slave_Mode mode;

						mode = Get_Cur_Master_Slave_Mode();
#if 0//def BT_DEBUG_MSG	
						_DBH(mode);
#endif
						if(mode == Switch_Slave_Mode)
						{
							switch(data[0])
							{
								case 0x01: //0x16 : 0x02 : 0x01 //Disconnected
#if 0//def BT_DEBUG_MSG
									_DBG("\n\r###Profile State : Connected ");
#endif
									bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_DISCONNECTING; //Actually BT_PROFILE_STATE_READY
								break;
								
								case 0x03: //0x16 : 0x02 : 0x03 //Connected
#if 0//def BT_DEBUG_MSG
									_DBG("\n\r###Profile State : Connected ");
#endif
									uLast_Connection_Retry_Count = 0;
									bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_CONNECTED;
								break;

								default:
#ifdef BT_DEBUG_MSG
									_DBG("\n\r###NG : ");
									_DBH(data[0]);
#endif

								break;
							}

							if(data[0] == 0x03) //0x16 : 0x02 : 0x03
								BBT_Is_Connected = TRUE;
							else
								BBT_Is_Connected = FALSE;
						}
#endif
					}
				break;

				case MINOR_ID_BA_MANUFACTURE_DATA_IND: //0x16 : 0x05
				{
#ifdef BLE_DATA_GET_FROM_MASTER_SPK
					static Bool BAlready_Set_Vol; //To set volume level from Master's information when power on and do not set volume level in power on function.
					static Bool BNeed_Mute_Off; //To set mute off from Master's information when power on and do not set mute off in power on function.
					Switch_Master_Slave_Mode mode;
						
					BAlready_Set_Vol = FALSE;
					BNeed_Mute_Off = FALSE;
					mode = Get_Cur_Master_Slave_Mode();

					if(mode == Switch_Slave_Mode) //BLE Data Get from Master SPK
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\r+++Ind : MINOR_ID_BA_MANUFACTURE_DATA_IND");
#endif
#ifdef TAS5806MD_ENABLE
						if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE) //2023-02-27_2 //2023-02-22_1 : TWS Slave BT SPK executes Amp init again. Sometimes, BT SPK get this interrupt during Amp Init and Amp Init has wrong data.
						{
#ifdef TAS5806MD_DEBUG_MSG
							_DBG("\n\r+++ Is_BAmp_Init is TRUE - 11");
#endif
							break;
						}
#endif
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
						if(data[0] == 0xAA && data_length == 0x09) //Remocon Data thru BLE
#else
						if(data[0] == 0xAA && data_length == 0x08) //Remocon Data thru BLE
#endif
						{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							//Data[0] = Header, Data[1~7] = Remocon Data, Data[8] = Checksum
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 8); //using checksum Data check whether it's valid or invalid
#else
							//Data[0] = Header, Data[1~6] = Remocon Data, Data[7] = Checksum
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 7); //using checksum Data check whether it's valid or invalid
#endif

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							if(bChecksum != data[8])
#else
							if(bChecksum != data[7])
#endif
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rReceive data thru BLE communication is NG because check sum is NG");
								_DBH(bChecksum);
								_DBG("/");
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
								_DBH(data[8]);
#else
								_DBH(data[7]);	
#endif
#endif
								//bPolling_Get_Data |= BCRF_SEND_BLE_REMOTE_DATA_NG; //Send BLE Response NG
								BRet = FALSE;
								
								break;
							}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							if(!MB3021_BT_Module_Check_Vailid_BLE_Remote_Data(data+1, 7)) //Check whether BLE remote Data is valid or not
#else
							if(!MB3021_BT_Module_Check_Vailid_BLE_Remote_Data(data+1, 6)) //Check whether BLE remote Data is valid or not
#endif
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rReceive data thru BLE communication is NG because some Data have invalid value");
#endif
								BRet = FALSE;
								break;
							}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
							for(i = 1; i<8; i++)
#else					
							for(i = 1; i<7; i++)
#endif
							{
#ifdef SLAVE_VOLUME_FORCED_SYNC
								if((i-1) == BLE_VOLUME_KEY) //Need to update volume info even though invalid value becasue Slave can get different volume level.
									uBLE_Remocon_Data_bk[i-1] = 0xff;
								else
#endif
								uBLE_Remocon_Data_bk[i-1] = uBLE_Remocon_Data[i];

								if(!Power_State()) //When Power is off, all keys execepting Power Key are ignored.
								{
									if(data[1] != 0x01) //POWER_KEY
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r7. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
										uBLE_Remocon_Data[1] = data[1]; //Need to save current power key information. if user turn off/on Master under Slave power off mode, Slave can't turn on.
										uBLE_Remocon_Data_bk[1] = data[1];
										BRet = FALSE;
										
										break;
									}
								}

								if(uBLE_Remocon_Data_bk[i-1] != data[i])
								{
									switch(i-1)
									{
										case BLE_POWER_KEY: //##BLE## Power On/Off - Power On : 0x01/Power Off : 0x00 //Power Key Action should be executed here because when Power off, we need to save amp value before Power Off
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 1.Power On/Off : For Slave");
#endif
											if(data[1] == 0x01)
												Power_Key_Action = REMOTE_POWER_ON_ACTION;// Remocon_Power_Key_Action(TRUE);
											else if(data[1] == 0x00)
											{
												Power_Key_Action = REMOTE_POWER_OFF_ACTION; //Remocon_Power_Key_Action(FALSE);
												//break; //When the key is Power_Off_Key, We just set Power_Off_Key only excepting other Keys !!! 
											}
											else
											{
												Power_Key_Action = REMOTE_POWER_NONE_ACTION; //Remocon_Power_Key_Action(FALSE);
												BRet = FALSE;
											}
										}
										break;
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
										case BLE_MUTE_KEY: //##BLE## Mute On/Off - Mute On : 0x01/Mute Off : 0x00
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 2.Mute On/Off : For Slave");
#endif
											if(data[2] == 0x01)
											{
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(TRUE, TRUE);
#else
												TAS5806MD_Amp_Mute(TRUE, TRUE);
#endif
#ifdef AUTO_VOLUME_LED_OFF
												bMute = TRUE;
#endif
											}
											else if(data[2] == 0x00)
											{
#if 0//def SLAVE_ADD_MUTE_DELAY_ENABLE //Don't use this statement because this makes mute on state even though it must be mute off
												if(Is_TIMER20_mute_flag_set() == 0)
#endif
												BNeed_Mute_Off = TRUE; //Power state is power off on booting so, Amp mute function do not work and need this flag to mute off.
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(FALSE, TRUE);
#else
												TAS5806MD_Amp_Mute(FALSE, TRUE);
#endif

											}
											else
												BRet = FALSE;
										}
										break;

										case BLE_VOLUME_KEY: //##BLE## Volume Setting - Level 0(0x00) ~ Level 15(0x0f)
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 3.Volume Setting : For Slave");
#endif
											if(data[3] <= 0x0f)
											{
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_2 : To disable BLE_VOLUME_KEY using BLE DATA from Master under BAP slave when Master & Slave are BAP
												if(B_Master_Is_BAP) //Master is BAP-01
													BRet = TRUE;
												else
												{
#ifdef ADC_VOLUME_64_STEP_ENABLE
													data[3] = Convert_16Step_to_64Step(data[3]); //2023-01-09_1 : To convert from 16-step to 64-step
#else //ADC_VOLUME_50_STEP_ENABLE
													data[3] = Convert_16Step_to_50Step(data[3]); //2023-02-27_3 : To convert from 16-step to 50-step
#endif //ADC_VOLUME_64_STEP_ENABLE
#ifdef AD82584F_ENABLE
													AD82584F_Amp_Volume_Set_with_Index(data[3], TRUE, FALSE);
#else //AD82584F_ENABLE
													TAS5806MD_Amp_Volume_Set_with_Index(data[3], TRUE, FALSE);
#endif //TAS5806MD_ENABLE
													BAlready_Set_Vol = TRUE;
												}
#else //#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
												if(Power_State()) //2023-05-15_4 : When Power Off and Power on under Broadcast Slave mode, Slave can't display Volume Level LED. This is side effect of //2023-04-06_3
												{
#ifdef AD82584F_ENABLE
													AD82584F_Amp_Volume_Set_with_Index(data[3], TRUE, FALSE);
#else //AD82584F_ENABLE
													TAS5806MD_Amp_Volume_Set_with_Index(data[3], TRUE, FALSE);
#endif //TAS5806MD_ENABLE
													BAlready_Set_Vol = TRUE;
												}
#endif //#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
											}
											else
												BRet = FALSE;
										}
										break;

										case BLE_EQ_KEY: //##BLE## Sound Effect(EQ) - Normal : 0x00/POP&ROCK : 0x01/CLUB : 0x02/JAZZ : 0x03/VOCAL : 0x04
										{
#ifdef BT_DEBUG_MSG					
											_DBG("\n\rxxx 4.Sound Effect(EQ) : For Slave");
#endif
											//To Do !!! EQ control function
											if(data[4] <= 0x04)
											{
#ifdef FIVE_USER_EQ_ENABLE
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#else //AD82584F_ENABLE
												TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle
#endif //TAS5806MD_ENABLE
												TIMER20_user_eq_mute_flag_start();	

												uBuf = data[4];
#ifdef USEN_TI_AMP_EQ_ENABLE //2023-02-27_1
												TAS5806MD_Amp_EQ_DRC_Control((EQ_Mode_Setting)uBuf);
#else
												MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uBuf);
#endif
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
												FlashSaveData(FLASH_SAVE_DATA_EQ, uBuf);
#endif
#endif
#ifdef AUTO_VOLUME_LED_OFF
												TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
#ifdef AD82584F_ENABLE
												uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();

												AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#else //AD82584F_ENABLE
												uVolume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
											
												TAS5806MD_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#endif //TAS5806MD_ENABLE
#endif
											}
											else
												BRet = FALSE;
										}
										break;
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)

										case BLE_REBOOT_KEY: //##BLE## Reboot On - Reboot On : 0x01/Reboot Off : 0x00
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 5.Reboot : For Slave");
#endif
											if(data[5] == 0x01)
												SW_Reset();
											else if(data[5] == 0x00)
											{
												NOP();
#ifdef BT_DEBUG_MSG
												_DBG("\n\rReboot Off - To Do !!!");
#endif
											}
											else
												BRet = FALSE;
										}
										break;

										case BLE_FACTORY_RESET_KEY: //##BLE## Factory Reset Key - Factory Reset On : 0x01/Factory Reset Off : 0x00
#if defined(PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE) && !defined(USEN_BAP) //BAP-01 Slave should be worked for Factory Reset Key thru BLE communication from BAP-01 Master //2023-01-05_3
										break; //Just ignore Factory Reset over BLE Data. Don't set BRet = FALSE because Power Action is not worked
#else //PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE										
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 6.Factory : For Slave");
#endif
											if(data[6] == 0x01)
											{
#ifdef SWITCH_BUTTON_KEY_ENABLE
												bFactory_Reset_Mode = TRUE;
#endif
#ifdef AUTO_VOLUME_LED_OFF
												TIMER20_auto_volume_led_off_flag_Stop(); //Must display LED volume Under Mute On
												uVolume_Level = AD82584F_Amp_Get_Cur_Volume_Level();
												AD82584F_Amp_Volume_Set_with_Index(uVolume_Level, FALSE, FALSE);
#endif
												Factory_Reset_Value_Setting();
											}								
											else if(data[6] == 0x00)
											{
												NOP();
#ifdef BT_DEBUG_MSG
												_DBG("\n\rFactory Reset Off 3 - To Do !!!");
#endif
											}
											else
												BRet = FALSE;
										}
										break;
#endif //PROHIBIT_FACTORY_RESET_UNDER_SLAVE_MODE

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
										case BLE_MUTE_OFF_DELAY: //##BLE## 
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx 7.Mute Off Delay : For Slave");
#endif						
											if(data[7] == 0x01)
												TIMER20_mute_flag_Start();
											else if(data[7] == 0x02)
											{
												TIMER20_mute_flag_Stop();
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //AD82584F_ENABLE
												TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
											}
											else
											{
												TIMER20_mute_flag_Stop();
#ifdef BT_DEBUG_MSG
												_DBG("\n\rBLE_MUTE_OFF_DELAY - No Effect !!!");
#endif
											}

											data[7] = 0; //To clear Mute off delay
										}
										break;
#endif
										default:
										break;
									}
								}
							}

							if(BRet)
							{
								//bPolling_Get_Data |= BCRF_SEND_BLE_REMOTE_DATA_OK; //Send BLE Data Receive OK
								
								for(i = 0; i<8; i++)
								{
									uBLE_Remocon_Data[i] = data[i]; //Save BLE Data finally !!!
								}
#ifdef AUTO_VOLUME_LED_OFF
								if(bMute == FALSE)
									TIMER20_auto_volume_led_off_flag_Start();
								else
									bMute = TRUE;
#endif
								switch(Power_Key_Action) //Power Key Action should be executed here because when Power off, we need to save amp value before Power Off
								{
									case REMOTE_POWER_ON_ACTION:
#ifdef SWITCH_BUTTON_KEY_ENABLE
										if(BAlready_Set_Vol)
											Remocon_Power_Key_Action(TRUE, FALSE, FALSE);
										else
											Remocon_Power_Key_Action(TRUE, FALSE, TRUE);

										if(data[2] == 0x01) //Mute On. When Power off state under Slave mode, we need to set mute on here.
										{
											TIMER20_mute_flag_Stop();
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Mute(TRUE, TRUE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Mute(TRUE, TRUE);
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
											
#if defined(UART_10_ENABLE) && defined(MB3021_ENABLE) //When Power on/off under slave mute on, slave miss BLE commnucation from Master very first one time.
											/*UART USART10 Configure*/
											Serial_Init(SERIAL_PORT10, 115200);
#endif
#ifdef AUTO_VOLUME_LED_OFF
											bMute = TRUE;
#endif
										}

										if(BNeed_Mute_Off) //Mute off. When Power off state under Slave mode, we need to set mute off here.
										{
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
											AD82584F_Amp_Mute(FALSE, TRUE);
#else //AD82584F_ENABLE
											TAS5806MD_Amp_Mute(FALSE, TRUE);
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
											BNeed_Mute_Off = FALSE;
										}
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Fixed Slave SPK has pop-noise when Power Plug-in
										//TIMER20_mute_flag_Start(); 
#endif //SLAVE_ADD_MUTE_DELAY_ENABLE
#endif
									break;
									case REMOTE_POWER_OFF_ACTION:
#ifdef SWITCH_BUTTON_KEY_ENABLE
										Remocon_Power_Key_Action(FALSE, FALSE, TRUE);
#endif
									break;

									default:
									break;
								}
							}
							/*else
								bPolling_Get_Data |= BCRF_SEND_BLE_REMOTE_DATA_NG; //Send BLE Response OK*/
#ifdef BT_DEBUG_MSG
							_DBG("\n\rBLE Data :");

							for(i = 0; i<8; i++)
							{
								_DBH(uBLE_Remocon_Data[i]);
							}
#endif
						}
#ifndef MASTER_MODE_ONLY //2023-03-28_2 : Deleted receiving extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //##BLE Extra Data## To get 64 Step volume level under BAP //2023-01-05_6
						if(data[0] == 0xCC && data_length == 0x03) //BAP-01 Extra Data thru BLE (Total 3 Byte = Header(0xCC-1Byte) + Param1(Volume-1Byte) + Checksum(1Byte))
						{
							//Data[0] = Header, Data[1] = Volume Data, Data[2] = Checksum
							bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 2); //using checksum Data check whether it's valid or invalid

							if(bChecksum != data[2])
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rReceive data thru BLE communication is NG because check sum is NG");
								_DBH(bChecksum);
								_DBG("/");
								_DBH(data[2]);
#endif
								//bPolling_Get_Data |= BCRF_SEND_BLE_REMOTE_DATA_NG; //Send BLE Response NG
								BRet = FALSE;
								
								break;
							}
							
							if(data[1] < 0x01 && data[1] > 0x40) //Check whether BLE remote Data is valid or not
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rReceive data thru BLE communication is NG because some Data have invalid value");
#endif
								BRet = FALSE;
								break;
							}
							
							for(i = 1; i<(BLE_EXT_DATA_END+1); i++)
							{
#if 0 //Need to add this statement when we use additinal data excepting volume data //2023-01-05_6
#ifdef SLAVE_VOLUME_FORCED_SYNC
								if((i-1) == BLE_VOLUME_KEY) //Need to update volume info even though invalid value becasue Slave can get different volume level.
									uBLE_Remocon_Data_bk[i-1] = 0xff;
								else
#endif
								uBLE_Remocon_Data_bk[i-1] = uBLE_Extra_Data[i];
#endif
								//2023-01-09_1 : Received 64 step volume from BAP-01. Need to check it w/ this flag.
								if(!B_BLE_Extra_Data)
								{
									B_BLE_Extra_Data = TRUE;
#ifdef FLASH_SELF_WRITE_ERASE
									
									Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

									if(uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x02)
									{
										FlashSaveData(FLASH_SAVE_SLAVE_LAST_CONNECTION, 2); //Save Last connection status of slave to Flash and Peerdevice & Slave are BAP-01										
#ifdef BT_DEBUG_MSG
										_DBG("\n\rCurrent Peerdevice & Slave are BAP");
#endif
									}
#endif
								}

								B_Master_Is_BAP = TRUE; //2023-01-09_2 : When BAP-01 slave is received BLE Extra Data from BAP-01, we can recognize it is BAP-01 Master

								if(!Power_State()) //When Power is off, all keys execepting Power Key are ignored.
								{
#ifdef BT_DEBUG_MSG
									_DBG("\n\r6. Under Power Off, only POWER_ON_KEY is valid key !!!");
#endif
									BRet = FALSE;
									
									break;
								}

								//if(uBLE_Remocon_Data_bk1[i-1] != data[i]) //Need to add this statement when we use additinal data excepting volume data //2023-01-05_6
								{
									switch(i-1)
									{
										case BLE_EXT_VOLUME_DATA: //##BLE Extera Data## Parameter 1
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rxxx BAP Volume Setting : For Slave ");
#endif
											if(data[1] >= 0x01 && data[1] <= 0x40) //Check whether BLE remote Data is valid or not
											{
#ifdef AD82584F_ENABLE
												AD82584F_Amp_Volume_Set_with_Index(data[1], FALSE, FALSE);
#else //AD82584F_ENABLE
#ifdef TAS5806MD_ENABLE
												TAS5806MD_Amp_Volume_Set_with_Index(data[1], FALSE, FALSE);
#endif
#endif //TAS5806MD_ENABLE
												//BAlready_Set_Vol = TRUE;
											}
											else
												BRet = FALSE;
										}
										break;

										default:
										break;
									}
								}
							}

							if(BRet)
							{
								//bPolling_Get_Data |= BCRF_SEND_BLE_REMOTE_DATA_OK; //Send BLE Data Receive OK
								for(i = 0; i<(BLE_EXT_DATA_END+1); i++)
								{
									uBLE_Extra_Data[i] = data[i]; //Save BLE Data finally !!!
								}
#ifdef AUTO_VOLUME_LED_OFF
								if(bMute == FALSE)
									TIMER20_auto_volume_led_off_flag_Start();
								else
									bMute = TRUE;
#endif
							}
							/*else
								bPolling_Get_Data |= BCRF_SEND_BLE_REMOTE_DATA_NG; //Send BLE Response OK*/
#ifdef BT_DEBUG_MSG
							_DBG("\n\rBLE Data :");

							for(i = 0; i<8; i++)
							{
								_DBH(uBLE_Extra_Data[i]);
							}
#endif
						}
#endif
#endif //#ifndef MASTER_MODE_ONLY

					}
#endif //BLE_DATA_GET_FROM_MASTER_SPK
				}
				break;
					
				default:
					break;
			}
			break;
				
		default:
			break;
	}
}


//Bool do_recovery - to send same data again which we receive error message
Bool MB3021_BT_Module_CMD_Execute(uint8_t major_id, uint8_t minor_id, uint8_t *data, uint16_t data_length, Bool do_recovery)
{
	Bool ret = TRUE;
	int i;
#ifdef TWS_MODE_ENABLE
	int j;
#endif
	
	switch(major_id)
	{
		case MAJOR_ID_GENERAL_CONTROL: //0x00
		{
			switch(minor_id)
			{
				case MINOR_ID_SET_CONNECTABLE_MODE: //0x00 : 0x08
				bPolling_Get_Data_backup &= (~BCRF_CLEAR_CONNECTABLE_MODE); //Clear flag
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\rRes: MINOR_ID_SET_CONNECTABLE_MODE");
#endif
					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; //Set flag to send same data again
					else
					{
#ifdef TWS_MODE_ENABLE
#ifdef SW1_KEY_TWS_MODE
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //TWS Mode
#endif
						{
							if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode) //TWS Slave
							{
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_16 : Added condition to avoid sending connectable mode and discoverable mode again and again.
								if(TWS_Slave_Status == TWS_SLAVE_NONE_CONNECTION) //To avoid unlimited repetition.
								{
									TWS_Slave_Status = TWS_SLAVE_DISCOVERABLE_CONNECTABLE_MODE_1;
									bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; //For TWS Slave
								}
#else
								bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; //For TWS Slave
#endif
							}								
							else
							{//TWS Master
#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE) && defined(TWS_MASTER_SLAVE_GROUPING) //2023-04-26_3 : To make New TWS Connection, we need to send TWS_CMD when first TWS connection under TWS master mode.
								Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
								
								if(uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] == 0x00 || uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] == 0xff) //we don't to execute this when SET_DEVICE_ID is 0xffffffffffff(6Byte)
								{
#ifdef BT_DEBUG_MSG
									_DBG("\n\rMaster Execute new TWS link start - MB3021_BT_Module_TWS_Start_Master_Slave_Grouping() !!!");
#endif
									MB3021_BT_TWS_Master_Slave_Grouping_Start();
								}
								else
								{
#ifndef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_20 : To play Aux under TWS Master
									if(Aux_In_Exist() && BTWS_Master_Slave_Connect < TWS_Get_Slave_Connected && BTWS_LIAC < TWS_Status_Master_LIAC) //2023-03-15_2 : When master is TWS Aux mode and BT is connecting with peer device, if user disconnect BT connection on peer device, MCU sends connection and discovery again & again, forever. So, we need to add condition to send disovery. //BTWS_LIAC != TWS_Status_Master_Mode_Control) //2023-02-21_2
									{
										bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; //2022-11-17_2 : Under Aux mode, TWS Setting
									}
									else
#endif
									{
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_16 : Added condition to avoid sending connectable mode and discoverable mode again and again.

										//if(Peer_Device_Status == PEER_DEVICE_NONE || Peer_Device_Status == PEER_DEVICE_DISCONNECTED)
										if(Peer_Device_Status == PEER_DEVICE_NONE) //2023-05-30_1 : Changed condition because we don't send SET_DISCOVERABLE_MODE
										{
											Peer_Device_Status = PEER_DEVICE_DISCOVERABLE_CONNECTABLE_MODE_1;
											bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE;
										}
#else
										bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE;
#endif
									}
								}
#else //#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE)
								if(Aux_In_Exist() && BTWS_LIAC < TWS_Status_Master_LIAC
#ifndef NEW_TWS_MASTER_SLAVE_LINK
								&& BTWS_Master_Slave_Connect < TWS_Get_Slave_Information_Done
#endif
								) //2023-03-15_2 : When master is TWS Aux mode and BT is connecting with peer device, if user disconnect BT connection on peer device, MCU sends connection and discovery again & again, forever. So, we need to add condition to send disovery. //BTWS_LIAC != TWS_Status_Master_Mode_Control) //2023-02-21_2
								{
									bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; //2022-11-17_2 : Under Aux mode, TWS Setting
								}
#endif //#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE)
							}
						}
						else //Broadcast mode
#endif
						{
#ifndef MASTER_MODE_ONLY
							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
							{
#if defined(BT_DISCONNECT_CONNECTABLE_ENABLE) || defined(BT_CLEAR_CONNECTABLE_MODE)						
								if(BKeep_Connectable)
									BKeep_Connectable = FALSE;
								else
#endif
#ifdef NO_BROADCAST_MODE
								bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; //Normal Mode - No Broadcast Mode
#else //NO_BROADCAST_MODE
								bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)
#endif //NO_BROADCAST_MODE
							}
						}
					}
					
					break;

				case MINOR_ID_SET_DISCOVERABLE_MODE: //0x00 : 0x07
				bPolling_Get_Data_backup &= (~BCRF_SET_DISCOVERABLE_MODE); //Clear flag
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\rRes: MINOR_ID_SET_DISCOVERABLE_MODE");
#endif
#ifdef TWS_MODE_ENABLE
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_21 : Under TWS Mode, changed some condition for response of MINOR_ID_SET_DISCOVERABLE_MODE.
					if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
					{
						if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
						{
							if(BTWS_LIAC == TWS_Status_Master_GIAC
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2022-12-27
								&& !BKeep_Connectable
#endif
							) //For TWS Device
							{
								if(do_recovery)
									bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; //Set flag to send same data again
								else //For TWS Connection
								{
#ifdef TWS_MASTER_SLAVE_GROUPING //2022-12-15 //TWS : Disable TWS CMD
									//Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //We already read flash data for SET DEVICE ID thru CMD_SET_DEVICE_ID_32
					
									if(BTWS_Master_Slave_Grouping || (uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff))
#endif
									{
										if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Is_TWS_Master_Slave_Connect() != TWS_Get_Information_Ready) //2023-03-08_2 : Added condtion to avoid sending TWS CMD again even though TWS Mode under Master Mode
											bPolling_Get_Data |= BCRF_INFORM_HOST_MODE;
										else
											bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL;
										
										BTWS_LIAC = TWS_Status_Master_Mode_Control;
									}
#ifdef TWS_MASTER_SLAVE_GROUPING
									else
									{
										if(Read_TWS_Connection_From_Flash()) //2023-04-26_16 : TWS Master need to send 
										{										
											if(Peer_Device_Status == PEER_DEVICE_DISCOVERABLE_CONNECTABLE_MODE_1 || Peer_Device_Status == PEER_DEVICE_DISCONNECTED) //To avoid unlimited repetition.
											{
												Peer_Device_Status = PEER_DEVICE_DISCOVERABLE_CONNECTABLE_MODE_2;
												bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE;
											}
										}
									}
#endif
								}
							}
							else //For A2DP Device
							{
								if(do_recovery)
									bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //Set flag to send same data again
								else
								{
									if(Peer_Device_Status == PEER_DEVICE_DISCOVERABLE_CONNECTABLE_MODE_1 || Peer_Device_Status == PEER_DEVICE_DISCONNECTED)
									{
										Peer_Device_Status = PEER_DEVICE_DISCOVERABLE_CONNECTABLE_MODE_2;
										bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE;
									}
								}
							}
						}

#if defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE) //2023-04-26_7 : TWS Slave grouping Start
						else //(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
						{
							if(!Read_TWS_Connection_From_Flash() && TWS_Slave_Status != TWS_SLAVE_GROUGPING_MODE)
							{
#ifdef BT_DEBUG_MSG
								_DBG("\n\rSlave Execute new TWS link start - MB3021_BT_Module_TWS_Start_Master_Slave_Grouping() !!!");
#endif
								TWS_Slave_Status = TWS_SLAVE_GROUGPING_MODE;
#if defined(MASTER_SLAVE_GROUPING) && defined(TWS_MASTER_SLAVE_GROUPING)
								MB3021_BT_TWS_Master_Slave_Grouping_Start();
#endif
							}
							else
							{
								if(TWS_Slave_Status != TWS_SLAVE_DISCOVERABLE_CONNECTABLE_MODE_2) //To avoid unlimited repetition.
								{
									if(TWS_Slave_Status == TWS_SLAVE_DISCOVERABLE_CONNECTABLE_MODE_1)
									{
										TWS_Slave_Status = TWS_SLAVE_DISCOVERABLE_CONNECTABLE_MODE_2;
									}
									else
									{
										TWS_Slave_Status = TWS_SLAVE_DISCOVERABLE_CONNECTABLE_MODE_1;
									}

									bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL;
								}
							}
						}

						break;
#endif //#if defined(SW1_KEY_TWS_MODE) && defined(FLASH_SELF_WRITE_ERASE)
					}
#else //NEW_TWS_MASTER_SLAVE_LINK
					if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_LIAC == TWS_Status_Master_LIAC
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2022-12-27
					&& !BKeep_Connectable
#endif
					)
					{
						if(do_recovery)
							bPolling_Get_Data |= BCRF_TWS_SET_DISCOVERABLE_MODE; //Set flag to send same data again
						else
						{
#ifdef TWS_MASTER_SLAVE_GROUPING //2022-12-15 //TWS : Disable TWS CMD
							//Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //We already read flash data for SET DEVICE ID thru CMD_SET_DEVICE_ID_32
			
							if(BTWS_Master_Slave_Grouping || (uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff))
#endif
							{
								if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Is_TWS_Master_Slave_Connect() != TWS_Get_Information_Ready
									) //2023-03-08_2 : Added condtion to avoid sending TWS CMD again even though TWS Mode under Master Mode
									bPolling_Get_Data |= BCRF_INFORM_HOST_MODE;
								else
									bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL;
								
								BTWS_LIAC = TWS_Status_Master_Mode_Control;
							}
						}
					}
					else
#endif //NEW_TWS_MASTER_SLAVE_LINK
#endif //TWS_MODE_ENABLE
					{
						if(do_recovery)
							bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //Set flag to send same data again
						else
						{
							bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; //For init sequence (Init Sequnece : Broadcaster -2)
						}
					}
					break;
					
				case MINOR_ID_INIT_SINK_MODE: //0x00 : 0x05
				bPolling_Get_Data_backup &= (~BCRF_INIT_SINK_MODE); //Clear flag
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\rRes: MINOR_ID_INIT_SINK_MODE");
#endif

					if(do_recovery)
						bPolling_Get_Data |= BCRF_INIT_SINK_MODE; //Set flag to send same data again
					break;

				case MINOR_ID_SET_DEVICE_ID: //0x00 : 0x03
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\rRes: MINOR_ID_SET_DEVICE_ID");
#endif
					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_DEVICE_ID_CONTROL; //Set flag to send same data again
					else //In normal case, we need to send SET_MODEL_NAME_CONTROL for init sequence (Init Sequnece : 3)
#if 0//def TWS_MODE_ENABLE //2023-02-21_4 : Need to Set MODEL NAME as USEN MUSIC LINK under TWS Slave also. to avoid "2023-02-20_1" issue. //2022-11-03
					if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
						bPolling_Get_Data |= BCRF_MODULE_POWER_CONTROL; //For init sequence (Init Sequnece : 4) //Last - 4
					else
#endif
						bPolling_Get_Data |= BCRF_SET_MODEL_NAME_CONTROL; //For init sequence (Init Sequnece : 3) //Last - 3

					break;

				case MINOR_ID_SET_MODEL_NAME: //0x00 : 0x02
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\rRes: MINOR_ID_SET_MODEL_NAME");
#endif
					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_MODEL_NAME_CONTROL; //Set flag to send same data again
					else //In normal case, we need to send BCRF_MODULE_POWER_CONTROL for init sequence (Init Sequnece : 4)					
					{
						delay_ms(100); //For flash saving time of BT module
						bPolling_Get_Data |= BCRF_MODULE_POWER_CONTROL; //For init sequence (Init Sequnece : 4) //Last - 4
					}
					break;
					
				case MINOR_ID_GET_PAIRED_DEVICE_LIST: //0x00 : 0x0A
				bPolling_Get_Data_backup &= (~BCRF_GET_PAIRED_DEVICE_LIST); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_GET_PAIRED_DEVICE_LIST");
#endif
					if(do_recovery)
					{
						bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //Set flag to send same data again
					}
					else
					{
#ifndef MASTER_MODE_ONLY
						if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
						{							
							uPaired_Device_Count = data[1]; //data[2] : index
#ifdef BT_DEBUG_MSG
							_DBG("\n\rDevice Count = ");_DBD(uPaired_Device_Count);
#endif
							if(uPaired_Device_Count) //Last Connection
							{
#ifdef TWS_MODE_ENABLE //2022-11-03
								//data_length
								//data[3 ~ 8]: Paired Device Address, data[9]: 0x0-remote device/0x1-TWS device
#ifdef BT_DEBUG_MSG
								_DBG("\n\rdata_length = ");_DBD(data_length);
#endif

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_12 : Under TWS Mode, The agorithm of last connection is changed.
								ret = FALSE;

								for(j=0;j<uPaired_Device_Count;j++) //Check Total Device
								{
									if(data[9+j*8] & 0x4)
									{
										for(i=0;i<6;i++) //Paired TWS Device Address
										{
											uLast_Connected_TWS_Device_Address[i] = data[i+3+(j*8)];
#ifdef BT_DEBUG_MSG	
											_DBH(uLast_Connected_TWS_Device_Address[i]);
#endif
										}
										
									}
									else
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\ruLast_Connected_Device_Address = ");
#endif
										for(i=0;i<6;i++) //Paired Device Address
										{
											uLast_Connected_Device_Address[i] = data[i+3+(j*8)];
											ret = TRUE;
#ifdef BT_DEBUG_MSG	
											_DBH(uLast_Connected_Device_Address[i]);
#endif
										}

										break;
									}
								}

								if(ret == FALSE) //TWS Device Address is invalid
								{
#ifdef BT_DEBUG_MSG
									_DBG("\n\rNo Last Connection Information !!!");
#endif
								}
#else //NEW_TWS_MASTER_SLAVE_LINK
								for(j=0;j<uPaired_Device_Count;j++) //Check Total Device
								{
									if(data[9+j*8] & 0x4)
									{
										for(i=0;i<6;i++) //Paired TWS Device Address
										{
											uLast_Connected_TWS_Device_Address[i] = data[i+3+(j*8)];
#ifdef BT_DEBUG_MSG	
											_DBH(uLast_Connected_TWS_Device_Address[i]);
#endif
										}
#ifdef BT_DEBUG_MSG
										_DBG("\n\rNo Last Connection Information !!!");
#endif
										break;
									}
								}
								
								for(j=0;j<uPaired_Device_Count;j++) //Check Total Device
								{
									if(!(data[9+j*8] & 0x4))
									{
#ifdef BT_DEBUG_MSG	
										_DBG("\n\r");
#endif

										for(i=0;i<6;i++) //Paired Device Address
										{
											uLast_Connected_Device_Address[i] = data[i+3+(j*8)];
#ifdef BT_DEBUG_MSG	
											_DBH(uLast_Connected_Device_Address[i]);
#endif
										}
										
										break;
									}
								}
#endif //NEW_TWS_MASTER_SLAVE_LINK
#else //TWS_MODE_ENABLE
								//for(i=0;i<uPaired_Device_Count*8;i++) //Total Device 
								for(i=0;i<6;i++)
								{
									uLast_Connected_Device_Address[i] = data[i+3];
#if 0//def BT_DEBUG_MSG	
									_DBH(uLast_Connected_Device_Address[i]);
#endif
								}
#endif //TWS_MODE_ENABLE
#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(FLASH_SELF_WRITE_ERASE) //2023-04-26_6 : To make New TWS Connection, we need to set BCRF_SET_DISCOVERABLE_MODE instead of BCRF_SET_LAST_CONNECTION when the last connection information is only TWS slave address.
								if(ret == FALSE)
									bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE;
								else
#endif
								bPolling_Get_Data |= BCRF_SET_LAST_CONNECTION; //For Last Connection //For init sequence (Init Sequnece : Broadcaster -0-1) //Last -6
#ifdef AVRCP_ENABLE
								BBT_Is_Last_Connection_for_AVRCP = TRUE;
#endif
							}
							else //No Paired Device List : Need to go Discoverable_Mode
							{
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-27_3 : After BT Long key on TWS Master, TWS Master can't connect with other device excepting previous peerdevice.
								if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
									bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE;
								else
#endif
								bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //For init sequence (Init Sequnece : Broadcaster -1)
#ifdef AVRCP_ENABLE
								BBT_Is_Last_Connection_for_AVRCP = FALSE;
#endif
							}
						}
					}
					break;

				case MINOR_ID_DELETE_PAIRED_DEVICE_LIST: //0x00 : 0x0B
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_DELETE_PAIRED_DEVICE_LIST");
#endif
					if(do_recovery)
					{
						bPolling_Get_Data |= MINOR_ID_DELETE_PAIRED_DEVICE_LIST; //Set flag to send same data again
					}
					else
					{
#ifdef LR_360_FACTORY_ENABLE //2023-04-06_2 : When SPK recevies response, we need to clear recovery action.
						TIMER20_factory_reset_cmd_recovery_flag_stop(); //To Clear Factory Reset recovery for factory_reset_delete_paired_list	becasuse SPK get response here.
#endif

#ifdef BT_ALWAYS_GENERAL_MODE //2023-01-31_3
						//if(!bFactory_Reset_Mode && mode == Switch_Master_Mode)
						if(!B_Delete_PDL_by_Factory_Reset
#ifndef MASTER_MODE_ONLY
							&& Get_Cur_Master_Slave_Mode() == Switch_Master_Mode //2023-03-29_2 : BT Long key is working as factory reset. so, we need to fix this one.
#endif
							) //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
						{
							FlashSaveData(FLASH_SAVE_DATA_PDL_NUM, 0); //Save PDL Device Number(0) from Flash							
							delay_ms(300); //2023-03-03_1 : For flash saving time of BT module during BT Long key action.
#ifdef MB3021_ENABLE
							MB3021_BT_Module_Init(FALSE);
#endif
							Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);

							break;
						}
#if defined(BT_ALWAYS_GENERAL_MODE) && defined(TWS_MODE_ENABLE)
						if(bFactory_Reset_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //2023-02-15_1 : Under TWS Mode, Factory Reset Mode should be cleared in here
							bFactory_Reset_Mode = FALSE;
#endif
#endif //BT_ALWAYS_GENERAL_MODE

#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2023-01-31_1 : To keep Alway General mode after factory reset key
						BBT_Pairing_Key_In = TRUE;
#else
#ifdef SWITCH_BUTTON_KEY_ENABLE
						BBT_Pairing_Key_In = FALSE;
#endif
#endif
						//TIMER20_factory_reset_cmd_recovery_flag_stop(); //Move to MB3021_BT_Module_Init() //To Clear Factory Reset recovery becasuse SPK get response here.

#ifdef FLASH_SELF_WRITE_ERASE
#if defined(MASTER_SLAVE_GROUPING)
						Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
#ifndef MASTER_MODE_ONLY
						//Factory Reset / Flash Data Clear - FLASH_SAVE_SLAVE_LAST_CONNECTION
						if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x00)
						{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
							_DBG("\n\rSave FLASH_SAVE_SLAVE_LAST_CONNECTION : 0");
#endif
							FlashSaveData(FLASH_SAVE_SLAVE_LAST_CONNECTION, 0); //Save Last connection status of slave from Flash
						}
#endif
						//Factory Reset / Flash Data Clear - FLASH_SAVE_DATA_PDL_NUM
						if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x00)
							FlashSaveData(FLASH_SAVE_DATA_PDL_NUM, 0); //Save PDL Device Number(0) from Flash
#elif defined(GIA_MODE_LED_DISPLAY_ENABLE) //Save whether Paired Device exist in Flash
						Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END); //Read PDL Device Number from Flash

						if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x00)
							FlashSaveData(FLASH_SAVE_DATA_PDL_NUM, 0); //Save PDL Device Number(0) from Flash
#endif
#endif
						//Do not Delete !!! BT Module needs to get enough time to save current setting.
						//delay_ms(150); //Need to delete !!! BT Module miss MINOR_ID_FIRMWARE_VERSION_IND under Master mode
#ifdef FACTORY_RESET_LED_DISPLAY
						LED_Display_All_On(); //Display All LED during 1 sec
						TIMER20_factory_reset_led_display_flag_Start(); //Start 1 sec timer
#else
						MB3021_BT_Module_Init(TRUE); //Factory Resest & BBT_Pairing_Key_In Init //To Do !!!
#endif
					}
						
					break;
					
				case MINOR_ID_MODULE_POWER_CONTROL: //0x00 : 0x11
				bPolling_Get_Data_backup &= (~BCRF_MODULE_POWER_CONTROL); //Clear flag
#if 0//def BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_MODULE_POWER_CONTROL");
#endif

					if(do_recovery)
						bPolling_Get_Data |= BCRF_MODULE_POWER_CONTROL; //Set flag to send same data again
#if 0//Move to MINOR_ID_MODULE_STATE_CHANGED_IND to stable action //2022-11-04
					else
					{
						if(mode == Switch_Master_Mode)
							bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //For init sequence (Init Sequnece : Broadcaster -0) //Last -5
						else
							bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Receiver -1)

						BBT_Init_OK = TRUE;
					}
#endif
					break;

					case MINOR_ID_INFORM_HOST_MODE: //0x00 : 0x17
					bPolling_Get_Data_backup &= (~BCRF_INFORM_HOST_MODE); //Clear flag
#if 0//def BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_INFORM_HOST_MODE");
#endif
					if(do_recovery)
						bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; //Set flag to send same data again
#ifndef AVRCP_CONNECTION_CONTROL_ENABLE
					else //Set EQ Setting after Power On
					{
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ) //Set EQ Setting after Power On
						Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);					

						if(!uEQ_Mode && uFlash_Read_Buf[FLASH_SAVE_DATA_EQ] != 0 && uFlash_Read_Buf[FLASH_SAVE_DATA_EQ] != 0xff)//Set cur EQ_MODE after Power On
						{
#ifdef _DBG_FLASH_WRITE_ERASE
							_DBG("\n\rEQ Mode : EQ Mode setting with flash data ===");
#endif
							uEQ_Mode = uFlash_Read_Buf[FLASH_SAVE_DATA_EQ];
#ifdef USEN_TI_AMP_EQ_ENABLE //2023-02-27_1
							TAS5806MD_Amp_EQ_DRC_Control(uEQ_Mode);
#else
							MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uEQ_Mode);
#endif
							FlashSaveData(FLASH_SAVE_DATA_EQ, uEQ_Mode);
						}
#endif
						TIMER20_Forced_Input_Audio_Path_Setting_flag_stop();
					}					
#else //AVRCP_CONNECTION_CONTROL_ENABLE
					else
					{
						bPolling_Get_Data |= BCRF_SET_AVRCP_CONNECTION_CONTROL; //Set AVRCP Connection Control Enable
						TIMER20_Forced_Input_Audio_Path_Setting_flag_stop();
					}
#endif //AVRCP_CONNECTION_CONTROL_ENABLE
					break;
					
#ifdef DEVICE_NAME_CHECK_PAIRING
					case MINOR_ID_SET_IOCAPABILITY_MODE: //0x00 : 0x1F
					bPolling_Get_Data_backup &= (~BCRF_SET_IO_CAPABILITY_MODE); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_SET_IOCAPABILITY_MODE");
#endif
					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_IO_CAPABILITY_MODE; //Set flag to send same data again

					break;
#endif //DEVICE_NAME_CHECK_PAIRING

				default:
					ret = FALSE;
					break;
			}
		}
		break;

		case MAJOR_ID_A2DP_CONTROL: //0x10
		{
			switch(minor_id)
			{
				case MINOR_ID_A2DP_CONNECTION_CONTROL: //0x10 : 0x00
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_A2DP_CONNECTION_CONTROL");
#endif
					if(do_recovery)
					{
						bPolling_Get_Data |= BCRF_SET_LAST_CONNECTION; //Set flag to send same data again
					}
					else
					{	
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
						if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
						{
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_12 : Under TWS Mode, The agorithm of last connection is changed.
							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode) //Under TWS Mode, the Master doesn't send INFORM_HOST_MODE when Last conneection is OK. So, the Master has BT output even though it has aux input when Power plug-out/In.
							{
								BTWS_LIAC = TWS_Status_Master_Mode_Control;
								bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL;
							}
#else //NEW_TWS_MASTER_SLAVE_LINK
							BTWS_LIAC = TWS_Status_Master_GIAC;

							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode) //2023-03-15_1 : Under TWS Mode, the Master doesn't send INFORM_HOST_MODE when Last conneection is OK. So, the Master has BT output even though it has aux input when Power plug-out/In.
								bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; 
#endif //NEW_TWS_MASTER_SLAVE_LINK
						}
						else //2022-11-09_1
						{
							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#ifdef NO_BROADCAST_MODE
								bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; 
#else
								bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)// Last connection is success(Last -7) !!! //To Do !!!
#endif
						}
#else //#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
#ifdef AVRCP_ENABLE
						if(BBT_Is_Last_Connection_for_AVRCP == FALSE)
#endif
						{
#ifndef MASTER_MODE_ONLY
							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
#ifdef NO_BROADCAST_MODE	
								bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; 
#else
								bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)// Last connection is success(Last -7) !!! //To Do !!!
#endif
						}
#endif //#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
					}
					break;
				
				case MINOR_ID_A2DP_ROUTING_CONTROL: //0x10 : 0x02
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_A2DP_ROUTING_CONTROL");
#endif
#if 0 //To Do !!!
					if(do_recovery)
					{
						bPolling_Get_Data |= BCRF_SET_LAST_CONNECTION; //Set flag to send same data again
					}
					else
					{
						if(mode == Switch_Master_Mode)
							bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)// Last connection is success(Last -7) !!! //To Do !!!
					}
#endif
					break;
				default:
					ret = FALSE;
					break;
			}
		}
		break;

		case MAJOR_ID_AVRCP_CONTROL: //0x11
		{
			switch(minor_id)
			{
#ifdef AVRCP_CONNECTION_CONTROL_ENABLE
				case MINOR_ID_AVRCP_CONNECTION_CONTROL: //0x11 : 0x00
					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_AVRCP_CONNECTION_CONTROL;
					break;
#endif
				default:
					ret = FALSE;
					break;
			}
		}
		break;

#ifdef TWS_MODE_ENABLE
		case MAJOR_ID_TWS_CONTROL: //0x15
		{
			switch(minor_id)
			{
				case MINOR_ID_TWS_MODE_CONTROL: //0x15 : 0x00
					if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
					{
						if(do_recovery)
							bPolling_Get_Data |= BCRF_TWS_MODE_CONTROL; //This is just called one time even though this is called in many place
						else
						{
							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
								bPolling_Get_Data |= BCRF_INFORM_HOST_MODE;
						}
					}
					break;

				default:
					ret = FALSE;
					break;
			}
		}
		break;
#endif

		case MAJOR_ID_BA_CONTROL: //0x16
		{
			switch(minor_id)
			{
				case MINOR_ID_BA_MODE_CONTROL: //0x16 : 0x00
				bPolling_Get_Data_backup &= (~BCRF_BA_MODE_CONTROL); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_BA_MODE_CONTROL");
#endif				
#ifdef MASTER_SLAVE_GROUPING
					if(uNext_Grouping_State > GROUPING_NONE_MODE) //Groping mode don't need next step here !!!
					{
						TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop();
						
						if(uPrev_Grouping_State == GROUPING_MASTER_NORMAL_MODE)
							uNext_Grouping_State = GROUPING_MASTER_BROADCASTER_MODE;
						else if(uPrev_Grouping_State == GROUPING_MASTER_BROADCASTER_MODE)
							uNext_Grouping_State = GROUPING_MASTER_SET_MANUFACTURE_DATA;
#ifndef MASTER_MODE_ONLY
						else if(uPrev_Grouping_State == GROUPING_SLAVE_NORMAL_MODE)
							uNext_Grouping_State = GROUPING_SLAVE_RECEIVER_MODE;
						else if(uPrev_Grouping_State == GROUPING_SLAVE_RECEIVER_MODE)
							uNext_Grouping_State = GROUPING_SLAVE_SET_MANUFACTURE_DATA;
#endif
						else
							__NOP();
						
						break;
					}
#endif
					if(do_recovery)
						bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Receiver -1)
					else
					{
#ifndef MASTER_MODE_ONLY
						if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode)
						{							
							bPolling_Get_Data |= BCRF_SET_BLE_MANUFACTURE_DATA; //For init sequence (Init Sequnece : Receiver -2)
						}
#endif
					}

					break;
				
				default:
					ret = FALSE;
					break;
			}
		}

		break;
		
		case MAJOR_ID_BLE_CONTROL: //0x17
		{
			switch(minor_id)
			{
				case MINOR_ID_ADVERTISING_CONTROL: //0x17 : 0x02
				bPolling_Get_Data_backup &= (~BCRF_ADVERTISING_CONTROL); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_ADVERTISING_CONTROL");
#endif
#ifdef MASTER_SLAVE_GROUPING
					if(uNext_Grouping_State > GROUPING_NONE_MODE) //Groping mode don't need next step here !!!
					{
						if(uPrev_Grouping_State == GROUPING_MASTER_ADVERTISING_ON)
						{
							TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop();
							
							if(Get_master_slave_grouping_flag() == TRUE)
								uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
							else
							{
								uNext_Grouping_State = GROUPING_NONE_MODE; //OFF
								uPrev_Grouping_State = GROUPING_NONE_MODE;
							}
						}
						
						break;
					}
#endif

					if(do_recovery)
						bPolling_Get_Data |= BCRF_ADVERTISING_CONTROL; //For init sequence (Init Sequnece : Broadcaster -5)
					else
					{
#ifndef MASTER_MODE_ONLY
						if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
#endif
						{
							bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; //For init sequence (Init Sequnece : Broadcaster -6)
						}
					}
					break;
					
				case MINOR_ID_SET_BLE_MANUFACTURE_DATA: //0x17 : 0x03
				bPolling_Get_Data_backup &= (~BCRF_SET_BLE_MANUFACTURE_DATA); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_SET_BLE_MANUFACTURE_DATA");
#endif
#ifdef MASTER_SLAVE_GROUPING
					if(uNext_Grouping_State > GROUPING_NONE_MODE) //Groping mode don't need next step here !!!
					{
						TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop();
						
						if(uPrev_Grouping_State == GROUPING_MASTER_SET_MANUFACTURE_DATA)
						{
							uNext_Grouping_State = GROUPING_MASTER_ADVERTISING_ON;
						}
#ifndef MASTER_MODE_ONLY
						else if(uPrev_Grouping_State == GROUPING_SLAVE_SET_MANUFACTURE_DATA)
						{
							if(Get_master_slave_grouping_flag() == TRUE)
							{
								uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
							}
							else
							{
								uNext_Grouping_State = GROUPING_NONE_MODE; //OFF
								uPrev_Grouping_State = GROUPING_NONE_MODE;

#if defined(MASTER_SLAVE_GROUPING_RECOVERY) && !defined(MASTER_MODE_ONLY)
								if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode) //To avoid Master/Slave connection NG, Execute BT init(Reset)
									MB3021_BT_Module_Init(FALSE);
#endif
							}
						}
#endif
						else
						{
							__NOP();
						}
						
						break;
					}
#endif

					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_BLE_MANUFACTURE_DATA; //For init sequence (Init Sequnece : Broadcaster -4)
					else
					{
						if(BMaster_Send_BLE_Remote_Data) //Master Mode Only
						{
#ifdef AVRCP_ENABLE
							if(BBT_Need_Sequence && Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
								bPolling_Get_Data |= BCRF_ADVERTISING_CONTROL; //For init sequence (Init Sequnece : Broadcaster -5)
							
							BBT_Need_Sequence = FALSE;
#endif
							//BMaster_Send_BLE_Remote_Data = FALSE; //2022-11-11 : Move to BA_SWITCH_MODE
						}
						else
						{
#ifdef MASTER_MODE_ONLY
							bPolling_Get_Data |= BCRF_ADVERTISING_CONTROL; //For init sequence (Init Sequnece : Broadcaster -5)
#else
							if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
								bPolling_Get_Data |= BCRF_ADVERTISING_CONTROL; //For init sequence (Init Sequnece : Broadcaster -5)
							else
								bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; //For init sequence (Init Sequnece : Receiver -3)
#endif
						}
					}

					break;
				
				default:
					ret = FALSE;
					break;
			}
		}
		break;
		
		case MAJOR_ID_QUALIFICATION_COMMANDS: //0x01
		case MAJOR_ID_HFP_CONTROL: //0x12
		case MAJOR_ID_SPP_CONTROL: //0x13
		case MAJOR_ID_UPGRADE_CONTROL: //0x1B
		break;

		default:
			ret = FALSE;
			break;
	}

	return ret;
}


void MB3021_BT_Module_Get_Auto_Response_Packet(uint8_t uBuf_Count, uint8_t* uRecData) //MB3021
{
	static uint16_t uCount = 0;
	static uint16_t uData_length_count = 0, uData_length = 0;
	static uint8_t uMajor_ID = 0, uMinor_ID = 0;
	static uint8_t uNext_state, uPacket_Type = 0;
	static uint8_t uDP_Source_Type = 0, uDP_Data_Ch = 0;
	uint8_t uChecksum = 0;
	
	switch(uNext_state)
	{
		case BT_PACKET_SYNC:
			if(*uRecData == SYNC_BYTE)
			{
				uCount = 0;
				uMajor_ID = 0;
				uMinor_ID = 0;
				uData_length_count = 0;
				uData_length = 0;
				uPacket_Type = 0;
				uDP_Source_Type = 0;
				uDP_Data_Ch = 0;
				uNext_state = BT_PACKET_TYPE;
				uAuto_receive_buf32[uCount] = *uRecData;
				uCount++;
			}
		break;

		case BT_PACKET_TYPE:
			if(*uRecData == PACKET_IND || *uRecData == PACKET_RESP)
			{
				uNext_state = BT_PACKET_MAJOR_ID;
				uAuto_receive_buf32[uCount] = *uRecData;
				uPacket_Type = *uRecData;
				uCount++;
			}
			else if(*uRecData == PACKET_DATA) //SPP
			{
				uNext_state = BT_PACKET_SS_SOURCE_TYPE; //SPP-1
				uAuto_receive_buf32[uCount] = *uRecData;
				uPacket_Type = *uRecData;
				uCount++;
			}
			else
				uNext_state = BT_PACKET_SYNC;
		break;

		case BT_PACKET_MAJOR_ID:
			if(uPacket_Type == PACKET_IND)
			{
				if(MB3021_BT_Module_Check_Valid_Major_Indication(*uRecData))
				{
					uNext_state = BT_PACKET_MINOR_ID;
					uAuto_receive_buf32[uCount] = *uRecData;
					uMajor_ID = *uRecData;
					uCount++;
				}
				else
					uNext_state = BT_PACKET_SYNC;
			}
			else if(uPacket_Type == PACKET_RESP)
			{
				if(MB3021_BT_Module_Check_Valid_Major_Response(*uRecData))
				{
					uNext_state = BT_PACKET_MINOR_ID;
					uAuto_receive_buf32[uCount] = *uRecData;
					uMajor_ID = *uRecData;
					uCount++;
				}
				else
					uNext_state = BT_PACKET_SYNC;
			}
			else
				uNext_state = BT_PACKET_SYNC;
		break;

		case BT_PACKET_MINOR_ID:
			if(uPacket_Type == PACKET_IND)
			{
				if(MB3021_BT_Module_Check_Valid_Minor_Indication(uMajor_ID, *uRecData))
				{
					uNext_state = BT_PACKET_LENGTH;
					uAuto_receive_buf32[uCount] = *uRecData;
					uMinor_ID = *uRecData;
					uCount++;
				}
				else
					uNext_state = BT_PACKET_SYNC;
			}
			else if(uPacket_Type == PACKET_RESP)
			{
				if(MB3021_BT_Module_Check_Valid_Minor_Response(uMajor_ID, *uRecData))
				{
					uNext_state = BT_PACKET_LENGTH;
					uAuto_receive_buf32[uCount] = *uRecData;
					uMinor_ID = *uRecData;
					uCount++;
				}
				else
					uNext_state = BT_PACKET_SYNC;
			}
			else
				uNext_state = BT_PACKET_SYNC;
		break;
		
		case BT_PACKET_LENGTH://_DBG("\n\r5");
			uData_length_count = *uRecData;
			uData_length = *uRecData;
			uNext_state = BT_PACKET_DATA;
			uAuto_receive_buf32[uCount] = *uRecData;
			uCount++;
		break;

		case BT_PACKET_DATA: //_DBG("\n\r6");
			//DATA[0] is Error Code when type is RESPONSE			
			uData_length_count--;
			
			if(uData_length_count)
				uNext_state = BT_PACKET_DATA;
			else
				uNext_state = BT_PACKET_CHECKSUM;

			uAuto_receive_buf32[uCount] = *uRecData;
			uCount++;
		break;

		case BT_PACKET_CHECKSUM:
			uAuto_receive_buf32[uCount] = *uRecData;
			uChecksum = MB3021_BT_Module_Calculate_Checksum(uAuto_receive_buf32, uCount); //Size = uCount(Except CheckSum byte)

			if(uChecksum == *uRecData)
			{
				if(uPacket_Type == PACKET_IND)
					MB3021_BT_Module_Receive_Data_IND(uMajor_ID, uMinor_ID, uAuto_receive_buf32+BT_PACKET_DATA, uData_length);
				else
					MB3021_BT_Module_Receive_Data_RESP(uMajor_ID, uMinor_ID, uAuto_receive_buf32+BT_PACKET_DATA, uData_length);
			}

			uNext_state = BT_PACKET_SYNC;
		break;

		case BT_PACKET_SS_SOURCE_TYPE:
			if(*uRecData == DATA_SOURCE_TYPE_SPP || *uRecData == DATA_SOURCE_TYPE_SPP) //SPP-1 //Only SPP & BLE is valid.
			{
				uDP_Source_Type = *uRecData;
				
				uNext_state = BT_PACKET_SS_DATA_CHANNEL;
				uAuto_receive_buf32[uCount] = *uRecData;
				uCount++;
			}
			else
				uNext_state = BT_PACKET_SYNC;
		break;
			
		case BT_PACKET_SS_DATA_CHANNEL:
			if(*uRecData <= 0x4) //SPP-1 
			{
				uDP_Data_Ch = *uRecData;
				
				uNext_state = BT_PACKET_SS_LENGTH_HIGH;
				uAuto_receive_buf32[uCount] = *uRecData;
				uCount++;
			}
			else
				uNext_state = BT_PACKET_SYNC;
		break;
			
		case BT_PACKET_SS_LENGTH_HIGH:
			//SPP : Data Length, High Byte
			{
				uNext_state = BT_PACKET_SS_LENGTH_LOW;
				uData_length_count = (*uRecData) << 8;
				uAuto_receive_buf32[uCount] = *uRecData;
				uCount++;
			}
		break;
			
		case BT_PACKET_SS_LENGTH_LOW:
			//SPP : Data Length, Low Byte
			{
				uNext_state = BT_PACKET_SS_DATA;
				uData_length_count |= (*uRecData);
				uData_length = uData_length_count;
				uAuto_receive_buf32[uCount] = *uRecData;
				uCount++;
			}
		break;
		
		case BT_PACKET_SS_DATA:
		//Because of uAuto_receive_buf32 Array, the Data size have upto 30bytes
			uData_length_count--;
			
			if(uData_length_count)
				uNext_state = BT_PACKET_SS_DATA;
			else
				uNext_state = BT_PACKET_SS_CHECKSUM;

			uAuto_receive_buf32[uCount] = *uRecData;
			uCount++;
		break;
		
		case BT_PACKET_SS_CHECKSUM:
			
			uAuto_receive_buf32[uCount] = *uRecData;
			uChecksum = MB3021_BT_Module_Calculate_Checksum(uAuto_receive_buf32, uCount); //Size = uCount(Except CheckSum byte)
			
			if(uChecksum == *uRecData)
			{
#if 0//def BT_DEBUG_MSG
				int i;

				_DBG("\n\rSSP :");

				for(i=0;i<32;i++)
					_DBH(uAuto_receive_buf32[i]);
#endif
				MB3021_BT_Module_Remote_Data_Receive(uDP_Source_Type, uDP_Data_Ch, uAuto_receive_buf32+BT_PACKET_DATA+1, uData_length); //SPP COM : Receive SPP Data/BLE Data
			}

			uNext_state = BT_PACKET_SYNC;
		break;
		
		default:
			uNext_state = BT_PACKET_SYNC;
			uCount = 0;
			break;
	}

	if(uNext_state == BT_PACKET_SYNC && *uRecData == SYNC_BYTE) //To recover, SPP communication Error
	{
		uCount = 0;
		uMajor_ID = 0;
		uMinor_ID = 0;
		uData_length_count = 0;
		uData_length = 0;
		uPacket_Type = 0;
		uDP_Source_Type = 0;
		uDP_Data_Ch = 0;
		uNext_state = BT_PACKET_TYPE;
		uAuto_receive_buf32[uCount] = *uRecData;
		uCount++;
	}
}


//UART Task
void Do_taskUART(void) //Just check UART receive data from Buffer
{
	static uint8_t uBuf[32] = {0,};
	static uint32_t uCount = 0, uTimerVal = 0;

	if(bPolling_Get_Data_backup) //Sometimes, Slave can't link with Master because Slave's MCU send BA_SWITCH_MODE (Receiver) to BT Moduel but BT Module can't recognize it. (USEN#43)
	{
		if(uTimerVal == 0x2FFFD) //3s
		{
			uTimerVal = 0;
#ifdef BT_DEBUG_MSG
			_DBG("\n\r Resend Sequence CMD = ");
			_DBH32(bPolling_Get_Data_backup);
#endif
			bPolling_Get_Data |= bPolling_Get_Data_backup;
		}
		else
		{
			uTimerVal++;
		}
	}
	
	//Move to here to response faster than before //Check bPolling_Get_Data
	if(bPolling_Get_Data & BCRF_SEND_SPP_RECEIVE_DATA_OK || bPolling_Get_Data & BCRF_SEND_SPP_RECEIVE_DATA_NG)
	{//70 01 aa 01 (55 - checksum)
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SEND_BLE_REMOTE_DATA");
#endif

#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
		{
			int i;

			uBuf[0] = SYNC_BYTE;
			uBuf[1] = PACKET_DATA;
			uBuf[2] = DATA_SOURCE_TYPE_SPP;
			uBuf[3] = DATA_CH_ANDROID1;
			uBuf[4] = 0x00; //Data Size - High Byte
			uBuf[5] = 0x03; //Data Size - Low Byte
			uBuf[6] = 0xaa;

			if(bPolling_Get_Data & BCRF_SEND_SPP_RECEIVE_DATA_OK)
			{
				uBuf[7] = 0x01;
				uBuf[8] = 0x55;

				MB3021_BT_Module_Send_Data_Packcet(uBuf, 9); //SPP COM : Send SPP OK Response to Peer Device - without checksum
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) //In case of TWS Mode
				if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
				{
					if(BTWS_Master_Slave_Connect) //2023-02-21_2
					{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
						for(i=0; i<9; i++)
#else
						for(i=0; i<8; i++)
#endif
						{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
							if(i == (BLE_MUTE_OFF_DELAY+1)) //mute off delay
								uBuf[i] = 0x00;
#ifdef SPP_EXTENSION_V44_ENABLE
							else if(i == (BLE_BT_SHORT_KEY+2)) //send checksum for slave SPK
								uBuf[(BLE_BT_SHORT_KEY+2)] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[0], 8); //Check sum
#endif
							else
#endif
							uBuf[i] = uSPP_receive_buf8[i];
							//_DBH(uBuf[i+6]);//_DBH(uSPP_receive_buf8[i]);
						}

						strncpy((char *)(&uBuf[i]), uBT_Cur_TWS_Device_Address, 6);
#ifndef TWS_MODE_AP_TEST
						MB3021_BT_Module_Send_cmd_param(CMD_SEND_SYNC_DATA_32, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#endif //TWS_MODE_AP_TEST
					}
				}
				else
#endif
				{

					//delay_ms(100); //Deleted 100ms delay because of speed of MCU response for Tablet App

					uBuf[0] = BLE_DATA_0;
					uBuf[1] = BLE_DATA_1;
					//To Do !!! Need to check with customer
					uBuf[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
					uBuf[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 
#ifdef MASTER_SLAVE_GROUPING
					if(Get_master_slave_grouping_flag() == TRUE)
					{
						uBuf[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
						uBuf[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
					}
					else
#endif
					{
						uBuf[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
						uBuf[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;
					}

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
					for(i=0; i<9; i++)
#else
					for(i=0; i<8; i++)
#endif
					{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
						if(i == 7) //mute off delay
							uBuf[i+6] = 0x00;
						else if(i == 8) //send checksum for slave SPK
							uBuf[i+6] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 8); //Check sum
						else
#endif
						uBuf[i+6] = uSPP_receive_buf8[i];
						//_DBH(uBuf[i+6]);//_DBH(uSPP_receive_buf8[i]);
					}

					BMaster_Send_BLE_Remote_Data = TRUE;

					if(uBuf[6] == 0xff) //When we didn't get SPP data from Tablet, we don't need to send SPP receive data.
					{
						MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32-0x0100, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
					}
					else
					{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Added mute off delay for Slave SPK
						MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#else
						MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0700, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#endif
					}
				}
			}
			else //BCRF_SEND_SPP_RECEIVE_DATA_NG
			{
#ifdef SPP_EXTENSION_ENABLE														
				uBuf[7] = uSPP_RECEIVE_DATA_ERROR;
				uBuf[8] = SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 2);
#else
				uBuf[7] = 0xFF;
				uBuf[8] = 0x57;
#endif		
				MB3021_BT_Module_Send_Data_Packcet(uBuf, 9); //SPP COM : Send SPP OK Response to Peer Device - without checksum
			}			
		}

		if(bPolling_Get_Data & BCRF_SEND_SPP_RECEIVE_DATA_OK)
		{
			bPolling_Get_Data &= (~BCRF_SEND_SPP_RECEIVE_DATA_OK); //Clear flag

			if(Is_SSP_REBOOT_KEY_In()) //check whether it receive reboot key thru SPP and execute reboot if it receives reboot key
			{
				delay_ms(200); //2023-04-27_2 : When TWS Master get reboot CMD over USEN Tablet remocon App, TWS Master need some delay to send reboot CMD to TWS Slave.
				MB3021_BT_Disconnect_All_ACL(); //To avoid last connection fail after reboot //Reboot recovery solution - 1
				delay_ms(1000); //delay for send BLE Data to Slave SPK(Receiver)
				SW_Reset();
			}

			if(Is_SSP_FACTORY_RESET_KEY_In()) //check whether it receive factory reset key thru SPP and execute reboot if it receives factory reset key
			{
				B_SSP_FACTORY_RESET_KEY_In = FALSE;
#ifdef SWITCH_BUTTON_KEY_ENABLE
				bFactory_Reset_Mode = TRUE;
#endif
				Factory_Reset_Value_Setting();
			}
		}
		else
			bPolling_Get_Data &= (~BCRF_SEND_SPP_RECEIVE_DATA_NG); //Clear flag
			
#ifndef MASTER_MODE_ONLY  //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
		//bPolling_Get_Data |= BCRF_SEND_BLE_EXTRA_DATA; //2023-01-05_6 : To set 64 Step volume level under BAP when BAP receives SPP data from peerdevice
		if(uNext_Grouping_State == GROUPING_NONE_MODE) //Excepting GROUPING
			TIMER20_BT_send_extra_data_flag_start(); //2023-01-05_6 : Changed SW which send 64 step volume to BAP
#endif
#endif
	}

	if(uCount == 0x3332) //200ms //0xffff = 1s, 0xffff/2(0x7fff) = 500ms/0x1999 = 100ms
	{
		uCount = 0;
	}
	else
	{
		uCount++;
		return;
	}

#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) && defined(MASTER_SLAVE_GROUPING)
	if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
		uNext_Grouping_State = GROUPING_NONE_MODE;
#endif

#ifdef MASTER_SLAVE_GROUPING
	switch(uNext_Grouping_State)
	{		
		case GROUPING_EVENT_WAIT_STATE:
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_EVENT_WAIT_STATE");
#endif

		break;
					
		case GROUPING_MASTER_NORMAL_MODE:
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_MASTER_NORMAL_MODE");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
			uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
			uBuf[0] = 0x00; //Noraml Mode
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
			MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02); //For Slave Mute
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
			AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
			TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_MASTER_NORMAL_MODE);
			MB3021_BT_Module_Send_cmd_param(CMD_BA_MODE_CONTROL_32, uBuf);
		break;

		case GROUPING_MASTER_BROADCASTER_MODE:
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_MASTER_BROADCASTER_MODE");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
			uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
			uBuf[0] = 0x01; //Broadcaster Mode
			TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_MASTER_BROADCASTER_MODE);
			MB3021_BT_Module_Send_cmd_param(CMD_BA_MODE_CONTROL_32, uBuf);			
		break;

		case GROUPING_MASTER_SET_MANUFACTURE_DATA:
		{
#ifndef MASTER_MODE_ONLY
			Switch_Master_Slave_Mode mode;

			mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_MASTER_SET_MANUFACTURE_DATA");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
			
#ifndef MASTER_MODE_ONLY		
			if(mode == Switch_Master_Mode)
#endif
			{
				int i;

				uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
					
				uBuf[0] = BLE_DATA_0;
				uBuf[1] = BLE_DATA_1;
				//To Do !!! Need to check with customer
				uBuf[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
				uBuf[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 

				if(Get_master_slave_grouping_flag() == TRUE)
				{
					uBuf[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
					uBuf[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
				}
				else
				{
					uBuf[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
					uBuf[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;
				}

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
				for(i=0; i<9; i++)
#else
				for(i=0; i<8; i++)
#endif
				{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
					if(i == 7) //mute off delay
						uBuf[i+6] = 0x00; //If we send 0x02(Mute On), Slave make mute on even though mute off because this communication is reached slower than actual mute off.
					else if(i == 8) //send checksum for slave SPK
						uBuf[i+6] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 8); //Check sum
					else
#endif
					uBuf[i+6] = uSPP_receive_buf8[i];
					//_DBH(uBuf[i+6]);//_DBH(uSPP_receive_buf8[i]);
				}

				TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_MASTER_SET_MANUFACTURE_DATA);

#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Added mute off delay for Slave SPK
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#else
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0700, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#endif
			}
#ifndef MASTER_MODE_ONLY
			else
			{
				uNext_Grouping_State = GROUPING_NONE_MODE; //Ignore !!!
				uPrev_Grouping_State = GROUPING_NONE_MODE;
			}
#endif
		}
		break;

		case GROUPING_MASTER_ADVERTISING_ON:
		{
#ifndef MASTER_MODE_ONLY
			Switch_Master_Slave_Mode mode;
	
			mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_MASTER_ADVERTISING_ON");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
#ifndef MASTER_MODE_ONLY
			if(mode == Switch_Master_Mode)
#endif
			{
				uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
				
				uBuf[0] = 0x01; //Advertising On
				TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_MASTER_ADVERTISING_ON);
				MB3021_BT_Module_Send_cmd_param(CMD_ADVERTISING_CONTROL_32, uBuf);
			}
#ifndef MASTER_MODE_ONLY
			else
			{
				uNext_Grouping_State = GROUPING_NONE_MODE; //Ignore !!!
				uPrev_Grouping_State = GROUPING_NONE_MODE;
			}
#endif
		}
		
#if defined(AUX_INPUT_DET_ENABLE) //2023-06-07_4 : Uner Aux mode, BSP-01T shoud send "BCRF_INFORM_HOST_MODE" like 2023-04-13_2 solution. //&& defined(USEN_BAP) //2023-04-13_2 : Under Aux mode, when broadcast master/slave grouping, we need to make mute off.
		if(Aux_In_Exist() //Need to keep LED off under Aux Mode
#ifdef AD82584F_USE_POWER_DOWN_MUTE
		&& !IS_Display_Mute()//This is mute off delay and that's means this action should be worked in mute off. //if(Is_Mute())
#else
#ifdef TAS5806MD_ENABLE
		&& Is_Mute()
#endif
#endif
		)
		{
#ifdef USEN_BAP
#ifdef AD82584F_ENABLE
			AD82584F_Amp_Mute(FALSE, FALSE); //MUTE OFF
#else //TAS5806MD_ENABLE						
#ifdef TAS5806MD_ENABLE
			TAS5806MD_Amp_Mute(FALSE, FALSE); //MUTE OFF
#endif
#endif //TAS5806MD_ENABLE
#endif
		MB3021_BT_Module_Forced_Input_Audio_Path_Setting();
		}
#endif //#if defined(AUX_INPUT_DET_ENABLE) && defined(USEN_BAP)

		break;

#ifndef MASTER_MODE_ONLY
		case GROUPING_SLAVE_NORMAL_MODE:
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_SLAVE_NORMAL_MODE");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
			uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
			uBuf[0] = 0x00; //Noraml Mode
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
			AD82584F_Amp_Mute(TRUE, FALSE); //MUTE ON
#else //TAS5806MD_ENABLE
			TAS5806MD_Amp_Mute(TRUE, FALSE); //MUTE ON
#endif
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
			TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_SLAVE_NORMAL_MODE);
			MB3021_BT_Module_Send_cmd_param(CMD_BA_MODE_CONTROL_32, uBuf);			
		break;

		case GROUPING_SLAVE_RECEIVER_MODE:
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_SLAVE_RECEIVER_MODE");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
			uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
			uBuf[0] = 0x02; //Receiver Mode
			TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_SLAVE_RECEIVER_MODE);
			MB3021_BT_Module_Send_cmd_param(CMD_BA_MODE_CONTROL_32, uBuf);		
		break;

		case GROUPING_SLAVE_SET_MANUFACTURE_DATA:
		{
			Switch_Master_Slave_Mode mode;
	
			mode = Get_Cur_Master_Slave_Mode();

			if(mode == Switch_Master_Mode)
			{
				uPrev_Grouping_State = GROUPING_NONE_MODE;
				uNext_Grouping_State = GROUPING_NONE_MODE;
				
				break;
			}

			uPrev_Grouping_State = uNext_Grouping_State;
			uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
			
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_SLAVE_SET_MANUFACTURE_DATA");
#endif
			uBuf[0] = BLE_DATA_0;
			uBuf[1] = BLE_DATA_1;
			//To Do !!! Need to check with customer
			uBuf[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
			uBuf[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 
#ifdef MASTER_SLAVE_GROUPING
			if(Get_master_slave_grouping_flag() == TRUE)
			{
				uBuf[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
				uBuf[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
				uBuf[6] = 0x01; //0x01 : New Broadcast Connected Mode
			}
			else
#endif
			{
#if !defined(MASTER_SLAVE_GROUPING_SLAVE_EMPTY) && defined(FLASH_SELF_WRITE_ERASE)
				Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

				//we changed product ID to another one(0x20) instead of normal one(0x00) when slave still doesn't have last connection info.
				if(uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x01
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_1
					&& uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x02
#endif
					)
				{
					uBuf[4] = BLE_DATA_GROUPING_PRODUCT0_ID_HIGH_BYTE;
					uBuf[5] = BLE_DATA_GROUPING_PRODUCT0_ID_LOW_BYTE;
					uBuf[6] = 0x00; //0x00 : Last Connected Mode
				}
				else
#endif
				{
					uBuf[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
					uBuf[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;
					uBuf[6] = 0x00; //0x00 : Last Connected Mode
				}
			}
			
			TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_SLAVE_SET_MANUFACTURE_DATA);				
			MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32, uBuf);
		}

		break;
#endif //MASTER_MODE_ONLY
		default:
		break;
	}

	if(bPolling_Set_Action & A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE) //To get MINOR_ID_BA_MODE_CONTROL response under master slave grouping mode //To avoid missing interrupt
	{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
		_DBG("\n\rSet : A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE");
#endif
#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef AD82584F_ENABLE
		AD82584F_Amp_Mute(TRUE, FALSE); //Mute On
#else //TAS5806MD_ENABLE
		TAS5806MD_Amp_Mute(TRUE, FALSE); //Mute On
#endif //TAS5806MD_ENABLE
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
		bPolling_Set_Action &= (~A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE); //Clear flag
	}
#endif //MASTER_SLAVE_GROUPING
	
	//Check bPolling_Get_BT_Profile_State
	if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_INITIALISING)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_INITIALISING");
#endif

#ifdef TIMER21_LED_ENABLE
#if defined(GIA_MODE_LED_DISPLAY_ENABLE) && !defined(BT_ALWAYS_GENERAL_MODE) //2023-01-31_2 : When Master is disconnected with Peerdevice, Master should be disconnected mode.
		if(Get_Cur_Status_LED_Mode() != STATUS_BT_GIA_PAIRING_MODE) //When current mode is STATUS_BT_GIA_PAIRING_MODE, it need to keep cur mode.
#endif
		{
#if (defined(BT_ALWAYS_GENERAL_MODE) || defined(BT_GENERAL_MODE_KEEP_ENABLE)) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //2022-12-27 : To Keep GIA Mode LED Display
			Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
#ifdef BT_ALWAYS_GENERAL_MODE //2023-01-31_2 : When Master don't have PLD List, we use fast white bliking LED display. //2023-02-14_2 : don't need LED display setting as STATUS_BT_GIA_PAIRING_MODE under slave mode
			if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x01 
#ifndef MASTER_MODE_ONLY
				&& Get_Cur_Master_Slave_Mode() == Switch_Master_Mode
#endif
				)
#else //BT_ALWAYS_GENERAL_MODE
			if(BBT_Pairing_Key_In)
#endif //BT_ALWAYS_GENERAL_MODE
			Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
			else
#endif //#if (defined(BT_ALWAYS_GENERAL_MODE) || defined(BT_GENERAL_MODE_KEEP_ENABLE)) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //2022-12-27 : To Keep GIA Mode LED Display
			Set_Status_LED_Mode(STATUS_BT_FAIL_OR_DISCONNECTION_MODE);
		}
#endif //TIMER21_LED_ENABLE
		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_INITIALISING); //Clear flag
	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_READY)
	{
#ifdef BT_DISCONNECT_CONNECTABLE_ENABLE
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;
		
		mode = Get_Cur_Master_Slave_Mode();
#endif
#endif

#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_READY");
#endif
#ifdef TIMER21_LED_ENABLE
#if (defined(BT_ALWAYS_GENERAL_MODE) || defined(BT_GENERAL_MODE_KEEP_ENABLE)) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //2022-12-27 : To Keep GIA Mode LED Display
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
#ifdef BT_ALWAYS_GENERAL_MODE//2023-01-31_2 : When Master don't have PLD List, we use fast white bliking LED display. //2023-02-14_2 : don't need LED display setting as STATUS_BT_GIA_PAIRING_MODE under slave mode
		if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x01
#ifndef MASTER_MODE_ONLY
			&& mode == Switch_Master_Mode
#endif
			)
#else //BT_ALWAYS_GENERAL_MODE
		if(BBT_Pairing_Key_In)
#endif //BT_ALWAYS_GENERAL_MODE
		Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
		else
#endif //#if (defined(BT_ALWAYS_GENERAL_MODE) || defined(BT_GENERAL_MODE_KEEP_ENABLE)) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //2022-12-27 : To Keep GIA Mode LED Display
		Set_Status_LED_Mode(STATUS_BT_FAIL_OR_DISCONNECTION_MODE);
#endif
#ifdef BT_DISCONNECT_CONNECTABLE_ENABLE
#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
		{
			BKeep_Connectable = TRUE;
#if 0 //2023-05-30_1 : Added this condition. Only current A2DP device is available until user select BT Long Key to connect other device.
//defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) //2022-12-27 : To keep connectable mode when BT is not connected with Peer Device
			if(BBT_Pairing_Key_In)
				bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device
			else
#endif
			bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device
		}
#endif
		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_READY); //Clear flag
	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_CONNECTING)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_CONNECTING");
#endif

#ifdef GIA_MODE_LED_DISPLAY_ENABLE
		if(Get_Cur_Status_LED_Mode() != STATUS_BT_GIA_PAIRING_MODE
#ifdef TWS_MODE_ENABLE
			&& !(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && BBT_Is_Connected) //2023-04-03_2 : When TWS slave is conected, we don't need to display BT STATUS LED on TWS Master.
#endif
			) //When current mode is STATUS_BT_GIA_PAIRING_MODE, it need to keep cur mode
#endif
#ifdef TIMER21_LED_ENABLE
		Set_Status_LED_Mode(STATUS_BT_PAIRING_MODE);
#endif

#if defined(BT_DISCONNECT_CONNECTABLE_ENABLE) || defined(BT_CLEAR_CONNECTABLE_MODE)
		BKeep_Connectable = FALSE;
#endif
		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_CONNECTING); //Clear flag
	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_CONNECTED)
	{
#if defined(FLASH_SELF_WRITE_ERASE) && (defined(MASTER_SLAVE_GROUPING) || defined(GIA_MODE_LED_DISPLAY_ENABLE)) && !defined(MASTER_MODE_ONLY)
		Switch_Master_Slave_Mode mode;
		
		mode = Get_Cur_Master_Slave_Mode();
#endif //FLASH_SELF_WRITE_ERASE
		
#if defined(MASTER_SLAVE_GROUPING) && defined(FLASH_SELF_WRITE_ERASE) && !defined(MASTER_MODE_ONLY)
		if(mode == Switch_Slave_Mode
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) //2022-11-14_3 : Under TWS Mode, Master/Slave working is not permitted
			&& (Get_Cur_LR_Stereo_Mode() != Switch_LR_Mode)
#endif
			)
		{
			Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

			if(uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x01
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_1
				&& uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x02
#endif
				)
			{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
				_DBG("\n\rSave FLASH_SAVE_SLAVE_LAST_CONNECTION : 1");
#endif
				FlashSaveData(FLASH_SAVE_SLAVE_LAST_CONNECTION, 1); //Save Last connection status of slave to Flash
				TIMER20_Master_Slave_Grouping_flag_Start(); //To recover Original Product ID
			}
		}
#endif //MASTER_SLAVE_GROUPING

#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_CONNECTED");
#endif

#if defined(FLASH_SELF_WRITE_ERASE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //Save whether Paired Device exist in Flash
#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
			FlashSaveData(FLASH_SAVE_DATA_PDL_NUM, 1); //Save PDL List to Flash memory for SPP/BLE Data
#endif

#ifdef TIMER21_LED_ENABLE
		Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);
#endif
#ifdef NEW_TWS_MASTER_SLAVE_LINK
		Peer_Device_Status = PEER_DEVICE_PAIRED; //2023-04-26_15 : To make current status of Peer device(A2DP)

		if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //2023-04-26_14 : TWS Master is connected with A2DP device and then TWS Master send LIAC to prohibit access from other devices
		{
			MB3021_BT_Module_TWS_Set_Discoverable_Mode();
		}
#endif
#ifndef MASTER_MODE_ONLY //2023-05-30_2 : Under Broadcast mode, when A2DP is conected, we need to disable DICOVERABLE_MODE to avoid other device searching. Need to check this under BAP-01
		if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode && Get_Cur_LR_Stereo_Mode() == Switch_Stereo_Mode)
#endif
		{
			MB3021_BT_Module_Set_Discoverable_Mode_by_Param(SET_DISABLE_DISCOVERABLE_MODE);
		}
#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
		B_Auto_FactoryRST_On = TRUE; //2023-04-03_1
#endif
#ifdef SLAVE_AUTO_OFF_ENABLE
		TIMER20_Slave_auto_power_off_flag_Stop();
#endif
		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_CONNECTED); //Clear flag
#if defined(BT_DISCONNECT_CONNECTABLE_ENABLE) || defined(BT_CLEAR_CONNECTABLE_MODE)
		BKeep_Connectable = FALSE;
#endif
#if defined(SWITCH_BUTTON_KEY_ENABLE) && !defined(BT_GENERAL_MODE_KEEP_ENABLE) //To keep BBT_Pairing_Key_In value under BT_GENERAL_MODE_KEEP_ENABLE
		BBT_Pairing_Key_In = FALSE;
#endif
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
		if(Get_Cur_LR_Stereo_Mode() != Switch_LR_Mode)
#endif
#ifdef BT_CLEAR_CONNECTABLE_MODE
		bPolling_Get_Data |= BCRF_CLEAR_CONNECTABLE_MODE;
#endif
	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_DISCONNECTING)
	{
#if defined(BT_DISCONNECT_CONNECTABLE_ENABLE) && !defined(MASTER_MODE_ONLY)
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif

#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_DISCONNECTING");
#endif

#ifdef TIMER21_LED_ENABLE
#if defined(GIA_MODE_LED_DISPLAY_ENABLE) && !defined(BT_ALWAYS_GENERAL_MODE) //2023-01-31_2 : When Master is disconnected with Peerdevice, Master should be disconnected mode.
		if(Get_Cur_Status_LED_Mode() != STATUS_BT_GIA_PAIRING_MODE) //When current mode is STATUS_BT_GIA_PAIRING_MODE, it need to keep cur mode.
#endif
		{
#if (defined(BT_ALWAYS_GENERAL_MODE) || defined(BT_GENERAL_MODE_KEEP_ENABLE)) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //2022-12-27 : To Keep GIA Mode LED Display
			Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
#ifdef BT_ALWAYS_GENERAL_MODE //2023-01-31_2 : When Master don't have PLD List, we use fast white bliking LED display. //2023-02-14_2 : don't need LED display setting as STATUS_BT_GIA_PAIRING_MODE under slave mode
			if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x01 
#ifndef MASTER_MODE_ONLY
				&& mode == Switch_Master_Mode
#endif
				)
#else //BT_ALWAYS_GENERAL_MODE
			if(BBT_Pairing_Key_In)
#endif //BT_ALWAYS_GENERAL_MODE
			Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
			else
#endif //#if (defined(BT_ALWAYS_GENERAL_MODE) || defined(BT_GENERAL_MODE_KEEP_ENABLE)) && defined(SWITCH_BUTTON_KEY_ENABLE) && defined(GIA_MODE_LED_DISPLAY_ENABLE) //2022-12-27 : To Keep GIA Mode LED Display
			Set_Status_LED_Mode(STATUS_BT_FAIL_OR_DISCONNECTION_MODE);
		}
#endif //TIMER21_LED_ENABLE

#ifdef SLAVE_AUTO_OFF_ENABLE
		TIMER20_Slave_auto_power_off_flag_Start();
#endif
#ifdef BT_DISCONNECT_CONNECTABLE_ENABLE
#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
		{
#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) //2022-12-27
			if(BBT_Pairing_Key_In)
				bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device
			else
#endif
			bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device

			BKeep_Connectable = TRUE;
		}
#endif
#if defined(MASTER_SLAVE_GROUPING) && defined(USEN_BAP) && !defined(MASTER_MODE_ONLY)
		if(mode == Switch_Slave_Mode && uNext_Grouping_State == GROUPING_NONE_MODE) //Current Mode is not Groping mode
			B_Master_Is_BAP = FALSE;
#endif
#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED //2023-04-03_1
			if(B_Auto_FactoryRST_On)
			{
				B_Auto_FactoryRST_On = FALSE;
				Factory_Reset_Value_Setting();
			}
#endif

		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_DISCONNECTING); //Clear flag
	}
	else
		__NOP();

	//Check bPolling_Get_Data
#ifdef BT_CLEAR_CONNECTABLE_MODE
	if(bPolling_Get_Data & BCRF_CLEAR_CONNECTABLE_MODE) //0x100000 
	{
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();

		if(mode == Switch_Master_Mode)
#endif
		{
#ifdef BT_DEBUG_MSG	
			_DBG("\n\rDo : BCRF_SET_CONNECTABLE_MODE : disable");
#endif
#if defined(BT_DISCONNECT_CONNECTABLE_ENABLE) || defined(BT_CLEAR_CONNECTABLE_MODE)
			BKeep_Connectable = TRUE;
#endif
			uBuf[0] = 0x00; //0x0:Disable/0x01:Enable
			
			MB3021_BT_Module_Send_cmd_param(CMD_SET_CONNECTABLE_MODE_32, uBuf);
			bPolling_Get_Data_backup |= BCRF_CLEAR_CONNECTABLE_MODE;
		}

		bPolling_Get_Data &= (~BCRF_CLEAR_CONNECTABLE_MODE); //Clear flag
	}				
#endif
#ifdef DEVICE_NAME_CHECK_PAIRING
	if(bPolling_Get_Data & BCRF_SET_IO_CAPABILITY_MODE)
	{
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef BT_DEBUG_MSG
		_DBG("\n\rDo : BCRF_SET_IO_CAPABILITY_MODE");
#endif
#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
		{
			int i;

#ifdef TWS_MODE_ENABLE
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_11 : Changed IO_CAPABILITY_MODE condition to check whether current device is TWS slave or not using uBT_TWS_Remote_Device_Name
			if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && strncmp(uBT_Remote_Device_Address, uBT_TWS_Remote_Device_Address, 6) == FALSE //Need to check whether current BT address is TWS address or not
				&& (strncmp(strUSEN_Device_Name_2, uBT_TWS_Remote_Device_Name, 15) == FALSE //"15" is string length and it's "USEN MUSIC LINK"if(strUSEN_Device_Name_2)
#ifdef NEW_BT_FW_BUG_FIX //2023-02-20_1 : "10" is string length and it's "MB3021BNU0"if(strUSEN_Device_Name_3)
				|| strncmp(strUSEN_Device_Name_3, uBT_TWS_Remote_Device_Name, 10) == FALSE
#endif
				))
			{
				uBuf[0] = 0x03; //0x03 : Pairing Accept
				uBuf[1] = 0x00; //Sink mitm setting (0x00)
				
				for(i=0;i<6;i++)
					uBuf[i+2] = uBT_TWS_Remote_Device_Address[i]; //BT Address			

#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
				_DBG("\n\r0x");

				for(i=0;i<8;i++)
					_DBH(uBuf[i]);

				_DBG("\n\r Remote Addr =");
				for(i=0;i<6;i++)
					_DBH(uBT_Remote_Device_Address[i]);

				_DBG("\n\r TWS Addr =");
				for(i=0;i<6;i++)
					_DBH(uBT_TWS_Remote_Device_Address[i]);
#endif
				MB3021_BT_Module_Send_cmd_param(CMD_SET_IOCAPABILITY_MODE_32, uBuf);
			}
			else
#else //NEW_TWS_MASTER_SLAVE_LINK
			//if(BTWS_LIAC == TWS_Status_Master_Mode_Control && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
			if(BTWS_LIAC == TWS_Status_Master_Mode_Control && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && BTWS_Master_Slave_Connect == TWS_Get_Slave_Name) //2023-03-30_3 : Under TWS Master, the SETIO_CAPABILITY_MODE will be worked only that TWS_Get_Slave_Name is executed.
			{
				if(strncmp(strUSEN_Device_Name_2, uBT_TWS_Remote_Device_Name, 15)) //"15" is string length and it's "USEN MUSIC LINK"if(strUSEN_Device_Name_2)
				{
#ifdef NEW_BT_FW_BUG_FIX //2023-02-20_1 : "10" is string length and it's "MB3021BNU0"if(strUSEN_Device_Name_3)
					if(strncmp(strUSEN_Device_Name_3, uBT_TWS_Remote_Device_Name, 10))
					{
						uBuf[0] = 0x05; //0x05 : Pairing Reject
					}
					else
					{
						uBuf[0] = 0x03; //0x03 : Pairing Accept

					}
#else					
					uBuf[0] = 0x05; //0x05 : Pairing Reject
#endif
				}
				else
				{
					uBuf[0] = 0x03; //0x03 : Pairing Accept
				}

				uBuf[1] = 0x00; //Sink mitm setting (0x00)
				
				for(i=0;i<6;i++)
					uBuf[i+2] = uBT_TWS_Remote_Device_Address[i]; //BT Address			

#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
				_DBG("\n\r0x");

				for(i=0;i<8;i++)
					_DBH(uBuf[i]);
#endif
				MB3021_BT_Module_Send_cmd_param(CMD_SET_IOCAPABILITY_MODE_32, uBuf);
			}
			else
#endif //NEW_TWS_MASTER_SLAVE_LINK
#endif //TWS_MODE_ENABLE
			{
#ifdef TWS_MODE_ENABLE
				if(BBT_Is_Connected && strncmp(uBT_Cur_A2DP_Device_Address, uBT_Remote_Device_Address, 6)) //2023-04-04_1 : if the BT devcie is not cur A2DP under TWS Master, we don't need to apply it as peerdevice
				{
					uBuf[0] = 0x05; //0x05 : Pairing Reject
				}
				else
#endif
				if(strncmp(strUSEN_Device_Name_1, uBT_Remote_Device_Name, 17)) //check wheter "USEN MUSIC DEVICE" or not
				{//Current Peer Device is NOT "USEN MUSIC DEVICE"
#if defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) //2022-12-27
					if(BBT_Pairing_Key_In) //General Mode
					{
						uBuf[0] = 0x03; //0x03 : Pairing Accept
						bDevice_Paring_Accept = TRUE;
					}
					else //Device Name Check Mode
#endif
					{
						uBuf[0] = 0x05; //0x05 : Pairing Reject
						bDevice_Paring_Accept = FALSE;
					}
				}
				else
				{//Current Peer Device is "USEN MUSIC DEVICE"
#if !defined(BT_ALWAYS_GENERAL_MODE) && defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) //2023-02-02_1 //2022-12-27
					if(BBT_Pairing_Key_In) //General Mode
					{
						uBuf[0] = 0x05; //0x05 : Pairing Reject
						bDevice_Paring_Accept = FALSE;
					}
					else //Device Name Check Mode
#endif
					{
						uBuf[0] = 0x03; //0x03 : Pairing Accept
						bDevice_Paring_Accept = TRUE;
#ifdef USEN_TABLET_AUTO_ON //Check if current connected device is USEN MUSIC LINK?
						bIs_USEN_Device = TRUE;
#endif
					}
				}

				uBuf[1] = 0x00; //Sink mitm setting (0x00)
				
				for(i=0;i<6;i++)
					uBuf[i+2] = uBT_Remote_Device_Address[i]; //BT Address			

#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
				_DBG("\n\r0x");

				for(i=0;i<8;i++)
					_DBH(uBuf[i]);
#endif
				MB3021_BT_Module_Send_cmd_param(CMD_SET_IOCAPABILITY_MODE_32, uBuf);
				bPolling_Get_Data_backup |= BCRF_SET_IO_CAPABILITY_MODE;
			}
		}
#ifdef TWS_MODE_ENABLE //2023-03-09_2 : TWS can connect with USEN MUSIC LINK device Only. Sometimes If Peer Device has Slave address in its PDL, Peer device can connect with TWS Slave.
#ifndef MASTER_MODE_ONLY
		else //Slave Mode
		{
			if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
			{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG
				int i;

				_DBG("\n\r==== strUSEN_Device_Name_2 = ");
				
				for(i=0;i<15;i++)
					_DBH(strUSEN_Device_Name_2[i]);
				
				_DBG("\n\r==== uBT_TWS_Remote_Device_Name = ");
				
				for(i=0;i<15;i++)
					_DBH(uBT_TWS_Remote_Device_Name[i]);
#endif
					
				if(strncmp(strUSEN_Device_Name_2, uBT_Remote_Device_Name, 15)) //"15" is string length and it's "USEN MUSIC LINK"if(strUSEN_Device_Name_2)
				{
#ifdef NEW_BT_FW_BUG_FIX //2023-02-20_1 : "10" is string length and it's "MB3021BNU0"if(strUSEN_Device_Name_3)
					if(strncmp(strUSEN_Device_Name_3, uBT_Remote_Device_Name, 10))
					{
						MB3021_BT_Disconnect_ACL((uint8_t *)uBT_Remote_Device_Address); //2023-03-09_2 : Disconnect with Peer Device
					}
#else					
					MB3021_BT_Disconnect_ACL((uint8_t *)uBT_Remote_Device_Address); //2023-03-09_2 : Disconnect with Peer Device
#endif
				}
			}
		}
#endif
#endif //TWS_MODE_ENABLE

		bPolling_Get_Data &= (~BCRF_SET_IO_CAPABILITY_MODE); //Clear flag
	}
#endif

#ifdef AVRCP_CONNECTION_CONTROL_ENABLE
	if(bPolling_Get_Data & BCRF_SET_AVRCP_CONNECTION_CONTROL) //0x40000 Set AVRCP Connection Control Enable
	{
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();

#ifdef BT_DEBUG_MSG
		_DBG("\n\rDo : BCRF_SET_AVRCP_CONNECTION_CONTROL");
#endif
		if(mode == Switch_Master_Mode)
		{
			int i;

			uBuf[0] = 0x01; //0x0 : Disconnect AVRCP CT/0x01 : Connect AVRCP CT
			
			for(i=0;i<6;i++)
				uBuf[i+1] = uBT_Cur_A2DP_Device_Address[i];

			MB3021_BT_Module_Send_cmd_param(CMD_AVRCP_CONNECTION_CONTROL_32, uBuf);
		}

		bPolling_Get_Data &= (~BCRF_SET_AVRCP_CONNECTION_CONTROL); //Clear flag
	}				
#endif

#ifdef SPP_EXTENSION_ENABLE
	if(bPolling_Get_Data & BCRF_SEND_SPP_DATA_RESP) //0x20000
	{
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SEND_SPP_DATA_RESP");
#endif
				
#ifndef MASTER_MODE_ONLY				
		if(mode == Switch_Master_Mode)
#endif
		{
			int i;

			uBuf[0] = SYNC_BYTE;
			uBuf[1] = PACKET_DATA;
			uBuf[2] = DATA_SOURCE_TYPE_SPP;
			uBuf[3] = DATA_CH_ANDROID1;
			uBuf[4] = 0x00; //Data Size - High Byte
#ifdef SPP_EXTENSION_V50_ENABLE
			if(uCurrent_Status_buf8[1] == 0xAF) //MCU FW Version
				uBuf[5] = 0x09; //Data Size - Low Byte (Size of Data packet of MCU FW version )
			else if(uCurrent_Status_buf8[1] == 0xBF) //BT FW Version
				uBuf[5] = 0x0A; //Data Size - Low Byte (Size of Data packet of BT FW version )
			else
				uBuf[5] = 0x0F; //Data Size - Low Byte (Size of uCurrent_Status_buf8)
			
			for(i=0; i<uBuf[5]; i++)
				uBuf[i+6] = uCurrent_Status_buf8[i]; //The last byte is uBuf[13]

			MB3021_BT_Module_Send_Data_Packcet(uBuf, uBuf[5]+6); //SPP COM : Send SPP OK Response to Peer Device - without checksum			
#else //SPP_EXTENSION_V50_ENABLE
#ifdef SPP_EXTENSION_V42_ENABLE			
			uBuf[5] = 0x0F; //Data Size - Low Byte (Size of uCurrent_Status_buf8)
			
			for(i=0; i<15; i++)
				uBuf[i+6] = uCurrent_Status_buf8[i]; //The last byte is uBuf[13]

			MB3021_BT_Module_Send_Data_Packcet(uBuf, 21); //SPP COM : Send SPP OK Response to Peer Device - without checksum			
#else			
			uBuf[5] = 0x08; //Data Size - Low Byte (Size of uCurrent_Status_buf8)

			for(i=0; i<8; i++)
				uBuf[i+6] = uCurrent_Status_buf8[i]; //The last byte is uBuf[13]

			MB3021_BT_Module_Send_Data_Packcet(uBuf, 14); //SPP COM : Send SPP OK Response to Peer Device - without checksum			
#endif
#endif //SPP_EXTENSION_V50_ENABLE
		}

		bPolling_Get_Data &= (~BCRF_SEND_SPP_DATA_RESP); //Clear flag
	}
#endif
	
	if(bPolling_Get_Data & BCRF_DELETE_PAIRED_DEVICE_LIST)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_DELETE_PAIRED_DEVICE_LIST");
#endif

		uBuf[0] = 0xFF; //Delete All Device In Paired Device List		

#if 0//def BT_DEBUG_MSG	
		_DBH(uBuf[0] );
#endif

		MB3021_BT_Module_Send_cmd_param(CMD_DELETE_PAIRED_DEVICE_LIST_32, uBuf);
		
		bPolling_Get_Data &= (~BCRF_DELETE_PAIRED_DEVICE_LIST); //Clear flag
	}
	
	if(bPolling_Get_Data & BCRF_SET_LAST_CONNECTION) //For Last Connection //For init sequence (Init Sequnece : Broadcaster -0-1)
	{
		int i;
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#if 0//def BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_LAST_CONNECTION");
#endif
#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
		{
			uBuf[0] = 0x01; //Data[0] - 0x0 : Disconnect A2DP, 0x1 : Connect A2DP, 0x2 : Connect A2DP Connection
			
			for(i=0;i<6;i++)
				uBuf[i+1] = uLast_Connected_Device_Address[i]; //Data[1~6] - Peer Device Address 			

#if 0//def BT_DEBUG_MSG	
			_DBH(uBuf[0] );
#endif

			MB3021_BT_Module_Send_cmd_param(CMD_A2DP_CONNECTION_CONTROL_32, uBuf);
		}
		
		bPolling_Get_Data &= (~BCRF_SET_LAST_CONNECTION); //Clear flag
	}
	
	if(bPolling_Get_Data & BCRF_GET_PAIRED_DEVICE_LIST)
	{
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_GET_PAIRED_DEVICE_LIST =");
#endif
#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
		{
			MB3021_BT_Module_Send_cmd_param(CMD_GET_PAIRED_DEVICE_LIST_32, uBuf);
			bPolling_Get_Data_backup |= BCRF_GET_PAIRED_DEVICE_LIST;
		}
			
		bPolling_Get_Data &= (~BCRF_GET_PAIRED_DEVICE_LIST); //Clear flag
	}
	

#ifdef TWS_MODE_ENABLE
	if(bPolling_Get_Data & BCRF_TWS_MODE_CONTROL_RETRY) //TWS Mode
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_TWS_MODE_CONTROL =");
#endif

#ifdef SW1_KEY_TWS_MODE
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
#endif
		{			
			uBuf[0] = 0x00; //Set TWS Mode Exit
			MB3021_BT_Module_Send_cmd_param(CMD_ID_TWS_MODE_CONTROL_32, uBuf);		
		}

		bPolling_Get_Data &= (~BCRF_TWS_MODE_CONTROL_RETRY); //Clear flag
	}
	
	if(bPolling_Get_Data & BCRF_TWS_MODE_CONTROL) //TWS Mode - 1
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_TWS_MODE_CONTROL =");
#endif

#ifdef SW1_KEY_TWS_MODE
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
#endif
		{
			Switch_Master_Slave_Mode mode;

			mode = Get_Cur_Master_Slave_Mode();
			
			if(mode == Switch_Master_Mode)
			{
				uBuf[0] = 0x01; //Set TWS Master Mode
				MB3021_BT_Module_Send_cmd_param(CMD_ID_TWS_MODE_CONTROL_32, uBuf);
			}
			else
			{	
				uBuf[0] = 0x02; //Set TWS Slave Mode
				MB3021_BT_Module_Send_cmd_param(CMD_ID_TWS_MODE_CONTROL_32, uBuf);
			}
		}

		bPolling_Get_Data &= (~BCRF_TWS_MODE_CONTROL); //Clear flag
	}
#endif //TWS_MODE_CONTROL
	
	if(bPolling_Get_Data & BCRF_INFORM_HOST_MODE) //For init sequence (Init Sequnece : Broadcaster -6)
	{
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#if 0//def BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_INFORM_HOST_MODE =");
#endif
		if(BBT_Init_OK) //The Source change is only available when BT Init is finished
		{
#ifndef MASTER_MODE_ONLY
			if(mode == Switch_Master_Mode)
#endif
			{
#ifdef AUX_INPUT_DET_ENABLE
				if(Aux_In_Exist())
				{
					uBuf[0] = 0x50; //Aux Mode
#ifdef TIMER21_LED_ENABLE
					Set_Status_LED_Mode(STATUS_AUX_MODE);
#endif
#if 0//def BT_DEBUG_MSG	
					_DBG("AUX Mode");
#endif
				}
				else
#endif //AUX_INPUT_DET_ENABLE
				{
#ifdef TIMER21_LED_ENABLE
					Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
#endif
					uBuf[0] = 0x07; //Bluetooth Mode
#if 0//def BT_DEBUG_MSG	
					_DBG("BT Mode");
#endif
				}

#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE) //To avoid, BT has no output sometime when user alternates Aux mode / BT mode repeately
				if(uMode_Change == uBuf[0] 
#ifdef AD82584F_USE_POWER_DOWN_MUTE
					&& !IS_Display_Mute()
#else
					&& Is_Mute()
#endif
					)
				{
#ifdef AD82584F_ENABLE
					if(AD82584F_Amp_Get_Cur_CLK_Status())
#else //TAS5806MD_ENABLE
					if(TAS5806MD_Amp_Detect_FS(FALSE)) //2022-10-17_2
#endif //AD82584F_ENABLE
					{
						if(uMode_Change == 0x07 || uMode_Change == 0x50) //BT Mode or Aux Mode
						{
#ifdef BT_DEBUG_MSG	
							_DBG("\n\r Mute Off : To avoid, BT has no output sometime when user alternates Aux mode / BT mode repeately");
#endif
#ifdef AUTO_ONOFF_ENABLE
							TIMER20_auto_power_flag_Stop();
#endif
#ifndef USEN_BAP //2023-05-09_1 : Reduced the checking time from 5.3s to 3s and no need mute under BAP-01
						        TIMER20_mute_flag_Start();
#endif
						}
					}
				}
				else
					uMode_Change = uBuf[0];
#endif //#if defined(AD82584F_ENABLE) || defined(TAS5806MD_ENABLE)
#ifdef TWS_MODE_ENABLE //2023-01-26_2 : Under TWS Aux mode, Master can't output aux audio when user plug-out slave. Fixed it.
				if(uBuf[0] == 0x50 && Aux_In_Exist() && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //Aux
				{
					uint8_t Temp;
					Temp = 0x07;
					
					MB3021_BT_Module_Send_cmd_param(CMD_INFORM_HOST_MODE_32, &Temp);
				}
				
				MB3021_BT_Module_Send_cmd_param(CMD_INFORM_HOST_MODE_32, uBuf);
#else
				MB3021_BT_Module_Send_cmd_param(CMD_INFORM_HOST_MODE_32, uBuf);
#endif
			}
#ifndef MASTER_MODE_ONLY
			else
			{
				uBuf[0] = 0x07; //Bluetooth Mode
#if 0//def BT_DEBUG_MSG	
				_DBG("BT Mode");
#endif
				MB3021_BT_Module_Send_cmd_param(CMD_INFORM_HOST_MODE_32, uBuf);
			}
#endif
#ifdef SWITCH_BUTTON_KEY_ENABLE
			bFactory_Reset_Mode = FALSE;
#endif
			bPolling_Get_Data &= (~BCRF_INFORM_HOST_MODE); //Clear flag
			bPolling_Get_Data_backup |= BCRF_INFORM_HOST_MODE;
		}
	}

	if(bPolling_Get_Data & BCRF_ADVERTISING_CONTROL) //For init sequence (Init Sequnece : Broadcaster -5)
	{
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#if 0//def BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_ADVERTISING_CONTROL");
#endif

#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode)
#endif
		{
			uBuf[0] = 0x01; //Advertising On
			MB3021_BT_Module_Send_cmd_param(CMD_ADVERTISING_CONTROL_32, uBuf);
			bPolling_Get_Data_backup |= BCRF_ADVERTISING_CONTROL;
		}
		
		bPolling_Get_Data &= (~BCRF_ADVERTISING_CONTROL); //Clear flag
	}
	
	if(bPolling_Get_Data & BCRF_BA_MODE_CONTROL)
	{
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode) //Inavlid BCRF_BA_MODE_CONTROL
		{
			bPolling_Get_Data &= (~BCRF_BA_MODE_CONTROL); //Clear flag
		}
		else
#endif
		{
#ifndef MASTER_MODE_ONLY
			Switch_Master_Slave_Mode mode;

			mode = Get_Cur_Master_Slave_Mode();
#if 0//def BT_DEBUG_MSG	
			_DBG("\n\rDo : BCRF_BA_MODE_CONTROL");
#endif

			if(mode == Switch_Slave_Mode && uSlave_BA_MODE_CONTROL_count == 0) //If we use 200ms polling way, BA_MODE_CONTROL make fails under Slave mode.
			{
				uSlave_BA_MODE_CONTROL_count = 1;
			}
			else
#endif
			{
#ifndef MASTER_MODE_ONLY
				uSlave_BA_MODE_CONTROL_count = 0;
#endif
				
#ifdef MASTER_MODE_ONLY
				uBuf[0] = 0x01; //Broadcaster Mode
#else
				if(mode == Switch_Master_Mode)
				{
					uBuf[0] = 0x01; //Broadcaster Mode
				}
				else
					uBuf[0] = 0x02; //Receiver Mode
#endif
				MB3021_BT_Module_Send_cmd_param(CMD_BA_MODE_CONTROL_32, uBuf);
				
				BMaster_Send_BLE_Remote_Data = FALSE; //2022-11-11 : Move to Here
				
				bPolling_Get_Data &= (~BCRF_BA_MODE_CONTROL); //Clear flag
				bPolling_Get_Data_backup |= BCRF_BA_MODE_CONTROL;
			}
		}
	}

	if(bPolling_Get_Data & BCRF_SET_BLE_MANUFACTURE_DATA)
	{
		int i;
#ifndef MASTER_MODE_ONLY
		Switch_Master_Slave_Mode mode;

		mode = Get_Cur_Master_Slave_Mode();
#endif
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_BLE_MANUFACTURE_DATA");
#endif
		uBuf[0] = BLE_DATA_0;
		uBuf[1] = BLE_DATA_1;
		//To Do !!! Need to check with customer
		uBuf[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
		uBuf[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 
#ifdef MASTER_SLAVE_GROUPING
		if(Get_master_slave_grouping_flag() == TRUE) //If Slave doesn't have Last connection information, we change Product ID further below.
		{
			uBuf[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
			uBuf[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
		}
		else
#endif
		{
			uBuf[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
			uBuf[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;
		}
#ifndef MASTER_MODE_ONLY
		if(mode == Switch_Master_Mode) //Master SPK Send BLE data to Slave SPK !!!
#endif //#ifndef MASTER_MODE_ONLY
		{//To Do !!! Need to set each key values for BT Slave SPK
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK //Fixed wrong value(which is uSPP_receive_buf8 size from 9 to 8)
			for(i=0; i<9; i++)
#else
			for(i=0; i<8; i++)
#endif
			{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
				if(i == 7) //mute off delay
					uBuf[i+6] = 0x00; //If we send 0x02(Mute On), Slave make mute on even though mute off because this communication is reached slower than actual mute off.
				else if(i == 8) //send checksum for slave SPK
					uBuf[i+6] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 8); //Check sum
				else
#endif
				uBuf[i+6] = uSPP_receive_buf8[i];
				//_DBH(uBuf[i+6]);//_DBH(uSPP_receive_buf8[i]);
			}

			//BMaster_Send_BLE_Remote_Data = TRUE;

			//delay_ms(50); //Deleted 50ms delay because of speed of Master CMD for Slave
			
			if(uBuf[6] == 0xff) //When we didn't get SPP data from Tablet, we don't need to send SPP receive data.
			{
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE //this says now is init state so we need to send current Master setting to Slave
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Send mute off delay for Slave SPK
				for(i=0; i<9; i++)
#else
				for(i=0; i<8; i++)
#endif
				{
					uBuf[i+6] = uInput_Key_Sync_buf8[i];
#ifdef BT_DEBUG_MSG
					_DBH(uInput_Key_Sync_buf8[i]);
#endif
				}
				
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Added mute off delay for Slave SPK
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#else
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0700, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#endif
#else
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32-0x0100, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#endif
			}
			else
			{
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE //Added mute off delay for Slave SPK
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#else
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0700, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
#endif
			}
		}
#ifndef MASTER_MODE_ONLY
		else //mode == Switch_Slave_Mode // Receiver decides whether it's connected with New connection or Last coneection
		{//0x01 : New Broadcast Connected Mode/0x00 : Normal Connected Mode
#ifdef SWITCH_BUTTON_KEY_ENABLE
			if(bFactory_Reset_Mode)
			{
#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
				uBuf[6] = 0x01;
#else
				uBuf[6] = 0x00;//uBuf[6] = 0x01; //0x01 : New Broadcast Connected Mode //Changed BT module spec : The 0x00 parm can be worked as 0x01 after factory reset(Deleted Paired List)
#endif
				bFactory_Reset_Mode = FALSE;
			}
			else
#endif
			{
#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
				uBuf[6] = 0x01;
#else
				uBuf[6] = 0x00; //0x00 : Last Connected Mode
#endif
			}
			
#if defined(FLASH_SELF_WRITE_ERASE) && defined(MASTER_SLAVE_GROUPING)
			Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

			if(uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x01
#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-09_1
				&& uFlash_Read_Buf[FLASH_SAVE_SLAVE_LAST_CONNECTION] != 0x02
#endif
				) //If Slave doesn't have Last connection information, we change Product ID
			{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
				_DBG("\n\rChange Slave Product ID For Pairing : No Last connection information ~~~~!!!");
#endif
#ifdef MASTER_SLAVE_GROUPING_SLAVE_EMPTY
				uBuf[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
				uBuf[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
				uBuf[6] = 0x01; //0x01 : New Broadcast Connected Mode
#ifdef MASTER_SLAVE_GROUPING_LED_DISPLAY //Slave doesn't have last connection information
				Set_Status_LED_Mode(STATUS_BT_MASTER_SLAVE_PAIRING_MODE);
#endif
#else //MASTER_SLAVE_GROUPING_SLAVE_EMPTY
				uBuf[4] = BLE_DATA_GROUPING_PRODUCT0_ID_HIGH_BYTE;
				uBuf[5] = BLE_DATA_GROUPING_PRODUCT0_ID_LOW_BYTE;
				uBuf[6] = 0x01; //0x01 : New Broadcast Connected Mode
#endif //MASTER_SLAVE_GROUPING_SLAVE_EMPTY
			}
#endif //#if defined(FLASH_SELF_WRITE_ERASE) && defined(MASTER_SLAVE_GROUPING)
			
			TIMER20_mute_flag_Start();
			
			MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32, uBuf);
		}
#endif //#ifndef MASTER_MODE_ONLY
		bPolling_Get_Data &= (~BCRF_SET_BLE_MANUFACTURE_DATA); //Clear flag
		bPolling_Get_Data_backup |= BCRF_SET_BLE_MANUFACTURE_DATA;
	}

	if(bPolling_Get_Data & BCRF_INIT_SINK_MODE)
	{
		int i;
		
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_INIT_SINK_MODE");
#endif
#ifdef DEVICE_NAME_CHECK_PAIRING
		uBT_Remote_Device_Name_Size = 0; //Init under INIT_SINK_MODE

		for(i=0; i<6; i++)
		{
			uBT_Remote_Device_Address[i] = 0;
			uBT_Cur_A2DP_Device_Address[i] = 0;
		}
#else
		for(i=0; i<6; i++) //Clear variable upon Disconnecting
		{
			uBT_Cur_A2DP_Device_Address[i] = 0;
		}
#endif

		uBuf[0] = 0x91;//0x19; //No Delay/Left Justified/24bit/None(Sampling Frequency - To avoid BT DSP Resource issue)/I2S Master
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE)
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
			uBuf[1] = 0x03; //Enable TWS
		else
#endif
		uBuf[1] = 0x00; //Disable TWS
#if defined(TWS_MODE_ENABLE) && defined(SW1_KEY_TWS_MODE) //2022-12-07 : Under TWS Mode, we need to disable AAC because If we support AUX, AAC can't be supported.
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
			uBuf[2] = 0x01; //Enable AAC(b1)/A2DP Sink(b0)
		else
#endif
			uBuf[2] = 0x03; //Enable AAC(b1)/A2DP Sink(b0)
#if defined(AVRCP_ENABLE) || defined(TWS_MODE_ENABLE)
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
			uBuf[3] = 0x07;//Enable AVRCP
		else
			uBuf[3] = 0x00; //0x07;//Disable AVRCP
#else
		uBuf[3] = 0x00; //0x07;//Disable AVRCP
#endif
		uBuf[4] = 0x00; //Disable HFP Profile Features
		uBuf[5] = 0x00; //Disable HFP Supported Features
		uBuf[6] = 0x00; //Disable HFP Supported Features
		uBuf[7] = 0x00; //Disable HFP enable Feature
		uBuf[8] = 0x00; //Disable PBAP Profile Features
		
#ifndef MASTER_MODE_ONLY
		if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode) //2023-03-09_1 : When user used Master with USEN Remote APP and then user changed the master to slave, we need to avoid USEN Remote APP try to connect with Slave using SPP.
			uBuf[9] = 0x00; //Disable SSP Frofile Features
		else //Master
#endif
		uBuf[9] = 0x05; //SSP Frofile Features - Enable Android 1[b2] / Enable SPP Profile[b0]
		
#ifdef NO_BROADCAST_MODE
		uBuf[10] = 0x00; //Disable BLE Features
		uBuf[11] = 0x00; //Broadcast Audio Features - Enable Broadcast Audio[b0]
#else
		uBuf[10] = 0x01; //Enable BLE Features
		uBuf[11] = 0x01; //Broadcast Audio Features - Enable Broadcast Audio[b0]
#endif
		uBuf[12] = 0x00; //Disable Battery Features

#if defined(DEVICE_NAME_CHECK_PAIRING) && defined(SWITCH_BUTTON_KEY_ENABLE)
#ifdef TWS_MODE_ENABLE
		if(Get_Cur_Master_Slave_Mode() == Switch_Slave_Mode && Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode)
		{
			uBuf[13] = 0x00; //Disable I/O Capability Features
		}
		else
#endif
		{
#ifdef BT_ALWAYS_GENERAL_MODE //2023-01-31_1
#if defined(SWITCH_BUTTON_KEY_ENABLE) && defined(BT_GENERAL_MODE_KEEP_ENABLE) //2022-12-27 : To check REMOTE_DEVICE_NAME to disconnect USEN Tablet under General Mode
			uBuf[13] = 0x01; //Enable I/O Capability Features
#else
			uBuf[13] = 0x00; //Disable I/O Capability Features
#endif
#else //BT_ALWAYS_GENERAL_MODE
			if(BBT_Pairing_Key_In) //GIA_PAIRING_MODE
			{
#if defined(SWITCH_BUTTON_KEY_ENABLE) && defined(BT_GENERAL_MODE_KEEP_ENABLE) //2022-12-27 : To check REMOTE_DEVICE_NAME to disconnect USEN Tablet under General Mode
				uBuf[13] = 0x01; //Enable I/O Capability Features
#else
				uBuf[13] = 0x00; //Disable I/O Capability Features
#endif
			}
			else //DEVICE_NAME_CHEKING_PAIRING MODE
			{
				uBuf[13] = 0x01; //Enable I/O Capability Features
				
#if defined(FLASH_SELF_WRITE_ERASE) && defined(BT_GENERAL_MODE_KEEP_ENABLE) //2022-12-27 : To clear FLASH_SAVE_GENERAL_MODE_KEEP to call save function //2022-12-23
				Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

				if(uFlash_Read_Buf[FLASH_SAVE_GENERAL_MODE_KEEP] != 0x00)
				{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
					_DBG("\n\rSave FLASH_SAVE_GENERAL_MODE_KEEP : 0");
#endif
					FlashSaveData(FLASH_SAVE_GENERAL_MODE_KEEP, 0); //Save GENERAL MODE Status(DEVICE_NAME_CHEKING_PAIRING MODE) to Flash
				}
#endif
			}
#endif //BT_ALWAYS_GENERAL_MODE
		}
#else //DEVICE_NAME_CHECK_PAIRING
		uBuf[13] = 0x00; //Disable I/O Capability Features
#endif //DEVICE_NAME_CHECK_PAIRING
		
		MB3021_BT_Module_Send_cmd_param(CMD_INIT_SINK_MODE_32, uBuf);

		bPolling_Get_Data &= (~BCRF_INIT_SINK_MODE); //Clear flag
		bPolling_Get_Data_backup |= BCRF_INIT_SINK_MODE;
	}

	if(bPolling_Get_Data & BCRF_SET_DEVICE_ID_CONTROL)
	{ 
#ifdef BT_DEBUG_MSG
		_DBG("\n\rDo : BCRF_SET_DEVICE_ID_CONTROL");
#endif
#if defined(VERSION_INFORMATION_SUPPORT) && !defined(TWS_MASTER_SLAVE_COM_ENABLE)
		//strncpy(MCU_Version, uBuf, 6);
		MB3021_BT_Module_Send_cmd_param(CMD_SET_DEVICE_ID_32, (uint8_t *)MCU_Version);
#else

//SET_DEVICE_ID has not 0x00 00 00 00 00 00 (6 Byte) values.
#ifdef TWS_MASTER_SLAVE_GROUPING //2022-12-15 //TWS : Setting and Sending SET_DEVICE_ID under Master mode
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);	
		
		if(uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0x00 && uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0] != 0xff) //In case of valid SET_DEVICE_ID
		{//It has the history that Master is connected with TWS Slave
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-05-15_2 : If Saved TWS mode is not matched cur TWS Mode, we need to ignore the SET_DEVICE ID setting.
			uint8_t cur_Master_Slave_Mode;

			if(Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
			{
				cur_Master_Slave_Mode = 0x01;
			}
			else
			{
				cur_Master_Slave_Mode = 0x02;
			}
			
			if(uFlash_Read_Buf[FLASH_TWS_MASTER_SLAVE_ID] != cur_Master_Slave_Mode)
			{//It has NOT the history that Master is connected with TWS Slave
				if(cur_Master_Slave_Mode == 0x01) //Master
				{
					uBuf[0] = 0x00; //Vendor ID : High byte
					uBuf[1] = 0x00; //Vendor ID : Low byte
					uBuf[2] = 0x00; //Product ID : High byte
					uBuf[3] = 0x00; //Product ID : Low byte
					uBuf[4] = 0x00; //Product Version : High byte
					uBuf[5] = 0x00; //Product Version : Low byte
				}
				else
				{
					uBuf[0] = 0xFF; //Vendor ID : High byte
					uBuf[1] = 0xFF; //Vendor ID : Low byte
					uBuf[2] = 0xFF; //Product ID : High byte
					uBuf[3] = 0xFF; //Product ID : Low byte
					uBuf[4] = 0xFF; //Product Version : High byte
					uBuf[5] = 0xFF; //Product Version : Low byte
				}
#ifdef BT_DEBUG_MSG
				_DBG("\n\r ### Mismatching SET_DEVICE ID becuase Master/Slave mode is changed !!!");
#endif
			}
			else
#endif //NEW_TWS_MASTER_SLAVE_LINK
			{
				uBuf[0] = uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_0]; //Vendor ID : High byte
				uBuf[1] = uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_1]; //Vendor ID : Low byte
				uBuf[2] = uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_2]; //Product ID : High byte
				uBuf[3] = uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_3]; //Product ID : Low byte
				uBuf[4] = uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_4]; //Product Version : High byte
				uBuf[5] = uFlash_Read_Buf[FLASH_SAVE_SET_DEVICE_ID_5]; //Product Version : Low byte
#ifdef BT_DEBUG_MSG
				_DBG("\n\r ### SET_DEVICE ID setting is OK !!!");
#endif
			}
		}
		else
		{//It has NOT the history that Master is connected with TWS Slave
			uBuf[0] = 0xFF; //Vendor ID : High byte
			uBuf[1] = 0xFF; //Vendor ID : Low byte
			uBuf[2] = 0xFF; //Product ID : High byte
			uBuf[3] = 0xFF; //Product ID : Low byte
			uBuf[4] = 0xFF; //Product Version : High byte
			uBuf[5] = 0xFF; //Product Version : Low byte
		}
#else //TWS_MASTER_SLAVE_GROUPING
		uBuf[0] = 0x85; //Vendor ID : High byte
		uBuf[1] = 0x31; //Vendor ID : Low byte
		uBuf[2] = 0x50; //Product ID : High byte
		uBuf[3] = 0x57; //Product ID : Low byte
		uBuf[4] = 0x00; //Product Version : High byte
		uBuf[5] = 0x01; //Product Version : Low byte
#endif //TWS_MASTER_SLAVE_GROUPING
		MB3021_BT_Module_Send_cmd_param(CMD_SET_DEVICE_ID_32, uBuf);
#endif

		bPolling_Get_Data &= (~BCRF_SET_DEVICE_ID_CONTROL); //Clear flag		
	}

	if(bPolling_Get_Data & BCRF_SET_MODEL_NAME_CONTROL)
	{ 
#ifdef BT_NAME_EXTENSION
		int i, j;
		uint8_t uNaming[12];

#endif
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_MODEL_NAME_CONTROL");
#endif
		//To Do !!! - Need to check with customer about Name. Temperally the length is 6byte(USEN MUSIC LINK)
		uBuf[0] = 0x55; //MODEL NAME : U
		uBuf[1] = 0x53; //MODEL NAME : S
		uBuf[2] = 0x45; //MODEL NAME : E
		uBuf[3] = 0x4E; //MODEL NAME : N
		uBuf[4] = 0x20; //MODEL NAME :
		uBuf[5] = 0x4D; //MODEL NAME : M
		uBuf[6] = 0x55; //MODEL NAME : U
		uBuf[7] = 0x53; //MODEL NAME : S
		uBuf[8] = 0x49; //MODEL NAME : I
		uBuf[9] = 0x43; //MODEL NAME : C
		uBuf[10] = 0x20; //MODEL NAME : 
		uBuf[11] = 0x4C; //MODEL NAME : L
		uBuf[12] = 0x49; //MODEL NAME : I
		uBuf[13] = 0x4E; //MODEL NAME : N
		uBuf[14] = 0x4B; //MODEL NAME : K
#ifdef BT_NAME_EXTENSION
		for(i=0; i<6; i++)
		{
			for(j=0;j<2;j++) //Change to ASCII from Nibble Hexa Data
			{
				if(j==0)
					uNaming[i*2] = (uBT_Local_Address[i]&0xF0) >> 4;
				else			
					uNaming[i*2+1] = (uBT_Local_Address[i])&0x0F;

				if(uNaming[i*2+j] < 0xa)
					uNaming[i*2+j] += 0x30;
				else
					uNaming[i*2+j] += 0x37;					
			}
		}
#ifdef BT_DEBUG_MSG
		_DBG("\n\r");

		for(i=0;i<12;i++)
		{
			_DBH(uNaming[i]);
		}

		_DBG("\n\r");
#endif
		uBuf[15] = 0x5F; //'_'
		
#ifdef BT_DEBUG_MSG
		_DBG("\n\r");
#endif		
		for(i=0;i<6;i++) //Just add last 3BYTE(6 ASCII Code) address
		{
			uBuf[i+16] = uNaming[i+6];
#ifdef BT_DEBUG_MSG
			_DBH(uBuf[i+16]);
#endif
		}

#endif //BT_NAME_EXTENSION
		MB3021_BT_Module_Send_cmd_param(CMD_SET_MODEL_NAME_32, uBuf);

		bPolling_Get_Data &= (~BCRF_SET_MODEL_NAME_CONTROL); //Clear flag		
	}

	if(bPolling_Get_Data & BCRF_MODULE_POWER_CONTROL)
	{ 
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_MODULE_POWER_CONTROL");
#endif

		//To Do !!! - Need to check with customer about Name. Temperally the length is 4byte(USEN)
		uBuf[0] = 0x01; //Power On : 0x01, Power Off : 0x00
		
		MB3021_BT_Module_Send_cmd_param(CMD_MODULE_POWER_CONTROL_32, uBuf);

		bPolling_Get_Data &= (~BCRF_MODULE_POWER_CONTROL); //Clear flag		
		bPolling_Get_Data_backup |= BCRF_MODULE_POWER_CONTROL;
	}

#ifdef TWS_MODE_ENABLE
#ifdef TWS_MASTER_SLAVE_GROUPING
	if(bPolling_Get_Data & BCRF_TWS_SET_DEVICE_ID_SAVE) //2022-12-15 //TWS : Master execute BCRF_TWS_SET_DEVICE_ID_SAVE to send SET_DEVICE_ID to Slave and to save SET_DEVICE_ID to flash.
	{ 
#ifdef BT_DEBUG_MSG
		_DBG("\n\rDo : BCRF_TWS_SET_DEVICE_ID_SAVE");
#endif
#ifdef SW1_KEY_TWS_MODE
		if(Get_Cur_LR_Stereo_Mode() == Switch_LR_Mode && Get_Cur_Master_Slave_Mode() == Switch_Master_Mode)
		{
			Auto_addtime_for_master_slave_grouping(); //2023-02-20_2 : Timer Start
			TIMER20_TWS_Grouping_send_flag_start();
			MASTER_SLAVE_Grouping_Send_SET_DEVICE_ID(FALSE); //2023-02-20_2
			
			bPolling_Get_Data &= (~BCRF_TWS_SET_DEVICE_ID_SAVE); //Clear flag
		}
		else
			bPolling_Get_Data &= (~BCRF_TWS_SET_DEVICE_ID_SAVE); //Clear flag		
#endif
	}
#endif

	if(bPolling_Get_Data & BCRF_TWS_SET_DISCOVERABLE_MODE)
	{ 
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_TWS_SET_DISCOVERABLE_MODE");
#endif
		BTWS_LIAC = TWS_Status_Master_LIAC;
		uBuf[0] = 0x01; //Enable Limited Discoverable mode

		MB3021_BT_Module_Send_cmd_param(CMD_SET_DISCOVERABLE_MODE_32, uBuf);

		bPolling_Get_Data &= (~BCRF_TWS_SET_DISCOVERABLE_MODE); //Clear flag		
	}

	if(bPolling_Get_Data & BCRF_TWS_ROUTING_MODE_CONTROL)
	{ 
#ifdef BT_DEBUG_MSG //2022-11-17_1
		_DBG("\n\rDo : BCRF_TWS_ROUTING_MODE_CONTROL");
#endif
		uBuf[0] = 0x01; //Master Left, Slave Right

		MB3021_BT_Module_Send_cmd_param(CMD_ID_TWS_ROUTING_MODE_CONTROL_32, uBuf);

		bPolling_Get_Data &= (~BCRF_TWS_ROUTING_MODE_CONTROL); //Clear flag		
	}
#endif

	if(bPolling_Get_Data & BCRF_SET_DISCOVERABLE_MODE)
	{ 
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_DISCOVERABLE_MODE");
#endif
#ifdef INQUIRY_ACCESS_CODE_SUPPORT
#ifdef SWITCH_BUTTON_KEY_ENABLE
		if(BBT_Pairing_Key_In)
		{//LIAC : Limitted Inquiry Access Code
			uBuf[0] = 0x02; //0x0:Disable discoverable mode/0x1:Enable Limited Discoverable mode/0x2:Enable General Discoverable mode/0x3:Enable Dual Discoverable mode
			BBT_Pairing_Key_In = FALSE;
#ifdef BT_DEBUG_MSG	
			_DBG("\n\r@@@ Enable General Discoverable mode");
#endif
		}
		else
		{//GIAC : General Inquiry Access Code
#ifdef BT_DEBUG_MSG	
			_DBG("\n\r@@@ Enable Limited Discoverable mode");
#endif
#ifdef NO_BROADCAST_MODE
			uBuf[0] = 0x02;
#else
			uBuf[0] = 0x01;
#endif

		}
#else //SWITCH_BUTTON_KEY_ENABLE
		uBuf[0] = 0x02;
#endif //SWITCH_BUTTON_KEY_ENABLE
#else //INQUIRY_ACCESS_CODE_SUPPORT //0x2:Enable General Discoverable mode
#ifdef TWS_MODE_ENABLE
		if(BTWS_LIAC == TWS_Status_Master_LIAC) //TWS Ready
		{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE //2022-12-27
			if(BKeep_Connectable)
			{				
				BTWS_LIAC = TWS_Status_Master_GIAC;
				uBuf[0] = 0x02; //Enable General Discoverable mode
			}
			else
#endif
			uBuf[0] = 0x01; //Enable Limitted Access Code Mode
		}
		else
		{
#ifdef NEW_TWS_MASTER_SLAVE_LINK
			if(BTWS_LIAC != TWS_Status_Master_Mode_Control) //2023-04-26_9 : Enable Again //2023-03-30_1 : Disable 2023-03-15_6 solution //2023-03-15_6 : This case is only that TWS Master is connected with TWS Slave and PeerDevice is disconnected with TWS Master. We need to keep current TWS status.
#endif
			{
				BTWS_LIAC = TWS_Status_Master_GIAC;
			}
			
			uBuf[0] = 0x02; //Enable General Discoverable mode
		}
#else
		uBuf[0] = 0x02; //Enable General Discoverable mode
#endif
#endif //INQUIRY_ACCESS_CODE_SUPPORT
		MB3021_BT_Module_Send_cmd_param(CMD_SET_DISCOVERABLE_MODE_32, uBuf);

		bPolling_Get_Data &= (~BCRF_SET_DISCOVERABLE_MODE); //Clear flag		
		bPolling_Get_Data_backup |= BCRF_SET_DISCOVERABLE_MODE;
	}

	if(bPolling_Get_Data & BCRF_SET_CONNECTABLE_MODE)
	{ 
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_CONNECTABLE_MODE : enable");
#endif
		uBuf[0] = 0x01; //0x0:Disable/0x01:Enable
		
		MB3021_BT_Module_Send_cmd_param(CMD_SET_CONNECTABLE_MODE_32, uBuf);

		bPolling_Get_Data &= (~BCRF_SET_CONNECTABLE_MODE); //Clear flag
	}
	
#if 0//2023-01-05_6 : Changed SW which send 64 step volume to BAP //#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP) //2023-01-05_6 : To send 64 Step volume level under BAP. Don't move(because reciever can't get BLE DATA is too fast) !!!
	if(bPolling_Get_Data & BCRF_SEND_BLE_EXTRA_DATA)
	{
//		int i;

		uBuf[0] = BLE_DATA_0;
		uBuf[1] = BLE_DATA_1;
		//To Do !!! Need to check with customer
		uBuf[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
		uBuf[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 

		uBuf[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
		uBuf[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;

		//Making Actual BLE Extra Data : Param1 (64 Step Volume)
		uBuf[6] = 0xCC; //Start Code
		uBuf[7] = TAS5806MD_Amp_Get_Cur_Volume_Level(); //Parameter1
		uBuf[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 2); //Check sum

		BMaster_Send_BLE_Remote_Data = TRUE;
		
		MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0200, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum

		bPolling_Get_Data &= (~BCRF_SEND_BLE_EXTRA_DATA); //Clear flag
	}
#endif

}

#ifdef FIVE_USER_EQ_ENABLE
void MB3021_BT_Module_EQ(EQ_Mode_Setting EQ_mode)
{
	uint8_t uBuf;

	uBuf = (uint8_t)EQ_mode;
	
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_EQ = ");
	_DBH(uBuf);
#endif	

	MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uBuf);
#if defined(FLASH_SELF_WRITE_ERASE_EXTENSION) && !defined(FLASH_SELF_WRITE_ERASE_EXCEPTING_EQ)
	FlashSaveData(FLASH_SAVE_DATA_EQ, uBuf);
#endif
#if defined(INPUT_KEY_SYNC_WITH_SLAVE_ENABLE) && defined(EQ_TOGGLE_ENABLE)
	MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_EQ, uBuf);
#endif
#ifdef EQ_TOGGLE_ENABLE //2023-01-17 : EQ Mode Display
	LED_DIsplay_EQ_Mode(EQ_mode);
#endif
}
#endif

#ifdef SIG_TEST
void MB3021_BT_Module_SIG_Test(A2DP_PTS_Event Request)
{
	uint8_t uBuf;
	static Bool bFirst = FALSE;

	if(!bFirst)
	{
		MB3021_BT_Module_Send_cmd(CMD_A2DP_ENTER_PTS_TEST_MODE);
		bFirst = TRUE;
	}

#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_SIG_Test = ");
	_DBH(Request);
#endif

	if(Request == A2DP_Close_Request)
		uBuf = 0x00;
	else
		uBuf = 0x01;
	
	MB3021_BT_Module_Send_cmd_param(CMD_A2DP_PTS_EVENT, &uBuf);
}
#endif

uint8_t MB3021_BT_Module_Send_cmd(uint32_t code32) //Just in case of Request Length = 0
{

	uint8_t buf[32] = {0,};
	uint16_t code = 0;
	uint8_t request_len = 0, response_len = 0, last_len = 0;

	code = (code32 >> 16) & 0xffff;
 	
	buf[0] = SYNC_BYTE; //Header(1Byte) : 0x42 //'B'
	buf[1] = PACKET_CMD; // Type(1Byte) : PACKET_CMD = 0x43, PACKET_IND = 0x49, PACKET_RESP = 0x52, PACKET_DATA = 0x44
	buf[2] = (code & 0xff00) >> 8; //Major ID(1Byte)
	buf[3] = (uint8_t)(code & 0x00ff); //Minor ID(1Byte)
	buf[4] = request_len; //Param Size(1Byte)

	last_len = 5;

#if 0//def BT_DEBUG_MSG
	_DBG("\nLen : ");_DBH(last_len);_DBG("\n\r");
#endif

	buf[last_len] = MB3021_BT_Module_Calculate_Checksum(buf, last_len);

	Serial_Send(SERIAL_PORT10, buf, last_len+1);

	return response_len;
}

uint8_t MB3021_BT_Module_Send_cmd_param(uint32_t code32, uint8_t *param)
{
	uint8_t buf[32] = {0,};
	uint16_t code = 0;
	uint8_t request_len = 0, response_len = 0, i = 0, last_len = 0;

	response_len = (code32) & 0xff;
	request_len = (code32 >> 8) & 0xff;

#if 0//def BT_DEBUG_MSG
	_DBG("\n\rResp Len : ");_DBH(response_len);
	_DBG("\n\rReq Len : ");_DBH(request_len);
#endif
	
	code = (code32 >> 16) & 0xffff;
 	
	buf[0] = SYNC_BYTE; //Header(1Byte) : 0x42 //'B'
	buf[1] = PACKET_CMD; // Type(1Byte) : PACKET_CMD = 0x43, PACKET_IND = 0x49, PACKET_RESP = 0x52, PACKET_DATA = 0x44
	buf[2] = (code & 0xff00) >> 8; //Major ID(1Byte)
	buf[3] = (uint8_t)(code & 0x00ff); //Minor ID(1Byte)
	buf[4] = request_len; //Param Size(1Byte)

	if(buf[4] != 0)
	{
		for(i = 0; i<request_len; i++)
		{
			buf[i+5] = param[i]; //to do list : parameter //To Do !!!
			//_DBH(buf[i+5]);
		}
		
		last_len = i+5;
	}
#ifdef BT_DEBUG_MSG
	else
	{
		_DBG("\n\rUART Send Error : ID = ");
		_DBH16(code);
		_DBG("\n\r");
	}
#endif

#if 0//def BT_DEBUG_MSG
	_DBG("\nLen : ");_DBH(last_len);_DBG("\n\r");
#endif

	buf[last_len] = MB3021_BT_Module_Calculate_Checksum(buf, last_len);
	Serial_Send(SERIAL_PORT10, buf, last_len+1);

	return response_len;
}

void MB3021_BT_Module_Send_Data_Packcet(uint8_t *param, uint16_t size) //SPP COM : Send SPP OK Response to Peer Device - without checksum
{
	int i = 0;
	uint8_t buf[32] = {0,};

#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_Send_Data_Packcet() ");
#endif

	for(i = 0; i<size; i++)
	{
		buf[i] = param[i];
#if 0//def BT_DEBUG_MSG
		_DBH(buf[i]);
#endif

	}

	//The last value is replaced with checksum in here 
	buf[size] = MB3021_BT_Module_Calculate_Checksum(buf, size);

#if 0//def BT_DEBUG_MSG 
	_DBG("\n\rChecksum :  ");
	_DBH(buf[size]);
	_DBG("\n\rFinal Data :  ");

	for(i = 0; i<size+1; i++)
	{
		_DBH(buf[i]);
	}
#endif

	Serial_Send(SERIAL_PORT10, buf, size+1);
}

#endif //UART_10_ENABLE

#endif //MB3021_ENABLE

