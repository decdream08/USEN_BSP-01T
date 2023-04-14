/**********************************************************************//**
* @file		A31G21x_hal_i2c.h
* @brief	Contains all macro definitions and function prototypes
* 			support for I2C firmware library on A31G21X
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


#ifndef _I2C_H_
#define _I2C_H_

/* Includes ------------------------------------------------------------ */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__ARMCC_VERSION)
  #pragma anon_unions
#endif

/** I2C Slave Address registers bit mask */
#define I2C_I2ADR_BITMASK			((0xFF))

/* I2C state handle return values */
#define RECEIVE_MODE		1
#define TRANS_MODE			2
#define RECEIVE_DATA		3
#define TRANS_DATA			4
#define LOST_BUS				5
#define STOP_DECT			6


/* Public Types --------------------------------------------------------------- */
/**
 * @brief Master transfer setup data structure definitions
 */
typedef struct
{
  uint32_t          sl_addr7bit;				/*!< Slave address in 7bit mode */
  uint8_t*          tx_data;					/*!< Pointer to Transmit data - NULL if data transmit is not used */
  uint32_t          tx_length;					/*!< Transmit data length - 0 if data transmit is not used*/
  uint32_t          tx_count;					/*!< Current Transmit data counter */
  uint8_t*          rx_data;					/*!< Pointer to Receive data - NULL if data receive is not used */
  uint32_t          rx_length;					/*!< Receive data length - 0 if data receive is not used */
  uint32_t          rx_count;					/*!< Current Receive data counter */
} I2C_M_SETUP_Type;

/**
 * @brief Slave transfer setup data structure definitions
 */
typedef struct
{
  uint8_t*          tx_data;					/*!< Pointer to transmit data - NULL if data transmit is not used */
  uint32_t          tx_length;					/*!< Transmit data length - 0 if data transmit is not used */
  uint32_t          tx_count;					/*!< Current transmit data counter	*/
  uint8_t*          rx_data;					/*!< Pointer to receive data - NULL if data received is not used */
  uint32_t          rx_length;					/*!< Receive data length - 0 if data receive is not used */
  uint32_t          rx_count;					/*!< Current receive data counter */
} I2C_S_SETUP_Type;

/**
 * @brief Transfer option type definitions
 */
typedef enum {
	I2C_TRANSFER_POLLING = 0,		/*!< Transfer in polling mode */
	I2C_TRANSFER_INTERRUPT			/*!< Transfer in interrupt mode */
} I2C_TRANSFER_OPT_Type;


/* Public Functions ----------------------------------------------------------- */

/* I2C Init/DeInit functions ---------- */
HAL_Status_Type HAL_I2C_Init(I2C_Type *I2Cx, uint32_t clockrate);
HAL_Status_Type HAL_I2C_DeInit(I2C_Type* I2Cx);

/* I2C transfer data functions -------- */
Status HAL_I2C_MasterTransferData(I2C_Type *I2Cx, I2C_M_SETUP_Type *TransferCfg, I2C_TRANSFER_OPT_Type Opt);
Status HAL_I2C_SlaveTransferData(I2C_Type *I2Cx, I2C_S_SETUP_Type *TransferCfg, I2C_TRANSFER_OPT_Type Opt);
uint32_t HAL_I2C_Master_GetState(I2C_Type *I2Cx);
uint32_t HAL_I2C_Slave_GetState(I2C_Type *I2Cx);

void HAL_I2C_Master_IRQHandler_IT(I2C_Type  *I2Cx);
void HAL_I2C_Slave_IRQHandler_IT (I2C_Type  *I2Cx);

Status	HAL_I2C_Master_Transmit(I2C_Type* I2Cx, I2C_M_SETUP_Type *TransferCfg, I2C_TRANSFER_OPT_Type Opt);
Status	HAL_I2C_Master_Receive(I2C_Type* I2Cx, I2C_M_SETUP_Type *TransferCfg, I2C_TRANSFER_OPT_Type Opt);
Status	HAL_I2C_Slave_Receive(I2C_Type* I2Cx, I2C_S_SETUP_Type *TransferCfg, I2C_TRANSFER_OPT_Type Opt);

HAL_Status_Type HAL_I2C_Slave_SetAddress0(I2C_Type *I2Cx, uint8_t SlaveAddr_7bit, uint8_t GeneralCallState);
HAL_Status_Type HAL_I2C_Slave_SetAddress1(I2C_Type *I2Cx, uint8_t SlaveAddr_7bit, uint8_t GeneralCallState);

/* I2C Interrupt handler functions ------*/
void HAL_I2C_ConfigInterrupt(I2C_Type *I2Cx, Bool NewState);

#ifdef __cplusplus
}
#endif

#endif /* _I2C_H_ */

/* --------------------------------- End Of File ------------------------------ */
