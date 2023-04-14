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

#ifdef F1DQ3021_ENABLE
#ifdef TIMER21_LED_ENABLE
#include "led_display.h"
#endif
#ifdef UART_10_ENABLE
#include "serial.h"
#include "bt_F1DQ3021.h"

//Macro
#define HEADER_CHECKSUM_BYTE		6

#define PACKET_HEADER			0xAA
#define PACKET_TYPE_REQ		0x01
#define PACKET_TYPE_RSP		0x02
#define PACKET_TYPE_IND		0x03

#define PACKET_START_BYTE				0x00
#define PACKET_TYPE_BYTE				0x01
#define PACKET_ERROR_CODE_BYTE		0x05

#define PACKET_CMD_SHIFT_BIT			16

//Generic Message - REQ or RSP / 32 is composed of CMD + REQ_LEN BYTE VALUE + RSP_LEN BYTE VALUE
//Standard CMD
//0x000, 0x0001
#define CMD_GET_SWVER						0x0000UL
#define CMD_GET_SWVER_32					(0x000bUL |(CMD_GET_SWVER << PACKET_CMD_SHIFT_BIT))
#define CDM_SET_DEVICE_RESET				0x0001UL
#define CDM_SET_DEVICE_RESET_32			(0x0001UL |(CDM_SET_DEVICE_RESET << PACKET_CMD_SHIFT_BIT))

//Connection CMD
//0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027
#define CMD_GET_DISCOVERABLE				0x0020UL
#define CMD_GET_DISCOVERABLE_32			(0x0002UL |(CMD_GET_DISCOVERABLE << PACKET_CMD_SHIFT_BIT))
#define CMD_SET_DISCOVERABLE				0x0021UL
#define CMD_SET_DISCOVERABLE_32			(0x0101UL |(CMD_SET_DISCOVERABLE << PACKET_CMD_SHIFT_BIT))
#define CMD_GET_CONNECTABLE				0x0022UL
#define CMD_GET_CONNECTABLE_32			(0x0002UL |(CMD_GET_CONNECTABLE << PACKET_CMD_SHIFT_BIT))
#define CMD_SET_CONNECTABLE				0x0023UL
#define CMD_SET_CONNECTABLE_32			(0x0101UL |(CMD_SET_CONNECTABLE << PACKET_CMD_SHIFT_BIT))
#define CMD_GET_PDL						0x0024UL
#define CMD_GET_PDL_32						(0x0002UL |(CMD_GET_PDL << PACKET_CMD_SHIFT_BIT))
#define CMD_DEL_PDL						0x0025UL
#define CMD_DEL_PDL_32						(0x0102UL |(CMD_DEL_PDL << PACKET_CMD_SHIFT_BIT))
#define CMD_GET_LAST_CONNECTION			0x0026UL
#define CMD_GET_LAST_CONNECTION_32		(0x0007UL |(CMD_GET_LAST_CONNECTION << PACKET_CMD_SHIFT_BIT))
#define CMD_SET_LAST_CONNECTION			0x0027UL
#define CMD_SET_LAST_CONNECTION_32		(0x0101UL |(CMD_SET_LAST_CONNECTION << PACKET_CMD_SHIFT_BIT))


//BR/EDR CMD
//0x0040, 0x0041, 0x0042
#define CMD_GET_BT_BDADDR					0x0040UL
#define CMD_GET_BT_BDADDR_32				(0x0007UL |(CMD_GET_BT_BDADDR << PACKET_CMD_SHIFT_BIT))
#define CMD_GET_BT_NAME					0x0041UL
#define CMD_GET_BT_NAME_32				(0x000cUL |(CMD_GET_BT_NAME << PACKET_CMD_SHIFT_BIT))
#define CMD_SET_BT_NAME					0x0042UL
#define CMD_SET_BT_NAME_32				(0x0401UL |(CMD_SET_BT_NAME << PACKET_CMD_SHIFT_BIT))

//Broadcast CMD
//0x0080, 0x0081, 0x0082, 0x0083
#define CMD_GET_OPMODE					0x0080UL
#define CMD_GET_OPMODE_32				(0x0002UL |(CMD_GET_OPMODE << PACKET_CMD_SHIFT_BIT))
#define CMD_SET_OPMODE					0x0081UL
#define CMD_SET_OPMODE_32					(0x0101UL |(CMD_SET_OPMODE << PACKET_CMD_SHIFT_BIT))
#define CMD_GET_BA_ASSOCIATION			0x0082UL
#define CMD_GET_BA_ASSOCIATION_32		(0x0002UL |(CMD_GET_BA_ASSOCIATION << PACKET_CMD_SHIFT_BIT))
#define CMD_SET_BA_ASSOCIATION			0x0083UL
#define CMD_SET_BA_ASSOCIATION_32		(0x0101UL |(CMD_SET_BA_ASSOCIATION << PACKET_CMD_SHIFT_BIT))

//Audio CMD
//0x00A2, 0x00A3, 0x00A4
#define CMD_GET_AUDIO_OUT_PATH			0x00a2UL
#define CMD_GET_AUDIO_OUT_PATH_32		(0x0002UL |(CMD_GET_AUDIO_OUT_PATH << PACKET_CMD_SHIFT_BIT))
#define CMD_SET_AUDIO_OUT_PATH			0x00a3UL
#define CMD_SET_AUDIO_OUT_PATH_32		(0x0101UL |(CMD_SET_AUDIO_OUT_PATH << PACKET_CMD_SHIFT_BIT))
#define CMD_GET_ANALOG_DETECTED			0x00a4UL
#define CMD_GET_ANALOG_DETECTED_32		(0x0002UL |(CMD_GET_ANALOG_DETECTED << PACKET_CMD_SHIFT_BIT))

// Polling way check to get Receive Data
#define POLLING_GET_DISCOVERABLE				0x01 
#define POLLING_SET_DISCOVERABLE				0x02 
#define POLLING_GET_CONNECTABLE				0x04 
#define POLLING_SET_CONNECTABLE				0x08
#define POLLING_GET_PAIRED_LIST				0x10
#define POLLING_DEL_PAIRED_LIST				0x20
#define POLLING_GET_AUDIO_OUT_PATH			0x40
#define POLLING_SET_AUDIO_OUT_PATH			0x80
#define POLLING_GET_OPMODE					0x100
#define POLLING_SET_OPMODE					0x200
#define POLLING_GET_BA_ASSOCIATION			0x400
#define POLLING_SET_BA_ASSOCIATION			0x800
#define POLLING_GET_ANALOG_DETECTED			0x1000
#define POLLING_GET_LAST_CONNECTION			0x2000
#define POLLING_SET_LAST_CONNECTION			0x4000

//Error Code
#define CMD_RESP_SUCCESSFULLY				0x00
#define CMD_RESP_INITATED					0x01
#define CMD_RESP_REDUNDANT				0x02
#define CMD_RESP_INVALID_PARAM			0x03
#define CMD_WRONG_STATE					0x04
#define CMD_FAIL_UNKNOWN					0xff

typedef enum {
	GET_DISCOVERABLE = 0,
	SET_DISCOVERABLE,
	GET_CONNECTABLE,
	SET_CONNECTABLE,
	GET_PAIRED_LIST,
	SET_DEL_PAIRED_LIST,
	GET_AUDIO_OUT_PATH,
	SET_AUDIO_OUT_PATH,
	GET_OPMODE,
	SET_OPMODE,
	GET_BA_ASSOCIATION,
	SET_BA_ASSOCIATION,
	GET_ANALOG_DETECTED,
	GET_LAST_CONNECTION,
	SET_LAST_CONNECTION,
	GET_MAX
}StartCount;

typedef enum {
	BT_OPMODE_NORMAL,
	BT_OPMODE_BROADCASTER,
	BT_OPMODE_RECEIVER
}BT_OPMODE;

//Variable
Bool BBT_Is_Connected = FALSE;
Bool BDiscoverable_State = FALSE;
Bool BAudioout_I2S_State = FALSE;
Bool BBA_Association_State = FALSE;
Bool BAux_Detected_State = FALSE;
Bool BA2DP_MAC_ADDR_Exist = FALSE;
Bool BBA_Last_Connection_State = FALSE;
Bool BPaired_List_State = FALSE;

uint8_t uPDL_Number = 0;

BT_OPMODE Cur_BT_opmode;

static uint16_t bPolling_Get_Data = 0;
uint8_t uStart_Count_Array[GET_MAX];
uint8_t uA2DP_MAC_ADDR[6] = {0,};
BTPairingNextStage bBTPairingNextStage = BT_PAIRING_NONE; //For Disconnect --> Discoveralble --> Connectable Sequence

//Function
Bool Get_Connection_State(void) //220217
{
	return BBT_Is_Connected;
}

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
#ifdef BT_DEBUG_MSG
		_DBG("\n\rDo not use this function when parameter length is bigger than 0\n\r");
#endif
		return 0;
	}
	
	response_len = (code32) & 0xff;
	request_len = (code32 >> 8) & 0xff;

#ifdef BT_DEBUG_MSG
	_DBG("\nResp Len : ");_DBH(response_len);
#endif

	code = (code32 >> 16) & 0xffff;
 	
	buf[0] = PACKET_HEADER;
	buf[1] = PACKET_TYPE_REQ;
	buf[2] = (code & 0xff00) >> 8;
	buf[3] = (uint8_t)(code & 0x00ff);
	buf[4] = request_len;

	last_len = 5;

#ifdef BT_DEBUG_MSG
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

#ifdef BT_DEBUG_MSG
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
		_DBH(code);
		_DBG("\n\r");
	}
#endif

#ifdef BT_DEBUG_MSG
	_DBG("\nLen : ");_DBH(last_len);_DBG("\n\r");
#endif

	buf[last_len] = F1M22_Calculate_Checksum2(buf, last_len);

	Serial_Send(SERIAL_PORT10, buf, last_len+1);
	
	return response_len;
}


void F1M22_BT_Module_Init(void) //No need BT module Init. Just check ID
{	
	/* PC0 Output - MODULE_RESET */
	HAL_GPIO_ClearPin(PC, _BIT(0));
	delay_ms(500);
	HAL_GPIO_SetPin(PC, _BIT(0));
	delay_ms(500);
	//I2S Output
	F1M22_BT_Module_I2S_Set(BT_AudioOut_I2S);
	
	return;
}

void F1M22_BT_Module_Get_PDL_Send(void)
{
	F1M22_BT_Module_Send_cmd(CMD_GET_PDL_32);
}

void F1M22_BT_Module_Get_Analog_Detected_Send(void)
{
	F1M22_BT_Module_Send_cmd(CMD_GET_ANALOG_DETECTED_32);
}

void F1M22_BT_Module_Get_Last_Connection(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_SET_LAST_CONNECTION;
	uStart_Count_Array[SET_LAST_CONNECTION] = Count;	

	return;
}

void F1M22_BT_Module_Get_PDL(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_GET_PAIRED_LIST;
	uStart_Count_Array[GET_PAIRED_LIST] = Count;	

	return;
}

void F1M22_BT_Module_Get_Analog_Detected(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_GET_ANALOG_DETECTED;
	uStart_Count_Array[GET_ANALOG_DETECTED] = Count;	

	return;
}

void F1M22_BT_Module_Get_Audio_Output_Path(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_SET_AUDIO_OUT_PATH;
	uStart_Count_Array[SET_AUDIO_OUT_PATH] = Count;	

	return;
}

void F1M22_BT_Module_Get_OPMODE(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_SET_OPMODE;
	uStart_Count_Array[SET_OPMODE] = Count;	

	return;
}

void F1M22_BT_Module_Get_BA_Association(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_SET_BA_ASSOCIATION;
	uStart_Count_Array[SET_BA_ASSOCIATION] = Count;	

	return;
}

void F1M22_BT_Module_Get_Connectable(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_SET_CONNECTABLE;
	uStart_Count_Array[SET_CONNECTABLE] = Count;	

	return;
}

void F1M22_BT_Module_Get_Discoverable(uint8_t Count)
{
	bPolling_Get_Data |= POLLING_SET_DISCOVERABLE;
	uStart_Count_Array[SET_DISCOVERABLE] = Count;	

	return;
}

void F1M22_BT_Module_BA_Association_Set(void)
{
	uint8_t uData = 0;

	uData = FEATURE_ENABLE; //Enable
	F1M22_BT_Module_Send_cmd_param(CMD_SET_BA_ASSOCIATION_32, &uData, 1);

	bPolling_Get_Data |= POLLING_SET_BA_ASSOCIATION;
	uStart_Count_Array[SET_BA_ASSOCIATION] = uBuffer_Count;
}

void F1M22_BT_Module_Del_PDL_Set(void)
{
	uint8_t uData = 0;

	uData = 0xff; //Delete all paired list
	F1M22_BT_Module_Send_cmd_param(CMD_DEL_PDL_32, &uData, 1);

	bPolling_Get_Data |= POLLING_DEL_PAIRED_LIST;
	uStart_Count_Array[SET_DEL_PAIRED_LIST] = uBuffer_Count;
}

void F1M22_BT_Module_Last_Connection_Set(void)
{
	uint8_t uData;

	uData = 0x01;
#ifdef BT_DEBUG_MSG	
	_DBG("\n\rF1M22_BT_Module_Last_Connection_Set\n\r");
#endif
	F1M22_BT_Module_Send_cmd_param(CMD_SET_LAST_CONNECTION_32, &uData, 1); 
}

void F1M22_BT_Module_Master_Set(Switch_Master_Slave_Mode mode)
{
	uint8_t uData;
					
	if(mode == Switch_Master_Mode)
	{
		uData = 0x01;
#ifdef BT_DEBUG_MSG
		_DBG("\n\rF1M22_BT_Module_Master_Set : Master !!!\n\r");
#endif
	}
	else
	{	
		uData = 0x02;
#ifdef BT_DEBUG_MSG
		_DBG("\n\rF1M22_BT_Module_Master_Set : Slave !!!\n\r");
#endif
	}

	F1M22_BT_Module_Send_cmd_param(CMD_SET_OPMODE_32, &uData, 1); //Send CMD

	//bPolling_Get_Data |= POLLING_SET_OPMODE;
	//uStart_Count_Array[SET_OPMODE] = uBuffer_Count;
}

void F1M22_BT_Module_I2S_Set(BT_AudioOut audio_out) //0x00 : Analog, 0x01 : I2S
{
	uint8_t uData;
		
	if(audio_out == 0x01)
	{
		BAudioout_I2S_State = TRUE;
		uData = 0x01;
#ifdef BT_DEBUG_MSG
		_DBG("\n\rF1M22_BT_Module_I2S_Set : I2S !!!\n\r");
#endif
	}
	else
	{
		BAudioout_I2S_State = FALSE;
		uData = 0x00;
#ifdef BT_DEBUG_MSG
		_DBG("\n\rF1M22_BT_Module_I2S_Set : Analog !!!\n\r");
#endif
	} 

	F1M22_BT_Module_Send_cmd_param(CMD_SET_AUDIO_OUT_PATH_32, &uData, 1); //Send CMD

	bPolling_Get_Data |= POLLING_SET_AUDIO_OUT_PATH;
	uStart_Count_Array[SET_AUDIO_OUT_PATH] = uBuffer_Count;

	return;
}

void F1M22_BT_Module_Set_Discoverable(Enable_t Enable) //USEN_BT_SPK
{
	uint8_t uEnable;

	uEnable = Enable;
	
#ifdef BT_DEBUG_MSG					
	if(uEnable == 0x00)
		_DBG("\n\rSET_DISCOVERABLE Disable !!!\n\r");
	else
		_DBG("\n\rSET_DISCOVERABLE Enable !!!\n\r");
#endif

	F1M22_BT_Module_Send_cmd_param(CMD_SET_DISCOVERABLE_32, &uEnable, 1);

	//bPolling_Get_Data |= POLLING_SET_DISCOVERABLE;
	//uStart_Count_Array[SET_DISCOVERABLE] = uBuffer_Count;
	
	return;
}

void F1M22_BT_Module_Set_Connectable(Enable_t Enable) //USEN_BT_SPK
{
	uint8_t uEnable;

	uEnable = Enable;
	
#ifdef BT_DEBUG_MSG					
	if(uEnable == 0x00)
		_DBG("\n\rSET_CONNECTABLE Disable !!!\n\r");
	else
		_DBG("\n\rSET_CONNECTABLE Enable !!!\n\r");
#endif

	F1M22_BT_Module_Send_cmd_param(CMD_SET_CONNECTABLE_32, &uEnable, 1);

	//bPolling_Get_Data |= POLLING_SET_CONNECTABLE;
	//uStart_Count_Array[SET_CONNECTABLE] = uBuffer_Count;	

	return;
}

void F1M22_BT_Module_Get_Auto_Resp(uint8_t uCount, uint8_t* uRecData) //F1DQ3021
{
	static uint8_t next_state = 0, ID_state = 0;
	static uint8_t uDiscoverable_Start_Addr = 0;
	static uint8_t uConnectable_Start_Addr = 0;
	static uint8_t uOpmode_Start_Addr = 0;
	static uint8_t uBA_Association_Start_Addr = 0;
	
	//Try to get the MAC address of AVRCP device
	switch(next_state)
	{
		case BT_CMD_PACKET_HEADER:
			if(*uRecData == 0xaa)
			{
				ID_state = 0;
				uDiscoverable_Start_Addr = uCount;
				uConnectable_Start_Addr = uCount;
				uOpmode_Start_Addr = uCount;
				uBA_Association_Start_Addr = uCount;
				next_state = BT_CMD_PACKET_TYPE;
			}
		break;

		case BT_CMD_PACKET_TYPE:
			if(*uRecData == 0x03)
				next_state = BT_CMD_PACKET_ID1;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

//ID - 0x0021 : DISCOVERABLE, 0x0023 : CONNECTABLE, 0x0081 : SET_OPMODE, 0x0083 : SET_BA_ASSOCIATION
		case BT_CMD_PACKET_ID1:
			if(*uRecData == 0x00)
				next_state = BT_CMD_PACKET_ID2;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_ID2: //Do not confuse ID_state and next_state
			if(*uRecData == 0x21) 
				ID_state = BT_CMD_PACKET_PARM_DISCOVERABLE;
			else if(*uRecData == 0x23)
				ID_state = BT_CMD_PACKET_PARM_CONNECTABLE;
			else if(*uRecData == 0x81)
				ID_state = BT_CMD_PACKET_PARM_OPMODE;
			else if(*uRecData == 0x83)
				ID_state = BT_CMD_PACKET_PARM_BA_ASSOCIATION;
			else
			{
				next_state = BT_CMD_PACKET_HEADER;
				ID_state = 0;
			}

			if(next_state != BT_CMD_PACKET_HEADER)
				next_state = BT_CMD_PACKET_LENGTH;
		break;

		case BT_CMD_PACKET_LENGTH:
			if(*uRecData == 0x01)
				next_state = BT_CMD_PACKET_PARM;
			else
				next_state = BT_CMD_PACKET_HEADER;

		case BT_CMD_PACKET_PARM:
		{
			switch(ID_state)
			{
				case BT_CMD_PACKET_PARM_DISCOVERABLE:
					F1M22_BT_Module_Get_Discoverable(uDiscoverable_Start_Addr);
					next_state = BT_CMD_PACKET_HEADER;
					break;
					
				case BT_CMD_PACKET_PARM_CONNECTABLE:
					F1M22_BT_Module_Get_Connectable(uConnectable_Start_Addr);
					next_state = BT_CMD_PACKET_HEADER;
					break;
					
				case BT_CMD_PACKET_PARM_OPMODE:
					F1M22_BT_Module_Get_OPMODE(uOpmode_Start_Addr);
					next_state = BT_CMD_PACKET_HEADER;
					break;
					
				case BT_CMD_PACKET_PARM_BA_ASSOCIATION:
					F1M22_BT_Module_Get_BA_Association(uBA_Association_Start_Addr); //Need to get only 6 Byte address
					next_state = BT_CMD_PACKET_HEADER;
					break;
				default:
					next_state = BT_CMD_PACKET_HEADER;
					break;
			}
		}
		break;

		default:
				next_state = BT_CMD_PACKET_HEADER;
			break;
	}
}


void F1M22_BT_Module_Get_Resp(uint8_t uCount, uint8_t* uRecData) //F1DQ3021
{
	static uint8_t next_state = 0, ID_state = 0;
	static uint8_t uSet_Audio_Output_Path = 0, uGet_Analog_Detected = 0, uGet_PDL = 0, uGet_Last_Con = 0;
	
	switch(next_state)
	{
		case BT_CMD_PACKET_HEADER:
			if(*uRecData == 0xaa)
			{
				ID_state = 0;
				uSet_Audio_Output_Path = uCount;
				uGet_Analog_Detected = uCount;
				uGet_PDL = uCount;
				uGet_Last_Con = uCount;
				next_state = BT_CMD_PACKET_TYPE;
			}
		break;

		case BT_CMD_PACKET_TYPE:
			if(*uRecData == 0x02)
				next_state = BT_CMD_PACKET_ID1;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

//ID - 0x0080 : GET_OPMODE
		case BT_CMD_PACKET_ID1:
			if(*uRecData == 0x00)
				next_state = BT_CMD_PACKET_ID2;
			else
				next_state = BT_CMD_PACKET_HEADER;
		break;

		case BT_CMD_PACKET_ID2: //Do not confuse ID_state and next_state
			if(*uRecData == 0xa2) 
				ID_state = BT_CMD_PACKET_PARM_SET_AUDIO_OUTPUT_PATH;
			else if(*uRecData == 0xa4)
				ID_state = BT_CMD_PACKET_PARM_GET_ANALOG_DETECTED;
			else if(*uRecData == 0x24)
				ID_state = BT_CMD_PACKET_PARM_GET_PDL;
			else if(*uRecData == 0x27)
				ID_state = BT_CMD_PACKET_PARM_SET_LAST_CONNECTION;
			else
			{
				next_state = BT_CMD_PACKET_HEADER;
				ID_state = 0;
			}

			if(next_state != BT_CMD_PACKET_HEADER)
				next_state = BT_CMD_PACKET_LENGTH;
		break;

		case BT_CMD_PACKET_LENGTH:
			if(*uRecData != 0x00) // upto 3 Parmeters
				next_state = BT_CMD_PACKET_PARM;
			else
				next_state = BT_CMD_PACKET_HEADER;

		case BT_CMD_PACKET_PARM:
		{
			switch(ID_state)
			{
				case BT_CMD_PACKET_PARM_SET_AUDIO_OUTPUT_PATH:
					next_state = BT_CMD_PACKET_PARM1;
					break;
				case BT_CMD_PACKET_PARM_GET_ANALOG_DETECTED:
					next_state = BT_CMD_PACKET_PARM1;
					break;
				case BT_CMD_PACKET_PARM_GET_PDL:
					next_state = BT_CMD_PACKET_PARM1;
					break;
				case BT_CMD_PACKET_PARM_SET_LAST_CONNECTION:
					F1M22_BT_Module_Get_Last_Connection(uGet_Last_Con);
					next_state = BT_CMD_PACKET_HEADER;
					break;
				default:
					next_state = BT_CMD_PACKET_HEADER;
					break;
			}
		}
		break;
		case BT_CMD_PACKET_PARM1:
		{
			switch(ID_state)
			{
				case BT_CMD_PACKET_PARM_SET_AUDIO_OUTPUT_PATH:
					next_state = BT_CMD_PACKET_PARM2;
					break;
				case BT_CMD_PACKET_PARM_GET_ANALOG_DETECTED:
					F1M22_BT_Module_Get_Analog_Detected(uGet_Analog_Detected);
					next_state = BT_CMD_PACKET_HEADER;
					break;
				case BT_CMD_PACKET_PARM_GET_PDL:
					F1M22_BT_Module_Get_PDL(uGet_PDL);
					next_state = BT_CMD_PACKET_HEADER;
					break;

				default:
					next_state = BT_CMD_PACKET_HEADER;
					break;
			}
		}
		break;
		case BT_CMD_PACKET_PARM2:
		{
			switch(ID_state)
			{
				case BT_CMD_PACKET_PARM_SET_AUDIO_OUTPUT_PATH:
					F1M22_BT_Module_Get_Audio_Output_Path(uSet_Audio_Output_Path);
					next_state = BT_CMD_PACKET_HEADER;
					break;
				default:
					next_state = BT_CMD_PACKET_HEADER;
					break;
			}
		}
		break;

		default:
				next_state = BT_CMD_PACKET_HEADER;
			break;
	}
}

void Do_taskUART(void) //Just check UART receive data from Buffer
{
	uint8_t uBuf[20] = {0,}, i, uResp_len = 0, uChecksum = 0;
	static uint32_t uCount = 0;
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

	if((bPolling_Get_Data & POLLING_GET_LAST_CONNECTION) || (bPolling_Get_Data & POLLING_SET_LAST_CONNECTION))
	{
		if(bPolling_Get_Data & POLLING_GET_LAST_CONNECTION)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_GET_LAST_CONNECTION - check !!!");
#endif
			uResp_len = CMD_GET_LAST_CONNECTION_32& 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_LAST_CONNECTION]);
		}
		else //POLLING_SET_LAST_CONNECTION
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_SET_LAST_CONNECTION - check !!!");
#endif
			uResp_len = CMD_SET_LAST_CONNECTION_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_LAST_CONNECTION]);
		}
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_TYPE_BYTE] == 0x02)
				{
					if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					{
						ret = TRUE;
						
						if(bPolling_Get_Data & POLLING_GET_LAST_CONNECTION) //7 byte Mac address;
						{
							for(i=0; i<6; i++) //Mac Address size is 6 Byte
								uA2DP_MAC_ADDR[i] = uBuf[i+6];

								BA2DP_MAC_ADDR_Exist = TRUE;
						}
					}
					else //A response is error
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\rPOLLING_SET_LAST_CONNECTION - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						if(bPolling_Get_Data & POLLING_GET_LAST_CONNECTION) //7 byte Mac address;
						{
							for(i=0; i<6; i++) //Mac Address size is 6 Byte
								uA2DP_MAC_ADDR[i] = 0;

							BA2DP_MAC_ADDR_Exist = FALSE;
						}
						ret = FALSE;
					}
				}
				else if(uBuf[PACKET_TYPE_BYTE] == 0x03)
				{
					ret = TRUE;
					
					switch(uBuf[PACKET_ERROR_CODE_BYTE])
					{
						case 0x00:
							BBA_Last_Connection_State = FALSE;
							break;
						case 0x01:
							BBA_Last_Connection_State = TRUE;
							break;
						default:
							ret = FALSE;
							break;
					}
				}
				else
					ret = FALSE;

				if(ret)
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_SET_LAST_CONNECTION - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}
							
				if(bPolling_Get_Data & POLLING_GET_LAST_CONNECTION)
				{
					bPolling_Get_Data &= (~POLLING_GET_LAST_CONNECTION); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_LAST_CONNECTION]);
				}
				else
				{
					bPolling_Get_Data &= (~POLLING_SET_LAST_CONNECTION); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_LAST_CONNECTION]);
				}
			}
		}
	}
	
	if(bPolling_Get_Data & POLLING_GET_PAIRED_LIST)
	{
		//if(bPolling_Get_Data & POLLING_GET_ANALOG_DETECTED)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rCMD_GET_PDL_32 - check !!!");
#endif
			uResp_len = CMD_GET_PDL_32& 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_PAIRED_LIST]);
		}
#if 0
		else //POLLING_SET_BA_ASSOCIATION
		{
#ifdef _BT_MODULE_DEBUG_MSG
			_DBG("\n\rCMD_GET_ANALOG_DETECTED_32 - check !!!");
#endif
			uResp_len = CMD_SET_BA_ASSOCIATION_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_BA_ASSOCIATION]);
		}
#endif
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_TYPE_BYTE] == 0x02)
				{
					if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					{
						ret = TRUE;
						
						if(bPolling_Get_Data & POLLING_GET_PAIRED_LIST)
						{
							uPDL_Number = uBuf[PACKET_ERROR_CODE_BYTE+1];

							if(uPDL_Number)
								BPaired_List_State = TRUE;
							else
								BPaired_List_State = FALSE;
								
#ifdef BT_DEBUG_MSG
						_DBG("\n\rCMD_GET_PDL_32 - uPDL_Number : ");
						_DBD(uPDL_Number);_DBG("\n\r");
#endif		
						}
					}
					else //A response is error
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\rCMD_GET_PDL_32 - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						ret = FALSE;
					}
				}
				else if(uBuf[PACKET_TYPE_BYTE] == 0x03)
				{
					ret = TRUE;
					
					//To Do !!! : Paired Lists
				}
				else
					ret = FALSE;

				if(ret)
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rCMD_GET_PDL_32 - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}
				else
					BPaired_List_State = FALSE;
					
				if(BPaired_List_State)
					F1M22_BT_Module_Last_Connection_Set(); //Power On Step -3
				//if(bPolling_Get_Data & POLLING_GET_ANALOG_DETECTED)
				{
					bPolling_Get_Data &= (~POLLING_GET_PAIRED_LIST); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_PAIRED_LIST]);
				}
#if 0
				else
				{
					bPolling_Get_Data &= (~POLLING_SET_BA_ASSOCIATION); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_BA_ASSOCIATION]);
				}
#endif
			}
		}
	}

	if(bPolling_Get_Data & POLLING_GET_ANALOG_DETECTED)
	{
		//if(bPolling_Get_Data & POLLING_GET_ANALOG_DETECTED)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rCMD_GET_ANALOG_DETECTED_32 - check !!!");
#endif
			uResp_len = CMD_GET_ANALOG_DETECTED_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_ANALOG_DETECTED]);
		}
#if 0
		else //POLLING_SET_BA_ASSOCIATION
		{
#ifdef _BT_MODULE_DEBUG_MSG
			_DBG("\n\rCMD_GET_ANALOG_DETECTED_32 - check !!!");
#endif
			uResp_len = CMD_SET_BA_ASSOCIATION_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_BA_ASSOCIATION]);
		}
#endif
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_TYPE_BYTE] == 0x02)
				{
					if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					{
						ret = TRUE;
						
						if(bPolling_Get_Data & POLLING_GET_ANALOG_DETECTED)
						{
							switch(uBuf[PACKET_ERROR_CODE_BYTE+1])
							{
								case 0x00:
									BAux_Detected_State = FALSE;
									break;
								case 0x01:
									BAux_Detected_State = TRUE;
									break;
								default:
									ret = FALSE;
									break;
							}
						}
					}
					else //A response is error
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\rCMD_GET_ANALOG_DETECTED_32 - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						ret = FALSE;
					}
				}
				else if(uBuf[PACKET_TYPE_BYTE] == 0x03)
				{
					ret = TRUE;
					
					switch(uBuf[PACKET_ERROR_CODE_BYTE])
					{
						case 0x00:
							BAux_Detected_State = FALSE;
							break;
						case 0x01:
							BAux_Detected_State = TRUE;
							break;
						default:
							ret = FALSE;
							break;
					}
				}
				else
					ret = FALSE;

				if(ret)
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rCMD_GET_ANALOG_DETECTED_32 - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}
							
				//if(bPolling_Get_Data & POLLING_GET_ANALOG_DETECTED)
				{
					bPolling_Get_Data &= (~POLLING_GET_ANALOG_DETECTED); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_ANALOG_DETECTED]);
				}
#if 0
				else
				{
					bPolling_Get_Data &= (~POLLING_SET_BA_ASSOCIATION); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_BA_ASSOCIATION]);
				}
#endif
			}
		}
	}

	if((bPolling_Get_Data & POLLING_GET_BA_ASSOCIATION) || (bPolling_Get_Data & POLLING_SET_BA_ASSOCIATION))
	{
		if(bPolling_Get_Data & POLLING_GET_BA_ASSOCIATION)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_GET_BA_ASSOCIATION - check !!!");
#endif
			uResp_len = CMD_GET_BA_ASSOCIATION_32& 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_BA_ASSOCIATION]);
		}
		else //POLLING_SET_BA_ASSOCIATION
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_SET_BA_ASSOCIATION - check !!!");
#endif
			uResp_len = CMD_SET_BA_ASSOCIATION_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_BA_ASSOCIATION]);
		}
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_TYPE_BYTE] == 0x02)
				{
					if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					{
						ret = TRUE;
						
						if(bPolling_Get_Data & POLLING_GET_BA_ASSOCIATION)
						{
							switch(uBuf[PACKET_ERROR_CODE_BYTE+1])
							{
								case 0x00:
									BBA_Association_State = FALSE;
									break;
								case 0x01:
									BBA_Association_State = TRUE;
									break;
								default:
									ret = FALSE;
									break;
							}
						}
					}
					else //A response is error
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\rPOLLING_SET_BA_ASSOCIATION - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						ret = FALSE;
					}
				}
				else if(uBuf[PACKET_TYPE_BYTE] == 0x03)
				{
					ret = TRUE;
					
					switch(uBuf[PACKET_ERROR_CODE_BYTE])
					{
						case 0x00:
							BBA_Association_State = FALSE;
							break;
						case 0x01:
							BBA_Association_State = TRUE;
							break;
						default:
							ret = FALSE;
							break;
					}
				}
				else
					ret = FALSE;

				if(ret)
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_SET_BA_ASSOCIATION - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}
							
				if(bPolling_Get_Data & POLLING_GET_BA_ASSOCIATION)
				{
					bPolling_Get_Data &= (~POLLING_GET_BA_ASSOCIATION); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_BA_ASSOCIATION]);
				}
				else
				{
					bPolling_Get_Data &= (~POLLING_SET_BA_ASSOCIATION); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_BA_ASSOCIATION]);
				}
			}
		}
	}
	
	if((bPolling_Get_Data & POLLING_GET_OPMODE) || (bPolling_Get_Data & POLLING_SET_OPMODE))
	{
		if(bPolling_Get_Data & POLLING_GET_OPMODE)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_GET_OPMODE - check !!!");
#endif
			uResp_len = CMD_GET_OPMODE_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_OPMODE]);
		}
		else //POLLING_SET_OPMODE
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_SET_OPMODE - check !!!");
#endif
			uResp_len = CMD_SET_OPMODE_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_OPMODE]);
		}
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_TYPE_BYTE] == 0x02)
				{
					if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					{
						ret = TRUE;
						
						if(bPolling_Get_Data & POLLING_GET_OPMODE)
						{
							switch(uBuf[PACKET_ERROR_CODE_BYTE+1])
							{
								case 0x00:
									Cur_BT_opmode = BT_OPMODE_NORMAL;
									break;
								case 0x01:
									Cur_BT_opmode = BT_OPMODE_BROADCASTER;
									break;
								case 0x02:
									Cur_BT_opmode = BT_OPMODE_RECEIVER;
									break;
								default:
									ret = FALSE;
									break;
							}
						}
					}
					else //A response is error
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\rPOLLING_SET_OPMODE - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						ret = FALSE;
					}
				}
				else if(uBuf[PACKET_TYPE_BYTE] == 0x03)
				{
					ret = TRUE;
					
					switch(uBuf[PACKET_ERROR_CODE_BYTE])
					{
						case 0x00:
							Cur_BT_opmode = BT_OPMODE_NORMAL;
							break;
						case 0x01:
							Cur_BT_opmode = BT_OPMODE_BROADCASTER;
							break;
						case 0x02:
							Cur_BT_opmode = BT_OPMODE_RECEIVER;
							break;
						default:
							ret = FALSE;
							break;
					}
				}
				else
					ret = FALSE;

				if(ret)
				{
					Switch_Master_Slave_Mode mode;
					mode = Get_Cur_Master_Slave_Mode();

					if(mode == Switch_Master_Mode)
					{
						if(Cur_BT_opmode != BT_OPMODE_BROADCASTER)
							F1M22_BT_Module_Master_Set(mode);
						else
							F1M22_BT_Module_Get_PDL_Send(); //if Master mode, Send GET_PDL : Power On Step - 2 but If Slave mode, the step stops.
					}
					else //Switch_Slave_Mode
					{
						if(Cur_BT_opmode != BT_OPMODE_RECEIVER)
							F1M22_BT_Module_Master_Set(mode);
					}
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_SET_OPMODE - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
					//F1M22_BT_Module_BA_Association_Set();
				}
							
				if(bPolling_Get_Data & POLLING_GET_OPMODE)
				{
					bPolling_Get_Data &= (~POLLING_GET_OPMODE); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_OPMODE]);
				}
				else //POLLING_SET_OPMODE
				{
					bPolling_Get_Data &= (~POLLING_SET_OPMODE); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_OPMODE]);
				}
			}
		}
	}
	
	if((bPolling_Get_Data & POLLING_GET_DISCOVERABLE) || (bPolling_Get_Data & POLLING_SET_DISCOVERABLE))
	{
		if(bPolling_Get_Data & POLLING_GET_DISCOVERABLE)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_GET_DISCOVERABLE - check !!!");
#endif
			uResp_len = CMD_GET_DISCOVERABLE_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_DISCOVERABLE]);
		}
		else
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_SET_DISCOVERABLE - check !!!");
#endif
			uResp_len = CMD_SET_DISCOVERABLE_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_DISCOVERABLE]);
		}
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_TYPE_BYTE] == 0x02)
				{
					if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
						ret = TRUE;
					else //A response is error
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\rPOLLING_GET_DISCOVERABLE - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						ret = FALSE;
					}
				}
				else if(uBuf[PACKET_TYPE_BYTE] == 0x03)
				{
					switch(uBuf[PACKET_ERROR_CODE_BYTE])
					{
						case 0x00:
							BDiscoverable_State = FALSE;
							ret = TRUE;
							break;
						case 0x01:
							BDiscoverable_State = TRUE;
							ret = TRUE;
							break;
						default:
							ret = FALSE;
							break;
					}
				}
				else
					ret = FALSE;

				if(ret)
				{
#ifdef BT_DEBUG_MSG
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
							
				if(bPolling_Get_Data & POLLING_GET_DISCOVERABLE)
				{
					bPolling_Get_Data &= (~POLLING_GET_DISCOVERABLE); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_DISCOVERABLE]);
				}
				else
				{
					bPolling_Get_Data &= (~POLLING_SET_DISCOVERABLE); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_DISCOVERABLE]);
				}
			}
		}
	}

	if((bPolling_Get_Data & POLLING_GET_CONNECTABLE) || (bPolling_Get_Data & POLLING_SET_CONNECTABLE))
	{
#ifdef BT_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_CONNECTABLE - check !!!");
#endif
		if(bPolling_Get_Data & POLLING_GET_CONNECTABLE)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_GET_CONNECTABLE - check !!!");
#endif
			uResp_len = CMD_GET_CONNECTABLE_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_CONNECTABLE]);
		}
		else
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_SET_CONNECTABLE - check !!!");
#endif
			uResp_len = CMD_SET_CONNECTABLE_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_CONNECTABLE]);
		}
				
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];


		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_TYPE_BYTE] == 0x02)
				{
					if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
						ret = TRUE;
					else //A response is error
					{
#ifdef BT_DEBUG_MSG
						_DBG("\n\rPOLLING_GET_CONNECTABLE - NG !!! ++ Error code : ");
						_DBH(uBuf[5]);_DBG("\n\r");
#endif
						ret = FALSE;
					}
				}
				else if(uBuf[PACKET_TYPE_BYTE] == 0x03)
				{
					switch(uBuf[PACKET_ERROR_CODE_BYTE])
					{
						case 0x00:
							BBT_Is_Connected = TRUE; //connected!!
							ret = TRUE;
							break;
						case 0x01:
							BBT_Is_Connected = FALSE; //disconneted!!
							ret = TRUE;
							break;
						default:
							ret = FALSE;
							break;
					}
				}
				else
					ret = FALSE;

				if(ret)
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_CONNECTABLE - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
					bBTPairingNextStage = BT_PAIRING_NONE;
#ifdef TIMER21_LED_ENABLE
					if(BBT_Is_Connected)
						Set_Status_LED_Mode(STATUS_BT_PAIRED_MODE);
#endif
					
				}
				
				if(bPolling_Get_Data & POLLING_GET_CONNECTABLE)
				{
					bPolling_Get_Data &= (~POLLING_GET_CONNECTABLE); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_CONNECTABLE]);
				}
				else
				{
					bPolling_Get_Data &= (~POLLING_SET_CONNECTABLE); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_CONNECTABLE]);
				}
			}
		}
	}

	if(bPolling_Get_Data & POLLING_DEL_PAIRED_LIST)
	{
		uResp_len = CMD_DEL_PDL_32 & 0xff; //Get receive data length

#ifdef BT_DEBUG_MSG
		_DBG("\n\rPOLLING_GET_DEL_PAIRED_LIST - check !!!");
#endif
		Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_DEL_PAIRED_LIST]);
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];

		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
					ret = TRUE;
				else //A response is error
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_DEL_PAIRED_LIST - NG !!! ++ Error code : ");
					_DBH(uBuf[5]);_DBG("\n\r");
#endif
					ret = FALSE;
				}

				if(ret)
				{
					Switch_Master_Slave_Mode mode;

					mode = Get_Cur_Master_Slave_Mode();
					
					_DBD(mode);
					if(mode == Switch_Master_Mode)
						F1M22_BT_Module_Set_Discoverable(FEATURE_ENABLE);					
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_GET_DEL_PAIRED_LIST - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
				}
				
				bPolling_Get_Data &= (~POLLING_DEL_PAIRED_LIST); //Clear flag
				Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_DEL_PAIRED_LIST]);
			}
		}
	}

	/* Check Flag : Audio Out Path */
	if((bPolling_Get_Data & POLLING_GET_AUDIO_OUT_PATH) || (bPolling_Get_Data & POLLING_SET_AUDIO_OUT_PATH))
	{
		if(bPolling_Get_Data & POLLING_GET_AUDIO_OUT_PATH)
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_GET_AUDIO_OUT_PATH - check !!!");
#endif
			uResp_len = CMD_GET_AUDIO_OUT_PATH_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_AUDIO_OUT_PATH]);
		}
		else
		{
#ifdef BT_DEBUG_MSG
			_DBG("\n\rPOLLING_SET_AUDIO_OUT_PATH - check !!!");
#endif
			uResp_len = CMD_SET_AUDIO_OUT_PATH_32 & 0xff; //Get receive data length
			Serial_Data_Get(uBuf, uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_AUDIO_OUT_PATH]);
		}
		
		uChecksum = uBuf[uResp_len+HEADER_CHECKSUM_BYTE-1];
		
		if(uBuf[0] == PACKET_HEADER) //All CMD has start byte as 0xAA
		{
			if(uChecksum == F1M22_Calculate_Checksum2(uBuf, (uResp_len+HEADER_CHECKSUM_BYTE-1))) //Need to check 12 byte(remove checksum)
			{
				if(uBuf[PACKET_ERROR_CODE_BYTE] == CMD_RESP_SUCCESSFULLY) //A response is successful
				{
					ret = TRUE;
					
					if(bPolling_Get_Data & POLLING_GET_AUDIO_OUT_PATH)
					{
						if(uBuf[PACKET_ERROR_CODE_BYTE+1] == 0x01)
							BAudioout_I2S_State = TRUE;
						else
							BAudioout_I2S_State = FALSE;
					}
				}
				else //A response is error
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_AUDIO_OUT_PATH - NG !!! ++ Error code : ");
					_DBH(uBuf[5]);_DBG("\n\r");
#endif
					ret = FALSE;
				}

				if(ret)
				{
#ifdef BT_DEBUG_MSG
					_DBG("\n\rPOLLING_AUDIO_OUT_PATH - OK !!!\n\r");
					
					for(i=0;i<uResp_len+HEADER_CHECKSUM_BYTE;i++)
						_DBH(uBuf[i]);

					_DBG("\n\r");
#endif
					//Master/Slave mode
					//F1M22_BT_Module_Master_Set(Get_Cur_Master_Slave_Mode());
				}

				if(bPolling_Get_Data & POLLING_GET_AUDIO_OUT_PATH)
				{
					bPolling_Get_Data &= (~POLLING_GET_AUDIO_OUT_PATH); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[GET_AUDIO_OUT_PATH]);
				}
				else
				{
					bPolling_Get_Data &= (~POLLING_SET_AUDIO_OUT_PATH); //Clear flag
					Serial_Data_Clear(uResp_len+HEADER_CHECKSUM_BYTE, uStart_Count_Array[SET_AUDIO_OUT_PATH]);
				}
				
				bBTPairingNextStage = BT_PAIRING_NONE;				
				//F1M22_BT_Module_Get_Paired_Device_ID(); //Try to get the address of connected device
			}
		}
	}
}

#endif //UART_10_ENABLE

#endif //F1DQ3021_ENABLE

