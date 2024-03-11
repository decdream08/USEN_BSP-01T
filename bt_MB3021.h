
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
#include "AD85050.h" 

typedef enum {
	input_key_Sync_Power, 		//0
	input_key_Sync_Mute, 		//1
	input_key_Sync_Volume, 		//2
	input_key_Sync_EQ, 			//3
	input_key_Sync_Reboot, 		//4
	input_key_Sync_Factory_Reset,  //5
	Input_key_Sync_Slave_Mute_Off  //6
}Input_Key_Sync_With_Slave;

typedef enum {
	GROUPING_NONE_MODE,
	GROUPING_EVENT_WAIT_STATE,
	GROUPING_MASTER_NORMAL_MODE,
	GROUPING_MASTER_BROADCASTER_MODE,
	GROUPING_MASTER_SET_MANUFACTURE_DATA,
	GROUPING_MASTER_ADVERTISING_ON,
}Master_Slave_Grouping_State;

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

typedef enum {
	SET_DISABLE_DISCOVERABLE_MODE,
	SET_ENABLE_LIMITED_DISCOVERABLE_MODE,
	SET_ENABLE_GENERAL_DISCOVERABLE_MODE,
	SET_ENABLE_DUAL_DISCOVERABLE_MODE
} Set_Discoverable_Mode_Param; //2023-05-30_2

typedef enum {
    MB3021_POWER_DOWN,
    MB3021_POWER_UP,
    MB3021_RESET_ON,
    MB3021_RESET_OFF,
	MB3021_POWER_UP_COMPLETE,
	MB3021_POWER_CONTROL_CMD_PROCESS,
	MB3021_MODULE_INIT_CMD_PROCESS,
	MB3021_SW_RESETTING_CMD_PROCESS,
	MB3021_SW_RESET_CMD_PROCESS,
	MB3021_RUN,
}BT_Status;

void MB3021_10ms_timer(void);
void MB3021_Process(void);
void MB3021_PowerUp(void);

//Function
void MB3021_BT_Module_Set_Discoverable_Mode_by_Param(Set_Discoverable_Mode_Param uParam); //2023-05-30_2 : To make Limitted Access Code Mode

void MB3021_BT_Module_Forced_Input_Audio_Path_Setting(void);

uint8_t Get_Current_EQ_Mode(void);

void Init_uBLE_Remocon_Data(void);

Bool Is_Delete_PDL_by_Factory_Reset(void);

Bool BT_Is_Routed(void);
Bool Get_Connection_State(void) ;
Bool IS_BBT_Init_OK(void); //To avoid key input before BT_init
Bool Is_SSP_REBOOT_KEY_In(void);
Bool Is_SSP_FACTORY_RESET_KEY_In(void);

void Do_taskUART(void);
void MB3021_BT_Module_Init(Bool Factory_Reset);
void Set_MB3021_BT_Module_Source_Change(void);
void Set_MB3021_BT_Module_Source_Change_Direct(void);

void MB3021_BT_Delete_Paired_List_All(Bool Factory_Reset);

void MB3021_BT_Module_Get_Auto_Response_Packet(uint8_t uBuf_Count, uint8_t* uRecData);
void MB3021_BT_Module_EQ(EQ_Mode_Setting EQ_mode);

void MB3021_BT_Module_HW_Reset(void);
void MB3021_BT_A2DP_Connection_Control(Bool Connect);
void MB3021_BT_A2DP_Routing_Control(Bool Route); //TRUE : Route / FALSE : Unroute

void MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_Key_Sync_With_Slave Input_Key, uint8_t uValue);
void MB3021_BT_Module_Input_Key_Init(void);

void MB3021_BT_Master_Slave_Grouping_CMD_Set(uint8_t cmd);
void MB3021_BT_Master_Slave_Grouping_Start(void);
void MB3021_BT_Master_Slave_Grouping_Stop(void);

#ifdef SIG_TEST
void MB3021_BT_Module_SIG_Test(A2DP_PTS_Event Request);
#endif
void MB3021_BT_Disconnect_All_ACL(void);
void MB3021_BT_Disconnect_ACL(uint8_t *Addr); //2023-03-09_2

void Send_Cur_Master_Info_To_Tablet(void);
#endif //MB3021_ENABLE

#endif //BT_MB3021_H

