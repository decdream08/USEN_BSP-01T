#include "stdio.h"
#include "main_conf.h"
#ifdef TOUCHKEY_ENABLE
#include "A31G21x_hal_pcu.h"
#include "A31G21x_hal_scu.h"
#include "user.h"
#include "common.h"
#include "touch_key.h"
//#include "userTimer.h"
#ifdef _DBG_MSG_EN
#include "debug.h"
#endif
//#include "ledCtrl.h"


str_flag t_sysbit;

/***************************************************************************************
 *						A31G21x Device 
 *------------------------------------------------------------------------
 *	System Clock source		| High Speend Internal oscillator (HSI)
 *------------------------------------------------------------------------
 *	SYSCLK(MHz)				| 32MHz
 *------------------------------------------------------------------------
 *	HCLK(MHz) - Core Clock		| 32MHz
 *------------------------------------------------------------------------
  *   	PCLK(MHz) - Peri Clock		| 32MHz
 *------------------------------------------------------------------------
***************************************************************************************/



/*************************************************************
 - FUNCTION & CLASS NAME
	: void Init_Library(void)
	
 - DESCRIPTION
	:
	
 - PARAMETER & RETURN
	: 
	
*************************************************************/
void Init_Library(void)
{
	uint8_t i;
	
	for (i=0; i<t_ts.actCH_n; i++)
	{
		t_ts.pChTH[i] = 1000;
	}
	t_ts.actCH_v = (((uint32_t)CS00_USE<<0)+((uint32_t)CS01_USE<<1)+((uint32_t)CS02_USE<<2)+((uint32_t)CS03_USE<<3)+((uint32_t)CS04_USE<<4)+((uint32_t)CS05_USE<<5)+((uint32_t)CS06_USE<<6)+((uint32_t)CS07_USE<<7)+	\
					((uint32_t)CS08_USE<<8)+((uint32_t)CS09_USE<<9)+((uint32_t)CS10_USE<<10)+((uint32_t)CS11_USE<<11)+((uint32_t)CS12_USE<<12)+((uint32_t)CS13_USE<<13)+((uint32_t)CS14_USE<<14)+((uint32_t)CS15_USE<<15)+	\
					((uint32_t)CS16_USE<<16)+((uint32_t)CS17_USE<<17)+((uint32_t)CS18_USE<<18)+((uint32_t)CS19_USE<<19)+((uint32_t)CS20_USE<<20)+((uint32_t)CS21_USE<<21)+((uint32_t)CS22_USE<<22)+((uint32_t)CS23_USE<<23));
	t_ts.debouncN_det = 0;
	t_ts.debouncN_rel = 0;
	t_ts.iirOffset = 0;
	t_ts.reverseTime = 100;
	t_ts.releasRate = 50;
	t_ts.reversRate = 50;
	t_ts.traceStep = 2;
	t_ts.traceDelay = 500;
	t_ts.actCH_n = ACT_KEY_N;
	t_ts.pDelData	= &t_user.delData[0];
	t_ts.pRawData	= &t_user.rawData[0];
	t_ts.pBaseData	= &t_user.baseData[0];
	t_ts.pPreData	= &t_user.preData[0];
	t_ts.pZeroSCO	= &t_user.zeroSCO[0];
	t_ts.pAbnDel_n	= &t_user.abnDel_n[0];
	t_ts.pChTH		= &t_user.chTH[0];
	t_ts.pBaseHold_n = &t_user.baseHold_n[0];	
	t_ts.pDebounce_n = &t_user.debounce_n[0];
	t_ts.pActvCH_num = &t_user.actvCH_num[0];
	t_ts.pRCFilter	= &t_user.RCFilter[0];
	fpUserFilter 	= ts_UserFilter;

	t_sysbit.cal_ok = 0;
/////	t_sysbit.led_io = 0;
////	t_sysbit.timeDiv = _TS_LED_TIME_DIV;
}

/*************************************************************
 - FUNCTION & CLASS NAME
	: int main(void)
	
 - DESCRIPTION
	: LED pin setting (iCOMn:iSEGn)
	
 - PARAMETER & RETURN
	: 
	
*************************************************************/
void Do_TaskGPIO(void)
{
	static uint32_t test32 = 0;
	uint8_t key = 0;

	if(test32 != t_ts.detKey)
	{
#ifdef _DBG_MSG_EN
		_DBG("\n\rCS ++\n\r");
#endif
		test32 = t_ts.detKey;
#ifdef _DBG_MSG_EN
		_DBH32(test32);
#endif
		
		if (t_ts.detKey & BIT(0))
		{
#ifdef _DBG_MSG_EN
			_DBG("\n\rCS11++ VOLUME UP KEY\n\r");
#endif
			key = VOL_UP_KEY;
		}
		if (t_ts.detKey & BIT(1))
		{
#ifdef _DBG_MSG_EN
			_DBG("\n\rCS12++ VOLUME DOWN KEY\n\r");
#endif
			key = VOL_DOWN_KEY;
		}
		if (t_ts.detKey & BIT(2))
		{
#ifdef _DBG_MSG_EN
			_DBG("\n\rCS13++ PREVIOUS KEY\n\r");
#endif
			key = NUM_2_KEY;
		}
		if (t_ts.detKey & BIT(3))
		{
#ifdef _DBG_MSG_EN
			_DBG("\n\rCS14++ NEXT KEY\n\r");
#endif
			key = NUM_1_KEY;
		}
		if (t_ts.detKey & BIT(4))
		{
#ifdef _DBG_MSG_EN
			_DBG("\n\rCS15++ MUTE KEY\n\r");
#endif
			key = MUTE_KEY;
		}

		if(key)
			Send_Remote_Key_Event(key);
	}	
}
/*************************************************************
 - FUNCTION & CLASS NAME
	: int main(void)
	
 - DESCRIPTION
	: LED pin setting (iCOMn:iSEGn)
	
 - PARAMETER & RETURN
	: 
	
*************************************************************/
void TouchKey_Init(void)
{	
	SysTick_Config(SystemCoreClock/1000);  // ms interrupt

	Init_Library();	
	ts_Set_OpMode(NORMAL_MODE);				// 0:NORMAL_MODE, 1:ADJUST_MODE
	ts_Set_SumCount(5);
	ts_Set_IntegCount(5);
	ts_Set_FreqDelta(5);
	ts_Set_CommonTH(50);//ts_Set_CommonTH(300);
	ts_Set_BaseTrace(5, 500);				// step, delay
	ts_Set_ReleasRate(50);					// 50% of TH
	ts_Set_ReversRate(50);					// 50% of TH
	ts_Set_ReversTime(100);					// ms
	ts_Set_Debounce(2, 0);		// 2,0
	ts_Set_OffsetIIR(0);		// 1
}

#endif // TOUCHKEY_ENABLE

