/**********************************************************************//**
* @file		A31G21x_hal_spi.h
* @brief	Contains all macro definitions and function prototypes
* 			support for SPI firmware library on A31G21x
* @version	1.00
* @date		
* @author	ABOV AE1 team
*
* Copyright(C) 2018, ABOV Semiconductor
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


#ifndef _SPI_H_
#define _SPI_H_

/* Includes ------------------------------------------------------------- */
#include "A31G21x.h"
#include "A31G21x_hal_aa_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

/* Public Macros --------------------------------------------------------- */
/* --------------------- BIT DEFINITIONS -------------------------------- */
/**********************************************************************//**
 * Macro defines for CR register
 **********************************************************************/
/** SPI data size select */
#define SPI_DS_8BITS				((uint32_t)(0))
#define SPI_DS_9BITS				((uint32_t)(1))
#define SPI_DS_16BITS			((uint32_t)(2))
#define SPI_DS_17BITS			((uint32_t)(3))
#define SPI_DS_BITMASK		((uint32_t)(0x3))

/** Clock phase control bit */
#define SPI_CPHA_LO				((uint32_t)(0x00))//((uint32_t)(0<<3))
#define SPI_CPHA_HI				((uint32_t)(0x01))//((uint32_t)(1<<3))
	
#define SPI_CPOL_LO				((uint32_t)(0x00))//((uint32_t)(0<<2))
#define SPI_CPOL_HI				((uint32_t)(0x01))//((uint32_t)(1<<2))


/** SPI MSB/LSB Transmit select bit */
#define SPI_LSB_FIRST			((uint32_t)(0))
#define SPI_MSB_FIRST			((uint32_t)(1))
	
/** SPI master mode enable */
#define SPI_SLAVE_MODE				((uint32_t)(0))
#define SPI_MASTER_MODE			((uint32_t)(1))

/** SPI SS output signal control */
#define SPI_SSOUT_LO					((uint32_t)(0<<12))
#define SPI_SSOUT_HI					((uint32_t)(1<<12))

/** SPI SS mode */
#define SPI_SSMOD_AUTO				((uint32_t)(0<<13))
#define SPI_SSMOD_MANUAL			((uint32_t)(1<<13))

/** SPI loop back enable */
#define SPI_LBM_EN						((uint32_t)(1<<11)) 

/** SPI interrupt enable */
#define SPI_INTCFG_TXDIE		((uint32_t)(1<<18)) /** DMA TX done interrupt enable */
#define SPI_INTCFG_RXDIE		((uint32_t)(1<<17)) /** DMA RX done interrupt enable */
#define SPI_INTCFG_SSCIE		((uint32_t)(1<<16)) /** SS edge change interrupt enable */
#define SPI_INTCFG_TXIE		((uint32_t)(1<<15)) /** TX interrupt enable */
#define SPI_INTCFG_RXIE		((uint32_t)(1<<14)) /** RX interrupt enable */

/*********************************************************************//**
 * SPI Status defines
 **********************************************************************/

#define SPI_STAT_RXBUF_READY			((uint32_t)(1<<0))	/** SPI status RX buffer ready bit */
#define SPI_STAT_TXBUF_EMPTY			((uint32_t)(1<<1))	/** SPI status TX buffer empty bit */

#define SPI_STAT_IDLE							((uint32_t)(1<<2))	/** SPI status TX/RX IDLE bit */
#define SPI_STAT_TXUNDERRUN_ERR	((uint32_t)(1<<3))	/** SPI status TX underrun error bit */
#define SPI_STAT_RXOVERRUN_ERR		((uint32_t)(1<<4))	/** SPI status RX overrun error bit */

#define SPI_STAT_SS_ACT						((uint32_t)(1<<5))	/** SPI status SS active bit */
#define SPI_STAT_SS_DET						((uint32_t)(1<<6))	/** SPI status SS detect bit */

#define SPI_STAT_SBUSY				((uint32_t)(1<<7))	/** SPI status Sbusy Operation Flag */
#define SPI_STAT_RXDMA_DONE			((uint32_t)(1<<8))	/** SPI status RX DMA done bit */
#define SPI_STAT_TXDMA_DONE				((uint32_t)(1<<9))	/** SPI status TX DMA done bit */


enum {
	SS_AUTO		= 0,
	SS_MANUAL
} ;

enum {
	SS_OUT_LO		= 0,
	SS_OUT_HI
} ;

/* Public Types --------------------------------------------------------------- */
/** 
*@brief SPI configuration structure 
*/
typedef struct {
	uint32_t Databit; 		/** Databi
	t number, should be 
										- SPI_DS_8BITS :0
										- SPI_DS_9BITS :1
										- SPI_DS_16BITS :2
										- SPI_DS_17BITS :3 */
	uint32_t CPHA;			/** Clock phase, should be:
										- SSP_CPHA_FIRST: first clock edge
										- SSP_CPHA_SECOND: second clock edge */
	uint32_t CPOL;			/** Clock polarity, should be:
										- SSP_CPOL_HI: high level
										- SSP_CPOL_LO: low level */
	uint8_t DataDir;			/** SPI mode, should be:
										- SPI_LSB_FIRST
										- SPI_MSB_FIRST */
	uint32_t Mode;			/** SPI mode, should be:
										- SPI_MASTER_MODE: Master mode
										- SPI_SLAVE_MODE: Slave mode */
	uint32_t BaudRate;		/** PCLK / (BaudRate+1)  BaudRate>=2 (0x0002~0xFFFF */
} SPI_CFG_Type;

/**
 * @brief SPI Delay configuration structure
 */
typedef struct {
	uint8_t StartDelay;		/** insert start delay length in master mode >=1 (1~255) */		
	uint8_t BurstDelay;		/** insert burst delay length in master mode >=1 (1~255) */
	uint8_t StopDelay;		/** insert stop delay length in master mode >=1 (1~255) */	
} SPI_DELAY_CFG_Type;


/* Public Functions ----------------------------------------------------------- */
HAL_Status_Type HAL_SPI_Init(SPI_Type *SPIx, SPI_CFG_Type *SPI_ConfigStruct);
HAL_Status_Type HAL_SPI_DeInit(SPI_Type* SPIx);
HAL_Status_Type HAL_SPI_ConfigDelay(SPI_Type* SPIx, uint8_t StartDelay, uint8_t BurstDelay,uint8_t StopDelay);

HAL_Status_Type HAL_SPI_Enable(SPI_Type* SPIx, FunctionalState NewState);
HAL_Status_Type HAL_SPI_ConfigLoopBackMode(SPI_Type* SPIx, FunctionalState NewState);
HAL_Status_Type HAL_SPI_ConfigSSOutputMode(SPI_Type* SPIx, uint32_t State);
HAL_Status_Type HAL_SPI_SetSSOutput(SPI_Type* SPIx, uint32_t State);

HAL_Status_Type HAL_SPI_TransmitData(SPI_Type* SPIx, uint32_t Data);
uint32_t HAL_SPI_ReceiveData(SPI_Type* SPIx);

uint32_t HAL_SPI_GetStatus(SPI_Type* SPIx);
HAL_Status_Type HAL_SPI_ClearPending(SPI_Type* SPIx, uint32_t IntType);

HAL_Status_Type HAL_SPI_ConfigInterrupt(SPI_Type *SPIx, uint32_t IntType, FunctionalState NewState);
uint32_t HAL_SPI_GetControl(SPI_Type* SPIx);

#ifdef __cplusplus
}
#endif

#endif /* _SPI_H_ */

/* --------------------------------- End Of File ------------------------------ */
