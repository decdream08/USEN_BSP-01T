/**********************************************************************//**
* @file		A31G21x_hal_pwr.h
* @brief	Contains all macro definitions and function prototypes
* 			support for Power Control firmware library on A31G21x
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


#ifndef _A31G21X_PWR_H_
#define _A31G21X_PWR_H_

/* Includes -------------------------------------------------------------- */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"



/* LVI Definition ----------------------------------------------------------- */
#define LVI_DISABLE					(0x0uL<<7)
#define LVI_ENABLE					(0x1uL<<7)

#define LVI_INTDIS					(0x0uL<<5)
#define LVI_INTEN					(0x1uL<<5)

#define LVI_LVL_1_88V				(0x3uL<<0)
#define LVI_LVL_2_00V				(0x4uL<<0)
#define LVI_LVL_2_13V				(0x5uL<<0)
#define LVI_LVL_2_28V				(0x6uL<<0)
#define LVI_LVL_2_46V				(0x7uL<<0)
#define LVI_LVL_2_67V				(0x8uL<<0)
#define LVI_LVL_3_04V				(0x9uL<<0)
#define LVI_LVL_3_20V				(0xAuL<<0)
#define LVI_LVL_3_55V				(0xBuL<<0)
#define LVI_LVL_3_75V				(0xCuL<<0)
#define LVI_LVL_3_99V				(0xDuL<<0)
#define LVI_LVL_4_25V				(0xEuL<<0)
#define LVI_LVL_4_55V				(0xFuL<<0)



/**
 * @brief  LVI configuration structure definition
 */
typedef struct {
	uint32_t	EnSel;
	uint32_t	IntSel;
	
	uint32_t	LvlSel;
} LVI_CFG_Type;



#ifdef __cplusplus
extern "C"
{
#endif

/* Clock Generator */
void HAL_PWR_EnterSleepMode(void);
void HAL_PWR_EnterPowerDownMode(void);
HAL_Status_Type HAL_LVI_Init(LVI_CFG_Type *LVI_ConfigStruct);


#ifdef __cplusplus
}
#endif

#endif /* _A31G21X_PWR_H_ */
/* --------------------------------- End Of File ------------------------------ */
