/**********************************************************************//**
* @file		A31G21x_hal_pcu.h
* @brief	Contains all macro definitions and function prototypes
* 			support for PCU firmware library on A31G21x
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


#ifndef _PCU_H_
#define _PCU_H_

/* Includes ------------------------------------------------------------------- */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Private macros ------------------------------------------------------------- */
#define PORT_ACCESS_EN()    do { PCU->PORTEN=0x15; PCU->PORTEN=0x51; } while(0)
#define PORT_ACCESS_DIS()  do { PCU->PORTEN=0x00; } while(0) 

#define PCU_FUNC_Msk                        (0x0FUL)                            
#define PCU_MODE_Msk                        (0x03UL)     


/* Pin function */
#define FUNC0 			0x0				/** Function 0 	*/
#define FUNC1 			0x1				/** Function 1 	*/
#define FUNC2 			0x2				/** Function 2	*/
#define FUNC3 			0x3				/** Function 3	*/
#define FUNC4 			0x4				/** Function 4	*/


/*
 * @brief 	PCU port mode enumerate definition
 */
typedef enum {
	INPUT=0,
	PUSH_PULL_OUTPUT,
	ALTERN_FUNC,	
	OPEN_DRAIN_OUTPUT
}PCU_PORT_MODE;


#define DISPUPD			0x0				/** disable pu_pd */
#define ENPU   			0x1				/** enable pull up */
#define ENPD   	 		0x2				/** enable pull down */


enum {
	IER_DISABLE = 0,
	IER_LEVEL,
	IER_EDGE = 3
};

enum {
	ICR_PROHIBIT_INT = 0,
	ICR_LOW_LEVEL_INT = 1,
	ICR_FALLING_EDGE_INT = ICR_LOW_LEVEL_INT,
	ICR_HIGH_LEVEL_INT =2,
	ICR_RISING_EDGE_INT = ICR_HIGH_LEVEL_INT,
	ICR_BOTH_EDGE_INT =3	
};


/* 
 * @brief   GPIO Init structure definition  
 */
typedef struct
{
  uint32_t Pin;       /*!< Specifies the GPIO pins to be configured.
                           This parameter can be any value of @ref PCU_pins */

  uint32_t Output;      /*!< Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref PCU_ Output */

  uint32_t Pull;      /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
                           This parameter can be a value of @ref PCU_pull */

  uint32_t Function;  /*!< Peripheral to be connected to the selected pins 
                            This parameter can be a value of @ref function_selection */
}GPIO_CFG_Type;




void HAL_GPIO_Init(PORT_Type *PCx, GPIO_CFG_Type *GPIO_Init);
void HAL_GPIO_SetPin(PORT_Type *Px, uint16_t bitValue);
void HAL_GPIO_ClearPin(PORT_Type *Pn, uint16_t bitValue);
uint16_t HAL_GPIO_ReadPin(PORT_Type *Pn);
void HAL_GPIO_WritePin(PORT_Type *Pn, uint16_t Value);

void HAL_GPIO_ConfigFunction(PORT_Type *PCx, uint8_t pin_no, uint32_t func);
void HAL_GPIO_ConfigOutput(PORT_Type *PCx, uint8_t pin_no, PCU_PORT_MODE dir_type);
void HAL_GPIO_ConfigPullup (PORT_Type *PCx, uint8_t pin_no, uint8_t pullupdown);
void HAL_GPIO_ConfigureOutDataMask (PORT_Type *PCx, uint8_t pin_no, FunctionalState maskctrl);

void HAL_GPIO_EXTI_Config(PORT_Type *PCx, uint8_t pin_no, uint8_t pin_en, uint8_t int_mode);
uint32_t HAL_GPIO_EXTI_GetState(PORT_Type *PCx);
void HAL_GPIO_EXTI_ClearPin(PORT_Type *PCx, uint32_t value);

void HAL_GPIO_SetDebouncePin(PORT_Type *Pn, uint32_t u32Pins);



#ifdef __cplusplus
}
#endif

#endif /* end _PCU_H_ */

/* --------------------------------- End Of File ------------------------------ */
