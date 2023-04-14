/**********************************************************************//**
* @file		a31g21x_dmac.h
* @brief	        Contains all macro definitions and function prototypes
* 			support for DMAC firmware library on a31g21x
* @version	1.00
* @date		
* @author	ABOV Application 3 team
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


/* Peripheral group ----------------------------------------------------------- */
#ifndef _A31G21x_DMAC_H_
#define _A31G21x_DMAC_H_

/* Includes ------------------------------------------------------------------- */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

/* ================================================================================ */
/* ================          struct 'DMAC' Position & Mask          ================ */
/* ================================================================================ */

/**  DMAC_CR  ----------------------------------- */
#define DMAC_CR_DIR_Pos                        1                                                       /*!< DMAC CR: DIR Position                    */
#define DMAC_CR_DIR_Msk                        (0x01UL << DMAC_CR_DIR_Pos)                              /*!< DMAC CR: DIR Mask                        */
#define DMAC_CR_SIZE_Pos                       2                                                       /*!< DMAC CR: SIZE Position                   */
#define DMAC_CR_SIZE_Msk                       (0x03UL << DMAC_CR_SIZE_Pos)                             /*!< DMAC CR: SIZE Mask                       */
#define DMAC_CR_PERISEL_Pos                    8                                                       /*!< DMAC CR: PERISEL Position                */
#define DMAC_CR_PERISEL_Msk                    (0x0fUL << DMAC_CR_PERISEL_Pos)                          /*!< DMAC CR: PERISEL Mask                    */
#define DMAC_CR_TRANSCNT_Pos                   16                                                      /*!< DMAC CR: TRANSCNT Position               */
#define DMAC_CR_TRANSCNT_Msk                   (0x00000fffUL << DMAC_CR_TRANSCNT_Pos)                   /*!< DMAC CR: TRANSCNT Mask                   */

/* -----------------------------------  DMAC_SR  ----------------------------------- */
#define DMAC_SR_DMAEN_Pos                      0                                                       /*!< DMAC SR: DMAEN Position                  */
#define DMAC_SR_DMAEN_Msk                      (0x01UL << DMAC_SR_DMAEN_Pos)                            /*!< DMAC SR: DMAEN Mask                      */
#define DMAC_SR_EOT_Pos                        7                                                       /*!< DMAC SR: EOT Position                    */
#define DMAC_SR_EOT_Msk                        (0x01UL << DMAC_SR_EOT_Pos)                              /*!< DMAC SR: EOT Mask                        */

/* -----------------------------------  DMAC_PAR  ---------------------------------- */
#define DMAC_PAR_PAR_Pos                       0                                                       /*!< DMAC PAR: PAR Position                   */
#define DMAC_PAR_PAR_Msk                       (0xffffffffUL << DMAC_PAR_PAR_Pos)                       /*!< DMAC PAR: PAR Mask                       */

/* -----------------------------------  DMAC_MAR  ---------------------------------- */
#define DMAC_MAR_MAR_Pos                       0                                                       /*!< DMAC MAR: MAR Position                   */
#define DMAC_MAR_MAR_Msk                       (0x0000ffffUL << DMAC_MAR_MAR_Pos)                       /*!< DMAC MAR: MAR Mask                       */


/* Public Macros -------------------------------------------------------------- */
/** PERISEL Selection */
#define DMAC_PERISEL_IDLE			((0UL)) /**< CHANNEL IDLE */
#define DMAC_PERISEL_U0RX			((1UL)) /**< UART0 RX */
#define DMAC_PERISEL_U0TX			((2UL)) /**< UART0 TX */
#define DMAC_PERISEL_U1RX			((3UL)) /**< UART1 RX */
#define DMAC_PERISEL_U1TX			((4UL)) /**< UART1 TX */
#define DMAC_PERISEL_CRC            ((5UL)) /**< CRC */

/** Width in Source transfer width and Destination transfer width definitions */
#define DMAC_WIDTH_BYTE 		((0UL)) /**< Width = 1 byte */
#define DMAC_WIDTH_HALFWORD 	((1UL)) /**< Width = 2 bytes */
#define DMAC_WIDTH_WORD 		((2UL)) /**< Width = 4 bytes */

/** Width in Source transfer width and Destination transfer width definitions */
#define DMAC_DIR_TX 		((0UL)) /**< TX */
#define DMAC_DIR_RX 		((1UL)) /**< RX */

/* Public Types --------------------------------------------------------------- */
/**
 * @brief DMAC Channel configuration structure type definition
 */
typedef struct {
	uint32_t TransferCnt;	/** Length/Size of transfer */
	uint32_t PeriSel;	/** Peripheral selction */	
	uint32_t TransferWidth;	/** Transfer width */
	uint32_t Dir;	/** Select transfer direction */	
	uint32_t PeriAddr;	/** Peri address */
	uint32_t MemAddr;	/** Mem address */	
} DMA_Ch_CFG_Type;

/* Public Functions ----------------------------------------------------------- */
HAL_Status_Type HAL_DMAC_Init(void);

HAL_Status_Type HAL_DMAC_Setup(DMA_Type *DMACx, DMA_Ch_CFG_Type *DMACChConfig);
FlagStatus HAL_DMAC_GetStatus(DMA_Type *DMACx);
HAL_Status_Type HAL_DMAC_ChCmd(DMA_Type *DMACx, FunctionalState NewState);

#ifdef __cplusplus
}
#endif

#endif /* _AC33MX064_DMAC_H_ */

/* --------------------------------- End Of File ------------------------------ */
