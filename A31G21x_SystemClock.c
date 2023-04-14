/***********************************************************************
* @file		A31G21x_SystemClock.c
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library
* @version	1.00
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
***********************************************************************/
/***********************************************************************
 *                          A31G21x Device 
 *------------------------------------------------------------------------
 *    System Clock source       | High Speend Internal oscillator (HSI)
 *------------------------------------------------------------------------
 *    SYSCLK(MHz)               | 32MHz
 *------------------------------------------------------------------------
 *    HCLK(MHz) - Core Clock    | 32MHz
 *------------------------------------------------------------------------
  *   PCLK(MHz) - Peri Clock    | 32MHz
 *------------------------------------------------------------------------
*************************************************************************/
#include "main_conf.h"

/* Private typedef ------------------------------------------------------*/
/* Private define -------------------------------------------------------*/
/* Private macro --------------------------------------------------------*/
/* Private variables ----------------------------------------------------*/
/* Private define -------------------------------------------------------*/

#define USED_HSI          /* SystemClock HSI : 32MHz    */
/*#define USED_LSI */  		/* SystemClock LSI 			: 500KHz   */
/*#define USED_HSE */  		/* SystemClock  HSE 		: 8MHz     */
/*#define USED_LSE */		/* SystemClock LSE 			: 32.768KHz*/
/*#define USED_HSEPLL */ 	/* SystemClock HSE PLL 	: 48MHz    */ 
/*#define USED_HSIPLL */		/* SystemClock HSI PLL	: 48MHz    */  

/* System Clock source */ 		
#define HSI_OSC (32000000)    

/* Clock Out Selection ( Monitoring )*/
/*#define USED_CLKO*/


/* Private function prototypes ------------------------------------------*/
void SystemClock_Config (void);
/* Private variables ----------------------------------------------------*/


/**************************************************************************
 * @brief			Initialize default clock for A34M41x Board
 * @param[in]		None
 * @return			None
 **************************************************************************/
void SystemClock_Config (void)
{
	uint32_t i;

 /*  LSI Clock Setting  500kHz */	
#ifdef USED_LSI			
	HAL_SCU_LSI_ClockConfig(LSIOSC_EN); /*LSIOSC_EN_DIV2, LSIOSC_EN_DIV4*/
	SystemCoreClock=500000; 	/*500KHz*/
	SystemPeriClock=500000; 	/*500KHz*/	
	for (i=0;i<10;i++);	
	
	HAL_SCU_SystemClockChange(SC_LSIOSC);
#endif 

/*  SOSC Clock Setting  32.768khz */	
#ifdef USED_LSE  
	HAL_SCU_LSE_ClockConfig(EXSOSC_EN);
	SystemCoreClock=32768; /*32.768khz*/
	SystemPeriClock=32768; /*32.768khz*/	

	for (i=0;i<10;i++);	
	
	//HAL_SCU_LSE_ClockMonitoring();
	HAL_SCU_SystemClockChange(SC_SOSC);
#endif 	

/*  HSI Clock Setting  32kHz */
#ifdef USED_HSI		
	HAL_SCU_HSI_ClockConfig(HSIOSC_EN);
	SystemCoreClock=HSI_OSC; /*32MHz*/
	SystemPeriClock=HSI_OSC; /*32MHz*/	
	
	for (i=0;i<10;i++);	

	HAL_SCU_SystemClockChange(SC_HSIOSC);
#endif 	

/*  HSE Clock Setting  8Mhz */
#ifdef USED_HSE	

	HAL_SCU_HSE_ClockConfig(EXOSC_EN);
	SystemCoreClock=8000000uL; /*8MHz*/
	SystemPeriClock=8000000uL; /*8MHz*/

	for (i=0;i<10;i++);	

	//HAL_SCU_HSE_ClockMonitoring();
	HAL_SCU_SystemClockChange(SC_EXOSC);
#endif


/*  HSE PLL Clock Setting  48Mhz */
#ifdef USED_HSEPLL
  
  /*SCU_CSCR= Enable External main oscillator control*/	
  HAL_SCU_HSE_ClockConfig(EXOSC_EN);
	
	/*wait for stabilizing*/
	for (i=0;i<10;i++);
	
	/*HSE clock is used as FIN clock*/
	HAL_SCU_SystemClockFinClock(SC_FIN_HSE);
	/**
	************************************************************************************************
	* PLL setting 
	* FIN=PLLINCLK/(R+1)                                 			// R: Pre Divider   
	* FOUT=(FIN*(N1+1)*(D+1))  / ((N2+1)*(P+1))          			// N1: Post Divider1,N2:Post Div2, P:Out Div,      
	*             = FVCO *(D+1)                         		 	// D:Frequency Doubler
	*
	*ex)    FIN=PLLINCLK/(R+1) = 8M/(3+1) = 2M            		// R:3, PLLINCLK:8MHz(HSE)
	*       FOUT=(2M*(47+1)*(0+1)) / ((1+1)*(0+1) = 48MHz   // N1:47, D:0, N2:1, P:0
	*
	************************************************************************************************
	*/
	if (HAL_SCU_PLL_ClockConfig(ENABLE,  
		PLLCON_BYPASS_PLL,    		        /*PLLCON_BYPASS_FIN:0, PLLCON_BYPASS_PLL:1*/
		0,                                /*0:FOUT==VCO, 1:FOUT==2xVCO,  D=0*/
		3,                                /*PREDIV, R=1*/
		47,                               /*POSTDIV1, N1=47  */
		1,                                /*POSTDIV2, N2=1*/
		0)==ERROR)                    		/*OUTDIV P=0*/
	{
		/*error : user code*/
	}
        /* HSE -->  HSEPLL : PLL output clock*/
	HAL_SCU_SystemClockChange(SC_EXOSCPLL); 
        
	SystemCoreClock=48000000uL; 	/*48MHz*/
	SystemPeriClock=48000000uL;  	/*48MHz*/
#endif




/*  HSI PLL Clock Setting  48Mhz */	
#ifdef USED_HSIPLL

	/*SCU_CSCR= Enable High speed internal oscillator control*/	
	HAL_SCU_HSI_ClockConfig(HSIOSC_EN_DIV2);   /* 32MHz/2 = 16MHz */	
	
	/*wait for stabilizing*/
	for (i=0;i<10;i++);	

	/*HSE clock is used as FIN clock*/
	HAL_SCU_SystemClockFinClock(SC_FIN_HSI);
	
	/************************************************************************************************
	* PLL setting 
	* FIN=PLLINCLK/(R+1)                                 		// R: Pre Divider   
	* FOUT=(FIN*(N1+1)*(D+1))  / ((N2+1)*(P+1))          		// N1: Post Divider1,N2:Post Div2, P:Out Div,      
	*             = FVCO *(D+1)                         		// D:Frequency Doubler
	*
	*ex)    FIN=PLLINCLK/(R+1) = 16M/(7+1) = 2M            	        // R:7, PLLINCLK:16MHz(HSI)
	*       FOUT=(2M*(47+1)*(0+1)) / ((1+1)*(0+1)) = 48MHz          // N1:47, D:0, N2:1, P:0
	*
	************************************************************************************************/
	if (HAL_SCU_PLL_ClockConfig(ENABLE,  
		PLLCON_BYPASS_PLL,    /*PLLCON_BYPASS_FIN:0, PLLCON_BYPASS_PLL:1*/
		0,                    /*0:FOUT==VCO, 1:FOUT==2xVCO,  D=0*/
		7,                    /*PREDIV, R=7*/
		47,                   /*POSTDIV1, N1=47  */
		1,                    /*POSTDIV2, N2=1*/
		0)==ERROR)            /*OUTDIV P=0*/
	{
		/*error : user code*/
	}
        
        /*HSI -->  HSIPLL : PLL output clock*/
	HAL_SCU_SystemClockChange(SC_HSIOSCPLL); 
  
	SystemCoreClock=48000000uL; 	/*48MHz */
	SystemPeriClock=48000000uL; 	/*48MHz*/
	
#endif

        /* mclk monitoring enable */
	/*SCU->CMR|=(1<<7); */

        /* flash memory controller */
	FMC->MR = 0x81;       							/* after changing 0x81 -> 0x28 in MR reg, flash access timing will be able to be set.*/
	FMC->MR = 0x28;       							/* enter flash access timing changing mode*/
	FMC->CFG = (0x7858<<16) | (2<<8);  	/* WAIT is 011, flash access (2-wait) 	*/	
																			/* ex) if MCLK=48Mhz, */
																			/* 0wait = 48 (can't set no wait)*/
																			/* 1wait = 24 (1 wait is ok)*/
																			/* 2wait = 16 (2 wait is ok)*/
																			/* so, 2 wait is possible.*/
	FMC->MR = 0;	      			/* exit flash access timing --> normal mode		*/		
	
/*	CLKO function setting. check PORT setting (PF4) */
#ifdef USED_CLKO
	HAL_SCU_ClockOutput(4,ENABLE); 
#else
	HAL_SCU_ClockOutput(4,DISABLE);
#endif
}



