#ifndef __TOUCH_H__
#define __TOUCH_H__

#define NORMAL_MODE 	0
#define ADJUST_MODE 	1
#define TS_BREAK_TIME	0x01
#define TS_COMPLETE 	0x02
#define _MAX_KEY	24
enum{
	REG_TS_CON = 0, 	// (@ 0x40003700) Touch Sensor Control Register 
	REG_TS_MODE,		// (@ 0x40003704) Touch Sensor Mode Register	
	REG_TS_SUM_CNT, 	// (@ 0x40003708) Touch Sensor Sum Repeat Count Register
	REG_TS_CH_SEL,		// (@ 0x4000370C) Touch Sensor Channel Selection Register
	REG_RESERVED1,		// (@ 0x40003710)
	REG_TS_SLP_CR,		// (@ 0x40003714) Touch Sensor Low Pass Filter Control Register 
	REG_ADC_CH_SEL, 	// (@ 0x40003718) ADC Channel Selection Register
	REG_TS_INTEG_CNT,	// (@ 0x4000371C) Touch Sensor Sensing Integration Count Register
	REG_TS_FREQ_NUM,	// (@ 0x40003720) Touch Sensor Frequency Number Register
	REG_TS_FREQ_DEL,	// (@ 0x40003724) Touch Sensor Frequency Delta Register 
	REG_TS_CLK_CFG, 	// (@ 0x40003728) Touch Sensor Clock Configuration Register 			
	REG_TRIM_OSC,		// (@ 0x4000372C) Touch Sensor RING Oscillator Trimming Selection Register 
	REG_TRIM_A_OSC, 	// (@ 0x40003730) Touch Sensor RING Oscillator Trimming for ADC Register	
	REG_SCI,			// (@ 0x40003734) Touch Sensor Input Capacitor Selection Register
	REG_SCC,			// (@ 0x40003738) Touch Sensor Conversion Capacitor Selection Register
	REG_SVREF,			// (@ 0x4000373C) Touch Sensor VREF Resistor Selection Register 	
	REG_TAR,			// (@ 0x40003740) Touch Sensor Integration AMP Reset Register
	REG_TRST,			// (@ 0x40003744) Touch Sensor Reset time of Sensing Register
	REG_TDRV,			// (@ 0x40003748) Touch Sensor Sample time of Sensing Register	
	REG_TINT,			// (@ 0x4000374C) Touch Sensor Integration time of Sensing Register 
	REG_TD, 			// (@ 0x40003750) Touch Sensor Differential AMP Sampling Register
	REG_TWR,			// (@ 0x40003754) Touch Sensor Wait time Register	
	REG_TS_MAX, 		//
};
typedef struct{
	__I  uint32_t  TS_RAW[_MAX_KEY];	// (@0x40003600~@ 0x4000365C) Touch Sensor Channel 0~23 Sum Register
	__IO uint32_t  TS_SCO[_MAX_KEY];	// (@0x40003660~@0x400036BC) Touch Sensor Offset Capacitor Selection Register
	__I  uint32_t  RESERVED[16];
	__IO uint32_t  TS_REG[REG_TS_MAX];	// (@ 0x40003700~@ 0x40003758) Touch Sensor Control Register (from TS_CON)
}__TOUCH_REG;

typedef enum
{
	TASK_TOUCH = 0,
	TASK_UART,
	TASK_GPIO,
	TASK_MAX
} e_task;


#define _TRAW		((__TOUCH_REG*)TOUCH_BASE)	// 0x40003600UL
#define TS_RAW(X)	_TRAW->TS_RAW[X]	
#define TS_SCO(X)	_TRAW->TS_SCO[X]
#define TS(X)		_TRAW->TS_REG[X]

#define TS_REG_BASE 		0x40003700UL
#define FUSE_BASE_NOW		0x0F010200UL
#define FUSE_BASE			0x0F010800UL
#define FUSE_TOUC			*(volatile unsigned int *)(FUSE_BASE_NOW+0x24)

typedef struct
{ 
	int16_t *pDelData;
	int16_t *pChTH; 		
	uint16_t *pRawData;
	uint16_t *pBaseData;
	uint16_t *pPreData;
	uint16_t *pTestData0;
	uint16_t *pTestData1;
	uint16_t *pTestData2;
	uint16_t *pTestData3;
	uint16_t *pZeroSCO;
	uint8_t	*pAbnDel_n; 
	uint8_t	*pBaseHold_n;	
	uint8_t	*pDebounce_n;
	uint8_t	*pActvCH_num;
	uint8_t *pRCFilter;
	uint32_t detKey; 	
	uint8_t  keyNum;
	uint32_t detFlag;
	uint8_t	actvCH_i;
	uint8_t	senseTime;
	uint16_t mode;
	uint16_t maxSCO;
	uint8_t	actCH_n;
	uint32_t actCH_v;	
	uint8_t	debouncN_det;
	uint8_t	debouncN_rel;
	uint8_t	iirOffset;
	uint16_t reverseTime;
	uint8_t	releasRate;
	uint8_t	reversRate;
	uint8_t	traceStep;
	uint16_t traceDelay;
	uint8_t sumCount;
	uint8_t integCount;
	uint8_t freqDelta;
	uint8_t revCHIndx;
	struct{
		uint8_t baseInit :1;
		uint8_t tsStat	:2;
		uint8_t goOn 	:1; 	// 0: time division, 1: continuous
		uint8_t rsvd 	:4;
	} flag;
} str_tc;

extern void (*fpUserFilter)(void);

extern str_tc t_ts;
void Do_taskTouch(void);
void ts_StartTouch(uint8_t indx);
void ts_Set_OpMode(uint8_t opMode);
void ts_Set_SumCount(uint8_t sumCnt);
void ts_Set_IntegCount(uint8_t integCnt);
void ts_Set_FreqDelta(uint8_t freqDelta);
void ts_Set_CommonTH(int16_t tsTH);
void ts_Set_IndivTH(uint8_t tsCH, int16_t tsTH);
void ts_Set_BaseTrace(uint8_t traceStep, uint16_t traceDelay);
void ts_Set_ReleasRate(uint8_t relRate);
void ts_Set_ReversRate(uint8_t revRate);
void ts_Set_Debounce(uint8_t debounce_n_det, uint8_t debounce_n_rel);
void ts_Set_OffsetIIR(uint8_t offsetIIR);
void ts_Set_ReversTime(uint16_t revTime);
void ts_Set_ActvTimeMs(uint8_t slotMs);		
uint32_t ts_Get_Key(void);
uint16_t get_abs(int16_t value);

void Do_TaskGPIO(void);
void TouchKey_Init(void);
#endif	// __TOUCH_H__

