/**********************************************************************
* @file		A31G21x_PortInit.c
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library
* @version	1.0
* @date		
* @author	ABOV M team
*
* Copyright(C) 2019, ABOV Semiconductor
* All rights reserved.
*
**********************************************************************/
#include "main_conf.h"

/* Private typedef --------------------------------------------------*/
/* Private define ---------------------------------------------------*/
/* Private macro ----------------------------------------------------*/
/* Private variables ------------------------------------------------*/
/* Private define ---------------------------------------------------*/
/* Private function prototypes --------------------------------------*/
void Port_Init(void);
/* Private variables -------------------------------------------------*/




/**********************************************************************
 * @brief		This function handles NMI exception.
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void Port_Init(void)
{
	//Peripheral Enable Register 1	0:Disable, 1:Enable 
		SCU->PER1=SCU->PER1  
		| (1<<13)	// GPIOF
		| (1<<12)	// GPIOE
		| (1<<11)	// GPIOD
		| (1<<10)	// GPIOC
		| (1<<9)	// GPIOB
		| (1<<8)	// GPIOA
			;		
	//Peripheral Clock Enable Register 1 0:Disable, 1:Enable	
		SCU->PCER1=SCU->PCER1
		| (1<<13)	// GPIOF
		| (1<<12)	// GPIOE
		| (1<<11)	// GPIOD
		| (1<<10)	// GPIOC
		| (1<<9)	// GPIOB
		| (1<<8)	// GPIOA
			;	
	
		PORT_ACCESS_EN();  // enable writing permittion of ALL PCU register
	
		//--------------------------------------------------------------
		//	PORT INIT
		//	PA
		//--------------------------------------------------------------
		// PORT - A
		PA->MOD = 0 			 // 0 : Input Mode, 1 : Output Mode,	2 : Alternative function mode
		| (0x01<<14)			 // P7
		| (0x01<<12)			 // P6
		| (0x01<<10)			 // P5
		| (0x01<<8) 			 // P4
		| (0x01<<6) 			 // P3
		| (0x01<<4) 			 // P2
		| (0x01<<2) 			 // P1
		| (0x01<<0) 			 // P0
		;
		
		PA->TYP = 0 			 // 0 : Push-pull Output,	1 : Open-drain Output
		| (0x00<<7) 			 // P7
		| (0x00<<6) 			 // P6
		| (0x00<<5) 			 // P5
		| (0x00<<4) 			 // P4
		| (0x00<<3) 			 // P3
		| (0x00<<2) 			 // P2
		| (0x00<<1) 			 // P1
		| (0x00<<0) 			 // P0
		;
		
		PA->AFSR1 = 0
		| (0x00<<28)			  // P7   0 :			,	1 :T13O 		, 2 :	T13C	, 3 : AN7/DAO			, 4 : CS2/COM10 
		| (0x00<<24)			  // P6 	0 : 			,	1 :T11O 		, 2 :	T11C	, 3 : AN6					, 4 : CS1/COM11
		| (0x00<<20)			  // P5 	0 : 			,	1 :T12O 	, 2 : T12C	, 3 : AN5					, 4 : CS0/COM12
		| (0x00<<16)			  // P4 	0 : 			,	1 : 				, 2 :				, 3 : AN4					, 4 :	CS7/COM13
		| (0x00<<12)			  // P3 	0 : 			,	1 : 				, 2 :				, 3 : AN3					, 4 :	CS6/COM14
		| (0x00<<8) 			  // P2 	0 : 			,	1 : EC12		, 2 :				, 3 : AN2/AVREF 	, 4 :	CS5/COM15
		| (0x00<<4) 			  // P1 	0 : 			,	1 : SCL1		, 2 :				, 3 : AN1					, 4 :	CS4
		| (0x00<<0) 			  // P0 	0 : 			,	1 : SDA1		, 2 :				, 3 : AN0					, 4 :	CS3
		;
	
		
		PA->PUPD = 0				// 0 : Disable Pull-up/down,	1 : Enable Pull-up, 2 : Enable Pull-down
		| (0x0<<14) 				// P7
		| (0x0<<12) 				// P6
		| (0x0<<10) 				// P5
		| (0x0<<8)					// P4
		| (0x0<<6)					// P3
		| (0x0<<4)					// P2
		| (0x0<<2)					// P1
		| (0x0<<0)					// P0
		;
		
		PA->OUTDR = 0				// 0 : Output Low,	1 : Output High
		| (0x00<<7) 				// P7
		| (0x00<<6) 				// P6
		| (0x00<<5) 				// P5
		| (0x00<<4) 				// P4
		| (0x00<<3) 				// P3
		| (0x00<<2) 				// P2
		| (0x00<<1) 				// P1
		| (0x00<<0) 				// P0
		;
	
		//--------------------------------------------------------------
		//	PORT INIT
		//	PB
		//--------------------------------------------------------------
		PB->MOD = 0 			 // 0 : Input Mode, 1 : Output Mode,	2 : Alternative function mode
		| (0x01<<14)			 // P7
		| (0x01<<12)			 // P6
		| (0x02<<10)			 // P5	 SWDIO
		| (0x02<<8) 			 // P4	 SWCLK
		| (0x01<<6) 			 // P3
		| (0x01<<4) 			 // P2
		| (0x01<<2) 			 // P1
		| (0x01<<0) 			 // P0
		;
		
		PB->TYP = 0 			 // 0 : Push-pull Output,	1 : Open-drain Output
		| (0x00<<7) 			 // P7
		| (0x00<<6) 			 // P6
		| (0x00<<5) 			 // P5
		| (0x00<<4) 			 // P4
		| (0x00<<3) 			 // P3
		| (0x00<<2) 			 // P2
		| (0x00<<1) 			 // P1
		| (0x00<<0) 			 // P0
		;
		
		PB->AFSR1 = 0
		| (0x01<<28)			  // P7 	0 : 	,	1 : RXD1	, 2 :				, 3 : AN12			, 4 : CS18
		| (0x01<<24)			  // P6 	0 : 	,	1 : TXD1	, 2 :	EC11		, 3 :	AN11			, 4 : CS17
		| (0x02<<20)			  // P5 	0 : 	,	1 : RXD0	, 2 : SWDIO 	, 3 :						, 4 :  
		| (0x02<<16)			  // P4 	0 : 	,	1 : TXD0	, 2 : SWCLK 	, 3 :						, 4 :  
		| (0x01<<12)			  // P3 	0 : 	,	1 : BOOT	, 2 : SS10		, 3 :						, 4 : 
		| (0x00<<8) 			  // P2 	0 : 	,	1 : EC13	, 2 : SCK10 	, 3 : AN10			, 4 : CS10/COM7
		| (0x00<<4) 			  // P1 	0 : 	,	1 : RXD10	, 2 : MISO10	, 3 : AN9				, 4 : CS9/COM8
		| (0x00<<0) 			  // P0 	0 : 	,	1 : TXD10	, 2 : MOSI10	, 3 : AN8				, 4 : CS8/COM9
		;
	
		
		PB->PUPD = 0			  // 0 : Disable Pull-up/down,	1 : Enable Pull-up, 2 : Enable Pull-down
		| (0x0<<14) 				// P7
		| (0x0<<12) 				// P6
		| (0x0<<10) 				// P5
		| (0x0<<8)					// P4
		| (0x0<<6)					// P3
		| (0x0<<4)					// P2
		| (0x0<<2)					// P1
		| (0x0<<0)					// P0
		;
		
		PB->OUTDR = 0			  // 0 : Output Low,	1 : Output High 
		| (0x00<<7) 				// P7
		| (0x00<<6) 				// P6
		| (0x00<<5) 				// P5
		| (0x00<<4) 				// P4
		| (0x00<<3) 				// P3
		| (0x00<<2) 				// P2
		| (0x00<<1) 				// P1
		| (0x00<<0) 				// P0
		;

		//--------------------------------------------------------------
		//	PORT INIT
		//	PC
		//--------------------------------------------------------------
		PC->MOD = 0 			 // 0 : Input Mode, 1 : Output Mode,	2 : Alternative function mode
		| (0x02<<10)			 // P5
		| (0x01<<8) 			 // P4
		| (0x01<<6) 			 // P3
		| (0x01<<4) 			 // P2
		| (0x01<<2) 			 // P1
		| (0x01<<0) 			 // P0
		;
		
		PC->TYP = 0 			 // 0 : Push-pull Output,	1 : Open-drain Output
		| (0x00<<5) 			 // P5
		| (0x00<<4) 			 // P4
		| (0x00<<3) 			 // P3
		| (0x00<<2) 			 // P2
		| (0x00<<1) 			 // P1
		| (0x00<<0) 			 // P0
		;
		
		PC->AFSR1 = 0			
		| (0x0<<20) 			 // P5		0 : nRESET	, 1 :				, 2 :					, 3 :			, 4 :				
		| (0x0<<16) 			 // P4		0 : 				, 1 :					, 2 :	SCK20		, 3 :			, 4 : CS23/COM2  
		| (0x0<<12) 			 // P3		0 : 				, 1 : EC21		, 2 :	MISO20	, 3 :			, 4 : CS22/COM3  
		| (0x0<<8)				 // P2		0 : 				, 1 : EC20		, 2 :	MOSI20	, 3 :			, 4 : CS21/COM4  
		| (0x0<<4)				 // P1		0 : 				, 1 : T21O		, 2 : T21C		, 3 :			, 4 : CS20/COM5  
		| (0x0<<0)				 // P0		0 : 				, 1 : T20O		, 2 : T20C		, 3 :  AN13 , 4 : CS19/COM6  
		;
	
		
		PC->PUPD = 0			 // 0 : Disable Pull-up/down,	1 : Enable Pull-up, 2 : Enable Pull-down
		| (0x0<<10) 			 // P5
		| (0x0<<8)				 // P4
		| (0x0<<6)				 // P3
		| (0x0<<4)				 // P2
		| (0x0<<2)				 // P1
		| (0x0<<0)				 // P0
		;
		
		PC->OUTDR = 0				// 0 : Output Low,	1 : Output High
		| (0x0<<5)					// P5
		| (0x0<<4)					// P4
		| (0x0<<3)					// P3
		| (0x0<<2)					// P2
		| (0x0<<1)					// P1
		| (0x0<<0)					// P0
		;
	
		//--------------------------------------------------------------
		//	PORT INIT
		//	PD
		//--------------------------------------------------------------
		PD->MOD = 0 			 // 0 : Input Mode, 1 : Output Mode,	2 : Alternative function mode
		| (0x01<<10)			 // P5
		| (0x01<<8) 			 // P4
		| (0x01<<6) 			 // P3
		| (0x01<<4) 			 // P2
		| (0x01<<2) 			 // P1
		| (0x01<<0) 			 // P0
		;
		
		PD->TYP = 0 			 // 0 : Push-pull Output,	1 : Open-drain Output
		| (0x00<<5) 			 // P5
		| (0x00<<4) 			 // P4
		| (0x00<<3) 			 // P3
		| (0x00<<2) 			 // P2
		| (0x00<<1) 			 // P1
		| (0x00<<0) 			 // P0
		;
		
		PD->AFSR1 = 0
		| (0x00<<20)			  // P5 	0 : 	,	1 : 			, 2 : SS11			, 3 :		, 4 : SEG6				
		| (0x00<<16)			  // P4 	0 : 	,	1 : BLNK	, 2 : SCK11 		, 3 :		, 4 : SEG7
		| (0x00<<12)			  // P3 	0 : 	,	1 : RXD11	, 2 : MISO11		, 3 :		, 4 : SEG8
		| (0x00<<8) 			  // P2 	0 : 	,	1 : TXD11	, 2 : MOSI11		, 3 :		, 4 : SEG9			
		| (0x00<<4) 			  // P1 	0 : 	,	1 : SDA0	, 2 :	EC10			, 3 :		, 4 : COM0			 
		| (0x00<<0) 			  // P0 	0 : 	,	1 : SCL0	, 2 :	SS20			, 3 :		, 4 : COM1			 
		;
		
		PD->PUPD = 0				// 0 : Disable Pull-up/down,	1 : Enable Pull-up, 2 : Enable Pull-down
		| (0x0<<10) 				// P5
		| (0x0<<8)					// P4
		| (0x0<<6)					// P3
		| (0x0<<4)					// P2
		| (0x0<<2)					// P1
		| (0x0<<0)					// P0
		;
		
		PD->OUTDR = 0			  // 0 : Output Low,	1 : Output High
		| (0x00<<5) 				// P5
		| (0x00<<4) 				// P4
		| (0x00<<3) 				// P3
		| (0x00<<2) 				// P2
		| (0x00<<1) 				// P1
		| (0x00<<0) 				// P0
		;
	
		//--------------------------------------------------------------
		//	PORT INIT
		//	PE
		//--------------------------------------------------------------
		PE->MOD = 0 			 // 0 : Input Mode, 1 : Output Mode,	2 : Alternative function mode
		| (0x01<<14)			 // P7
		| (0x01<<12)			 // P6
		| (0x01<<10)			 // P5
		| (0x01<<8) 			 // P4
		| (0x01<<6) 			 // P3
		| (0x01<<4) 			 // P2
		| (0x01<<2) 			 // P1
		| (0x01<<0) 			 // P0
		;
		
		PE->TYP = 0 			 // 0 : Push-pull Output,	1 : Open-drain Output
		| (0x00<<7) 			 // P7
		| (0x00<<6) 			 // P6
		| (0x00<<5) 			 // P5
		| (0x00<<4) 			 // P4
		| (0x00<<3) 			 // P3
		| (0x00<<2) 			 // P2
		| (0x00<<1) 			 // P1
		| (0x00<<0) 			 // P0
		;
		
		PE->AFSR1 = 0
		| (0x00<<28)			 // P7		0 : 		, 1 : T11O			, 2 : T11C		, 3 :		, 4 : CS11/SEG5
		| (0x00<<24)			 // P6		0 : 		, 1 : T10O			, 2 : T10C		, 3 :		, 4 : CS12/SEG4
		| (0x00<<20)			 // P5		0 : 		, 1 : PWM30CB		, 2 :	MOSI21	, 3 :		, 4 : CS13/SEG3
		| (0x00<<16)			 // P4		0 : 		, 1 : PWM30CA		, 2 :	MISO21	, 3 :		, 4 : CS14/SEG2 
		| (0x00<<12)			 // P3		0 : 		, 1 : PWM30BB		, 2 :	SCK21		, 3 :		, 4 : CS15/SEG1 
		| (0x00<<8) 			 // P2		0 : 		, 1 : PWM30BA		, 2 :	SS21		, 3 :		, 4 : CS16/SEG0
		| (0x00<<4) 			 // P1		0 : 		, 1 : PWM30AB		, 2 :					, 3 :		, 4 :	
		| (0x00<<0) 			 // P0		0 : 		, 1 : PWM30AA		, 2 :	SS11		, 3 :		, 4 : 
		;
	
		
		PE->PUPD = 0			// 0 : Disable Pull-up/down,	1 : Enable Pull-up, 2 : Enable Pull-down
		| (0x0<<14) 			// P7
		| (0x0<<12) 			// P6
		| (0x0<<10) 			// P5
		| (0x0<<8)				// P4
		| (0x0<<6)				// P3
		| (0x0<<4)				// P2
		| (0x0<<2)				// P1
		| (0x0<<0)				// P0
		;
		
		PE->OUTDR = 0			// 0 : Output Low,	1 : Output High
		| (0x0<<7)				// P7
		| (0x0<<6)				// P6
		| (0x0<<5)				// P5
		| (0x0<<4)				// P4
		| (0x0<<3)				// P3
		| (0x0<<2)				// P2
		| (0x0<<1)				// P1
		| (0x0<<0)				// P0
		;
		
		//--------------------------------------------------------------
		//	PORT INIT
		//	PF
		//--------------------------------------------------------------
		PF->MOD = 0 			 // 0 : Input Mode, 1 : Output Mode,	2 : Alternative function mode
		| (0x01<<14)			 // P7
		| (0x01<<12)			 // P6
		| (0x01<<10)			 // P5
		| (0x02<<8) 			 // P4	   clko
		| (0x01<<6) 			 // P3
		| (0x01<<4) 			 // P2
		| (0x02<<2) 			 // P1
		| (0x02<<0) 			 // P0
		;
		
		PF->TYP = 0 						// 0 : Push-pull Output,	1 : Open-drain Output
		| (0x0<<7)				// P7
		| (0x0<<6)				// P6
		| (0x0<<5)				// P5
		| (0x0<<4)				// P4
		| (0x0<<3)				// P3
		| (0x0<<2)				// P2
		| (0x0<<1)				// P1
		| (0x0<<0)				// P0
		;
		
		PF->AFSR1 = 0
		| (0x0<<28) 			 // P7		0 : 			, 1 : T30C		, 2 : SDA0		, 3 :					, 4 :				 
		| (0x0<<24) 			 // P6		0 : 			, 1 : EC30		, 2 : SCL0		, 3 :					, 4 :				 
		| (0x0<<20) 			 // P5		0 : 			, 1 : BLNK		, 2 :			, 3 :					, 4 :				 
		| (0x1<<16) 			 // P4		0 : 			, 1 : CLKO		, 2 :					, 3 :				, 4 :				 
		| (0x0<<12) 			 // P3		0 : 			, 1 : RXD1		, 2 :	T30C		, 3 : SXOUT 	, 4 :				 
		| (0x0<<8)				 // P2		0 : 			, 1 : TXD1		, 2 :	EC30		, 3 : SXIN		, 4 :				 
		| (0x3<<4)				 // P1		0 : 			, 1 : SDA1		, 2 :					, 3 : XIN			, 4 :				 
		| (0x3<<0)				 // P0		0 : 			, 1 : SCL1		, 2 :					, 3 : XOUT		, 4 :	 
		;
	
		PF->PUPD = 0			 // 0 : Disable Pull-up/down,	1 : Enable Pull-up, 2 : Enable Pull-down
		| (0x0<<14) 			 // P7
		| (0x0<<12) 			 // P6
		| (0x0<<10) 			 // P5
		| (0x0<<8)				 // P4
		| (0x0<<6)				 // P3
		| (0x0<<4)				 // P2
		| (0x0<<2)				 // P1
		| (0x0<<0)				 // P0
		;
		
		PF->OUTDR = 0			   // 0 : Output Low,	1 : Output High 
		| (0x00<<7) 			 // P7
		| (0x00<<6) 			 // P6
		| (0x00<<5) 			 // P5
		| (0x00<<4) 			 // P4
		| (0x00<<3) 			 // P3
		| (0x00<<2) 			 // P2
		| (0x00<<1) 			 // P1
		| (0x00<<0) 			 // P0
		;
		 /* disable writing permittion of ALL PCU register */ 
			PORT_ACCESS_DIS(); 
	
}

