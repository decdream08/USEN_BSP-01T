
/**
 *
 * @file bt_F1DQ3021.h
 *
 * @author   ms kim
 *
 * @brief
 */

#ifndef BT_F1DQ3021_H
#define BT_F1DQ3021_H

#ifdef F1DQ3021_ENABLE
typedef enum {
	//BT_CMD_PACKET_WAITING,
	BT_CMD_PACKET_HEADER,
	BT_CMD_PACKET_TYPE,
	BT_CMD_PACKET_ID1,
	BT_CMD_PACKET_ID2,
	BT_CMD_PACKET_LENGTH,
#ifdef USEN_BT_SPK
	BT_CMD_PACKET_PARM_DISCOVERABLE,
	BT_CMD_PACKET_PARM_CONNECTABLE,
	BT_CMD_PACKET_PARM_OPMODE,
	BT_CMD_PACKET_PARM_BA_ASSOCIATION,
	BT_CMD_PACKET_PARM_SET_AUDIO_OUTPUT_PATH,
	BT_CMD_PACKET_PARM_GET_ANALOG_DETECTED,
	BT_CMD_PACKET_PARM_GET_PDL,
	BT_CMD_PACKET_PARM_GET_LAST_CONNECTION,
	BT_CMD_PACKET_PARM_SET_LAST_CONNECTION,
#endif //USEN_BT_SPK
	BT_CMD_PACKET_PARM,
	BT_CMD_PACKET_PARM1,
	BT_CMD_PACKET_PARM2,
	BT_CMD_PACKET_PARM3,
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

//0x00 : Analog, 0x01 : I2S
typedef enum {
	BT_AudioOut_Analog  = 0x00,
	BT_AudioOut_I2S		= 0x01
}BT_AudioOut;
//Function
Bool Get_Connection_State(void); //220217

void Set_BT_Pairing_Next_Stage(BTPairingNextStage Stage);
BTPairingNextStage Get_BT_Pairing_Next_Stage(BTPairingNextStage Stage);

void F1M22_BT_Module_Init(void);
void F1M22_BT_Module_Del_PDL_Set(void);
void F1M22_BT_Module_Set_Discoverable(Enable_t Enable);
void F1M22_BT_Module_Set_Connectable(Enable_t Enable);

void F1M22_BT_Module_Get_Analog_Detected_Send(void);
void F1M22_BT_Module_I2S_Set(BT_AudioOut audio_out); //0x00 : Analog, 0x01 : I2S
void F1M22_BT_Module_BA_Association_Set(void);
#ifdef SWITCH_BUTTON_KEY_ENABLE
void F1M22_BT_Module_Master_Set(Switch_Master_Slave_Mode mode);
#endif
void F1M22_BT_Module_Get_Auto_Resp(uint8_t uCount, uint8_t* uRecData); //F1DQ3021
void F1M22_BT_Module_Get_Resp(uint8_t uCount, uint8_t* uRecData);

void Do_taskUART(void);

#endif //BT_F1DQ3021_H

#endif //BT_MODULE_H

