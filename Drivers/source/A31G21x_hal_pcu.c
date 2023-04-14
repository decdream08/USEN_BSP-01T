/**********************************************************************//**
* @file		A31G21x_hal_pcu.c
* @brief	Contains all functions support for PCU firmware library on A31G21x
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


/* Includes --------------------------------------------------------- */
#include "A31G21x_hal_pcu.h"
#include "A31G21x_hal_libcfg.h"





/**********************************************************************//**
* @brief		Initialize GPIO(PCU) peripheral
* @param      	PCx : Pointer to selected PCU peripheral, should be: A,B,C,F
* @param      	GPIO_Init : Pointer contains configuration of GPIO
* @return		None
 **********************************************************************/
void HAL_GPIO_Init(PORT_Type *PCx, GPIO_CFG_Type *GPIO_Init)
{
	uint8_t pin_offset; 
	uint32_t reg_val; 
	uint32_t dir_type_temp;
		
	/* Check the parameters */
	//CHECK_PARAM(PCx != NULL);
	
	/* Check the parameters */
	//CHECK_PARAM(GPIO_Init != NULL);
	
	PORT_ACCESS_EN();	
      
	//Function
      if (GPIO_Init->Pin<8){ // 0~7
		//------------------------------------------------------------------------------
		// pin_offset = pin_no * 4
		//------------------------------------------------------------------------------
		pin_offset = (GPIO_Init->Pin << 2); 

		//------------------------------------------------------------------------------
		// MR
		//------------------------------------------------------------------------------
		reg_val = PCx->AFSR1;
		reg_val &= ~(PCU_FUNC_Msk << pin_offset); 
		reg_val |= (GPIO_Init->Function << pin_offset); 

		PCx->AFSR1=reg_val;
	}
	/* Output */
	
	dir_type_temp = GPIO_Init->Output;
	if (dir_type_temp==OPEN_DRAIN_OUTPUT)
		 GPIO_Init->Output = PUSH_PULL_OUTPUT;
	//------------------------------------------------------------------------------
	// pin_offset = pin_no * 2
	//------------------------------------------------------------------------------
	pin_offset = (GPIO_Init->Pin << 1); 
	
	PORT_ACCESS_EN();
	
	//------------------------------------------------------------------------------
	// PnMOD
	//------------------------------------------------------------------------------
	reg_val = PCx->MOD; 
	reg_val &= ~(PCU_MODE_Msk << pin_offset); 
	reg_val |= ( GPIO_Init->Output << pin_offset); 
	PCx->MOD=reg_val; 


	//------------------------------------------------------------------------------
	// PnTYP
	//------------------------------------------------------------------------------
	reg_val = PCx->TYP; 
	reg_val &= ~(1 << GPIO_Init->Pin); 	
	if (dir_type_temp==OPEN_DRAIN_OUTPUT){
		reg_val |= (1 << GPIO_Init->Pin); 
	}
	PCx->TYP=reg_val; 
	
	/*pullup*/
	pin_offset = (GPIO_Init->Pin << 1); 
	
	
	reg_val = PCx->PUPD; 
	reg_val &= ~(3<<pin_offset); 
	reg_val |= (GPIO_Init->Pull<<pin_offset); 
	PCx->PUPD=reg_val; 
	
	PORT_ACCESS_DIS();
}

/**********************************************************************//**
 * @brief 		Configure pin function
 * @param[in]	PCx	Pointer to selected PCU peripheral, should be: A,B,C,D,E,F
 * @param[in]	pin_no		Pin number, should be: 0..15
 * @param[in]	func 	Function mode, should be:
 * 					- FUNC0		:Function 0
 * 					- FUNC1		:Function 1
 * 					- FUNC2		:Function 2
 * 					- FUNC3		:Function 3
 * 					- FUNC4		:Function 4 
 * @return		None
 **********************************************************************/
void HAL_GPIO_ConfigFunction(PORT_Type *PCx, uint8_t pin_no, uint32_t func)
{
	uint8_t pin_offset; 
	uint32_t reg_val; 
	
	PORT_ACCESS_EN();	
	
	if (pin_no<8){ // 0~7
		//------------------------------------------------------------------------------
		// pin_offset = pin_no * 4
		//------------------------------------------------------------------------------
		pin_offset = (pin_no << 2); 

		//------------------------------------------------------------------------------
		// MR
		//------------------------------------------------------------------------------
		reg_val = PCx->AFSR1;
		reg_val &= ~(PCU_FUNC_Msk << pin_offset); 
		reg_val |= (func << pin_offset); 

		PCx->AFSR1=reg_val;
	}

	
	PORT_ACCESS_DIS();
}

/**********************************************************************//**
 * @brief 		Configure pin mode
 * @param[in]	PCx	Pointer to selected PCU peripheral, should be: A,B,C,D,E,F
 * @param[in]	pin_no		Pin number, should be: 0..15
 * @param[in]	dir_type	Pin mode, should be:
 * 					- INPUT                           :0
 * 					- PUSH_PULL_OUTPUT   :1
 * 					- ALTERN_FUNC              :2
 * 					- Reserved :3
 * @return		None
 **********************************************************************/
void HAL_GPIO_ConfigOutput(PORT_Type *PCx, uint8_t pin_no, PCU_PORT_MODE dir_type)
{
	uint8_t pin_offset; 
	uint32_t reg_val; 
	uint32_t dir_type_temp;
	
	dir_type_temp = dir_type;
	if (dir_type_temp==OPEN_DRAIN_OUTPUT)
		dir_type = PUSH_PULL_OUTPUT;
	//------------------------------------------------------------------------------
	// pin_offset = pin_no * 2
	//------------------------------------------------------------------------------
	pin_offset = (pin_no << 1); 
	
	PORT_ACCESS_EN();
	
	//------------------------------------------------------------------------------
	// MOD
	//------------------------------------------------------------------------------
	reg_val = PCx->MOD; 
	reg_val &= ~(PCU_MODE_Msk << pin_offset); 
	reg_val |= (dir_type << pin_offset); 
	PCx->MOD=reg_val; 


	//------------------------------------------------------------------------------
	// TYP
	//------------------------------------------------------------------------------
	reg_val = PCx->TYP; 
	reg_val &= ~(1 << pin_no); 	
	if (dir_type_temp==OPEN_DRAIN_OUTPUT){
		reg_val |= (1 << pin_no); 
	}
	PCx->TYP=reg_val; 
	
	PORT_ACCESS_DIS();
}

/**********************************************************************//**
 * @brief 		Configure pin pullup
 * @param[in]	PCx	Pointer to selected PCU peripheral, should be: A,B,C,D,E,F
 * @param[in]	pin_no		Pin number, should be: 0..15
 * @param[in]	pullupdown state, should be:
 * 					- 0: DISABLE
 * 					- 1: Enable pull-up resistor 		
 * 					- 2: Enable pull-down resistor 
 * 					- 3: reserved 
 * @return		None
 **********************************************************************/
void HAL_GPIO_ConfigPullup (PORT_Type *PCx, uint8_t pin_no, uint8_t pullupdown)
{
	uint32_t	reg_val; 
	uint8_t pin_offset; 
	
	pin_offset = (pin_no << 1); 
	
	PORT_ACCESS_EN();
	
	reg_val = PCx->PUPD; 
	reg_val &= ~(3<<pin_offset); 
	reg_val |= (pullupdown<<pin_offset); 
	PCx->PUPD=reg_val; 
	
	PORT_ACCESS_DIS();

}





/**********************************************************************//**
 * @brief 		Configure out data Mask
 * @param[in]	PCx	Pointer to selected PCU peripheral, should be: A,B,C,D,E,F
 * @param[in]	pin_no		Pin number, should be: 0..15
 * @param[in]	maskctrl state, should be:
 * 					- 0: DISABLE(Unmask)-changed 
 * 					- 1: ENABLE(Mask)-protected
 * @return		None
 **********************************************************************/
void HAL_GPIO_ConfigOutDataMask (PORT_Type *PCx, uint8_t pin_no, FunctionalState maskctrl)
{
	uint32_t	reg_val; 
	
	PORT_ACCESS_EN();
	
	reg_val = PCx->OUTDMSK; 
	reg_val &= ~(1<<pin_no); 
	reg_val |= (maskctrl<<pin_no); 
	PCx->OUTDMSK=reg_val; 
	
	PORT_ACCESS_DIS();

}

/**********************************************************************//**
 * @brief		Enable GPIO interrupt
 * @param[in]	PCx	Pointer to selected PCU peripheral, should be: A,B,C,D,E,F
 * @param[in]	pin_no		Pin number, should be: 0..15
 * @param[in]	pin_en 	Value that contains all bits on GPIO to enable,
 * 					- IER_DISABLE = 0
 *						- IER_LEVEL = 1
 *						- IER_EDGE = 3				
 * @param[in]	int_mode		interrupt mode, should be:
 * 					- ICR_PROHIBIT_INT = 0
 *                     if level
 * 					- ICR_LOW_LEVEL_INT = 1
 * 					- ICR_HIGH_LEVEL_INT = 2
 *                     if level
 * 					- ICR_FALLING_EDGE_INT = 1
 * 					- ICR_RISING_EDGE_INT = 2
 * 					- ICR_BOTH_EDGE_INT = 3
 * @return		None
 **********************************************************************/
void HAL_GPIO_EXTI_Config(PORT_Type *PCx, uint8_t pin_no, uint8_t pin_en, uint8_t int_mode)
{
	uint32_t temp_reg;
	
	PORT_ACCESS_EN();
	// port n interrupt enable: disable or level or edge
	temp_reg = PCx->IER & (uint32_t)(~(3 << (pin_no<<1)));
	temp_reg |= (uint32_t)(pin_en <<  (pin_no<<1));	
	PCx->IER = temp_reg;

	switch (pin_en){
		case 0: // disable 
			PCx->ICR = PCx->ICR & (uint32_t)(~(3 <<  (pin_no<<1)));
			break;			
		case 1: // level
		case 3: // edge			
			temp_reg = PCx->ICR & (uint32_t)(~(3 <<  (pin_no<<1)));
			temp_reg |= (uint32_t)(int_mode <<  (pin_no<<1));		
			PCx->ICR = temp_reg;
			break;			
		default:
			break;
	}
	
	PORT_ACCESS_DIS();

}


/**********************************************************************//**
 * @brief		Get GPIO Interrupt Status
 * @param[in]	PCx	Pointer to selected PCU peripheral, should be: A,B,C,D,E,F
 * @return		ISR register value 
 **********************************************************************/
uint32_t HAL_GPIO_EXTI_GetState(PORT_Type *PCx)
{
	return (PCx->ISR);
}

/**********************************************************************//**
 * @brief		Clear GPIO interrupt status
 * @param[in]	PCx	Pointer to selected PCU peripheral, should be: A,B,C,D,E,F
 * @return		None
 **********************************************************************/
void HAL_GPIO_EXTI_ClearPin(PORT_Type *PCx, uint32_t value)
{
	PORT_ACCESS_EN();
	
	PCx->ISR = value;
	
	PORT_ACCESS_DIS();

}


/**********************************************************************//**
* @brief  	Toggle the specified GPIO pin.
* @param  	PCx where x can be (A..F) to select the GPIO peripheral 
* @param  	GPIO_Pin specifies the pin to be toggled.
* @retval 	None
**********************************************************************/
void HAL_GPIO_TogglePin(PORT_Type* PCx, uint16_t GPIO_Pin)
{
	PORT_ACCESS_EN();
	
	PCx->OUTDR ^= GPIO_Pin;
	
	PORT_ACCESS_DIS();
	
	
}



/**********************************************************************//**
 * @brief		Set Value for bits that have output direction on GPIO port.
 * @param[in]	Pn selected port, should be in range from PA to PF
 * @param[in]	bitValue Value that contains all bits on GPIO to set, should
 * 				be in range from 0 to 0xFF.
 * 				example: value 0x3 to set bit 0 and bit 1.
 * @return		None
 **********************************************************************/
void HAL_GPIO_SetPin(PORT_Type *Px, uint16_t bitValue)
{
	PORT_ACCESS_EN();
	
	Px->BSR = bitValue;
	
	PORT_ACCESS_DIS();
	
}


/**********************************************************************//**
 * @brief		Clear Value for bits that have output direction on GPIO port.
 * @param[in]	Pn selected port, should be in range from PA to PF
 * @param[in]	bitValue Value that contains all bits on GPIO to clear, should
 * 				be in range from 0 to 0xFF.
 * 				example: value 0x3 to clear bit 0 and bit 1.
 * @return		None
 **********************************************************************/
void HAL_GPIO_ClearPin(PORT_Type *Pn, uint16_t bitValue)
{
	PORT_ACCESS_EN();
	
	Pn->BCR = bitValue;
	
	PORT_ACCESS_DIS();
	
}


/**********************************************************************//**
 * @brief		Read Current state on port pin that have input direction of GPIO
 * @param[in]	Pn selected port, should be in range from PA to PF
 * @return		Current value of GPIO port.
 *
 * Note: Return value contain state of each port pin (bit) on that GPIO regardless
 * its direction is input or output.
 **********************************************************************/
uint16_t HAL_GPIO_ReadPin(PORT_Type *Pn)
{
	return Pn->INDR;
}

/**********************************************************************//**
 * @brief		Write Value on port that have output direction of GPIO
 * @param[in]	Pn selected port, should be in range from PA to PF
 * @return		None
 **********************************************************************/
void HAL_GPIO_WritePin(PORT_Type *Pn, uint16_t Value)
{		
	PORT_ACCESS_EN();
	
	Pn->OUTDR = Value;
	
	PORT_ACCESS_DIS();
	
}

/**********************************************************************//**
   @brief        Set PCU Debounce  The Debounce is for PB[7:0], PC[3:0], PE[3:0]
   @param[in]    *Pn                Target Port:                          PB, PC, PE
   @param[in]    u32Pins            Pins for Debounce Filter
   @param[in]    u32Debnc           Sampling Clock Selection:             HCLK_1, HCLK_4, HCLK_16, HCLK_64, HCLK_256, HCLK_1024
 *
 * @return       None
 *
 * @explain      This function set the debounce filter sampling clock and pins for PA, PB, PC, PD, PE, PF
 **********************************************************************/

void HAL_GPIO_SetDebouncePin(PORT_Type *Pn, uint32_t u32Pins)
{
	uint32_t reg_val; 
	
	PORT_ACCESS_EN();
	
	reg_val = (0x00ff & Pn->DBCR);
	reg_val |= (0x01 << u32Pins);
	Pn->DBCR = reg_val;
	
	PORT_ACCESS_DIS();
	
}

/* --------------------------------- End Of File ------------------------------ */
