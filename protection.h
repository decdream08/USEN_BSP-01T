/**********************************************************************
* @file		protection.h
* @brief	protection
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/
typedef enum
{
  ProtectionNone,
  ProtectionETC,
  ProtectionAMP,
  ProtectionLED,
} ProtectionMode;

typedef struct {
	ProtectionMode mode;
	uint8_t etc_count;
	uint8_t amp_count;
	uint8_t led_count;
}Protect_Type;

#define PROTECTION_COUNT	 10

void Protection_10ms_timer(void);
void Protection_Process(void);
ProtectionMode protection_check(void);
uint8_t protection_status(void);
void init_protect_values(void);

extern uint8_t protection_check_flag;
extern ProtectionMode protection_mode;

