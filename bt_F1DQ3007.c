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

Hardware requirement: f1media - F1M22\n
Port Configuration: 

 ****************************************************************************************
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "main_conf.h"

#ifdef F1DQ3007_ENABLE
#ifdef TIMER21_LED_ENABLE
#include "led_display.h"
#endif
#ifdef UART_10_ENABLE
#include "serial.h"
#include "bt_F1DQ3007.h"

//Macro
#define HEADER_CHECKSUM_BYTE		6

#define PACKET_HEADER			0xAA
#define PACKET_TYPE_REQ		0x01
#define PACKET_TYPE_RSP		0x02
#define PACKET_TYPE_IND		0x03

#define PACKET_START_BYTE				0x00
#define PACKET_ERROR_CODE_BYTE		0x05

//Generic Message - REQ or RSP / 32 is composed of CMD + REQ_LEN + RSP_LEN
#define CMD_F1_GEN_SET_BTNAME_32			0x00010000
#define CMD_F1_GEN_SET_BTNAME			0x0001
#define CMD_F1_GET_BTNAME_32				0x0002000c
#define CMD_F1_GET_BTNAME				0x0002
#define CMD_F1_GEN_SET_FEATURES			0x0003
#define CMD_F1_GEN_SET_FEATURES_32			0x00030201
#define CMD_F1_GEN_SET_FEATURES_REQ_LEN			0x02
#define CMD_F1_GEN_SET_FEATURES_RSP_LEN			0x01
#define CMD_F1_GEN_GET_FEATURES			0x0004
#define CMD_F1_GEN_GET_FEATURES_32			0x00040003
#define CMD_F1_GEN_GET_FEATURES_REQ_LEN			0x00
#define CMD_F1_GEN_GET_FEATURES_RSP_LEN			0x03
#define CMD_F1_GEN_SET_DICOVERABLE		0x0005
#define CMD_F1_GEN_SET_DICOVERABLE_32		0x00050101
#define CMD_F1_GEN_SET_DICOVERABLE_REQ_LEN			0x01
#define CMD_F1_GEN_SET_DICOVERABLE_RSP_LEN			0x01
#define CMD_F1_GEN_SET_CONNECTABLE		0x0006
#define CMD_F1_GEN_SET_CONNECTABLE_32		0x00060101
#define CMD_F1_GEN_SET_CONNECTABLE_REQ_LEN		0x01
#define CMD_F1_GEN_SET_CONNECTABLE_RSP_LEN		0x01
#define CMD_F1_GEN_GET_PARIRED_LIST		0x0007
#define CMD_F1_GEN_GET_PARIRED_LIST_32		0x00070002
#define CMD_F1_GEN_GET_PARIRED_LIST_REQ_LEN		0x00
#define CMD_F1_GEN_GET_PARIRED_LIST_RSP_LEN		0x02
#define CMD_F1_GEN_DEL_PAIRED_LIST		0x0008
#define CMD_F1_GEN_DEL_PAIRED_LIST_32		0x00080101
#define CMD_F1_GEN_DEL_PAIRED_LIST_REQ_LEN		0x01
#define CMD_F1_GEN_DEL_PAIRED_LIST_RSP_LEN		0x01
#define CMD_F1_GEN_READ_SW_VER			0x0009
#define CMD_F1_GEN_READ_SW_VER_32			0x000900ff
#define CMD_F1_GEN_READ_SW_VER_REQ_LEN			0x00
#define CMD_F1_GEN_READ_SW_VER_RSP_LEN			0xff //N

#define CMD_F1_GEN_READ_BDADDR			0x000A
#define CMD_F1_GEN_READ_BDADDR_32			0x000A0007
#define CMD_F1_GEN_READ_BDADDR_REQ_LEN			0x00
#define CMD_F1_GEN_READ_BDADDR_RSP_LEN			0x07
#define CMD_F1_GEN_LAST_CONNECT			0x000B
#define CMD_F1_GEN_LAST_CONNECT_32			0x000B0001
#define CMD_F1_GEN_LAST_CONNECT_REQ_LEN			0x00
#define CMD_F1_GEN_LAST_CONNECT_RSP_LEN			0x01
#define CMD_F1_GEN_ALL_DISCONNECT		0x000C
#define CMD_F1_GEN_ALL_DISCONNECT_32		0x000C0001
#define CMD_F1_GEN_ALL_DISCONNECT_REQ_LEN		0x00
#define CMD_F1_GEN_ALL_DISCONNECT_RSP_LEN		0x01
#define CMD_F1_GEN_SET_AUDIO_INTERFACE			0x000D
#define CMD_F1_GEN_SET_AUDIO_INTERFACE_32			0x000D0601
#define CMD_F1_GEN_SET_AUDIO_INTERFACE_REQ_LEN			0x06
#define CMD_F1_GEN_SET_AUDIO_INTERFACE_RSP_LEN			0x01
#define CMD_F1_GEN_GET_AUDIO_INTERFACE		0x000E
#define CMD_F1_GEN_GET_AUDIO_INTERFACE_32		0x000E0007
#define CMD_F1_GEN_GET_AUDIO_INTERFACE_REQ_LEN		0x00
#define CMD_F1_GEN_GET_AUDIO_INTERFACE_RSP_LEN		0x07
#define CMD_F1_GEN_SET_DEBUG				0x000F
#define CMD_F1_GEN_SET_DEBUG_32				0x000F0101
#define CMD_F1_GEN_SET_DEBUG_REQ_LEN				0x01
#define CMD_F1_GEN_SET_DEBUG_RSP_LEN				0x01
#define CMD_F1_GEN_GET_DEBUG				0x0010
#define CMD_F1_GEN_GET_DEBUG_32				0x00100002
#define CMD_F1_GEN_GET_DEBUG_REQ_LEN				0x00
#define CMD_F1_GEN_GET_DEBUG_RSP_LEN				0x02
#define CMD_F1_GEN_SET_BLE_SECURE		0x0011
#define CMD_F1_GEN_SET_BLE_SECURE_32		0x00110101
#define CMD_F1_GEN_SET_BLE_SECURE_REQ_LEN		0x01
#define CMD_F1_GEN_SET_BLE_SECURE_RSP_LEN		0x01
#define CMD_F1_GEN_GET_BLE_SECURE		0x0012
#define CMD_F1_GEN_GET_BLE_SECURE_32		0x00120002
#define CMD_F1_GEN_GET_BLE_SECURE_REQ_LEN		0x00
#define CMD_F1_GEN_GET_BLE_SECURE_RSP_LEN		0x02
#define CMD_F1_GEN_SET_TEST_RSSI			0x0013
#define CMD_F1_GEN_SET_TEST_RSSI_32			0x00130101
#define CMD_F1_GEN_SET_TEST_RSSI_REQ_LEN			0x01
#define CMD_F1_GEN_SET_TEST_RSSI_RSP_LEN			0x01
#define CMD_F1_GEN_GET_TEST_RSSI			0x0014
#define CMD_F1_GEN_GET_TEST_RSSI_32			0x00140002
#define CMD_F1_GEN_GET_TEST_RSSI_REQ_LEN			0x00
#define CMD_F1_GEN_GET_TEST_RSSI_RSP_LEN			0x02
#define CMD_F1_GEN_PLAY_TONE				0x0015
#define CMD_F1_GEN_PLAY_TONE_32				0x00150101
#define CMD_F1_GEN_PLAY_TONE_REQ_LEN				0x01
#define CMD_F1_GEN_PLAY_TONE_RSP_LEN				0x01
#define CMD_F1_GEN_STOP_TONE				0x0016
#define CMD_F1_GEN_STOP_TONE_32				0x00160001
#define CMD_F1_GEN_STOP_TONE_REQ_LEN				0x00
#define CMD_F1_GEN_STOP_TONE_RSP_LEN				0x01
#define CMD_F1_GEN_AUDIO_PROMPT_PLAY		0x0017
#define CMD_F1_GEN_AUDIO_PROMPT_PLAY_32		0x00170101
#define CMD_F1_GEN_AUDIO_PROMPT_PLAY_REQ_LEN		0x01
#define CMD_F1_GEN_AUDIO_PROMPT_PLAY_RSP_LEN		0x01

//Indiacation -IND
#define CMD_F1_GEN_STARTUP_IND			0x0000
#define CMD_F1_GEN_STARTUP_IND_LEN			0x01
#define CMD_F1_GEN_PAIRED_DEVCIE_IND		0x0007
#define CMD_F1_GEN_PAIRED_DEVCIE_IND_REQ_LEN		0x01
#define CMD_F1_GEN_PAIRED_DEVCIE_IND_RSP_LEN		0x07
#define CMD_F1_GEN_PAIRED_DEVCIE_IND_32				0x00070107 //This will be implemented up to N but T.B.D

//#define CMD_F1_GEN_PAIRED_DEVCIE_IND_LEN		N//0x0001
#define CMD_F1_TEST_RSSI_IND				0x0008
#define CMD_F1_TEST_RSSI_IND_LEN				0x07

//A2DP - REQ or RSP
#define CMD_F1_A2DP_CONNECTION			0x0100
#define CMD_F1_A2DP_CONNECTION_32		0x01000701
#define CMD_F1_A2DP_CONNECTION_REQ_LEN			0x07
#define CMD_F1_A2DP_CONNECTION_RSP_LEN			0x01
#define CMD_F1_A2DP_STREAM_CONTROL		0x0101
#define CMD_F1_A2DP_STREAM_CONTROL_32		0x01010701
#define CMD_F1_A2DP_STREAM_CONTROL_REQ_LEN		0x07
#define CMD_F1_A2DP_STREAM_CONTROL_RSP_LEN		0x01
#define CMD_F1_A2DP_CONTROL				0x0101
#define CMD_F1_A2DP_CONTROL_REQ_LEN		0x07
#define CMD_F1_A2DP_CONTROL_RSP_LEN		0x01

//A2DP Indication
#define CMD_F1_A2DP_STATUS_IND			0x0100
#define CMD_F1_A2DP_STATUS_IND_LEN			0x07
#define CMD_F1_A2DP_STREAM_CONTROL_IND			0x0101
#define CMD_F1_A2DP_STREAM_CONTROL_IND_LEN			0x07
#define CMD_F1_CODEC_IND					0x0102
#define CMD_F1_CODEC_IND_LEN					0x07

//AVRCP Indication
#define CMD_F1_AVRCP_CONNECTION			0x0200
#define CMD_F1_AVRCP_CONNECTION_32			0x02000701
#define CMD_F1_AVRCP_CONNECTION_REQ_LEN		0x07
#define CMD_F1_AVRCP_CONNECTION_RSP_LEN		0x01
#define CMD_F1_AVRCP_BUTTON				0x0201
#define CMD_F1_AVRCP_BUTTON_32				0x02010801
#define CMD_F1_AVRCP_BUTTON_REQ_LEN		0x08
#define CMD_F1_AVRCP_BUTTON_RSP_LEN		0x01

//Error Code
#define CMD_RESP_SUCCESSFULLY				0x00
#define CMD_RESP_INITATED					0x01
#define CMD_RESP_REDUNDANT				0x02
#define CMD_RESP_INVALID_PARAM			0x03
#define CMD_WRONG_STATE					0x04
#define CMD_FAIL_UNKNOWN					0xff

// Polling way check to get Receive Data
#define POLLING_GET_INIT						0x01
#define POLLING_GET_AVRCP_DEVICE_ID			0x02
#define POLLING_GET_AVRCP_BUTTON				0x04
#define POLLING_GET_LAST_CONNECT				0x08
#define POLLING_GET_PAIRED_DEVICE_ID			0x10
#define POLLING_GET_ALL_DISCONNECT			0x20
#define POLLING_GET_DISCOVERABLE				0x40
#define POLLING_GET_CONNECTABLE				0x80
#define POLLING_GET_A2DP_DEVICE_ID			0x100
#define POLLING_GET_DEL_PAIRED_LIST			0x200

typedef enum {
	GET_INIT = 0,
	GET_PAIRED_DEVICE_ID,
	GET_AVRCP_DEVICE_ID,
	GET_AVRCP_BUTTON,
	GET_LAST_CONNECT,
	GET_ALL_DISCONNECT,
	GET_DISCOVERABLE,
	GET_CONNECTABLE,
	GET_A2DP_DEVICE_ID,
	GET_DEL_PAIRED_LIST,
	GET_MAX
}StartCount;

//Variable
uint8_t uAVRCP_MAC_ADDR[6] = {0,};
uint8_t uA2DP_MAC_ADDR[6] = {0,};
Bool uAVRCP_MAC_ADDR_Exist = FALSE;
Bool uA2DP_MAC_ADDR_Exist = FALSE;
static uint16_t bPolling_Get_Data = 0;
uint8_t uStart_Count_Array[GET_MAX];
BTPairingNextStage bBTPairingNextStage = BT_PAIRING_NONE; //For Disconnect --> Discoveralble --> Connectable Sequence

//Function

void Set_BT_Pairing_Next_Stage(BTPairingNextStage Stage)
{
	bBTPairingNextStage = Stage;
}

BTPairingNextStage Get_BT_Pairing_Next_Stage(BTPairingNextStage Stage)
{
	return bBTPairingNextStage;
}

static uint16_t F1M22_Calculate_Checksum2(uint8_t *data, uint16_t data_length)
{
	uint8_t checksum = 0x00;
	uint32_t i = 0;

	for(i=0;i<data_length;i++)
		checksum += data[i];

	checksum = 0x100 - (checksum & 0xff);

	return checksum;
}

uint8_t F1M22_BT_Module_Send_cmd(uint32_t code32) //Just in case of Request Length = 0
{
	uint8_t buf[32] = {0,};
	uint16_t code = 0;
	uint8_t request_len = 0, response_len = 0, last_len = 0;

	if(buf[4] > 0)
	{
#ifdef F1DQ3007_DEBUG_MSG
		_DBG("\n\rDo not use this function when parameter length is bigger than 0\n\r");
#endif
		return 0;
	}
	
	response_len = (code32) & 0xff;
	request_len = (code32 >> 8) & 0xff;

#ifdef F1DQ3007_DEBUG_MSG
	_DBG("\nResp Len : ");_DBH(response_len);
#endif

	code = (code32 >> 16) & 0xffff;
 	
	buf[0] = PACKET_HEADER;
	buf[1] = PACKET_TYPE_REQ;
	buf[2] = (code & 0xff00) >> 8;
	buf[3] = (uint8_t)(code & 0x00ff);
	buf[4] = request_len;

	last_len = 5;

#ifdef F1DQ3007_DEBUG_MSG
	_DBG("\nLen : ");_DBH(last_len);_DBG("\n\r");
#endif

	buf[last_len] = F1M22_Calculate_Checksum2(buf, last_len);

	Serial_Send(SERIAL_PORT10, buf, last_len+1);
	
	return response_len;
}

uint8_t F1M22_BT_Module_Send_cmd_param(uint32_t code32, uint8_t *param, uint8_t param_size)
{
	uint8_t buf[32] = {0,};
	uint16_t code = 0;
	uint8_t request_len = 0, response_len = 0, i = 0, last_len = 0;

	response_len = (code32) & 0xff;
	request_len = (code32 >> 8) & 0xff;

#ifdef F1DQ3007_DEBUG_MSG
	_DBG("\nResp Len : ");_DBH(response_len);
#endif
	
	code = (code32 >> 16) & 0xffff;
 	
	buf[PACKET_START_BYTE] = PACKET_HEADER; //Header(1Byte) : 0xaa
	buf[1] = PACKET_TYPE_REQ; // Type(1Byte) : REQ=0x1, RSP=0x2, IND=0x3
	buf[2] = (code & 0xff00) >> 8; //ID(2Byte)
	buf[3] = (uint8_t)(code & 0x00ff); //ID continue...
	buf[4] = request_len; //Param Size(1Byte)

	if(buf[4] != 0)
	{
		for(i = 0; i<request_len; i++)
			buf[i+5] = param[i]; //to do list : parameter
			last_len = i+5;
	}
#ifdef F1DQ3007_DEBUG_MSG
	else
	{
		_DBG("\n\rUART Send Error : ID = ");
		_DBH(code);
		_DBG("\n\r");
	}
#endif

#ifdef F1DQ3007_DEBUG_MSG
	_DBG("\nLen : ");_DBH(last_len);_DBG("\n\r");
#endif

	buf[last_len] = F1M22_Calculate_Checksum2(buf, last_len);

	Serial_Send(SERIAL_PORT10, buf, last_len+1);
	
	return response_len;
}


void F1M22_BT_Module_Init(void) //No need BT module Init. Just check ID
{	
	F1M22_BT_Module_Send_cmd(CMD_F1_GET_BTNAME_32); //Send CMD

	/* Setting Receive data using polling way */
	uStart_Count_Array[GET_INIT] = uBuffer_Count;
	bPolling_Get_Data |= POLLING_GET_INIT;
	
	return;
}

void F1M22_BT_Module_Last_Connect(void)
{
	F1M22_BT_Module_Send_cmd(CMD_F1_GEN_LAST_CONNECT_32);

	bPolling_Get_Data |= POLLING_GET_LAST_CONNECT;
	uStart_Count_Array[GET_LAST_CONNECT] = uBuffer_Count;

	return;
}

void F1M22_BT_Module_AVRCP_Button(AVRCP_Key KEY)
{
	uint8_t uParam[8] = {0,}, i;
	
	//F1M22_BT_Module_Send_cmd(CMD_F1_AVRCP_BUTTON_32); // Need to add param

	if(uAVRCP_MAC_ADDR_Exist)
	{
		for(i=0; i<6; i++) //Mac Address size is 6 Byte
			uParam[i] = uAVRCP_MAC_ADDR[i];

		uParam[i++] = 0x02; //press and release
		uParam[i] = KEY;
		
		F1M22_BT_Module_Send_cmd_param(CMD_F1_AVRCP_BUTTON_32, uParam, 8);

		bPolling_Get_Data |= POLLING_GET_AVRCP_BUTTON;
		uStart_Count_Array[GET_AVRCP_BUTTON] = uBuffer_Count;
	}
	
	return;
}

void F1M22_BT_Module_Get_AVRCP_Device_ID(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_GET_AVRCP_DEVICE_ID;
	uStart_Count_Array[GET_AVRCP_DEVICE_ID] = Count;	

	return;
}

void F1M22_BT_Module_Get_A2DP_Device_ID(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_GET_A2DP_DEVICE_ID;
	uStart_Count_Array[GET_A2DP_DEVICE_ID] = Count;	

	return;
}

void F1M22_BT_Module_Get_Paired_Device_ID(void)
{
	F1M22_BT_Module_Send_cmd(CMD_F1_GEN_GET_PARIRED_LIST_32);

	bPolling_Get_Data |= POLLING_GET_PAIRED_DEVICE_ID;
	uStart_Count_Array[GET_PAIRED_DEVICE_ID] = uBuffer_Count;	

	return;
}

void F1M22_BT_Module_All_Disconnect(void)
{
	F1M22_BT_Module_Send_cmd(CMD_F1_GEN_ALL_DISCONNECT_32);

	bPolling_Get_Data = 0;
	bPolling_Get_Data |= POLLING_GET_ALL_DISCONNECT;
	uStart_Count_Array[GET_ALL_DISCONNECT] = uBuffer_Count;	

	return;
}

void F1M22_BT_Module_Del_Paired_List(void)
{
	uint8_t uData = 0;

	uData = 0xff; //Delete all paired list
	F1M22_BT_Module_Send_cmd_param(CMD_F1_GEN_DEL_PAIRED_LIST_32, &uData, 1);

	bPolling_Get_Data |= POLLING_GET_DEL_PAIRED_LIST;
	uStart_Count_Array[GET_DEL_PAIRED_LIST] = uBuffer_Count;
}

void F1M22_BT_Module_Set_Discoverable(Enable_t Enable)
{
	uint8_t uEnable;

	uEnable = Enable;
	
#ifdef F1DQ3007_DEBUG_MSG					
	if(uEnable == 0x00)
		_DBG("\n\rSET_DISCOVERABLE Disable !!!\n\r");
	else
		_DBG("\n\rSET_DISCOVERABLE Enable !!!\n\r");
#endif
	
	F1M22_BT_Module_Send_cmd_param(CMD_F1_GEN_SET_DICOVERABLE_32, &uEnable, 1);

	bPolling_Get_Data |= POLLING_GET_DISCOVERABLE;
	uStart_Count_Array[GET_DISCOVERABLE] = uBuffer_Count;
	
	return;
}

void F1M22_BT_Module_Set_Connectable(Enable_t Enable)
{
	uint8_t uEnable;

	uEnable = Enable;
	
#ifdef F1DQ3007_DEBUG_MSG					
	if(uEnable == 0x00)
		_DBG("\n\rSET_CONNECTABLE Disable !!!\n\r");
	else
		_DBG("\n\rSET_CONNECTABLE Enable !!!\n\r");
#endif

	F1M22_BT_Module_Send_cmd_param(CMD_F1_GEN_SET_CONNECTABLE_32, &uEnable, 1);

	bPolling_Get_Data |= POLLING_GET_CONNECTABLE;
	uStart_Count_Array[GET_CONNECTABLE] = uBuffer_Count;	

	return;
}

void F1M22_BT_Module_Get_AVRCP_Mac_Address(uint8_t uCount, uint8_t* uRecData)
{
	static uint8_t next_state = 0;
	static uint8_t uAVRCP_Mac_Start_Addr = 0;
	
	//Try to get the MAC address of AVRCP device
	switch(next_state)
	{
		case BT_CMD_PACKET_HEADER:
			if(*uRecData == 0xaa)
			{
				uAVRCP_Mac_Start_Addr = uCount;
				next_state = BT_CMD_PACKET_TYPE;
			}
		break;

		case BT_CMD_PACKET_TYPE:
			if(*uRecData == 0x03)
				next_state = BT_CMD_PACKET_ID;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_ID:
			if(*uRecData == 0x02)
				next_state = BT_CMD_PACKET_LENGTH;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_LENGTH:
			if(*uRecData == 0x00)
				next_state = BT_CMD_PACKET_PARM;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_PARM:
			if(*uRecData == 0x07)
				F1M22_BT_Module_Get_AVRCP_Device_ID(uAVRCP_Mac_Start_Addr); //Need to get only 6 Byte address
			else
				next_state = BT_CMD_PACKET_HEADER;

		default:
				next_state = BT_CMD_PACKET_HEADER;
			break;
	}
}

void F1M22_BT_Module_A2DP_Connection(uint8_t uCount, uint8_t* uRecData)
{
	static uint8_t next_state = 0;
	static uint8_t uA2DP_Connection_Start_Addr = 0;
	
	//Try to get the MAC address of AVRCP device
	switch(next_state)
	{
		case BT_CMD_PACKET_HEADER:
			if(*uRecData == 0xaa)
			{
				uA2DP_Connection_Start_Addr = uCount;
				next_state = BT_CMD_PACKET_TYPE;
			}
		break;

		case BT_CMD_PACKET_TYPE:
			if(*uRecData == 0x03)
				next_state = BT_CMD_PACKET_ID;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_ID:
			if(*uRecData == 0x01)
				next_state = BT_CMD_PACKET_LENGTH;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_LENGTH:
			if(*uRecData == 0x00)
				next_state = BT_CMD_PACKET_PARM;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_PARM:
			if(*uRecData == 0x07)
				F1M22_BT_Module_Get_A2DP_Device_ID(uA2DP_Connection_Start_Addr); //Need to get only 6 Byte address
			else
				next_state = BT_CMD_PACKET_HEADER;

		default:
				next_state = BT_CMD_PACKET_HEADER;
			break;
	}
}

void Do_taskUART(void) //Just check UART receive data from Buffer
{
	uint8_t uBuf[20] = {0,}, i, uResp_len = 0, uChecksum = 0;
	static uint32_t uCount = 0;
	static uint8_t uRetry1 = 0; //For retry action
#ifdef F1DQ3007_DEBUG_MSG
	static uint8_t uRetry2 = 0; //For retry action
#endif
	Bool ret;

	if(uCount == 0x7fff) //0xffff = 1s, 0xffff/2(0x7fff) = 500ms
	{
		uCount = 0;
	}
	else
	{
		uCount++;
		return;
	}

	/* Check Flag : INIT result (using Get BT Name) */
	if(bPolling_Get_Data & POLLING_GET_INIT)
	{
		char cDeviceName[11] = "F1BM Series";
#if 0//def F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_INIT - check !!!");
#endif
		uResp_len = CMD_F1_GET_BTNAME_32 & 0xff; //Get receive data length of BT Name
		
		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_INIT]);
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];
		
		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				ret = TRUE;
				
				for(i=0; i<11;i++)
				{
					if(uBuf[i+HEADER_CHECKSUM_BYTE] != cDeviceName[i])
					{
						ret = FALSE;
#ifdef F1DQ3007_DEBUG_MSG					
						_DBG("\n\rBT Modue Init - NG !!!\n\r");

						_DBH(i);
						_DBH(uBuf[i+6]);
						_DBH(cDeviceName[i]);
#endif
						F1M22_BT_Module_Init(); //For recovery
						
						break;
					}
				}

				if(ret)
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rBT Modue Init - OK !!!\n\r");
#endif
					bPolling_Get_Data &= (~POLLING_GET_INIT); //Clear flag
					uRetry1 = 0;
					
					F1M22_BT_Module_Last_Connect(); //Try to connect the last connection device
				}
				else
				{
					if(uRetry1 == 10) //Try 10 times
					{
						bPolling_Get_Data &= (~POLLING_GET_INIT); //Clear flag
						uRetry1 = 0;
					}
					else
						uRetry1++;
				}

				Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_INIT]);
			}
		}

	}

	/* Check Flag : LAST_CONNECT */
	if(bPolling_Get_Data & POLLING_GET_LAST_CONNECT)
	{
#if 0//def F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_LAST_CONNECT - check !!!");
#endif
		uResp_len = CMD_F1_GEN_LAST_CONNECT_32 & 0xff; //Get receive data length of LAST CONNECT
		
		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_LAST_CONNECT]);

		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];
		
		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					ret = TRUE;
				else //A response is error
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rF1M22_BT_Module_Last_Connect - NG !!! ++ Error code : ");
					_DBH(uBuf[5]);_DBG("\n\r");
#endif
					ret = FALSE;
				}

				if(ret)
				{
#ifdef TIMER21_LED_ENABLE
					Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);
#endif
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rBT Modue LAST CONNECT - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}

				//We don't do the retry-action here because the LAST connect can't receive CMD_RESP_SUCCESSFULLY when the LAST connect device isn't ready.
				
				bPolling_Get_Data &= (~POLLING_GET_LAST_CONNECT); //Clear flag
				Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_LAST_CONNECT]);
#ifdef HIDE_CONNECTABLE_ENABLE //When we can't work LAST CONNECT, we should execute DISCOVERABLE & CONNECTABLE to connect other device.
					{
						bBTPairingNextStage = BT_PAIRING_DISCOVERABLE;
						F1M22_BT_Module_Set_Discoverable(FEATURE_ENABLE);
					}
#else //When we can't work LAST CONNECT, we should execute DISCOVERABLE & CONNECTABLE to connect other device.
				bBTPairingNextStage = BT_PAIRING_NONE;
#endif

#ifdef TIMER21_LED_ENABLE
				Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);
#endif
				//F1M22_BT_Module_Get_Paired_Device_ID(); //Try to get the address of connected device
			}
		}
	}

	if(bPolling_Get_Data & POLLING_GET_A2DP_DEVICE_ID)
	{
#if 0//def F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_A2DP_DEVICE_ID - check !!!");
#endif
		Serial_Data_Get(uBuf, 13, uStart_Count_Array[GET_A2DP_DEVICE_ID]);

		uChecksum = uBuf[13-1];

		if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (13 -1))) //Need to check 12 byte(remove checksum)
		{
			if(uBuf[11] == 0x01 || uBuf[11] == 0x00) //0x0 : connecting, 0x1 : connected, 0x2 : disconnected, 0x3 : failed connecting
			{
				for(i=0; i<6; i++) //Mac Address size is 6 Byte
					uA2DP_MAC_ADDR[i] = uBuf[i+5];

				uA2DP_MAC_ADDR_Exist = TRUE;
				ret = TRUE;
#ifdef F1DQ3007_DEBUG_MSG
				_DBG("\n\rA2DP CONNECTED !!!\n\r");
#endif
#ifdef HIDE_CONNECTABLE_ENABLE //When we success LAST CONNECT, we should execute DISCOVERABLE to avoid other device.
			bBTPairingNextStage = BT_PAIRING_NONE;
			F1M22_BT_Module_Set_Discoverable(FEATURE_DISABLE); //Disable discoverable to avoid finding this device from other and just keep only current connection
#endif
			}
			else if(uBuf[11] == 0x02) //disconnected
			{
				for(i=0; i<6; i++) //Mac Address size is 6 Byte
					uA2DP_MAC_ADDR[i] = 0;

				uA2DP_MAC_ADDR_Exist = FALSE;
				ret = TRUE;
#ifdef F1DQ3007_DEBUG_MSG
			_DBG("\n\rA2DP disconnected !!!\n\r");
#endif
			}
			else
				ret = FALSE;
		}
		else
			ret = FALSE;

		if(ret)
		{
#ifdef F1DQ3007_DEBUG_MSG
			_DBG("\n\rBT Modue POLLING_GET_A2DP_DEVICE_ID - OK !!!\n\r");

			for(i=0;i<6;i++)
				_DBH(uA2DP_MAC_ADDR[i]);
#endif
			bPolling_Get_Data &= (~POLLING_GET_A2DP_DEVICE_ID); //Clear flag
#ifdef F1DQ3007_DEBUG_MSG
			uRetry2 = 0; //Clear retry flag
#endif
			bBTPairingNextStage = BT_PAIRING_NONE;
			F1M22_BT_Module_Set_Discoverable(FEATURE_DISABLE); //Disable discoverable to avoid finding this device from other and just keep only current connection
		}
#ifdef F1DQ3007_DEBUG_MSG
		else
		{
			_DBG("\n\rBT Modue POLLING_GET_A2DP_DEVICE_ID - NG !!!\n\r");

			if(uRetry2 == 10) //Try 10 times
			{
				bPolling_Get_Data &= (~POLLING_GET_A2DP_DEVICE_ID); //Clear flag
				uRetry2 = 0; //Clear retry flag
			}
			else
				uRetry2++;
		}
#endif			
		Serial_Data_Clear(13, uStart_Count_Array[GET_A2DP_DEVICE_ID]);
	}
		
	if(bPolling_Get_Data & POLLING_GET_AVRCP_DEVICE_ID)
	{
#if 0//def F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_AVRCP_DEVICE_ID - check !!!");
#endif
		Serial_Data_Get(uBuf, 13, uStart_Count_Array[GET_AVRCP_DEVICE_ID]);

		uChecksum = uBuf[13-1];

		if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (13 -1))) //Need to check 12 byte(remove checksum)
		{
			if(uBuf[11] == 0x01 || uBuf[11] == 0x00) //0x0 : connecting, 0x1 : connected, 0x2 : disconnected, 0x3 : failed connecting
			{
				for(i=0; i<6; i++) //Mac Address size is 6 Byte
					uAVRCP_MAC_ADDR[i] = uBuf[i+5];

				uAVRCP_MAC_ADDR_Exist = TRUE;
				ret = TRUE;
#ifdef F1DQ3007_DEBUG_MSG
			_DBG("\n\rAVRCP CONNECTED !!!\n\r");
#endif
			}
			else if(uBuf[11] == 0x02) //disconnected
			{
				for(i=0; i<6; i++) //Mac Address size is 6 Byte
					uAVRCP_MAC_ADDR[i] = 0;

				uAVRCP_MAC_ADDR_Exist = FALSE;
				ret = TRUE;
#ifdef F1DQ3007_DEBUG_MSG
			_DBG("\n\rAVRCP DISCONNECTED !!!\n\r");
#endif
			}
			else
				ret = FALSE;
		}
		else
			ret = FALSE;

		if(ret)
		{
#ifdef F1DQ3007_DEBUG_MSG
			_DBG("\n\rBT Modue GET_AVRCP_DEVICE_ID - OK !!!\n\r");

			for(i=0;i<6;i++)
				_DBH(uAVRCP_MAC_ADDR[i]);
#endif
			bPolling_Get_Data &= (~POLLING_GET_AVRCP_DEVICE_ID); //Clear flag
#ifdef F1DQ3007_DEBUG_MSG
			uRetry2 = 0; //Clear retry flag
#endif
#ifdef HIDE_CONNECTABLE_ENABLE
			bBTPairingNextStage = BT_PAIRING_NONE;
			F1M22_BT_Module_Set_Discoverable(FEATURE_DISABLE); //Disable discoverable to avoid finding this device from other and just keep only current connection
#endif
		}
#ifdef F1DQ3007_DEBUG_MSG
		else
		{
			_DBG("\n\rBT Modue GET_AVRCP_DEVICE_ID - NG !!!\n\r");

			if(uRetry2 == 10) //Try 10 times
			{
				bPolling_Get_Data &= (~POLLING_GET_AVRCP_DEVICE_ID); //Clear flag
				uRetry2 = 0; //Clear retry flag
			}
			else
				uRetry2++;
		}
#endif			
		Serial_Data_Clear(13, uStart_Count_Array[GET_AVRCP_DEVICE_ID]);
	}

	if(bPolling_Get_Data & POLLING_GET_ALL_DISCONNECT)
	{
#ifdef F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_ALL_DISCONNECT - check !!!");
#endif
		uResp_len = CMD_F1_GEN_ALL_DISCONNECT_32 & 0xff; //Get receive data length

		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_ALL_DISCONNECT]);
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];


		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					ret = TRUE;
				else //A response is error
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_ALL_DISCONNECT - NG !!! ++ Error code : ");
					_DBH(uBuf[5]);_DBG("\n\r");
#endif
					ret = FALSE;
				}

				if(ret)
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_ALL_DISCONNECT - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}

#if 0
				/* BT Pairing Button Action : When BT connection is none, ret is FALSE so we need to implement in both case(FALSE/TRUE) */
				bBTPairingNextStage = TRUE;
				F1M22_BT_Module_Set_Discoverable(FEATURE_ENABLE);
#endif				
				//We don't do the retry-action here because the LAST connect can't receive CMD_RESP_SUCCESSFULLY when the LAST connect device isn't ready.
				
				bPolling_Get_Data &= (~POLLING_GET_ALL_DISCONNECT); //Clear flag
				Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_ALL_DISCONNECT]);

#ifdef TIMER21_LED_ENABLE
				Set_Status_LED_Mode(STATUS_BT_PAIRING_MODE);
#endif
				//F1M22_BT_Module_Get_Paired_Device_ID(); //Try to get the address of connected device
			}
		}
	}

	if(bPolling_Get_Data & POLLING_GET_DISCOVERABLE)
	{
#ifdef F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_DISCOVERABLE - check !!!");
#endif
		uResp_len = CMD_F1_GEN_SET_DICOVERABLE_32 & 0xff; //Get receive data length

		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_DISCOVERABLE]);
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];


		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					ret = TRUE;
				else //A response is error
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_DISCOVERABLE - NG !!! ++ Error code : ");
					_DBH(uBuf[5]);_DBG("\n\r");
#endif
					ret = FALSE;
				}

				if(ret)
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_DISCOVERABLE - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
					if(bBTPairingNextStage == BT_PAIRING_DISCOVERABLE)
					{
						bBTPairingNextStage = BT_PAIRING_CONNECTABLE;
						F1M22_BT_Module_Set_Connectable(FEATURE_ENABLE);
					}
				}
				
				bPolling_Get_Data &= (~POLLING_GET_DISCOVERABLE); //Clear flag
				Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_DISCOVERABLE]);
			}
		}
	}

	if(bPolling_Get_Data & POLLING_GET_CONNECTABLE)
	{
#ifdef F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_CONNECTABLE - check !!!");
#endif
		uResp_len = CMD_F1_GEN_SET_CONNECTABLE_32 & 0xff; //Get receive data length

		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_CONNECTABLE]);
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];


		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					ret = TRUE;
				else //A response is error
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_CONNECTABLE - NG !!! ++ Error code : ");
					_DBH(uBuf[5]);_DBG("\n\r");
#endif
					ret = FALSE;
				}

				if(ret)
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_CONNECTABLE - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
					bBTPairingNextStage = BT_PAIRING_NONE;
				}
				
				bPolling_Get_Data &= (~POLLING_GET_CONNECTABLE); //Clear flag
				Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_CONNECTABLE]);
			}
		}
	}

	if(bPolling_Get_Data & POLLING_GET_DEL_PAIRED_LIST)
	{
#ifdef F1DQ3007_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_DEL_PAIRED_LIST - check !!!");
#endif
		uResp_len = CMD_F1_GEN_DEL_PAIRED_LIST_32 & 0xff; //Get receive data length

		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_DEL_PAIRED_LIST]);
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];


		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					ret = TRUE;
				else //A response is error
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_DEL_PAIRED_LIST - NG !!! ++ Error code : ");
					_DBH(uBuf[5]);_DBG("\n\r");
#endif
					ret = FALSE;
				}

				if(ret)
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_DEL_PAIRED_LIST - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}
				
				bPolling_Get_Data &= (~POLLING_GET_DEL_PAIRED_LIST); //Clear flag
				Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_DEL_PAIRED_LIST]);
			}
		}
	}

#if 0
	if(bPolling_Get_Data & POLLING_GET_PAIRED_DEVICE_ID)
	{
		uint8_t uList_len = 0;

		uResp_len = CMD_F1_GEN_GET_PARIRED_LIST_32 & 0xff; //Get receive data length of PAIRED DEVICE
		_DBD(uStart_Count_Array[GET_PAIRED_DEVICE_ID]);
		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_PAIRED_DEVICE_ID]);

		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		ret = FALSE;
		
		//Resp packet - 1
		if((uBuf[0] == PACKET_HEADER) && (uBuf[1] == 0x02) && (uBuf[2] == 0x00) && (uBuf[3] == 0x07))
		{ //Check Resp : AA-02-00-07
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
				{
					ret = TRUE;
					uList_len = uBuf[6]; //Number of connected devices
				}
				else //A response is error
				{
#ifdef F1DQ3007_DEBUG_MSG
					_DBG("\n\rCMD_F1_GEN_GET_PARIRED_LIST - NG !!! ++ Error code : ");
					_DBH(uBuf[PACKET_ERROR_CODE_BYTE]);_DBG("\n\r");
#endif
					ret = FALSE;
				}
			}
		}

		
		if(ret)
		{
#ifdef F1DQ3007_DEBUG_MSG
			_DBG("\n\rCMD_F1_GEN_GET_PARIRED_LIST - 1st Resp - OK !!!\n\r");
			
			for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
				_DBH(uBuf[i]);

			_DBG("\n\r");
#endif
			//Resp packet - Last packet(this is depended on number of paired device which is "uList_len")
			//Each Lists have 13 Byte packet. So, the start address of last packet should be added (uList_len -1) * 13 + 8 Byte
			Serial_Data_Get(uBuf, 13, (uStart_Count_Array[GET_PAIRED_DEVICE_ID]+8+(13*(uList_len-1))));

			uChecksum = uBuf[13-1];

#ifdef F1DQ3007_DEBUG_MSG	
			for(i=0;i<13;i++)
				_DBH(uBuf[i]);

			_DBG("\n\r");
#endif
			//The last packet. We suppose the last packet(MAC address) is connecting with BT Module.
			if((uBuf[0] == PACKET_HEADER) && (uBuf[1] == 0x03) && (uBuf[2] == 0x00) && (uBuf[3] == 0x07))
			{
				if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (13 -1))) //Need to check 12 byte(remove checksum)
				{
					if(uBuf[5] == (uList_len-1)) //A response is successful
					{
						ret = TRUE;
						
						for(i=0; i<6; i++) //Mac Address size is 6 Byte
							uMAC_ADDR[i] = uBuf[i+6];
					}
					else //A response is error
					{
#ifdef F1DQ3007_DEBUG_MSG
						_DBG("\n\rCMD_F1_GEN_GET_PARIRED_LIST / GET MAC_ADDR - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						ret = FALSE;
					}

					if(ret)
					{
#ifdef F1DQ3007_DEBUG_MSG
						_DBG("\n\rCMD_F1_GEN_GET_PARIRED_LIST / GET MAC_ADDR - OK !!! ");
						
						for(i=0;i<6;i++)
							_DBH(uMAC_ADDR[i]);

						_DBG("\n\r");
#endif
						bPolling_Get_Data &= (~POLLING_GET_PAIRED_DEVICE_ID); //Clear flag
					}
				}
			}
		}
	}
#endif
}

#endif //UART_10_ENABLE
#endif //F1DQ3007_ENABLE

