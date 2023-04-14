
/**
 *
 * @file bt_F1DQ3007.h
 *
 * @author   ms kim
 *
 * @brief
 */

#ifndef BT_F1DQ3007_H
#define BT_F1DQ3007_H

#ifdef F1DQ3007_ENABLE

typedef enum {
	AVRCP_POWER = 0x40,
	AVRCP_VOLUME_UP, /* 0x41 */
	AVRCP_VOLUME_DOWN, /* 0x42 */
	AVRCP_MUTE, /* 0x43 */
	AVRCP_PLAY, /* 0x44 */
	AVRCP_STOP, /* 0x45 */
	AVRCP_PAUSE, /* 0x46 */
	AVRCP_RECORD, /* 0x47 */
	AVRCP_REWIND, /* 0x48 */
	AVRCP_FAST_FORWARD, /* 0x49 */
	AVRCP_EJECT, /* 0x4A */
	AVRCP_FORWARD, /* 0x4B */
	AVRCP_BACKWARD, /* 0x4C */
	AVRCP_PLAY_PAUSE = 0xff
} AVRCP_Key;

typedef enum {
	//BT_CMD_PACKET_WAITING,
	BT_CMD_PACKET_HEADER,
	BT_CMD_PACKET_TYPE,
	BT_CMD_PACKET_ID,
	BT_CMD_PACKET_LENGTH,
	BT_CMD_PACKET_PARM,
	BT_CMD_PACKET_CHECKSUM
}BT_CMD_PACKET_STATUS;

typedef enum {
	FEATURE_DISABLE = 0x0,
	FEATURE_ENABLE = 0x1
}Enable_t;

typedef enum {
	BT_PAIRING_NONE,
	BT_PAIRING_DISCONNECT,
	BT_PAIRING_DISCOVERABLE,
	BT_PAIRING_CONNECTABLE
}BTPairingNextStage;

//Function

void Set_BT_Pairing_Next_Stage(BTPairingNextStage Stage);
BTPairingNextStage Get_BT_Pairing_Next_Stage(BTPairingNextStage Stage);

void F1M22_BT_Module_Init(void);
void F1M22_BT_Module_Del_Paired_List(void);
void F1M22_BT_Module_Get_Paired_Device_ID(void);
void F1M22_BT_Module_Last_Connect(void);
void F1M22_BT_Module_All_Disconnect(void);
void F1M22_BT_Module_Set_Discoverable(Enable_t Enable);
void F1M22_BT_Module_Set_Connectable(Enable_t Enable);
void F1M22_BT_Module_Get_AVRCP_Device_ID(uint8_t Count);
void F1M22_BT_Module_AVRCP_Button(AVRCP_Key KEY);
void F1M22_BT_Module_Get_AVRCP_Mac_Address(uint8_t uCount, uint8_t* uRecData);

void Do_taskUART(void);

#endif //F1DQ3007_ENABLE

#endif //BT_F1DQ3007_H

