/*************************************************************
 - ���ϸ�	: usertimer.h
 - ���� 	: ����� Ÿ�̸Ӽ����� ���� �Լ� ���� �� ��Ÿ �������� ����Ѵ�. 
 - �ۼ���	: �㿵�� 
 - ��¥ 	: 2013.02.10 ~
 - ���� 	: 2013.02.10 - ���� ���� 
		: 
*************************************************************/
#ifndef __USERTIMER_H__
#define __USERTIMER_H__

typedef enum 
{
	UT_MIN		=0x00000000,
	UT_1		=0x00000001,	
	UT_REVERSE	=0x00000002,	
	UT_BASE_HOLD =0x00000004,
	UT_4		=0x00000008,
	UT_5		=0x00000010,	
	UT_6		=0x00000020,
	UT_7		=0x00000040,
	UT_8		=0x00000080,
	UT_9		=0x00000100,
	UT_MAX		=0x0000ffff
} eUTIDType;


void ut_InitParam(void);
void ut_SetTimer(uint32_t m_utid, uint32_t time_ms);
uint32_t ut_GetSETinfo(void);
void ut_ClrTimer(uint32_t m_utid);
uint8_t ut_IsExpired(uint32_t m_utid);

#endif	// __USERTIMER_H__

