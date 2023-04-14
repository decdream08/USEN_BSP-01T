#ifndef __USER_H___
#define __USER_H__

/* Debug Mode */
//#define DBG_FLASH_SAVE	0
/* Touch */
#define NORMAL_MODE 	0
#define ADJUST_MODE 	1

/*************************
	TOUCH CH
*************************/
#define CS00_USE	0	
#define CS01_USE	0	
#define CS02_USE	0	
#define CS03_USE	0	
#define CS04_USE	0	
#define CS05_USE	0	
#define CS06_USE	0	
#define CS07_USE	0	
#define CS08_USE	0	
#define CS09_USE	0	
#define CS10_USE	0
#define CS11_USE	1
#define CS12_USE	1
#define CS13_USE	1
#define CS14_USE	1
#define CS15_USE	1
#define CS16_USE	0
#define CS17_USE	0
#define CS18_USE	0
#define CS19_USE	0
#define CS20_USE	0
#define CS21_USE	0
#define CS22_USE	0	
#define CS23_USE	0	

/*******************************************************************************************************************************************/
/*-------------------------------------------------- don't modify ---------------------------------------------------*/
#define ACT_KEY_N		(CS00_USE+CS01_USE+CS02_USE+CS03_USE+CS04_USE+CS05_USE+CS06_USE+CS07_USE+	\
						 CS08_USE+CS09_USE+CS10_USE+CS11_USE+CS12_USE+CS13_USE+CS14_USE+CS15_USE+	\
						 CS16_USE+CS17_USE+CS18_USE+CS19_USE+CS20_USE+CS21_USE+CS22_USE+CS23_USE)	
#define RAW_Q_SIZE 7		//7

typedef struct{
	int16_t delData[ACT_KEY_N]; 
	uint16_t raw[ACT_KEY_N];
	uint16_t rawData[ACT_KEY_N];
	uint16_t rawQueue[ACT_KEY_N][RAW_Q_SIZE];
	uint16_t rawQindx;
	uint16_t baseData[ACT_KEY_N]; 
	uint16_t preData[ACT_KEY_N];  
	uint16_t zeroSCO[ACT_KEY_N];
	uint8_t	abnDel_n[ACT_KEY_N];
	uint8_t	baseHold_n[ACT_KEY_N];	
	uint8_t	debounce_n[ACT_KEY_N];
	int16_t chTH[ACT_KEY_N];
	uint8_t	actvCH_num[ACT_KEY_N];
	uint8_t RCFilter[ACT_KEY_N];
}str_user;

extern str_user t_user;
#ifdef TOUCHKEY_ENABLE
void ts_UserFilter(void);
#endif

/*******************************************************************************************************************************************/




#endif	// __USER_H__

