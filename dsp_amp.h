/**********************************************************************
* @file		dsp_amp.h
* @brief	IR code
* @version	1.0
* @date		
* @author	MS Kim
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#ifndef DSP_AMP_H
#define DSP_AMP_H

//Macro
typedef enum {
	Volume_Up,
	Volume_Down
}Vol_Setting;

//Function
void TAS3251_DSP_Amp_Init(uint8_t I2C_ADR);
void TAS3251_DSP_Amp_Mute(TAS3251_ADDR I2C_ADR, Bool Mute_On);
void TAS3251_DSP_Amp_Volume_Control(Vol_Setting Vol_mode, uint8_t I2C_ADR);
#ifdef AMP_1_1CH_WORKAROUND
void TAS3251_DSP_Amp_Dect_FS(void);
#endif
#endif //DSP_AMP_H
