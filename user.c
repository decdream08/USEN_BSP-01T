#include "main_conf.h"
#include "A31G21x_hal_scu.h"
#include "common.h"
#include "user.h"
#include "touch_key.h"


str_user t_user;


/*************************************************************
 - FUNCTION & CLASS NAME
	: void sort_upward(u8* mp_start, u8 len)
	
 - DESCRIPTION
	: 
	
 - PARAMETER & RETURN
	: 
	
*************************************************************/
uint16_t get_max(uint16_t* mp_start, uint8_t m_offset, uint8_t m_len)
{
	uint8_t i;
	uint16_t max;
	
	max = 0;
	for (i=0; i<m_len; i++)
	{
		if(*(mp_start+i*m_offset) > max) 
		{
			max = *(mp_start+i*m_offset);
		}
	}

	return max;
}

#ifdef TOUCHKEY_ENABLE
/*************************************************************
 - FUNCTION & CLASS NAME
	: void ts_UserFilter(void)
	
 - DESCRIPTION
	: 
	
 - PARAMETER & RETURN
	: 
	
*************************************************************/
void ts_UserFilter(void)
{
	uint8_t i;
	uint8_t j;
	static uint8_t relLevel_n[ACT_KEY_N] = {0,};
	
	for (i=0; i<t_ts.actCH_n; i++)
	{
		t_user.raw[i] = t_user.rawData[i];		
		if (t_ts.detFlag & BIT(i))
		{
			t_user.rawQueue[i][t_user.rawQindx++] = t_user.rawData[i];
			t_user.rawQindx %= RAW_Q_SIZE;			
			if (	(t_user.rawData[i] < (t_user.baseData[i]+t_ts.pChTH[i]*t_ts.releasRate/100)) && \
				 	(t_user.rawData[i] > (t_user.baseData[i]-t_ts.pChTH[i]*t_ts.releasRate/100)) )
			{
				relLevel_n[i]++;

				if (relLevel_n[i] >= 4)
				{
					relLevel_n[i] = 0;
					for (j=0; j<RAW_Q_SIZE; j++)
					{
						t_user.rawQueue[i][j] = t_user.rawData[i];
					}
				}
			}
			else
			{
				relLevel_n[i] = 0;
			}
			t_user.rawData[i] = get_max(&t_user.rawQueue[i][0], 1 ,RAW_Q_SIZE);
		}
		else
		{
			relLevel_n[i] = 0;
		}
	}
}
#endif

