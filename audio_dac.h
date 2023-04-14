
/**
 *
 * @file Audio_dac.h
 *
 * @author   ms kim
 *
 * @brief
 */

#ifndef AUDIO_DAC_H
#define AUDIO_DAC_H

//Macro
typedef enum {
	Volume_Up,
	Volume_Down
}Vol_Setting;

typedef enum {
	SABRE9006A_DEVICE_ADDR_L = (0x48), //8bit - 0x90 /7bit - 100 1000
	SABRE9006A_DEVICE_ADDR_H = (0x49) //8bit - 0x92 / 7bit - 100 1001
}SABRE9006A_ADDR;

//Function
void Audio_Dac_Init(void);
void Audio_Dac_Mute(void);
void Audio_Dac_Volume_Control(Vol_Setting Vol_mode);
void Audio_Dac_Read_Register(void);
#endif //AUDIO_DAC_H


