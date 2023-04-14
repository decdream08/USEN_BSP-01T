
/**
 *
 * @file AD82584F.h
 *
 * @author   ms kim
 *
 * @brief
 */

#ifndef AD82584F_AMP_H
#define AD82584F_AMP_H

//Macro
typedef enum {
	Volume_Up,
	Volume_Down
}Vol_Setting;

typedef enum {
	LL_MODE,
	RR_MODE,
	STEREO_MODE
}Audio_Output_Setting;

//Function
Bool Is_Mute(void);
void Set_Is_Mute(Bool mute_on);
#ifdef AD82584F_USE_POWER_DOWN_MUTE
Bool IS_Display_Mute(void);
#endif

#ifdef FLASH_SELF_WRITE_ERASE
void AD82584F_Amp_Init(Bool Power_On_Init);
#else
void AD82584F_Amp_Init(void);
#endif
void AD82584F_Amp_Reset(Bool Reset_On);
void AD82584F_Amp_Mute(Bool Mute_On, Bool LED_Display);
void AD82584F_Amp_Mute_Toggle(void);

uint8_t AD82584F_Amp_Volume_Set_with_Index(uint8_t Vol_Level, Bool Inverse, Bool Actual_Key);
void AD82584F_Amp_Volume_Control(Vol_Setting Vol_mode);
void AD82584F_Amp_Mode_Control(Audio_Output_Setting mode);
void AD82584F_Amp_EQ_DRC_Control(EQ_Mode_Setting EQ_mode);

Bool AD82584F_Amp_Get_Cur_Mute_Status(void); //TRUE : Mute On / FALSE : Mute Off

void AD82584F_Amp_Set_Cur_Volume_Level(uint8_t volume);
uint8_t AD82584F_Amp_Get_Cur_Volume_Level(void);
uint8_t AD82584F_Amp_Get_Cur_Volume_Level_Inverse(void);

#ifdef DRC_TOGGLE_TEST
void AD82584F_DRC_On(void);
void AD82584F_DRC_Off(void);
#endif

Bool AD82584F_Amp_Get_Cur_CLK_Status(void); //TRUE : Clock Exist / FALSE : Clock absence

void AD82584F_Amp_Set_Default_Volume(void);

#endif //DSP_AMP_H

