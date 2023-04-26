
/**
 *
 * @file bt_MB3021.h
 *
 * @author   ms kim
 *
 * @brief
 */

#ifndef BT_MB3021_H
#define BT_MB3021_H

#ifdef MB3021_ENABLE
#ifdef FIVE_USER_EQ_ENABLE
#ifdef AD82584F_ENABLE
#include "ad82584f.h"
#endif
#ifdef TAS5806MD_ENABLE
#include "tas5806md.h" 
#endif
#endif


#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
typedef enum {
	input_key_Sync_Power, 		//0
	input_key_Sync_Mute, 		//1
	input_key_Sync_Volume, 		//2
	input_key_Sync_EQ, 			//3
	input_key_Sync_Reboot, 		//4
	input_key_Sync_Factory_Reset,  //5
#ifdef SLAVE_ADD_MUTE_DELAY_ENABLE
	Input_key_Sync_Slave_Mute_Off  //6
#endif
}Input_Key_Sync_With_Slave;
#endif

#ifdef MASTER_SLAVE_GROUPING
typedef enum {
	GROUPING_NONE_MODE,
	GROUPING_EVENT_WAIT_STATE,
	GROUPING_MASTER_NORMAL_MODE,
	GROUPING_MASTER_BROADCASTER_MODE,
	GROUPING_MASTER_SET_MANUFACTURE_DATA,
	GROUPING_MASTER_ADVERTISING_ON,
#ifndef MASTER_MODE_ONLY
	GROUPING_SLAVE_NORMAL_MODE,
	GROUPING_SLAVE_RECEIVER_MODE,
	GROUPING_SLAVE_SET_MANUFACTURE_DATA
#endif
}Master_Slave_Grouping_State;
#endif //MASTER_SLAVE_GROUPING

typedef enum {
	BT_PACKET_SYNC,
	BT_PACKET_TYPE,
	BT_PACKET_MAJOR_ID,
	BT_PACKET_MINOR_ID,
	BT_PACKET_LENGTH,
	BT_PACKET_DATA,
	BT_PACKET_CHECKSUM,
	BT_PACKET_SS_SOURCE_TYPE,
	BT_PACKET_SS_DATA_CHANNEL,
	BT_PACKET_SS_LENGTH_HIGH,
	BT_PACKET_SS_LENGTH_LOW,
	BT_PACKET_SS_DATA,
	BT_PACKET_SS_CHECKSUM
}BT_PACKET_STATUS;


typedef enum {
	FEATURE_DISABLE = 0x0,
	FEATURE_ENABLE = 0x1
}Enable_t;


//0x00 : Analog, 0x01 : I2S
typedef enum {
	BT_AudioOut_Analog  = 0x00,
	BT_AudioOut_I2S		= 0x01
}BT_AudioOut;

#ifdef SIG_TEST
typedef enum {
	A2DP_Close_Request,
	A2DP_Abort_Request
} A2DP_PTS_Event;
#endif

#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_15 : To make current status of Peer device(A2DP) and TWS Slave device
typedef enum {
	PEER_DEVICE_NONE,
	PEER_DEVICE_DISCOVERABLE_CONNECTABLE_MODE_1,
	PEER_DEVICE_DISCOVERABLE_CONNECTABLE_MODE_2,
	PEER_DEVICE_PAIRED,
	PEER_DEVICE_DISCONNECTED
} Peer_Device_Connection_Status;

typedef enum {
	TWS_SLAVE_NONE_CONNECTION,
	TWS_SLAVE_DISCOVERABLE_CONNECTABLE_MODE_1,
	TWS_SLAVE_DISCOVERABLE_CONNECTABLE_MODE_2,
	TWS_SLAVE_GROUGPING_MODE,
	TWS_SLAVE_CONNECTTED_WITH_MASTER,
	TWS_SLAVE_DISCONNECTED_WITH_MASTER,
} TWS_Slave_Connection_Status;

#endif

#ifdef TWS_MODE_ENABLE
#ifdef TWS_MASTER_SLAVE_GROUPING //2023-02-20_2
typedef enum {
	TWS_Grouping_Master_Ready,
	TWS_Grouping_Master_Connect,
	TWS_Grouping_Master_Send_DeviceID1,
	TWS_Grouping_Master_Send_DeviceID2,//1sec
	TWS_Grouping_Master_Send_Cur_Status1,//2sec
	TWS_Grouping_Master_Send_Cur_Status2,//3sec
} TWS_Grouping_Master_Status;
#endif

#ifdef TWS_MASTER_SLAVE_COM_ENABLE //2023-02-21_2 : Change Bool type to enum
typedef enum {
	TWS_Get_Information_Ready,
	TWS_Get_Slave_Name,
	TWS_Get_Slave_Address,
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-25_2
	TWS_Get_Slave_Connected,
	TWS_Get_Slave_Disconnection,
#else
	TWS_Get_Slave_Information_Done,
#endif
} TWS_Connect_Status; //Just check whether current status is TWS connection or not
#endif

typedef enum {
	TWS_Status_Master_Ready, //Init, TWS Fail, MINOR_ID_ACL_OPENED_IND - No device
	TWS_Status_Master_GIAC, //Last connection OK, Discoverable setting under BKeep_Connectable = 1 & LIAC, Discoverable setting
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_9 : Changed TWS Satus sequence
	TWS_Status_Master_Mode_Control, //TWS CMD Sending - Master/Slave grouping start, response of MINOR_ID_SET_DISCOVERABLE_MODE
	TWS_Status_Master_LIAC //BCRF_TWS_SET_DISCOVERABLE_MODE
#else //NEW_TWS_MASTER_SLAVE_LINK
	TWS_Status_Master_LIAC, //BCRF_TWS_SET_DISCOVERABLE_MODE
	TWS_Status_Master_Mode_Control //TWS CMD Sending - Master/Slave grouping start, response of MINOR_ID_SET_DISCOVERABLE_MODE
#endif //NEW_TWS_MASTER_SLAVE_LINK
}TWS_Status;

extern TWS_Status BTWS_LIAC;
#endif

#ifdef USEN_TABLET_AUTO_ON
extern Bool bIs_USEN_Device; //Check if current connected device is USEN MUSIC LINK?
#endif

//Function
#ifdef NEW_TWS_MASTER_SLAVE_LINK //2023-04-26_15 : To make current status of Peer device(A2DP)
void Set_Peer_Device_Status(Peer_Device_Connection_Status Status);
#endif

#ifdef TWS_MASTER_SLAVE_COM_ENABLE
TWS_Connect_Status Is_TWS_Master_Slave_Connect(void); //TRUE : TWS Mode / FALSE : Not TWS Mode(But it's not Broadcast Mode and may just TWS Mode Ready)
#endif
#ifdef TWS_MODE_ENABLE
#if defined(NEW_TWS_MASTER_SLAVE_LINK) && defined(FLASH_SELF_WRITE_ERASE) //2023-04-26_8 : To check whether SPK has the history of TWS connection or not
Bool Read_TWS_Connection_From_Flash(void);
#endif
void MB3021_BT_Module_Set_Discoverable_Mode(void);
void MB3021_BT_Module_TWS_Set_Discoverable_Mode(void);
void MB3021_BT_Module_TWS_Mode_Exit(void);
void MB3021_BT_Module_TWS_Mode_Set_Again(void);
#endif

void MB3021_BT_Module_Forced_Input_Audio_Path_Setting(void);

#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
uint8_t Get_Current_EQ_Mode(void);
#endif

void Init_uBLE_Remocon_Data(void);

#ifdef BT_ALWAYS_GENERAL_MODE //2023-03-13_1 : Just check whether input is BT Long Key or Factory Reset Key
Bool Is_Delete_PDL_by_Factory_Reset(void);
#endif
Bool BT_Is_Routed(void);
Bool Get_Connection_State(void) ;
Bool IS_BBT_Init_OK(void); //To avoid key input before BT_init
Bool Is_SSP_REBOOT_KEY_In(void);
Bool Is_SSP_FACTORY_RESET_KEY_In(void);

void Do_taskUART(void);
void MB3021_BT_Module_Init(Bool Factory_Reset);
void Set_MB3021_BT_Module_Source_Change(void);
#ifdef BT_ALWAYS_GENERAL_MODE //2023-02-15_1 : Added parameter for checking whether factory reset(TRUE) or BT Long Key(FALSE)
void MB3021_BT_Delete_Paired_List_All(Bool Factory_Reset);
#else
void MB3021_BT_Delete_Paired_List_All(void);
#endif
void MB3021_BT_Module_Get_Auto_Response_Packet(uint8_t uBuf_Count, uint8_t* uRecData);
#ifdef FIVE_USER_EQ_ENABLE
void MB3021_BT_Module_EQ(EQ_Mode_Setting EQ_mode);
#endif

void MB3021_BT_Module_HW_Reset(void);
void MB3021_BT_A2DP_Connection_Control(Bool Connect);
void MB3021_BT_A2DP_Routing_Control(Bool Route); //TRUE : Route / FALSE : Unroute

#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
void MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_Key_Sync_With_Slave Input_Key, uint8_t uValue);
void MB3021_BT_Module_Input_Key_Init(void);
#endif
#ifdef MASTER_SLAVE_GROUPING
#ifdef TWS_MASTER_SLAVE_GROUPING
void MB3021_BT_TWS_Master_Slave_Grouping_Start(void);
void MB3021_BT_TWS_Master_Slave_Grouping_Stop(void);
#endif
void MB3021_BT_Master_Slave_Grouping_CMD_Set(uint8_t cmd);
void MB3021_BT_Master_Slave_Grouping_Start(void);
void MB3021_BT_Master_Slave_Grouping_Stop(void);
#endif
#ifdef SIG_TEST
void MB3021_BT_Module_SIG_Test(A2DP_PTS_Event Request);
#endif
void MB3021_BT_Disconnect_All_ACL(void);
void MB3021_BT_Disconnect_ACL(uint8_t *Addr); //2023-03-09_2

#if defined(ADC_VOLUME_STEP_ENABLE) && defined(USEN_BAP)
#ifndef MASTER_MODE_ONLY //2023-03-28_1 : Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode
void MB3021_BT_Module_Send_Extra_Data(void);
#endif //#ifndef MASTER_MODE_ONLY
#endif
#ifdef TWS_MASTER_SLAVE_GROUPING //2023-02-20_2
void MASTER_SLAVE_Grouping_Send_SET_DEVICE_ID(Bool B_Send_Again);
void Set_Cur_TWS_Grouping_Status(TWS_Grouping_Master_Status set_status);
TWS_Grouping_Master_Status Get_Cur_TWS_Grouping_Status(void);
#endif
#ifdef TWS_MASTER_SLAVE_COM_ENABLE //2023-02-22_3
void TWS_Power_Init_Master_Send_Data_Start(void);
void TWS_Power_Init_Master_Send_Data_Stop(void);
#endif
#ifdef SPP_CMD_AND_MASTER_INFO_SEND
void Send_Cur_Master_Info_To_Tablet(void);
#endif

#endif //MB3021_ENABLE

#endif //BT_MB3021_H

