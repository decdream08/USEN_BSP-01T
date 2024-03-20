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

#ifdef MB3021_ENABLE
#include "led_display.h"
#include "timer21.h"
#ifdef UART_10_ENABLE
#include "serial.h"
#include "bt_MB3021.h"

#include "remocon_action.h"
#include "key.h"
#include "timer20.h"

#include "AD85050.h" 
#include "flash.h"
#include "power.h"

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

//MAJOR_ID_AVRCP_CONTROL(0x11)/MINOR ID & CMD
#define MINOR_ID_AVRCP_CONNECTION_CONTROL		0x00
#define CMD_AVRCP_CONNECTION_CONTROL_32			(0x0701UL|((MINOR_ID_AVRCP_CONNECTION_CONTROL|(MAJOR_ID_AVRCP_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MINOR_ID_SEND_SYNC_DATA					0x13
#define CMD_SEND_SYNC_DATA_32					(0x0F01UL|((MINOR_ID_SEND_SYNC_DATA|(MAJOR_ID_AVRCP_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

//MAJOR_ID_TWS_CONTROL(0x15)/MINOR ID & CMD
#define MINOR_ID_TWS_MODE_CONTROL				0x00
#define CMD_ID_TWS_MODE_CONTROL_32				(0x0101UL|((MINOR_ID_TWS_MODE_CONTROL|(MAJOR_ID_TWS_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_TWS_ROUTING_MODE_CONTROL		0x01
#define CMD_ID_TWS_ROUTING_MODE_CONTROL_32		(0x0101UL|((MINOR_ID_TWS_ROUTING_MODE_CONTROL|(MAJOR_ID_TWS_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

//MAJOR_ID_BA_CONTROL(0x16)/MINOR ID & CMD
#define MINOR_ID_BA_MODE_CONTROL			0x00
#define CMD_BA_MODE_CONTROL_32				(0x0101UL|((MINOR_ID_BA_MODE_CONTROL|(MAJOR_ID_BA_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

//MAJOR_ID_BLE_CONTROL(0x17)/MINOR ID & CMD
#define MINOR_ID_ADVERTISING_CONTROL			0x02
#define CMD_ADVERTISING_CONTROL_32				(0x0101UL|((MINOR_ID_ADVERTISING_CONTROL|(MAJOR_ID_BLE_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))
#define MINOR_ID_SET_BLE_MANUFACTURE_DATA		0x03
#define CMD_SET_BLE_MANUFACTURE_DATA_32			(0x0701UL|((MINOR_ID_SET_BLE_MANUFACTURE_DATA|(MAJOR_ID_BLE_CONTROL << 8)) << PACKET_CMD_SHIFT_BIT))

#define MAJOR_ID_MCU_VERSION					0x20
#define CMD_SEND_MCU_VERSION_INFO_32			(0x0601UL|((MAJOR_ID_MCU_VERSION << 8) << PACKET_CMD_SHIFT_BIT))

//INDIGATOR
//MINOR ID - MAJOR_ID_GENERAL_CONTROL
#define MINOR_ID_MODULE_STATE_CHANGED_IND		0x00 //Data[0] : 0x0 - initialising / 0x1 - Ready
#define MINOR_ID_REMOTE_DEVICE_NAME_IND			0x0C //Data[0] : Result code / Data[1-6] : Bluetooth Device Address / Data[7~N] : BT Device Name
#define MINOR_ID_FIRMWARE_VERSION_IND			0x0E //7Byte(Data[0~1]:Year/Data[2~3]:Month/Data[4~5]:Day/Data[6]:Index)
#define MINOR_ID_LOCAL_ADDRESS_NAME_IND			0x0F //Data[0-5]:Bluetooth Device Address / Data[6-N]:Bluetooth Device Name

#define MINOR_ID_ROUTING_STATE_IND				0x22 //Data[0] - 0x0:Disconnect, 0x1:Connect/Data[1] - 0x0:Disconnect Audio Routing, 0x2:Analog, 0x6:A2DP_0
#define MINOR_ID_ACL_OPENED_IND					0x23 //Data[0-5]:Bluetooth Device Address / / Data[6]: status: 0x0 -OK, 0x4 - No device, 0xb - Peerdevice busy /Data[7],Data[8]
#define MINOR_ID_ACL_CLOSED_IND					0x24 //Data[0-5]:Bluetooth Device Address / Data[6]: Disconnect : 0x13/BT Delete : 0x16/PeerDevice Off : 0x08 

//MINOR ID - MAJOR_ID_A2DP_CONTROL - 0x10
#define MINOR_ID_A2DP_PROFIL_STATE_CHANGED_IND		0x00 //Data[0-5] : BDA/Data[6]:Status/Data[7]:Connection Direction/Data[8]:Address Type
#define MINOR_ID_A2DP_MEDIA_STATE_CHANGED_IND		0x01 //Data[0-5] : BT Address/Data[6] : 0x01 - Closed, 0x02  - Opened, 0x03 - Streaming, 0x04 - Suspended
#define MINOR_ID_A2DP_STREAM_ROUTING_CHANGED_IND		0x02 //Data[0] : Routing state(0x0:UnRouted,0x1:Routed)/Data[1~6]:Routed Device Address
#define MINOR_ID_A2DP_SELECTED_CODEC_IND			0x05 //Data[0-5] : BT Address/Data[6-7]: Selected Codec Flags
#define MINOR_ID_A2DP_CONNECT_FAIL_ERRORCODE_IND	0x70 //Data[0-5] : BDA/Data[6]:Status

//MINOR ID -MAJOR_ID_AVRCP_CONTROL - 0x11
#ifdef TWS_MASTER_SLAVE_COM_ENABLE
#define MINOR_ID_AVRCP_PROFILE_STATE_CHANGED_IND		0x00 //Data[0-5] : BT Address / Data[6] : Profile State
#endif
#define MINOR_ID_AVRCP_PLAYBACK_STATUS_CHANGED_IND	0x01 //Data[0-5] : BT Address / Data[6] : Profile State
#define MINOR_ID_AVRCP_VOLUME_CHANGED_IND		0x02 //Data[0-5] : BT Address / Data[6] : Volume Value(0-127)
#define MINOR_ID_AVRCP_CAPABILITY_LIST_IND			0x03 //Data[0-5] : BT Address/Data[6-7]: Capability List //Need to check !!! 0x11 : 0x03
#define MINOR_ID_AVRCP_SEND_SYNC_DATA_IND						0x13

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
#define BCRF_SEND_SPP_DATA_RESP					0x20000
#define BCRF_SET_AVRCP_CONNECTION_CONTROL		0x40000
#define BCRF_SET_IO_CAPABILITY_MODE				0x80000
#define BCRF_CLEAR_CONNECTABLE_MODE				0x100000

#define BCRF_TWS_MODE_CONTROL					0x200000
#define BCRF_TWS_MODE_CONTROL_RETRY				0x400000
#define BCRF_TWS_SET_DISCOVERABLE_MODE			0x800000
#define BCRF_TWS_ROUTING_MODE_CONTROL			0x1000000
#define BCRF_TWS_SET_DEVICE_ID_SAVE				0x2000000

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

#define SPP_ERROR_CHECKSUM_ERROR				0xF1
#define SPP_ERROR_INVALID_DATA					0xF2
#define SPP_ERROR_POWER_OFF_STATE				0xF3
#define SPP_ERROR_LENGTH_ERROR					0xF4
#define SPP_ERROR_START_CODE_NG					0xF5
#define SPP_ERROR_INVALID_DATA_UNDER_CUR_STATE				0xF6 //2022-09-22
#define SPP_ERROR_BT_SPK_BUSY					0xF7 //To make invalid key action excepting under Master/Slave Grouping mode. To avoid minor issues(LED Display)

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

#define BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE			0x10
#define BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE			0x10
#define BLE_DATA_GROUPING_PRODUCT0_ID_HIGH_BYTE			0x20
#define BLE_DATA_GROUPING_PRODUCT0_ID_LOW_BYTE			0x20

#define MCU_FW_VERSION_INDICATOR				0xAF
#define BT_FW_VERSION_INDICATOR					0xBF

typedef enum {
	BLE_POWER_KEY,
	BLE_MUTE_KEY,
	BLE_VOLUME_KEY,
	BLE_EQ_KEY,
	BLE_REBOOT_KEY,
	BLE_FACTORY_RESET_KEY,
	BLE_MUTE_OFF_DELAY, //0x00 : Invalid, 0x01 : Mute Off delay, 0x02 : Mute On
	BLE_BT_SHORT_KEY = (BLE_FACTORY_RESET_KEY+1), //Master Mode Only - 0x00 : BT short key off, 0x01 : BT short key on
}BLE_Remocon_Key;

typedef enum {
	BLE_EXT_VOLUME_DATA,
	//BLE_EXT_ATTENUATOR_DATA,
	BLE_EXT_DATA_END
}BLE_Extra_Data;

typedef enum {
	REMOTE_POWER_NONE_ACTION,
	REMOTE_POWER_ON_ACTION,
	REMOTE_POWER_OFF_ACTION
}Remote_Power_Key_Action;

//Variable
char MCU_Version[6] = "230727"; //MCU Version Info
char BT_Version[7]; //MCU Version Info

Bool BBT_Init_OK = FALSE;
Bool BSPP_Ready_OK = FALSE;
Bool BBT_Is_Connected = FALSE; //To check whether A2DP(Peer Device) is connected or not
Bool BMaster_Send_BLE_Remote_Data = FALSE; //To avoid init sequence action when BLE data is sent
Bool B_SSP_REBOOT_KEY_In = FALSE; //To send Reboot BLE DATA and execute Reboot
Bool B_SSP_FACTORY_RESET_KEY_In = FALSE; //To send FACTORY RESET KEY and execute it.
Bool BBT_Is_Routed = FALSE; //To check whether BT(excepting Aux) has music steam or not but TWS Mode has some different/wrong value.

Bool BDoNotSend_Connectable_Mode = FALSE;

Bool BKeep_Connectable = FALSE; //To avoid BA_MODE_CONTROL execution by sequnece. When this flag is set, it's disconnected with peerdevice.

Bool B_BLE_Extra_Data = FALSE; //2023-01-09_1 : Just check wether Slave has been recevied 64 step volume or not. If it has been received 64 step volume, it means slave is BAP-01 and save it to flash.
Bool B_Master_Is_BAP = FALSE; //2023-01-09_2 : To disable BLE_VOLUME_KEY using BLE DATA from Master under BAP slave when Master & Slave are BAP

uint8_t uPaired_Device_Count = 0; //First time, we use this to chech if last conection is available and second time, we use this to check if current connection is last connection.
uint8_t uAuto_receive_buf32[32] = {0,};
uint8_t uSPP_receive_buf8[9] = {0xff,};

uint8_t uCurrent_Status_buf8[15] = {0xff,};

uint8_t uInput_Key_Sync_buf8[9] = {0xff,}; //For Master
uint8_t uBLE_Remocon_Data[9] = {0xff,}; //For Slave //Added mute off delay for SLAVE SPK

uint8_t uBLE_Extra_Data[BLE_EXT_DATA_END+1] = {0xff,}; //For Slave //To get/send 64 Step volume level under BAP //2023-01-05_6

char uBT_Local_Address[6] = {0,}; //BT Local_Addres(USEN Address) : 6Byte
char uBT_Cur_A2DP_Device_Address[6] = {0,}; //Current Connected Device Address from A2DP
char uCur_SPP_Device_Address[6] = {0,}; //Current Connected SPP protocol Device Address
char uLast_Connected_Device_Address[6] = {0,}; //Last connection Device Address from MINOR_ID_GET_PAIRED_DEVICE_LIST

char strUSEN_Device_Name[] = "Lenovo Tab M8";
char strUSEN_Device_Name_1[] = "USEN MUSIC DEVICE";

char uBT_Remote_Device_Name[32] = {0,}; //BT Remote Device Name : Before connection //Can check if we already get Device name
char uBT_Remote_Device_Address[6] = {0,}; //BT Remote Device Address : Before connection

uint8_t uBT_Remote_Device_Name_Size = 0; //BT Remote Device Name Size : Before connection

Bool bDevice_Paring_Accept = FALSE; //For checking, if Device Pairing Accept is OK : Before connection

uint8_t uMode_Change = 0; //To avoid, BT has no output sometime when user alternates Aux mode. This variable just saved previous mode(Aux/BT)

uint8_t uFlash_Read_Buf[FLASH_SAVE_DATA_END];

uint8_t uLast_Connection_Retry_Count; //For last connection retry - 2 times

//uint8_t uBT_Local_Name[6] = {0,}; //BT  Name : NByte. Need too much space.
static uint32_t bPolling_Get_Data = 0;
static uint32_t bPolling_Get_Data_backup = 0; //Sometimes, Slave can't link with Master because Slave's MCU send BA_SWITCH_MODE (Receiver) to BT Moduel but BT Module can't recognize it. (USEN#43)
static uint16_t bPolling_Get_BT_Profile_State = 0;
static uint16_t bPolling_Set_Action = 0; //To avoid, INT message missing because of MB3021_BT_Module_Input_Key_Sync_With_Slave()

static uint8_t uSPP_RECEIVE_DATA_ERROR = 0;
static uint8_t uEQ_Mode = 0;

static uint8_t uNext_Grouping_State = 0;
static uint8_t uPrev_Grouping_State = 0;

Bool B_Delete_PDL_by_Factory_Reset = FALSE;

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

uint32_t mb3021_timer;
BT_Status mb3021_status;

void MB3021_10ms_timer(void)
{
	if (mb3021_timer != df10msTimer0ms)
		--mb3021_timer;
}

void MB3021_Process(void)
{
	switch (mb3021_status)
	{
		case MB3021_POWER_UP:
			MB3021_BT_Module_Init(FALSE);
			MB3021_BT_Module_Input_Key_Init();

			mb3021_status = MB3021_RESET_ON;
			break;

		case MB3021_RESET_ON:
			/* PC0 Output - MODULE_RESET */
			HAL_GPIO_ClearPin(PC, _BIT(0));

			mb3021_status = MB3021_RESET_OFF;
			mb3021_timer = df10msTimer500ms;;
			break;

		case MB3021_RESET_OFF:
			if(mb3021_timer == df10msTimer0ms)
			{
				HAL_GPIO_SetPin(PC, _BIT(0));
				mb3021_status = MB3021_POWER_UP_COMPLETE;
			}
			break;

		case MB3021_POWER_UP_COMPLETE:
			mb3021_status = MB3021_RUN;
			break;

		case MB3021_POWER_CONTROL_CMD_PROCESS:
			if(mb3021_timer == df10msTimer0ms)
			{
				bPolling_Get_Data |= BCRF_MODULE_POWER_CONTROL;
				mb3021_status = MB3021_RUN;
			}
			break;

		case MB3021_MODULE_INIT_CMD_PROCESS:
			if(mb3021_timer == df10msTimer0ms)
			{
				MB3021_BT_Module_Init(FALSE);
				Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
				mb3021_status = MB3021_RUN;
			}
			break;

		case MB3021_SW_RESETTING_CMD_PROCESS:
			if(mb3021_timer == df10msTimer0ms)
			{
				MB3021_BT_Disconnect_All_ACL(); //To avoid last connection fail after reboot //Reboot recovery solution - 1

				mb3021_status = MB3021_SW_RESET_CMD_PROCESS;
				mb3021_timer = df10msTimer1s;
			}
			break;
			
		case MB3021_SW_RESET_CMD_PROCESS:
			if(mb3021_timer == df10msTimer0ms)
			{
				SW_Reset();
				mb3021_status = MB3021_RUN;
			}
			break;

		case MB3021_RUN:
			Do_taskUART();
			break;

		case MB3021_POWER_DOWN:
			break;	

		default:
			break;
	}
}

void MB3021_PowerUp(void)
{
	mb3021_status = MB3021_POWER_UP;
}

Bool Is_Delete_PDL_by_Factory_Reset(void)
{
	return B_Delete_PDL_by_Factory_Reset;
}

uint8_t Convert_50Step_to_16Step(uint8_t uVol) //2023-02-28_1 : Changed volume table //2023-02-27_3 : To set from 64 Step volume level to 16 step under BAP
{
	uint8_t uMatching_Vol = 0;
	
#ifdef BT_DEBUG_MSG
	_DBG("\n\rConvert_50Step_to_16Step()");
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
	else if(uVol >= 2) //2023-06-20_1 : Add volume table for 0 level
		uMatching_Vol = 1;
	else 
		uMatching_Vol = 0;

#ifdef BT_DEBUG_MSG
	_DBG("\n\ruMatching_Vol = ");_DBD(uMatching_Vol);
#endif

	return uMatching_Vol;
}

uint8_t Convert_16Step_to_50Step(uint8_t uVol) //2023-02-28_1 : Changed volume table //2023-02-27_3 : To set from 16 Step volume level to 50 step under BAP
{
	uint8_t uMatching_Vol = 0;
	
#ifdef BT_DEBUG_MSG
	_DBG("\n\rConvert_16Step_to_50Step()");
	_DBG("\n\rConvert_In_Vol = ");_DBD(uVol);
#endif

	//2023-06-20_1 : +1 for each value
	if(uVol >= 15)
		uMatching_Vol = 50; //49+i;
	else if(uVol >= 14)
		uMatching_Vol = 49; //48+i;
	else if(uVol >= 13)
		uMatching_Vol = 48; //47+i;
	else if(uVol >= 12)
		uMatching_Vol = 45; //44+i;
	else if(uVol >= 11)
		uMatching_Vol = 42; //41+i;
	else if(uVol >= 10)
		uMatching_Vol = 39; //38+i;
	else if(uVol >= 9)
		uMatching_Vol = 36; //35+i;
	else if(uVol >= 8)
		uMatching_Vol = 32; //31+i;
	else if(uVol >= 7)
		uMatching_Vol = 28; //27+i;
	else if(uVol >= 6)
		uMatching_Vol = 24; //23+i;
	else if(uVol >= 5)
		uMatching_Vol = 21; //20+i;
	else if(uVol >= 4)
		uMatching_Vol = 18; //17+i;
	else if(uVol >= 3)
		uMatching_Vol = 15; //14+i;
	else if(uVol >= 2)
		uMatching_Vol = 7; //6+i;
	else if(uVol >= 1) //2023-06-20_1 : Add volume table for 0 level
		uMatching_Vol = 3; //2+i;
	else
		uMatching_Vol = 1;

#ifdef BT_DEBUG_MSG
	_DBG("\n\rMatching_Vol = ");_DBD(uMatching_Vol);
#endif

	return uMatching_Vol;
}

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

void MB3021_BT_Module_Forced_Input_Audio_Path_Setting(void)
{
	bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; 
}

void Init_uBLE_Remocon_Data(void) //Under Slave and Power off mode, Slave work all CMD thru BLE
{
	int i;

	for(i=0;i<9;i++)
	uBLE_Remocon_Data[i] = 0xff;
}

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

void MB3021_BT_Master_Slave_Grouping_CMD_Set(uint8_t cmd)
{
	uNext_Grouping_State = cmd;
}

void MB3021_BT_Master_Slave_Grouping_Start(void)
{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
	_DBG("\n\r2. MB3021_BT_Master_Slave_Grouping_Start()");
#endif	
	TIMER20_Master_Slave_Grouping_flag_Start();
	uNext_Grouping_State = GROUPING_MASTER_NORMAL_MODE;
}

void MB3021_BT_Master_Slave_Grouping_Stop(void)
{
	Bool Mute_On;

#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG
		_DBG("\n\r99. MB3021_BT_Master_Slave_Grouping_Stop()");
#endif

	uNext_Grouping_State = GROUPING_MASTER_NORMAL_MODE;

	Mute_On = IS_Display_Mute();

	if(Power_State()) //When SPK is Power Off, we don't need to recovery into previous mode
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
	else //When SPK is Power Off, we don't need to recovery into previous mode
	{
		TIMER21_Periodic_Mode_Run(FALSE); //Blinking Timer Off
	}
}

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
		uBT_Remote_Device_Name[i] =0;

		if(i < 9)
		{
			uBLE_Remocon_Data[i] = 0xff;

			if(i == (BLE_EQ_KEY+1)) //2023-02-03_2 : To Delete Mute On/Off action when USEN Tablet send first data
				uSPP_receive_buf8[i] = 0x00;
			else
				uSPP_receive_buf8[i] = 0xff;
		}

		if(i< 8)
		{
			if(i < 6)
			{
				uBT_Local_Address[i] = 0; //BT Local_Addres : 6Byte
				uBT_Cur_A2DP_Device_Address[i] = 0; //Current Connected Device Address
				uLast_Connected_Device_Address[i] = 0;
				uCur_SPP_Device_Address[i] = 0;
			}
		}
	}

	BBT_Init_OK = FALSE;
	BSPP_Ready_OK = FALSE;
	BMaster_Send_BLE_Remote_Data = FALSE;
	BBT_Is_Connected = FALSE;
	BKeep_Connectable = FALSE;

	uNext_Grouping_State = 0;
	uPrev_Grouping_State = 0;

	bPolling_Get_Data = 0;
	bPolling_Get_Data_backup = 0;

	bPolling_Get_BT_Profile_State = 0;
	uPaired_Device_Count = 0;

	bPolling_Set_Action =0;

	uBT_Remote_Device_Name_Size = 0;

	uLast_Connection_Retry_Count = 2;
	B_Delete_PDL_by_Factory_Reset = FALSE;

	B_Master_Is_BAP = FALSE; //2023-02-28_2 : Need to init to check whether BAP-01 Slave is connected with BAP-01 Master

#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
	B_Auto_FactoryRST_On = FALSE; //2023-04-03_1
#endif
	BDoNotSend_Connectable_Mode = FALSE;
}

void MB3021_BT_Module_HW_Reset(void)
{
#if 1
	mb3021_status = MB3021_RESET_ON;
#else
	/* PC0 Output - MODULE_RESET */
	HAL_GPIO_ClearPin(PC, _BIT(0));
	delay_ms(500);
	HAL_GPIO_SetPin(PC, _BIT(0));
	//delay_ms(500);
#endif
}

void MB3021_BT_Module_Input_Key_Init(void) //To Do!!! - Need to change later using Flash data
{
#ifdef BT_DEBUG_MSG
	int i;

	_DBG("\n\rMB3021_BT_Module_Input_Key_Init()");
#endif

	Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);					

	uInput_Key_Sync_buf8[0] = 0xAA; //Start Code
	uInput_Key_Sync_buf8[1] = uFlash_Read_Buf[FLASH_SAVE_DATA_POWER]; //Power On/Off Button

	if(uInput_Key_Sync_buf8[1] == 0xff)
		uInput_Key_Sync_buf8[1] = 1; //Power On
		
	uInput_Key_Sync_buf8[2] = uFlash_Read_Buf[FLASH_SAVE_DATA_MUTE]; //Mute On/Off Button

	if(uInput_Key_Sync_buf8[2] == 0xff)
		uInput_Key_Sync_buf8[2] = 0; //Mute Off

	uInput_Key_Sync_buf8[3] = (uint8_t)(AD85050_Amp_Get_Cur_Volume_Level() & 0x0000ff); //Volume Level

	uInput_Key_Sync_buf8[4] = 0x00; //EQ

	uInput_Key_Sync_buf8[5] = 0x00; //Reboot On/Off Button
	uInput_Key_Sync_buf8[6] = 0x00; //Factory Reset On/Off Button

	uInput_Key_Sync_buf8[7] = 0x00; //Slave mute off delay - Off
	uInput_Key_Sync_buf8[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uInput_Key_Sync_buf8, 8); //Check sum

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

void Send_Cur_Master_Info_To_Tablet(void)
{
	uint8_t uVol_Level = 0;

	uCurrent_Status_buf8[0] = 0xBB; 
	
	if(Power_State() == TRUE)
		uCurrent_Status_buf8[1] = 0x01; //Power On/Off : Power On mode
	else
		uCurrent_Status_buf8[1] = 0x00; //Power On/Off : Power Off mode

	if(IS_Display_Mute() == TRUE) //When Mute On status, we don't need to mute off. This function is for LED Display
		uCurrent_Status_buf8[2] = 0x01; //Mute On/Off : Mute On mode
	else
	{
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
			uCurrent_Status_buf8[2] = 0x00; //Mute On/Off : Mute Off mode
	}

	uVol_Level = AD85050_Amp_Get_Cur_BT_Volume_Level_Inverse(); //Volume Level
	uCurrent_Status_buf8[3] = Convert_50Step_to_16Step(uVol_Level);

	if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
	{
		uEQ_Mode = EQ_NORMAL_MODE;
	}

	uCurrent_Status_buf8[4] = uEQ_Mode; //Sound EQ mode
	uCurrent_Status_buf8[5] = 0x00; //Reboot Off
	uCurrent_Status_buf8[6] = 0x00; //Factory Reset Off

	uCurrent_Status_buf8[7] = 0x42;
	uCurrent_Status_buf8[8] = 0x41;
	uCurrent_Status_buf8[9] = 0x50;
	uCurrent_Status_buf8[10] = 0x2D;
	uCurrent_Status_buf8[11] = 0x30;
	uCurrent_Status_buf8[12] = 0x32;
	uCurrent_Status_buf8[13] = 0x20;
	uCurrent_Status_buf8[14] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 14);

	bPolling_Get_Data |= BCRF_SEND_SPP_DATA_RESP; //Send SPP Response NG
}

void MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_Key_Sync_With_Slave Input_Key, uint8_t uValue) //Only Available under Master Mode
{
	static uint8_t uBuf[32] = {0,};
	int i;

#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_Input_Key_Sync_With_Slave() - Start");
#endif

	if(uNext_Grouping_State > GROUPING_EVENT_WAIT_STATE && uNext_Grouping_State != GROUPING_MASTER_SET_MANUFACTURE_DATA && Input_Key == Input_key_Sync_Slave_Mute_Off) //To avoid Slave pop-up upon Maste/Slave pairing
		return;

	if(Input_Key == input_key_Sync_Volume)
		uInput_Key_Sync_buf8[Input_Key+1] = Convert_50Step_to_16Step(uValue);
	else
		uInput_Key_Sync_buf8[Input_Key+1] = uValue; //+1 for Start Code(0xAA)

	if(!IS_BBT_Init_OK())
		return;

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

	//When Mute Off, we need to set Input_key_Sync_Slave_Mute_Off value into 0x01 or 0x00.
	//For example, unless this if statement, When user press BT Key(for grouping) on just master Slave always mute state even though use press mute off.
	if(Input_Key == input_key_Sync_Mute && uValue == 0x00)
	{
		uInput_Key_Sync_buf8[Input_key_Sync_Slave_Mute_Off+1] = 0x01;
	}
	
	uInput_Key_Sync_buf8[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uInput_Key_Sync_buf8, 8); //Check sum
	
	//Configure Send Data
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

#ifdef BT_DEBUG_MSG
		_DBG("\n\r");
#endif

	for(i=0; i<9; i++)
	{
		uBuf[i+6] = uInput_Key_Sync_buf8[i];

		if(i<7)
			uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i];

#ifdef BT_DEBUG_MSG
		_DBH(uInput_Key_Sync_buf8[i]);
#endif
	}

	BMaster_Send_BLE_Remote_Data = TRUE;

	MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum

	if(Input_Key != Input_key_Sync_Slave_Mute_Off)
  	Send_Cur_Master_Info_To_Tablet();
}

void MB3021_BT_Disconnect_All_ACL(void)
{
	uint8_t uBuf[7] = {0,};
	int i;
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Disconnect_All_ACL()");
#endif
	uBuf[0] = 0xff; //Data[0] - 0x0 : Disconnect ACL, 0x1 : Connect ACL, 0xff : Disconnect All ACL
		
	for(i=0;i<6;i++)
		uBuf[i+1] = uLast_Connected_Device_Address[i]; //Data[0~5] - Peer Device Address			
		
		//uBuf[6] = 0xff; //Data[0] - 0x0 : Disconnect ACL, 0x1 : Connect ACL, 0xff : Disconnect All ACL
#if 0//def BT_DEBUG_MSG	
		_DBH(uBuf[0] );
#endif

	MB3021_BT_Module_Send_cmd_param(CMD_CONTROL_ACL_CONNECTION_32, uBuf);		
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

void MB3021_BT_Delete_Paired_List_All(Bool Factory_Reset)
{
	uint8_t uBuf;
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Delete_Paired_List_All()");
#endif
	uBuf = 0xFF; //Delete All Device In Paired Device List

	B_Delete_PDL_by_Factory_Reset = Factory_Reset;

	//To recovery Factory Reset CMD
	TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_delete_paired_list);
	
	MB3021_BT_Module_Send_cmd_param(CMD_DELETE_PAIRED_DEVICE_LIST_32, &uBuf);		
}

void MB3021_BT_Read_FW_Version(void)
{
	MB3021_BT_Module_Send_cmd(CMD_READ_FW_VERSION_32);	
}

void MB3021_BT_A2DP_Connection_Control(Bool Connect) //TRUE : connect / FALSE : disconnect
{
	uint8_t uBuf[7] = {0,};
	int i;

#ifdef BT_DEBUG_MSG	
	_DBG("\n\rMB3021_BT_A2DP_Connection_Control() : connect =");
	_DBH(Connect);
#endif

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

void MB3021_BT_A2DP_Routing_Control(Bool Route) //TRUE : Route / FALSE : Unroute
{
	uint8_t uBuf[7] = {0,};
	int i;

#ifdef BT_DEBUG_MSG	
	_DBG("\n\rMB3021_BT_A2DP_Routing_Control() : Route =");
	_DBH(Route);
#endif
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

void MB3021_BT_Module_Init(Bool Factory_Reset) //No need BT module Init. Just check ID
{
	int i;
	
	TIMER20_factory_reset_cmd_recovery_flag_stop(); //To Clear Factory Reset recovery for factory_reset_delete_paired_list  becasuse SPK get response here.
	TIMER20_user_eq_mute_flag_stop(); //To avoid noise when user tries eq mode change - 300ms mute
	TIMER20_Forced_Input_Audio_Path_Setting_flag_stop();

	MB3021_BT_Module_Value_Init(); //Variable Init	

	if(Factory_Reset) // To clear Factory Reset Value, when user select factory reset over Remote App : Start
	{
		FlashEraseOnly();

		AD85050_Amp_Set_Default_Volume(); //Set Default Volume //This function should be called before "uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i]" and MB3021_BT_Module_Input_Key_Init() because Master must send correct vol_level after factory reset
		MB3021_BT_Module_Input_Key_Init();

		for(i=0; i<9; i++)
		{
			if(i<7)
				uSPP_receive_buf8[i] = uInput_Key_Sync_buf8[i];
		}

#if 1 //need to check
    Set_Display_Mute(FALSE);
#else
    AD85050_Amp_Mute(FALSE, TRUE); //Mute release after init
    AD85050_Amp_Mute(TRUE, FALSE); //Mute release after init
#endif

#ifdef _DBG_FLASH_WRITE_ERASE
		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

		for(i=0;i<FLASH_SAVE_DATA_END;i++)
			_DBH(uFlash_Read_Buf[i]);
#endif
		TIMER20_auto_power_flag_Stop();
	}
	
	TIMER20_factory_reset_cmd_recovery_flag_start(factory_reset_firmware_version); //To recover, Firmware version is missed

	TIMER21_Periodic_Mode_Run(FALSE); //Blinking Timer Off
	TIMER20_auto_power_flag_Start();

	MB3021_BT_Module_HW_Reset(); //HW Reset

	TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop();
	TIMER20_Master_Slave_Grouping_flag_Stop(TRUE);
}

void Set_MB3021_BT_Module_Source_Change_Direct(void)
{
	static uint8_t uBuf[2] = {0,};
#if 0//def BT_DEBUG_MSG	
	_DBG("\n\rSet_MB3021_BT_Module_Source_Change_Direct()");
#endif
	if(BBT_Init_OK) //The Source change is only available when BT Init is finished
	{
		if(Aux_In_Exist())
		{
			uBuf[0] = 0x50; //Aux Mode
			Set_Status_LED_Mode(STATUS_AUX_MODE);
#if 0//def BT_DEBUG_MSG	
			_DBG("AUX Mode");
#endif
		}
		else
		{
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
			uBuf[0] = 0x07; //Bluetooth Mode
#if 0//def BT_DEBUG_MSG	
			_DBG("BT Mode");
#endif
		}

		if(uMode_Change == uBuf[0] 
			&& !IS_Display_Mute()
			)
		{
      if(AD85050_Amp_Get_Cur_CLK_Status())
			{
				if(uMode_Change == 0x07 || uMode_Change == 0x50) //BT Mode or Aux Mode
				{
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r Mute Off : To avoid, BT has no output sometime when user alternates Aux mode / BT mode repeately");
#endif
				}
			}
		}
		else
			uMode_Change = uBuf[0];

		if(uBuf[0] == 0x07) //Bluetooth Mode
			AD85050_Dac_Volume_Set(FALSE);
		else //Aux Mode
			AD85050_Dac_Volume_Set(TRUE);

		MB3021_BT_Module_Send_cmd_param(CMD_INFORM_HOST_MODE_32, uBuf);
		bFactory_Reset_Mode = FALSE;
	}
}

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
				case MINOR_ID_SET_IOCAPABILITY_MODE: //0x00 : 0x1F
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
				case MINOR_ID_REMOTE_DEVICE_NAME_IND: //0x00 : 0x0C
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

			case BLE_MUTE_OFF_DELAY: //mute off delay - Noraml : 0x00 / mute off delay : 0x01 /mute On : 0x02
			{
				if(data[BLE_MUTE_OFF_DELAY] > 0x04) //0x03 ~ 0x04 : Dummy for forced volume setting
					return FALSE;
			}
			break;

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

			case BLE_BT_SHORT_KEY: //BT Short Key(Master Only) - Noraml : 0x00 / BT Short Key : 0x01
			{
				if(data[BLE_BT_SHORT_KEY] > 0x03)
					return FALSE;
			}
			break;

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
	uint8_t uVol_buf = 0;
	uint8_t uSPP_receive_buf8_bk[7]; //To recover when parameter is NG. If parameter is all OK, we save the data value thru SPP to uSPP_receive_buf8 buffer.
	
	//To Do !!! The data length shuld be less than 8
	switch(source_type)
	{
		case DATA_SOURCE_TYPE_SPP: //SPP COM : Receive SPP Data from Peer Device
		{
			{						
				if(data[0] == 0xAA)
				{
					if(data_length == 0x09) //Remocon Data thru SPP
					{
						//Data[0] = Header, Data[1~7] = Remocon Data, Data[8] = Checksum
						bChecksum = SPP_BLE_COM_Calculate_Checksum(data, 8); //using checksum Data check whether it's valid or invalid

						if(bChecksum != data[8])
						{
#ifdef BT_DEBUG_MSG
							_DBG("\n\rReceive data thru SPP communication is NG because check sum is NG");
							_DBH(bChecksum);
							_DBG("/");
							_DBH(data[8]);
#endif
							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_CHECKSUM_ERROR;
							BRet = FALSE;

							break;
						}

						if(!MB3021_BT_Module_Check_Vailid_SSP_Remote_Data(data+1, 7)) //Check whether SSP remote Data is valid or not
						{
#ifdef BT_DEBUG_MSG
							_DBG("\n\rReceive data thru SPP communication is NG because some Data have invalid value");
#endif							
							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA;

							BRet = FALSE;
							break;
						}
						
						for(i = 1; i<8; i++) //Actual Data is data[1] ~ data[7]
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

									uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_POWER_OFF_STATE;

									BRet = FALSE;

									break;
								}
							}

							if(Is_BAmp_Init() == TRUE || Is_I2C_Access_OK() == FALSE) //2023-02-27_2 //2023-02-22_1 : TWS Slave BT SPK executes Amp init again. Sometimes, BT SPK get this interrupt during Amp Init and Amp Init has wrong data.
							{
#ifdef AD85050_DEBUG_MSG
								_DBG("\n\r+++ Is_BAmp_Init is TRUE - 22");
#endif
								uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_BT_SPK_BUSY; //Temparary

								BRet = FALSE;

								break;
							}

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

									case BLE_MUTE_KEY: //##SPP ## Mute On/Off - Mute On : 0x01/Mute Off : 0x00
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 2.Mute On/Off : ");
#endif
										if(data[2] == 0x01)
										{
											AD85050_Amp_Mute(TRUE, TRUE);
										}
										else if(data[2] == 0x00)
										{
											AD85050_Amp_Mute(FALSE, TRUE);
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
											uint32_t l_volume = 0xffff00;

											uVol_buf = Convert_16Step_to_50Step(data[3]); //2023-02-27_3 : To convert from 16-step to 50-step

											l_volume |= uVol_buf;
											AD85050_Amp_Volume_Set_with_Index(l_volume, TRUE, FALSE);
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
										/*if(Get_Cur_BAP_EQ_Mode() == Switch_EQ_NORMAL_Mode)
										{
											uEQ_Mode = EQ_NORMAL_MODE;
											
											break;
										}
										else*/
										//To Do !!! EQ control function
										if(data[4] <= 0x04)
										{
											AD85050_Amp_Mute(TRUE, FALSE); //MUTE ON //Adding Mute when EQ Toggle

											TIMER20_user_eq_mute_flag_start();
#if 1
											uEQ_Mode = data[4];
											AD85050_Amp_EQ_DRC_Control((EQ_Mode_Setting)uEQ_Mode);
#else
											uEQ_Mode = data[4];
											MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uEQ_Mode);
#endif
										}
										else
											BRet = FALSE;
									}
									break;

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

									case BLE_BT_SHORT_KEY: //##SPP ## Factory Reset Key - Factory Reset On : 0x01/Factory Reset Off : 0x00
									{
#ifdef BT_DEBUG_MSG
										_DBG("\n\r+++ 7.BT Short Key");
#endif
										if(data[7] == 0x01)
										{
											if(Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
											{
												BRet = FALSE;											
												bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
												uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA_UNDER_CUR_STATE;
												return;
											}

											MB3021_BT_Master_Slave_Grouping_Start();
										}
										else if(data[7] == 0x00)
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\rBT Short/Long Key Off - To Do !!!");
#endif
										}
										else if(data[7] == 0x02)
										{
#ifdef BT_DEBUG_MSG
											_DBG("\n\r+++ 7-1. BT Long Key");
#endif
											if(Aux_In_Exist()) //Under Aux mode, BT Key is invlaid.
											{
												BRet = FALSE;											
												bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
												uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA_UNDER_CUR_STATE;
												return;
											}
											Send_Remote_Key_Event(BT_PAIRING_KEY);
										}
										else
											BRet = FALSE;
									}
									break;

									default:
										BRet = FALSE;
									break;
								}
							}
						}

						if(BRet)
						{
							bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_OK; //Send SPP Response OK
							
							for(i = 0; i<9; i++) //including checksum - data[8]
							{
								if(i == 7)
									uSPP_receive_buf8[i] = 0x00; //To make valid value for next BT Shortk
								else
								{
									uSPP_receive_buf8[i] = data[i]; //Save SPP Data finally !!!
								}

								if(i<7) //No need checksum
									uInput_Key_Sync_buf8[i] = uSPP_receive_buf8[i];
							}

							switch(Power_Key_Action) //Power Key Action should be executed here because when Power off, we need to save amp value before Power Off
							{
								case REMOTE_POWER_ON_ACTION:
									Remocon_Power_Key_Action(TRUE, FALSE, TRUE);
									break;
								case REMOTE_POWER_OFF_ACTION:
									Remocon_Power_Key_Action(FALSE, FALSE, TRUE);
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
							uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_INVALID_DATA;
						}
#ifdef BT_DEBUG_MSG
						_DBG("\n\rSPP Data :");

						for(i = 0; i<9; i++)
						{
							_DBH(uSPP_receive_buf8[i]);
						}
#endif
					}
					else
					{						
						bPolling_Get_Data |= BCRF_SEND_SPP_RECEIVE_DATA_NG; //Send SPP Response NG
						uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_LENGTH_ERROR;
#ifdef BT_DEBUG_MSG
						_DBG("\n\rSPP_ERROR_LENGTH_ERROR");
#endif
					}
				}
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

								if(IS_Display_Mute() == TRUE) //When Mute On status, we don't need to mute off. This function is for LED Display
									uCurrent_Status_buf8[2] = 0x01; //Mute On/Off : Mute On mode
								else
								{
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
										uCurrent_Status_buf8[2] = 0x00; //Mute On/Off : Mute Off mode
								}

								uVol_buf = AD85050_Amp_Get_Cur_BT_Volume_Level_Inverse();
								uCurrent_Status_buf8[3] = Convert_50Step_to_16Step(uVol_buf);

								uCurrent_Status_buf8[4] = uEQ_Mode; //Sound EQ mode
								
								uCurrent_Status_buf8[5] = 0x00; //Reboot Off
								uCurrent_Status_buf8[6] = 0x00; //Factory Reset Off
								uCurrent_Status_buf8[7] = 0x42;
								uCurrent_Status_buf8[8] = 0x41;
								uCurrent_Status_buf8[9] = 0x50;
								uCurrent_Status_buf8[10] = 0x2D;
								uCurrent_Status_buf8[11] = 0x30;
								uCurrent_Status_buf8[12] = 0x32;
								uCurrent_Status_buf8[13] = 0x20; //0x01; //EQ NORMAL

								uCurrent_Status_buf8[14] = SPP_BLE_COM_Calculate_Checksum(uCurrent_Status_buf8, 14);

								bPolling_Get_Data |= BCRF_SEND_SPP_DATA_RESP; //Send SPP Response
							}
							else
							{
								uSPP_RECEIVE_DATA_ERROR = SPP_ERROR_CHECKSUM_ERROR;
							}
						}
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
			}
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
		MB3021_BT_Module_CMD_Execute(major_id, minor_id, data, data_length, FALSE);	
	}
}

static void MB3021_BT_Module_Receive_Data_IND(uint8_t major_id, uint8_t minor_id, uint8_t *data, uint16_t data_length)
{	
	int i = 0;
	char Address_buf[6];
	
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
					bPolling_Get_Data |= BCRF_GET_PAIRED_DEVICE_LIST; //For init sequence (Init Sequnece : Broadcaster -0) //Last -5
					BBT_Init_OK = TRUE;
				}
				break;
				
				case MINOR_ID_REMOTE_DEVICE_NAME_IND: //Init //0x00 : 0x0C
				{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
					_DBG("\n\r++Ind : MINOR_ID_REMOTE_DEVICE_NAME_IND");
					_DBG(" : ");
					_DBH(uPaired_Device_Count);
#endif
					//if(uPaired_Device_Count && SUM_of_Array(uBT_Cur_A2DP_Device_Address)) //Under Last Connection, we don't need this case statement
					if((strncmp(uLast_Connected_Device_Address, uBT_Cur_A2DP_Device_Address, 6) == 0) && SUM_of_Array(uBT_Cur_A2DP_Device_Address)) //Under Last Connection, we don't need this case statement
					{
#ifdef DEVICE_NAME_CHECK_PAIRING_DEBUG_MSG	
						_DBG("\n\r=== break : Last Connection Mode");
#endif
						break;
					}

					if(SUM_of_Array(uBT_Remote_Device_Address) && strncmp(uBT_Remote_Device_Address, uBT_Cur_A2DP_Device_Address, 6) == 0) //Get MINOR_ID_REMOTE_DEVICE_NAME_IND twice so we need to ignore Second one
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

				case MINOR_ID_FIRMWARE_VERSION_IND: //Init //0x00 : 0x0E
					TIMER20_factory_reset_cmd_recovery_flag_stop(); //To Clear Factory Reset recovery for factory_reset_firmware_version becasuse SPK get response here.
					bPolling_Get_Data |= BCRF_INIT_SINK_MODE; //For init sequence (Init Sequnece : 1) //Last - 1
					strncpy(BT_Version, (char *)data, 7); //Save BT Version information - 7 Byte

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
							MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);

							AD85050_Amp_Mute(TRUE, FALSE); //MUTE ON
							break;
							
						case 0x01: //0x00 : 0x22 : 0x01 // Routing Connect //Mute Off

							switch(data[1])
							{
								case 0x02: //0x00 : 0x22 : 0x01 : 0x02 //Analog
								if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
								TIMER20_mute_flag_Start();

								break;

								case 0x06: //0x00 : 0x22 : 0x01 : 0x06 //A2DP
								break;

								case 0x09: //0x00 : 0x22 : 0x01 : 0x09 //Broadcaster
								if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
								TIMER20_mute_flag_Start();

								break;

								default:
								break;
							}							
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
						break;
						case 0x04: //No Device
						default:
							uLast_Connection_Retry_Count = 0; //Need to clear in case of other(excepting 0x0b)
						break;
					}
				break;

				case MINOR_ID_ACL_CLOSED_IND: //0x00 : 0x24
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r++Ind : MINOR_ID_ACL_CLOSED_IND : "); //Data[6]: Disconnect : 0x13/BT Delete : 0x16/TWS Disconnection : 0x08(Timeout)
#endif
					BBT_Is_Routed = FALSE;

#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
					_DBG("\n\rA2DP Device is disconnected(MINOR_ID_ACL_CLOSED_IND) !!!");
					_DBG("\n\rCur Addr : ");
					for(i=0;i<6;i++)
					_DBH(data[i]);

					_DBG("\n\rA2DP Addr : ");
					for(i=0;i<6;i++)
					_DBH(uBT_Cur_A2DP_Device_Address[i]);
#endif
					BBT_Is_Connected = FALSE; //2023-03-29_1 : When user disconnects BSP-01 in BT Menu on Peerdevice, If user executes power off->on over power button, Master has BT LED On(It should be blinking).
					bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_READY;

					//uPaired_Device_Count = 0;
					uBT_Remote_Device_Name_Size = 0;
			
					for(i=0; i<6; i++)
					{
						uBT_Remote_Device_Address[i] = 0;
						{
							uBT_Cur_A2DP_Device_Address[i] = 0;
							uLast_Connected_Device_Address[i] = 0;
						}
					}
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

#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
						_DBG("\n\rA2DP Device is disconnected(READY) !!!");
#endif
					bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_READY;
					BBT_Is_Routed = FALSE;

					//uPaired_Device_Count = 0;
					uBT_Remote_Device_Name_Size = 0;
		
					for(i=0; i<6; i++)
					{
						uBT_Remote_Device_Address[i] = 0;
						uBT_Cur_A2DP_Device_Address[i] = 0;
					}
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
					bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_CONNECTED;
				}
				else //0x10 : 0x00 (data[6] == 0x04) //Disconnecting
				{
#if 0//def BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Disconnecting");
#endif

					for(i=0; i<6; i++)
					{
						Address_buf[i] = data[i];
					}

					//Check if current connected device is A2DP device?
					if(strncmp(uBT_Cur_A2DP_Device_Address, Address_buf, 6) == 0)
					{
#ifdef BT_GENERAL_MODE_KEEP_ENABLE_DEBUG_MSG
						_DBG("\n\rA2DP Device is disconnected !!!");
#endif
						bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_DISCONNECTING;
						BBT_Is_Routed = FALSE;
					}

					uBT_Remote_Device_Name_Size = 0;

					for(i=0; i<6; i++)
					{
						uBT_Remote_Device_Address[i] = 0;
						uBT_Cur_A2DP_Device_Address[i] = 0;
					}
				}

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
				break;

				case MINOR_ID_A2DP_MEDIA_STATE_CHANGED_IND:
					break;

				case MINOR_ID_A2DP_STREAM_ROUTING_CHANGED_IND: //0x10 : 0x02
					switch(data[0])
					{
						case 0x0: //0x10 : 0x02 //UnRouted
						{							
							BBT_Is_Routed = FALSE;

							if(uNext_Grouping_State > GROUPING_NONE_MODE) //To get MINOR_ID_BA_MODE_CONTROL response under master slave grouping mode //To avoid missing interrupt
								bPolling_Set_Action |= A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE;
							else
							{
								MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
								if(!Aux_In_Exist())
								{
									AD85050_Amp_Mute(TRUE, FALSE); //Mute On
								}
							}
						}
						break;
						
						case 0x1: //0x10 : 0x02 //Routed
						BBT_Is_Routed = TRUE;

						if(!IS_Display_Mute()) //When Mute On status, we don't need to mute off. This function is for LED Display
							TIMER20_mute_flag_Start();

						break;
						
						default:
						MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02);
						AD85050_Amp_Mute(TRUE, FALSE); //Mute On
						break;
					}
					break;
					

				case MINOR_ID_A2DP_SELECTED_CODEC_IND: //0x10 : 0x05
				break;

				case MINOR_ID_A2DP_CONNECT_FAIL_ERRORCODE_IND: //0x10 : 0x70 //LAST CONNECTION FAIL
#ifdef BT_DEBUG_MSG
					_DBG("\n\r###Profile State : Disconnecting");
					_DBG("\n\r###LAST CONNECTION FAIL");
#endif
					//Need to delete below because Aux is NG when Power Off --> Plug out --> Plug In
					//bPolling_Get_Data = 0;
					//bPolling_Get_BT_Profile_State = 0;
					bPolling_Set_Action =0;
					bPolling_Get_BT_Profile_State |= BT_PROFILE_STATE_DISCONNECTING;
					BBT_Is_Connected = FALSE;

					break;					
					
				default:
					break;
			}
			break;

		case MAJOR_ID_SPP_CONTROL: //0x13
			switch(minor_id)
			{
				case MINOR_ID_SPP_PROFILE_STATE_CHANGED_IND: //0x13 : 0x00
#ifdef BT_DEBUG_MSG	
					_DBG("\n\r+++Ind : MINOR_ID_SPP_PROFILE_STATE_CHANGED_IND");
#endif
					for (i=0;i<6;i++)
						uCur_SPP_Device_Address[i] = data[i]; //For SPP Communication

					if(data[6] == 0x03)
						BSPP_Ready_OK = TRUE;
					else
						BSPP_Ready_OK = FALSE;

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
					if(uNext_Grouping_State > GROUPING_NONE_MODE) //Groping mode don't need next step here !!!
						break;

					bPolling_Get_Data |= BCRF_SET_BLE_MANUFACTURE_DATA; //For init sequence (Init Sequnece : Broadcaster -4) //For init sequence (Init Sequnece : Receiver -2)
					TIMER20_Forced_Input_Audio_Path_Setting_flag_start(); //To avoid, Audio audio output NG
					break;

				case MINOR_ID_RECEIVER_CONNECTION_STATUS_IND: //0x16 : 0x02
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\r+++Ind : MINOR_ID_RECEIVER_CONNECTION_STATUS_IND");
#endif
				break;

				case MINOR_ID_BA_MANUFACTURE_DATA_IND: //0x16 : 0x05
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
						if(BKeep_Connectable)
							BKeep_Connectable = FALSE;
						else
							bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)
					}
					
					break;

				case MINOR_ID_SET_DISCOVERABLE_MODE: //0x00 : 0x07
				bPolling_Get_Data_backup &= (~BCRF_SET_DISCOVERABLE_MODE); //Clear flag
#if 0//def BT_DEBUG_MSG	
				_DBG("\n\rRes: MINOR_ID_SET_DISCOVERABLE_MODE");
#endif
					{
						if(do_recovery)
							bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //Set flag to send same data again
						else
						{
							if(!BDoNotSend_Connectable_Mode)
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
#if 1
						mb3021_status = MB3021_POWER_CONTROL_CMD_PROCESS;
						mb3021_timer = df10msTimer100ms;
#else
						delay_ms(100); //For flash saving time of BT module
						bPolling_Get_Data |= BCRF_MODULE_POWER_CONTROL; //For init sequence (Init Sequnece : 4) //Last - 4
#endif
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
						uPaired_Device_Count = data[1]; //data[2] : index
#ifdef BT_DEBUG_MSG
						_DBG("\n\rDevice Count = ");_DBD(uPaired_Device_Count);
#endif
						if(uPaired_Device_Count) //Last Connection
						{
							//for(i=0;i<uPaired_Device_Count*8;i++) //Total Device 
							for(i=0;i<6;i++)
							{
								uLast_Connected_Device_Address[i] = data[i+3];
#if 0//def BT_DEBUG_MSG	
								_DBH(uLast_Connected_Device_Address[i]);
#endif
							}

							bPolling_Get_Data |= BCRF_SET_LAST_CONNECTION; //For Last Connection //For init sequence (Init Sequnece : Broadcaster -0-1) //Last -6
						}
						else //No Paired Device List : Need to go Discoverable_Mode
						{
							bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //For init sequence (Init Sequnece : Broadcaster -1)
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
						//if(!bFactory_Reset_Mode && mode == Switch_Master_Mode)
						if(!B_Delete_PDL_by_Factory_Reset) //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
						{
							FlashSaveData(FLASH_SAVE_DATA_PDL_NUM, 0); //Save PDL Device Number(0) from Flash							
#if 1
							mb3021_status = MB3021_MODULE_INIT_CMD_PROCESS;
							mb3021_timer = df10msTimer300ms;


#else
							delay_ms(300); //2023-03-03_1 : For flash saving time of BT module during BT Long key action.
							MB3021_BT_Module_Init(FALSE);
							Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
#endif

							break;
						}

						BBT_Pairing_Key_In = TRUE;
						//TIMER20_factory_reset_cmd_recovery_flag_stop(); //Move to MB3021_BT_Module_Init() //To Clear Factory Reset recovery becasuse SPK get response here.

						Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);

						//Factory Reset / Flash Data Clear - FLASH_SAVE_DATA_PDL_NUM
						if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x00)
							FlashSaveData(FLASH_SAVE_DATA_PDL_NUM, 0); //Save PDL Device Number(0) from Flash

						//Do not Delete !!! BT Module needs to get enough time to save current setting.
						//delay_ms(150); //Need to delete !!! BT Module miss MINOR_ID_FIRMWARE_VERSION_IND under Master mode

						LED_Display_All_On(); //Display All LED during 1 sec
						TIMER20_factory_reset_led_display_flag_Start(); //Start 1 sec timer
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
					else //Set EQ Setting after Power On
					{
						TIMER20_Forced_Input_Audio_Path_Setting_flag_stop();
					}					
					break;
					
					case MINOR_ID_SET_IOCAPABILITY_MODE: //0x00 : 0x1F
					bPolling_Get_Data_backup &= (~BCRF_SET_IO_CAPABILITY_MODE); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_SET_IOCAPABILITY_MODE");
#endif
					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_IO_CAPABILITY_MODE; //Set flag to send same data again

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
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_A2DP_CONNECTION_CONTROL");
#endif
					if(do_recovery)
					{
						bPolling_Get_Data |= BCRF_SET_LAST_CONNECTION; //Set flag to send same data again
					}
					else
					{	
						bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Broadcaster -3)// Last connection is success(Last -7) !!! //To Do !!!
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
				default:
					ret = FALSE;
					break;
			}
		}
		break;

		case MAJOR_ID_TWS_CONTROL: //0x15
			break;

		case MAJOR_ID_BA_CONTROL: //0x16
		{
			switch(minor_id)
			{
				case MINOR_ID_BA_MODE_CONTROL: //0x16 : 0x00
				bPolling_Get_Data_backup &= (~BCRF_BA_MODE_CONTROL); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_BA_MODE_CONTROL");
#endif				
					if(uNext_Grouping_State > GROUPING_NONE_MODE) //Groping mode don't need next step here !!!
					{
						TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop();
						
						if(uPrev_Grouping_State == GROUPING_MASTER_NORMAL_MODE)
							uNext_Grouping_State = GROUPING_MASTER_BROADCASTER_MODE;
						else if(uPrev_Grouping_State == GROUPING_MASTER_BROADCASTER_MODE)
							uNext_Grouping_State = GROUPING_MASTER_SET_MANUFACTURE_DATA;
						else
							__NOP();
						
						break;
					}

					if(do_recovery)
						bPolling_Get_Data |= BCRF_BA_MODE_CONTROL; //For init sequence (Init Sequnece : Receiver -1)
					else
						;

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

					if(do_recovery)
						bPolling_Get_Data |= BCRF_ADVERTISING_CONTROL; //For init sequence (Init Sequnece : Broadcaster -5)
					else
					{
						bPolling_Get_Data |= BCRF_INFORM_HOST_MODE; //For init sequence (Init Sequnece : Broadcaster -6)
					}
					break;
					
				case MINOR_ID_SET_BLE_MANUFACTURE_DATA: //0x17 : 0x03
				bPolling_Get_Data_backup &= (~BCRF_SET_BLE_MANUFACTURE_DATA); //Clear flag
#ifdef BT_DEBUG_MSG	
					_DBG("\n\rRes: MINOR_ID_SET_BLE_MANUFACTURE_DATA");
#endif
					if(uNext_Grouping_State > GROUPING_NONE_MODE) //Groping mode don't need next step here !!!
					{
						TIMER20_Master_Slave_Grouping_cmd_recovery_flag_stop();
						
						if(uPrev_Grouping_State == GROUPING_MASTER_SET_MANUFACTURE_DATA)
						{
							uNext_Grouping_State = GROUPING_MASTER_ADVERTISING_ON;
						}
						else
						{
							__NOP();
						}
						
						break;
					}

					if(do_recovery)
						bPolling_Get_Data |= BCRF_SET_BLE_MANUFACTURE_DATA; //For init sequence (Init Sequnece : Broadcaster -4)
					else
					{
						if(BMaster_Send_BLE_Remote_Data) //Master Mode Only
						{
							//BMaster_Send_BLE_Remote_Data = FALSE; //2022-11-11 : Move to BA_SWITCH_MODE
						}
						else
						{
							bPolling_Get_Data |= BCRF_ADVERTISING_CONTROL; //For init sequence (Init Sequnece : Broadcaster -5)
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
		if(uTimerVal == df10msTimer3s/*0x2FFFD*/) //3s
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
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SEND_BLE_REMOTE_DATA");
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
				{
					//delay_ms(100); //Deleted 100ms delay because of speed of MCU response for Tablet App

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

					for(i=0; i<9; i++)
					{
						if(i == 7) //mute off delay
							uBuf[i+6] = 0x00;
						else if(i == 8) //send checksum for slave SPK
							uBuf[i+6] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 8); //Check sum
						else
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
						MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
					}
				}
			}
			else //BCRF_SEND_SPP_RECEIVE_DATA_NG
			{
				uBuf[7] = uSPP_RECEIVE_DATA_ERROR;
				uBuf[8] = SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 2);

				MB3021_BT_Module_Send_Data_Packcet(uBuf, 9); //SPP COM : Send SPP OK Response to Peer Device - without checksum
			}			
		}

		if(bPolling_Get_Data & BCRF_SEND_SPP_RECEIVE_DATA_OK)
		{
			bPolling_Get_Data &= (~BCRF_SEND_SPP_RECEIVE_DATA_OK); //Clear flag

			if(Is_SSP_REBOOT_KEY_In()) //check whether it receive reboot key thru SPP and execute reboot if it receives reboot key
			{
#if 1
				mb3021_status = MB3021_SW_RESETTING_CMD_PROCESS;
				mb3021_timer = df10msTimer200ms;
#else
				delay_ms(200); //2023-04-27_2 : When TWS Master get reboot CMD over USEN Tablet remocon App, TWS Master need some delay to send reboot CMD to TWS Slave.
				MB3021_BT_Disconnect_All_ACL(); //To avoid last connection fail after reboot //Reboot recovery solution - 1
				delay_ms(1000); //delay for send BLE Data to Slave SPK(Receiver)
				SW_Reset();
#endif
			}

			if(Is_SSP_FACTORY_RESET_KEY_In()) //check whether it receive factory reset key thru SPP and execute reboot if it receives factory reset key
			{
				B_SSP_FACTORY_RESET_KEY_In = FALSE;
				bFactory_Reset_Mode = TRUE;
				Factory_Reset_Value_Setting();
			}
		}
		else
			bPolling_Get_Data &= (~BCRF_SEND_SPP_RECEIVE_DATA_NG); //Clear flag
	}

	if(uCount == df10msTimer200ms /*0x3332*/) //200ms //0xffff = 1s, 0xffff/2(0x7fff) = 500ms/0x1999 = 100ms
	{
		uCount = 0;
	}
	else
	{
		uCount++;
		return;
	}

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

			MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x02); //For Slave Mute

			AD85050_Amp_Mute(TRUE, FALSE); //MUTE ON

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
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_MASTER_SET_MANUFACTURE_DATA");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
			
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

				for(i=0; i<9; i++)
				{
					if(i == 7) //mute off delay
						uBuf[i+6] = 0x00; //If we send 0x02(Mute On), Slave make mute on even though mute off because this communication is reached slower than actual mute off.
					else if(i == 8) //send checksum for slave SPK
						uBuf[i+6] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 8); //Check sum
					else
						uBuf[i+6] = uSPP_receive_buf8[i];
					//_DBH(uBuf[i+6]);//_DBH(uSPP_receive_buf8[i]);
				}

				TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_MASTER_SET_MANUFACTURE_DATA);

				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
			}
		}
		break;

		case GROUPING_MASTER_ADVERTISING_ON:
		{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
			_DBG("\n\rGp : GROUPING_MASTER_ADVERTISING_ON");
#endif
			uPrev_Grouping_State = uNext_Grouping_State;
			{
				uNext_Grouping_State = GROUPING_EVENT_WAIT_STATE;
				
				uBuf[0] = 0x01; //Advertising On
				TIMER20_Master_Slave_Grouping_cmd_recovery_flag_start(GROUPING_MASTER_ADVERTISING_ON);
				MB3021_BT_Module_Send_cmd_param(CMD_ADVERTISING_CONTROL_32, uBuf);
			}
		}
		
		if(Aux_In_Exist() //Need to keep LED off under Aux Mode
		&& !IS_Display_Mute()//This is mute off delay and that's means this action should be worked in mute off. //if(Is_Mute())
		)
		{
			AD85050_Amp_Mute(FALSE, FALSE); //MUTE OFF
			MB3021_BT_Module_Forced_Input_Audio_Path_Setting();
		}

		break;

		default:
		break;
	}

	if(bPolling_Set_Action & A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE) //To get MINOR_ID_BA_MODE_CONTROL response under master slave grouping mode //To avoid missing interrupt
	{
#ifdef MASTER_SLAVE_GROUPING_DEBUG_MSG	
		_DBG("\n\rSet : A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE");
#endif
		AD85050_Amp_Mute(TRUE, FALSE); //Mute On
		bPolling_Set_Action &= (~A2DP_STREAM_ROUTING_CHANGED_IND_UNROUTE); //Clear flag
	}
	
	//Check bPolling_Get_BT_Profile_State
	if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_INITIALISING)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_INITIALISING");
#endif

		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
		if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x01)
			Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
		else
		{
			BBT_Is_Connected = FALSE;
			Set_Status_LED_Mode(STATUS_BT_FAIL_OR_DISCONNECTION_MODE);
		}

		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_INITIALISING); //Clear flag
	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_READY)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_READY");
#endif

		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
		if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x01)
			Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
		else
		{
			BBT_Is_Connected = FALSE;
			Set_Status_LED_Mode(STATUS_BT_FAIL_OR_DISCONNECTION_MODE);
		}

		BKeep_Connectable = TRUE;
#if 0 //2023-05-30_1 : Added this condition. Only current A2DP device is available until user select BT Long Key to connect other device.
//defined(BT_GENERAL_MODE_KEEP_ENABLE) && defined(SWITCH_BUTTON_KEY_ENABLE) //2022-12-27 : To keep connectable mode when BT is not connected with Peer Device
		if(BBT_Pairing_Key_In)
			bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device
		else
#endif
		bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device

		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_READY); //Clear flag
	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_CONNECTING)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_CONNECTING");
#endif

		if(Get_Cur_Status_LED_Mode() != STATUS_BT_GIA_PAIRING_MODE) //When current mode is STATUS_BT_GIA_PAIRING_MODE, it need to keep cur mode
			Set_Status_LED_Mode(STATUS_BT_PAIRING_MODE);

		BKeep_Connectable = FALSE;
		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_CONNECTING); //Clear flag
	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_CONNECTED)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_CONNECTED");
#endif

		FlashSaveData(FLASH_SAVE_DATA_PDL_NUM, 1); //Save PDL List to Flash memory for SPP/BLE Data

		Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);

		BDoNotSend_Connectable_Mode = TRUE;
		MB3021_BT_Module_Set_Discoverable_Mode_by_Param(SET_DISABLE_DISCOVERABLE_MODE);

#ifdef PRODUCT_LINE_TEST_MASTER_ID2_FIXED
		B_Auto_FactoryRST_On = TRUE; //2023-04-03_1
#endif
		bPolling_Get_BT_Profile_State &= (~BT_PROFILE_STATE_CONNECTED); //Clear flag
		BKeep_Connectable = FALSE;

		bPolling_Get_Data |= BCRF_CLEAR_CONNECTABLE_MODE;

	}
	else if(bPolling_Get_BT_Profile_State & BT_PROFILE_STATE_DISCONNECTING)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BT_PROFILE_STATE_DISCONNECTING");
#endif

		Flash_Read(FLASH_SAVE_START_ADDR, uFlash_Read_Buf, FLASH_SAVE_DATA_END);
		if(uFlash_Read_Buf[FLASH_SAVE_DATA_PDL_NUM] != 0x01)
			Set_Status_LED_Mode(STATUS_BT_GIA_PAIRING_MODE);
		else
		{
			BBT_Is_Connected = FALSE;
			Set_Status_LED_Mode(STATUS_BT_FAIL_OR_DISCONNECTION_MODE);
		}

		if(BBT_Pairing_Key_In)
			bPolling_Get_Data |= BCRF_SET_DISCOVERABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device
		else
			bPolling_Get_Data |= BCRF_SET_CONNECTABLE_MODE; //To keep connectable mode when BT is not connected with Peer Device

		BKeep_Connectable = TRUE;

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
	if(bPolling_Get_Data & BCRF_CLEAR_CONNECTABLE_MODE) //0x100000 
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_CONNECTABLE_MODE : disable");
#endif
		BKeep_Connectable = TRUE;
		uBuf[0] = 0x00; //0x0:Disable/0x01:Enable
			
		MB3021_BT_Module_Send_cmd_param(CMD_SET_CONNECTABLE_MODE_32, uBuf);
		bPolling_Get_Data_backup |= BCRF_CLEAR_CONNECTABLE_MODE;

		bPolling_Get_Data &= (~BCRF_CLEAR_CONNECTABLE_MODE); //Clear flag
	}				

	if(bPolling_Get_Data & BCRF_SET_IO_CAPABILITY_MODE)
	{
#ifdef BT_DEBUG_MSG
		_DBG("\n\rDo : BCRF_SET_IO_CAPABILITY_MODE");
#endif
		int i;

		if(strncmp(strUSEN_Device_Name_1, uBT_Remote_Device_Name, 17)) //check wheter "USEN MUSIC DEVICE" or not
		{//Current Peer Device is NOT "USEN MUSIC DEVICE"
			if(BBT_Pairing_Key_In) //General Mode
			{
				uBuf[0] = 0x03; //0x03 : Pairing Accept
				bDevice_Paring_Accept = TRUE;
			}
			else //Device Name Check Mode
			{
				uBuf[0] = 0x05; //0x05 : Pairing Reject
				bDevice_Paring_Accept = FALSE;
			}
		}
		else
		{//Current Peer Device is "USEN MUSIC DEVICE"
			uBuf[0] = 0x03; //0x03 : Pairing Accept
			bDevice_Paring_Accept = TRUE;
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

		bPolling_Get_Data &= (~BCRF_SET_IO_CAPABILITY_MODE); //Clear flag
	}

	if(bPolling_Get_Data & BCRF_SEND_SPP_DATA_RESP) //0x20000
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SEND_SPP_DATA_RESP");
#endif
		int i;

		uBuf[0] = SYNC_BYTE;
		uBuf[1] = PACKET_DATA;
		uBuf[2] = DATA_SOURCE_TYPE_SPP;
		uBuf[3] = DATA_CH_ANDROID1;
		uBuf[4] = 0x00; //Data Size - High Byte

		if(uCurrent_Status_buf8[1] == 0xAF) //MCU FW Version
			uBuf[5] = 0x09; //Data Size - Low Byte (Size of Data packet of MCU FW version )
		else if(uCurrent_Status_buf8[1] == 0xBF) //BT FW Version
			uBuf[5] = 0x0A; //Data Size - Low Byte (Size of Data packet of BT FW version )
		else
			uBuf[5] = 0x0F; //Data Size - Low Byte (Size of uCurrent_Status_buf8)
			
		for(i=0; i<uBuf[5]; i++)
			uBuf[i+6] = uCurrent_Status_buf8[i]; //The last byte is uBuf[13]

		MB3021_BT_Module_Send_Data_Packcet(uBuf, uBuf[5]+6); //SPP COM : Send SPP OK Response to Peer Device - without checksum			

		bPolling_Get_Data &= (~BCRF_SEND_SPP_DATA_RESP); //Clear flag
	}
	
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
#if 0//def BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_LAST_CONNECTION");
#endif
		uBuf[0] = 0x01; //Data[0] - 0x0 : Disconnect A2DP, 0x1 : Connect A2DP, 0x2 : Connect A2DP Connection
			
		for(i=0;i<6;i++)
			uBuf[i+1] = uLast_Connected_Device_Address[i]; //Data[1~6] - Peer Device Address 			

#if 0//def BT_DEBUG_MSG	
			_DBH(uBuf[0] );
#endif
		MB3021_BT_Module_Send_cmd_param(CMD_A2DP_CONNECTION_CONTROL_32, uBuf);
		bPolling_Get_Data &= (~BCRF_SET_LAST_CONNECTION); //Clear flag
	}
	
	if(bPolling_Get_Data & BCRF_GET_PAIRED_DEVICE_LIST)
	{
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_GET_PAIRED_DEVICE_LIST =");
#endif
		MB3021_BT_Module_Send_cmd_param(CMD_GET_PAIRED_DEVICE_LIST_32, uBuf);
		bPolling_Get_Data_backup |= BCRF_GET_PAIRED_DEVICE_LIST;
			
		bPolling_Get_Data &= (~BCRF_GET_PAIRED_DEVICE_LIST); //Clear flag
	}
	
	if(bPolling_Get_Data & BCRF_INFORM_HOST_MODE) //For init sequence (Init Sequnece : Broadcaster -6)
	{
#if 0//def BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_INFORM_HOST_MODE =");
#endif
		if(BBT_Init_OK) //The Source change is only available when BT Init is finished
		{
			if(Aux_In_Exist())
			{
				uBuf[0] = 0x50; //Aux Mode
				Set_Status_LED_Mode(STATUS_AUX_MODE);
#if 0//def BT_DEBUG_MSG	
				_DBG("AUX Mode");
#endif
			}
			else
			{
				Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
				uBuf[0] = 0x07; //Bluetooth Mode
#if 0//def BT_DEBUG_MSG	
				_DBG("BT Mode");
#endif
			}

			if(uMode_Change == uBuf[0] && !IS_Display_Mute())
			{
				if(AD85050_Amp_Get_Cur_CLK_Status())
				{
					if(uMode_Change == 0x50) //Aux Mode
					{
#ifdef BT_DEBUG_MSG	
							_DBG("\n\r Mute Off : To avoid, BT has no output sometime when user alternates Aux mode / BT mode repeately");
#endif
		        TIMER20_mute_flag_Start();
					}
				}
			}
			else
				uMode_Change = uBuf[0];

			if(uBuf[0] == 0x07) //Bluetooth Mode
				AD85050_Dac_Volume_Set(FALSE);
			else //Aux Mode
				AD85050_Dac_Volume_Set(TRUE);

			MB3021_BT_Module_Send_cmd_param(CMD_INFORM_HOST_MODE_32, uBuf);

			bFactory_Reset_Mode = FALSE;

			bPolling_Get_Data &= (~BCRF_INFORM_HOST_MODE); //Clear flag
			bPolling_Get_Data_backup |= BCRF_INFORM_HOST_MODE;
		}
	}

	if(bPolling_Get_Data & BCRF_ADVERTISING_CONTROL) //For init sequence (Init Sequnece : Broadcaster -5)
	{
#if 0//def BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_ADVERTISING_CONTROL");
#endif
		uBuf[0] = 0x01; //Advertising On
		MB3021_BT_Module_Send_cmd_param(CMD_ADVERTISING_CONTROL_32, uBuf);
		bPolling_Get_Data_backup |= BCRF_ADVERTISING_CONTROL;
		
		bPolling_Get_Data &= (~BCRF_ADVERTISING_CONTROL); //Clear flag
	}
	
	if(bPolling_Get_Data & BCRF_BA_MODE_CONTROL)
	{
		uBuf[0] = 0x01; //Broadcaster Mode
		MB3021_BT_Module_Send_cmd_param(CMD_BA_MODE_CONTROL_32, uBuf);
				
		BMaster_Send_BLE_Remote_Data = FALSE; //2022-11-11 : Move to Here
				
		bPolling_Get_Data &= (~BCRF_BA_MODE_CONTROL); //Clear flag
		bPolling_Get_Data_backup |= BCRF_BA_MODE_CONTROL;
	}

	if(bPolling_Get_Data & BCRF_SET_BLE_MANUFACTURE_DATA)
	{
		int i;
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_BLE_MANUFACTURE_DATA");
#endif
		uBuf[0] = BLE_DATA_0;
		uBuf[1] = BLE_DATA_1;
		//To Do !!! Need to check with customer
		uBuf[2] = BLE_DATA_VENDOR_ID_HIGH_BYTE;
		uBuf[3] = BLE_DATA_VENDOR_ID_LOW_BYTE; 

		if(Get_master_slave_grouping_flag() == TRUE) //If Slave doesn't have Last connection information, we change Product ID further below.
		{
			uBuf[4] = BLE_DATA_GROUPING_PRODUCT_ID_HIGH_BYTE;
			uBuf[5] = BLE_DATA_GROUPING_PRODUCT_ID_LOW_BYTE;
		}
		else
		{
			uBuf[4] = BLE_DATA_PRODUCT_ID_HIGH_BYTE;
			uBuf[5] = BLE_DATA_PRODUCT_ID_LOW_BYTE;
		}

		{//To Do !!! Need to set each key values for BT Slave SPK
			for(i=0; i<9; i++)
			{
				if(i == 7) //mute off delay
					uBuf[i+6] = 0x00; //If we send 0x02(Mute On), Slave make mute on even though mute off because this communication is reached slower than actual mute off.
				else if(i == 8) //send checksum for slave SPK
					uBuf[i+6] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(&uBuf[6], 8); //Check sum
				else
					uBuf[i+6] = uSPP_receive_buf8[i];
				//_DBH(uBuf[i+6]);//_DBH(uSPP_receive_buf8[i]);
			}

			//BMaster_Send_BLE_Remote_Data = TRUE;

			//delay_ms(50); //Deleted 50ms delay because of speed of Master CMD for Slave
			
			if(uBuf[6] == 0xff) //When we didn't get SPP data from Tablet, we don't need to send SPP receive data.
			{
				for(i=0; i<9; i++)
				{
					if(i == 8)
						uInput_Key_Sync_buf8[8] = (uint8_t)SPP_BLE_COM_Calculate_Checksum(uInput_Key_Sync_buf8, 8); //Check sum

					uBuf[i+6] = uInput_Key_Sync_buf8[i];
#ifdef BT_DEBUG_MSG
					_DBH(uInput_Key_Sync_buf8[i]);
#endif
				}
				
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum

				TIMER20_power_on_volume_sync_flag_start();
			}
			else
			{
				MB3021_BT_Module_Send_cmd_param(CMD_SET_BLE_MANUFACTURE_DATA_32+0x0800, uBuf); //BLE COM : Send SPP data to Slave SPK thru BLE Data - without checksum
			}
		}

		bPolling_Get_Data &= (~BCRF_SET_BLE_MANUFACTURE_DATA); //Clear flag
		bPolling_Get_Data_backup |= BCRF_SET_BLE_MANUFACTURE_DATA;
	}

	if(bPolling_Get_Data & BCRF_INIT_SINK_MODE)
	{
		int i;
		
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_INIT_SINK_MODE");
#endif

		uBT_Remote_Device_Name_Size = 0; //Init under INIT_SINK_MODE

		for(i=0; i<6; i++)
		{
			uBT_Remote_Device_Address[i] = 0;
			uBT_Cur_A2DP_Device_Address[i] = 0;
		}

		uBuf[0] = 0x91;//0x19; //No Delay/Left Justified/24bit/None(Sampling Frequency - To avoid BT DSP Resource issue)/I2S Master
		uBuf[1] = 0x00; //Disable TWS
		uBuf[2] = 0x03; //Enable AAC(b1)/A2DP Sink(b0)
		uBuf[3] = 0x00; //0x07;//Disable AVRCP
		uBuf[4] = 0x00; //Disable HFP Profile Features
		uBuf[5] = 0x00; //Disable HFP Supported Features
		uBuf[6] = 0x00; //Disable HFP Supported Features
		uBuf[7] = 0x00; //Disable HFP enable Feature
		uBuf[8] = 0x00; //Disable PBAP Profile Features
		uBuf[9] = 0x05; //SSP Frofile Features - Enable Android 1[b2] / Enable SPP Profile[b0]
		uBuf[10] = 0x01; //Enable BLE Features
		uBuf[11] = 0x01; //Broadcast Audio Features - Enable Broadcast Audio[b0]
		uBuf[12] = 0x00; //Disable Battery Features
		uBuf[13] = 0x01; //Enable I/O Capability Features
		
		MB3021_BT_Module_Send_cmd_param(CMD_INIT_SINK_MODE_32, uBuf);

		bPolling_Get_Data &= (~BCRF_INIT_SINK_MODE); //Clear flag
		bPolling_Get_Data_backup |= BCRF_INIT_SINK_MODE;
	}

	if(bPolling_Get_Data & BCRF_SET_DEVICE_ID_CONTROL)
	{ 
#ifdef BT_DEBUG_MSG
		_DBG("\n\rDo : BCRF_SET_DEVICE_ID_CONTROL");
#endif
		//strncpy(MCU_Version, uBuf, 6);
		MB3021_BT_Module_Send_cmd_param(CMD_SET_DEVICE_ID_32, (uint8_t *)MCU_Version);
		bPolling_Get_Data &= (~BCRF_SET_DEVICE_ID_CONTROL); //Clear flag		
	}

	if(bPolling_Get_Data & BCRF_SET_MODEL_NAME_CONTROL)
	{ 
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

	if(bPolling_Get_Data & BCRF_SET_DISCOVERABLE_MODE)
	{ 
#ifdef BT_DEBUG_MSG	
		_DBG("\n\rDo : BCRF_SET_DISCOVERABLE_MODE");
#endif
		uBuf[0] = 0x02; //Enable General Discoverable mode

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

void MB3021_BT_Module_EQ(EQ_Mode_Setting EQ_mode)
{
	uint8_t uBuf;

	uBuf = (uint8_t)EQ_mode;
	
#ifdef BT_DEBUG_MSG
	_DBG("\n\rMB3021_BT_Module_EQ = ");
	_DBH(uBuf);
#endif	

	MB3021_BT_Module_Send_cmd_param(CMD_A2DP_USER_EQ_CONTROL_32, &uBuf);
}

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

