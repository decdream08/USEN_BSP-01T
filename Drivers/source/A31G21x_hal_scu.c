/**********************************************************************//**
* @file		A31G21x_hal_scu.c
* @brief	Contains all functions support for SCU firmware library on A31G21x
* @version	1.00
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
************************************************************************
* ABOV Disclaimer
*
*IMPORTANT NOTICE ? PLEASE READ CAREFULLY
*ABOV Semiconductor ("ABOV") reserves the right to make changes, corrections, enhancements, 
*modifications, and improvements to ABOV products and/or to this document at any time without notice. 
*ABOV does not give warranties as to the accuracy or completeness of the information included herein.
*Purchasers should obtain the latest relevant information of ABOV products before placing orders. 
*Purchasers are entirely responsible for the choice, selection, and use of ABOV products and 
*ABOV assumes no liability for application assistance or the design of purchasers¡¯ products. No license, 
*express or implied, to any intellectual property rights is granted by ABOV herein. 
*ABOV disclaims all express and implied warranties and shall not be responsible or
*liable for any injuries or damages related to use of ABOV products in such unauthorized applications. 
*ABOV and the ABOV logo are trademarks of ABOV.
*All other product or service names are the property of their respective owners. 
*Information in this document supersedes and replaces the information previously
*supplied in any former versions of this document.
*2020 ABOV Semiconductor  All rights reserved
*
**********************************************************************/


/* Includes ------------------------------------------------------------ */
#include "A31G21x_hal_scu.h"


#define PLL_DELAY	1750	// @HSE 8MHz

/**********************************************************************//**
 * @brief 		Configure external sub clock  
 *             Before exteranl crystal enable setting, you have to set clock ports as SXIN,SXOUT in MOD,AFSRx reg
 * @param[in]	esosccon 	Function mode, should be:
 * 					- EXSOSC_STOP	   : 0
 * 					- EXSOSC_EN    	   : 8
 * 					- EXSOSC_EN_DIV2 : 9
  * 					- EXSOSC_EN_DIV2 : 10 
 * @return		None
 **********************************************************************/
void HAL_SCU_LSE_ClockConfig(uint32_t esosccon)
{
	uint32_t reg_val32;
	
// enable external sub clock  
// Before exteranl crystal enable setting, you have to set clock ports as SXIN,SXOUT in MOD,AFSRx reg
	reg_val32=SCU->CSCR;
	reg_val32&=~(0x0f<<12); // 
	reg_val32|=(esosccon<<12); // 0:Stop SOSC, 8:Enable SOSC/1, 9:Enable SOSC/2, 10:Enable SOSC/4
	SCU->CSCR=reg_val32 | (0xa507UL<<16);		
}
 
/**********************************************************************//**
 * @brief 	  monitoring External sub oscillator  
 * @param  none
 * @return	status enumeration value:
 *              - ERROR
 *              - SUCCESS
 **********************************************************************/
Status HAL_SCU_LSE_ClockMonitoring(void)
{
	__IO uint32_t StartUpCounter = 0;
	__IO uint16_t status;
	uint32_t i;
		
	// monitoring External sub oscillator 
	SCU->CMR|=(1<<11); // 0:External sub oscillator monitoring disabled, 1:External sub oscillator monitoring enable
	/* Wait till SOSC is ready and if timeout is reached exit */
//	do 
//	{
//		status = (SCU->CMR & SCU_CMR_SOSCSTS_Msk);
//		StartUpCounter++;  
//	} while((StartUpCounter != SOSC_STARTUP_TIMEOUT) && (status != SCU_CMR_SOSCSTS_Msk));	
	
	/* need more Wait for SOSC stable, check SXOUT pin oscillation  */	
	for (i=0;i<0x100;i++)
	{
          NOP();	
	}
	
//	status = (SCU->CMR & SCU_CMR_SOSCSTS_Msk);
//	if (status != SCU_CMR_SOSCSTS_Msk){
//		return ERROR;
//	}
//	else {
		return SUCCESS;
//	}
}

/**********************************************************************//**
 * @brief 		Configure low speed internal osc clock  
 * @param[in]	ringosc Function mode, should be:
 * 					- LSIOSC_STOP        : 0
 * 					- LSIOSC_EN             : 8
 * 					- LSIOSC_EN_DIV2     : 9
 * 					- LSIOSC_EN_DIV4     : 10
 * @return		None
 **********************************************************************/
void HAL_SCU_LSI_ClockConfig(uint32_t ringosc)
{
	uint32_t reg_val32;
	
// enable internal clock  
	reg_val32=SCU->CSCR;
	reg_val32&=~(0x0f<<8); // 
	reg_val32|=(ringosc<<8); // 0:Stop LSI, 8:Enable LSI/1, 9:Enable LSI/2, 10:Enable LSI/4
	SCU->CSCR=reg_val32 | (0xa507UL<<16);		
}

/**********************************************************************//**
 * @brief 		Configure high speed internal osc clock  
 * @param[in]	hsiosc Function mode, should be:
 * 					- HSIOSC_STOP        : 0
 * 					- HSIOSC_EN             : 8
 * 					- HSIOSC_EN_DIV2     : 9
 * 					- HSIOSC_EN_DIV4     : 10
 * 					- HSIOSC_EN_DIV8     : 11
 * 					- HSIOSC_EN_DIV16    : 12
 * 					- HSIOSC_EN_DIV32    : 13
 * @return		None
 **********************************************************************/
void HAL_SCU_HSI_ClockConfig(uint32_t hsiosc)
{
	uint32_t reg_val32;
	
// enable internal clock  
	reg_val32=SCU->CSCR;
	reg_val32&=~(0x0f<<4); // 
	reg_val32|=(hsiosc<<4); // 0:Stop LSI, 8:Enable LSI/1, 9:Enable LSI/2, 10:Enable LSI/4, 11:Enable LSI/8, 12:Enable LSI/16, 13:Enable LSI/32
	SCU->CSCR=reg_val32 | (0xa507UL<<16);		
}

/**********************************************************************//**
 * @brief 		Configure external clock  
 *             Before exteranl crystal enable setting, you have to set clock ports as XIN,XOUT in MR.
 * @param[in]	eosccon 	Function mode, should be:
 * 					- EXOSC_STOP        : 0
 * 					- EXOSC_EN             : 8
 * 					- EXOSC_EN_DIV2     : 9
 * 					- EXOSC_EN_DIV4     : 10
 * @return		None
 **********************************************************************/
void HAL_SCU_HSE_ClockConfig(uint32_t eosccon)
{
	uint32_t reg_val32;
	
// enable external clock  
// Before exteranl crystal enable setting, you have to set clock ports as XIN,XOUT in MOD,AFSRx reg
	reg_val32=SCU->CSCR;
	reg_val32&=~(0x0f<<0); // 
	reg_val32|=(eosccon<<0); // 0:Stop MOSC, 8:Enable MOSC/1, 9:Enable MOSC/2, 10:Enable MOSC/4
	SCU->CSCR=reg_val32 | (0xa507UL<<16);	
}

/**********************************************************************//**
 * @brief 	  monitoring External oscillator  
 * @param  none
 * @return	status enumeration value:
 *              - ERROR
 *              - SUCCESS
 **********************************************************************/
Status HAL_SCU_HSE_ClockMonitoring(void)
{
	__IO uint32_t StartUpCounter = 0;
	__IO uint16_t status;
	uint32_t i;

	// monitoring External oscillator 
	SCU->CMR|=(1<<3); // 0:External oscillator monitoring disabled, 1:External oscillator monitoring enable
	
	/* Wait till MOSC is ready and if timeout is reached exit */
	do 
	{
		status = (SCU->CMR & SCU_CMR_MOSCSTS_Msk);
		StartUpCounter++;  
	} while((StartUpCounter != MOSC_STARTUP_TIMEOUT) && (status != SCU_CMR_MOSCSTS_Msk));	
	
	/* need more Wait for MOSC stable, check XOUT pin oscillation  */	
	for (i=0;i<0x800;i++)
	{

           __NOP();
		
	}
	
	status = (SCU->CMR & SCU_CMR_MOSCSTS_Msk);
	if (status != SCU_CMR_MOSCSTS_Msk){
		return ERROR;
	}
	else {
		return SUCCESS;
	}
}

/**********************************************************************//**
 * @brief 		change system clock control   
 * @param[in]	sysclkcon mode, should be:
 * 					- SC_LSIOSC		         : 0
 * 					- SC_SOSC  	               : 1 
 * 					- SC_HSIOSC  	               : 2
 * 					- SC_HSIOSCPLL            : 3
 * 					- SC_EXOSC                   : 6
 * 					- SC_EXOSCPLL             : 7
 * @return		None
 **********************************************************************/
void HAL_SCU_SystemClockChange(uint32_t sysclkcon)
{ 
	uint32_t reg_val32;
	
	reg_val32=SCU->SCCR;
	reg_val32&=~(7<<0); 
	reg_val32|=sysclkcon;  
	SCU->SCCR=reg_val32 | (0x570aUL<<16);		
}


/**********************************************************************//**
 * @brief 		change system clock control   
 * @param[in]	sysclkcon mode, should be:
 * 					- SC_FIN_HSI		         : 0
 * 					- SC_FIN_HSE 	           : 1 
 * @return		None
 **********************************************************************/
void HAL_SCU_SystemClockFinClock(uint32_t sysclkcon)
{ 
  uint32_t reg_val32;
	
	reg_val32=SCU->SCCR;
	reg_val32&=~(1<<2); 
	reg_val32|=(sysclkcon<<2);  
	SCU->SCCR=reg_val32 | (0x570aUL<<16);		
}



/**********************************************************************//**
 * @brief 		Configure clock out  on PF4
 *       Before this function setting, you have to set PF4 as CLKO in PF.MODand PF.AFSR0.
 * @param[in]	divval(=CLKODIV) : 0~15  // CLKO=MCLK/(2*(CLKODIV+1))
 *                   ex) MCLK=20Mh, CLKODIV=4,    20Mhz/ (2*(4+1)) = 2Mhz
 * @param[in]	FunctionalState, should be:
 * 					- DISABLE		   : 0
 * 					- ENABLE    	   : 1
 * @return		None
 **********************************************************************/
void HAL_SCU_ClockOutput(uint8_t divval, FunctionalState endis)
{
	SCU->COR=(endis<<4)|(divval&0xf);      
}

/**********************************************************************//**
 * @brief 	  monitoring External oscillator  
 * @param  none
 * @return	status enumeration value:
 *              - ERROR
 *              - SUCCESS
 **********************************************************************/
Status HAL_SCU_PLL_ClockConfig(FunctionalState pllsetstate, uint8_t selbypass, uint8_t selfin,  uint8_t PREDIV, uint8_t POSTDIV1, uint8_t POSTDIV2, uint8_t OUTDIV)
{
	__IO uint32_t StartUpCounter = 0;
	__IO uint32_t status;
	uint32_t i;
	
	if (pllsetstate == DISABLE){
		SCU->PLLCON=0;
		return SUCCESS;
	}
	else {
	/* PLLenable    PLL freq = (FIN or FIN/2) * M / N */
		SCU->PLLCON = 0 |(1<<22); // PLLEn	
		__NOP();                  // 1us wait
		SCU->PLLCON |= (1<<23)    // PLL reset is negated
		|((selbypass&1)<<21)     //BYPASS 0:FOUT is bypassed as FIN, 1:FOUT is PLL output
		|((selfin&1)<<20)        //PLL VCO mode  0:VCO is the same with FOUT, 1:VCO frequency is x2 of FOUT   D
		|((PREDIV&0x7)<<16)      //PREDIV        R
		|((POSTDIV1&0xff)<<8)    //POSTDIV1   N1			
		|((POSTDIV2&0xf)<<4)     //POSTDIV2   N2
		|((OUTDIV&0xf)<<0)       //OUTDIV	     P 
			;
    
		
		/*wait over 190us*/		 
		for (i=0;i<PLL_DELAY;i++); 
		
	  /* Wait till PLL LOCK is ready and if timeout is reached exit */
		while((SCU->PLLCON & (1UL<<31)) == RESET )
		{
			/* Time out for HSE start up, in 1ms */
			if(StartUpCounter++ > PLL_STARTUP_TIMEOUT)
			{
				return ERROR;
			}
		}
		return SUCCESS;
	}
}

/**********************************************************************//**
   @brief        Set Timer1n Clock
   @param[in]    t1nclk                           T2NCLK_MCCR1, T2NCLK_PCLK
   @explain      This macro sets timer1n clock
 **********************************************************************/

void HAL_SCU_Timer1n_ClockConfig(uint32_t t1nclk)
{
	uint32_t temp;
	
	temp = SCU->PPCLKSR;
	temp &= ~(1<<22);
	temp |= (t1nclk<<22);
	SCU->PPCLKSR = temp;	      
}




/**********************************************************************//**
   @brief        Set Timer20 Clock
   @param[in]    t20clk                           TIMER20CLK_MCCR2, TIMER20CLK_PCLK
   @explain      This macro sets timer20 clock
 **********************************************************************/

void HAL_SCU_Timer20_ClockConfig(uint32_t t20clk)
{
	uint32_t temp;
	
	temp = SCU->PPCLKSR;
	temp &= ~(1<<20);
	temp |= (t20clk<<20);
	SCU->PPCLKSR = temp;	      
}


/**********************************************************************//**
   @brief        Set Timer30 Clock
   @param[in]    t30clk                           T30CLK_MCCR2, T30CLK_PCLK
   @explain      This macro sets timer30 clock
 **********************************************************************/

void HAL_SCU_Timer30_ClockConfig(uint32_t t30clk)
{
	uint32_t temp;
	
	temp = SCU->PPCLKSR;
	temp &= ~(1<<17);
	temp |= (t30clk<<17);
	SCU->PPCLKSR = temp;	      
}

/**********************************************************************//**
   @brief        Set LED Clock
   @param[in]    ledclk                           LEDCLK_MCCR5, LEDCLK_PCLK
   @explain      This macro sets LED clock
 **********************************************************************/

void HAL_SCU_LED_ClockConfig(uint32_t ledclk)
{
	uint32_t temp;
	
	temp = SCU->PPCLKSR;
	temp &= ~(1<<10);
	temp |= (ledclk<<10);
	SCU->PPCLKSR = temp;	      
}



/**********************************************************************//**
   @brief        Set Watch Timer Clock
   @param[in]    wtclk                           WTCLK_MCCR3, WTCLK_SOSC, WTCLK_WDTRC
   @explain      This macro sets watch timer clock
 **********************************************************************/

void HAL_SCU_WT_ClockConfig(uint32_t wtclk)
{
	uint32_t temp;
	
	temp = SCU->PPCLKSR;
	temp &= ~(3<<3);
	temp |= (wtclk<<3);
	SCU->PPCLKSR = temp;	      
}

/**********************************************************************//**

   @brief        Set WatchDog Timer Clock
   @param[in]    wdtclk                           WDTCLK_WDTRC, WDTCLK_MCCR3
   @explain      This macro sets watchdog timer clock
 **********************************************************************/

void HAL_SCU_WDT_ClockConfig(uint32_t wdtclk)
{
	uint32_t temp;
	
	temp = SCU->PPCLKSR;
	temp &= ~(1<<0);
	temp |= (wdtclk<<0);
	SCU->PPCLKSR = temp;	      
}

/**********************************************************************//**
 * @brief 		Configure peri clock setting
 * @param[in]	mccrnum: select REG number
 *                     - MCCR1 : 1, * - MCCR2 : 2
 *                     - MCCR3 : 3, * - MCCR4 : 4
 *                     - MCCR5 : 5, * - MCCR6 : 6 
 * @param[in]	type : 
 *	                     SYSTICK_TYPE, TIMER10_TYPE, 
 *	                     TIMER20_TYPE, TIMER30_TYPE,
 *						 WDT_TYPE, WT_TYPE, 
 *						 PD0_TYPE, PD1_TYPE,
 *						 LED_TYPE, LCD_TYPE,
 *						 USB_TYPE
 * @param[in]	clksrc : 
 *						- CLKSRC_LSI=0,
 *						- CLKSRC_SOSC=3,
 *						- CLKSRC_MCLK=4,	
 *						- CLKSRC_HSI=5,
 *						- CLKSRC_MOSC=6,
 *                     - CLKSRC_PLL=7	
 * @param[in]	clkdiv : 
 * @return		None
 **********************************************************************/
void HAL_SCU_MiscClockConfig(uint8_t mccrnum, uint8_t type, uint8_t clksrc, uint8_t clkdiv)
{
	uint32_t tmp;
	
	switch(mccrnum){
		case 1:
			tmp=SCU->MCCR1;
			if (type==SYSTICK_TYPE){
				tmp&=0xFFFFF800;
				tmp|= (((clksrc&7)<<8)|((clkdiv&0xFF)<<0));		
			}
			else if (type==TIMER10_TYPE){
				tmp&=0xF800FFFF;
				tmp|= (((clksrc&7)<<24)|((clkdiv&0xFF)<<16));					
			}			
			SCU->MCCR1=tmp;
			break;
			
		case 2:	
			tmp=SCU->MCCR2;
			if (type==TIMER20_TYPE){
				tmp&=0xFFFFF800;
				tmp|= (((clksrc&7)<<8)|((clkdiv&0xFF)<<0));		
			}
			else if (type==TIMER30_TYPE){
				tmp&=0xF800FFFF;
				tmp|= (((clksrc&7)<<24)|((clkdiv&0xFF)<<16));					
			}
			SCU->MCCR2=tmp;			
			break;
			
		case 3:
			tmp=SCU->MCCR3;
			if (type==WDT_TYPE){
				tmp&=0xFFFFF800;
				tmp|= (((clksrc&7)<<8)|((clkdiv&0xFF)<<0));		
			}
			else if (type==WT_TYPE){
				tmp&=0xF800FFFF;
				tmp|= (((clksrc&7)<<24)|((clkdiv&0xFF)<<16));					
			}
			SCU->MCCR3=tmp;			
			break;
			
		case 4:
			tmp=SCU->MCCR4;
			if (type==PD0_TYPE){
				tmp&=0xFFFFF800;
				tmp|= (((clksrc&7)<<8)|((clkdiv&0xFF)<<0));		
			}
			else if (type==PD1_TYPE){
				tmp&=0xF800FFFF;
				tmp|= (((clksrc&7)<<24)|((clkdiv&0xFF)<<16));					
			}
			SCU->MCCR4=tmp;			
			break;
			
		case 5:
			tmp=SCU->MCCR5;
			if (type==LED_TYPE){
				tmp&=0xFFFFF800;
				tmp|= (((clksrc&7)<<8)|((clkdiv&0xFF)<<0));		
			}
			SCU->MCCR5=tmp;			
			break;		
				
	}
}

/**********************************************************************//**
 * @brief 		set LVR level   
 * @param[in]	bodlevel, should be:
 * 					- LVD_LEVEL_1_65 : 0
 * 					- LVD_LEVEL_2_5 : 1
 * 					- LVD_LEVEL_3_6 : 2
 * 					- LVD_LEVEL_4_2 : 3
 * @return		None
 **********************************************************************/
void HAL_SCU_LVR_SetLevel(uint32_t lvdlevel)
{
	
	SCULV->LVRCNFIG = ((0x72A5<<16) | (0x00<<8) | (lvdlevel<<0));

}

/**********************************************************************//**
 * @brief	 	enable/disable LVR
 * @param[in]	NewState
 * 					- ENABLE  	:Set LVD enable
 * 					- DISABLE 	:Disable LVD
 * @return 		None
 **********************************************************************/
void HAL_SCU_LVRCmd(FunctionalState NewState)
{
		
	if (NewState == ENABLE)
	{
		SCULV->LVRCR = 0x00;
		SCULV->LVRCNFIG = ((0x72A5<<16) | (0x00<<8));
		
	}
	else
	{
		SCULV->LVRCR = 0x55;
		SCULV->LVRCNFIG = ((0x72A5<<16) | (0xAA<<8));
	}
}

/**********************************************************************//**
* @brief	 	Get value of LVI interrupt flag
* @param		None
* @return 		return LVI interrupt flag
**********************************************************************/
uint32_t HAL_SCU_LVI_GetLviFlag(void)
{
	return (SCULV->LVICR & (1<<4));	
}

/**********************************************************************//**

* @brief	 	Clear value of LVI interrupt flag
* @param		None
* @return 		None
**********************************************************************/
void HAL_SCU_LVI_ClrLviFlag(void)
{
	SCULV->LVICR |= (1<<4);
}

/**********************************************************************//**
 * @brief	 	enable/disable WakeUp Source
 * @param[in]	WakeUpSrc, oring is possible:
 * 					- WAKEUP_GPIOD :    (1UL<<11)
 * 					- WAKEUP_GPIOC :    (1UL<<10)
 * 					- WAKEUP_GPIOB :    (1UL<<9)
 * 					- WAKEUP_GPIOA :   (1UL<<8)
*						- WAKEUP_FRT     :   (1UL<<2)
 * 					- WAKEUP_WDT    :    (1UL<<1)
 * 					- WAKEUP_LVD    :    (1UL<<0)
 * @param[in]	NewState
 * 					- ENABLE  	:Set WakeUp Source enable
 * 					- DISABLE 	:Disable WakeUp Source
 * @return 		None
 **********************************************************************/
void HAL_SCU_WakeUpSRCCmd(uint32_t WakeUpSrc, FunctionalState NewState)
{
	if (NewState == ENABLE)
	{
		SCU->WUER |=  (WakeUpSrc);
	}
	else
	{
		SCU->WUER &= ~(WakeUpSrc);
	}
}

/*********************************************************************//**
 * @brief 		Get current value of WakeUpSRC Status.
 * @param[in]	None.
 * @return		Current value of WakeUpSRC Status register.
 *********************************************************************/
uint32_t HAL_SCU_GetWakeUpSRCStatus(void)
{
	return (SCU->WUSR);
}

/**********************************************************************//**
 * @brief	 	enable/disable WakeUp Source
 * @param[in]	WakeUpSrc
 * @return 		None
 **********************************************************************/
void HAL_SCU_ClearWakeUpSRC(uint32_t WakeUpStatus)
{
	SCU->WUSR = WakeUpStatus;
}

/*********************************************************************//**
 * @brief 		Get current value of NMI Status.
 * @param[in]	None.
 * @return		Current value of NMI Status register.
 *********************************************************************/
uint32_t HAL_SCU_GetNMIStatus(void)
{
	return (SCU->NMIR);
}

/**********************************************************************//**
 * @brief	 	enable/disable NMI Source
 * @param[in]	nmisrc
 * @return 		None
 **********************************************************************/
void HAL_SCU_SetNMI(uint32_t nmisrc)
{
	SCU->NMIR = (0xA32CUL<<16) | (nmisrc & 0x1F);
}


/**********************************************************************//**
 * @brief	 	enable/disable Reset Source
 * @param[in]	RstSrc, oring is possible:
 * 					- RST_PINRST :    (1UL<<11)
 * 					- RST_CPURST :    (1UL<<10)
 * 					- RST_SWRST :    (1UL<<9)
 * 					- RST_WDTRST :   (1UL<<8)
 *						- RST_MCKFRST     :   (1UL<<2)
 * 					- RST_MOFRST    :    (1UL<<1)
 * 					- RST_LVDRST    :    (1UL<<0)
 * @param[in]	NewState
 * 					- ENABLE  	:Set Reset Source enable
 * 					- DISABLE 	:Disable Reset Source
 * @return 		None
 **********************************************************************/
void HAL_SCU_SetResetSrc(uint32_t RstSrc, FunctionalState NewState)
{
	if (NewState == ENABLE)
	{
		SCU->RSER |=  (RstSrc);
	}
	else
	{
		SCU->RSER &= ~(RstSrc);
	}
}

/*********************************************************************//**
 * @brief 		Get current value of RSSR Status.
 * @param[in]	None.
 * @return		Current value of RSSR Status register.
 *********************************************************************/
uint32_t HAL_SCU_GetResetSrc(void)
{
	return (SCU->RSSR);
}

/**********************************************************************//**
 * @brief	 	clear Reset Status
 * @param[in]	rststatus
 * @return 		None
 **********************************************************************/
void HAL_SCU_ClearResetStatus(uint32_t rststatus)
{
	SCU->RSSR = rststatus;
}


/* --------------------------------- End Of File ------------------------------ */
