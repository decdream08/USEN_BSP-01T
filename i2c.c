/**********************************************************************
* @file		main.c
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
#ifdef I2C_0_ENABLE
#include "i2c.h"
#ifdef USEN_BT_SPK
#include "remocon_action.h"
#endif
#ifdef SOC_ERROR_ALARM
#include "timer20.h"
#include "led_display.h"
#endif

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/

/** Own Slave address in Slave I2C device */
#define I2CDEV_S_ADDR	(0xC0>>1)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
/* Private define ----------------------------------------------------*/
/* Private function prototypes ---------------------------------------*/
void I2C_Error_Handler(void);

/* Private variables ---------------------------------------------------*/


/** Max buffer length */
#define BUFFER_SIZE			10

Bool complete;
Bool isMasterMode = TRUE;
/*These global variables below used in interrupt mode - Slave device */
uint8_t Master_Buf[BUFFER_SIZE];
uint8_t dispSlave_Buf[BUFFER_SIZE];
uint8_t buffer = 0xFF;

/**********************************************************************
 * @brief		I2C0_IRQHandler_IT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void I2C0_IRQHandler_IT(void)
{
	if (isMasterMode)/* Master Mode*/
	{
		HAL_I2C_Master_IRQHandler_IT(I2C0);
		if (HAL_I2C_Master_GetState(I2C0)){
			complete = TRUE;
		}
	}
	else/* Slave Mode*/
	{
		HAL_I2C_Slave_IRQHandler_IT(I2C0);
		if (HAL_I2C_Slave_GetState(I2C0))
		{
			complete = TRUE;
		}
	}	
}

/**********************************************************************
 * @brief		I2C_Configure
 * @param[in]	None
 * @return 	None
 **********************************************************************/
void I2C_Configure(void)
{
	 /*Initialize Slave I2C peripheral*/
#ifdef AD82584F_ENABLE
	if(HAL_I2C_Init(I2C0, 100000) != HAL_OK) //100Kbps - Low speed mode
#else //TAS5806MD_ENABLE
	if(HAL_I2C_Init(I2C0, 400000) != HAL_OK) //400kbps - Fast mode
#endif //AD82584F_ENABLE
	{
		 /* Initialization Error */
     		I2C_Error_Handler();
	}
	/* Set  Own slave address for I2C device */
	HAL_I2C_Slave_SetAddress0(I2C0, I2CDEV_S_ADDR, DISABLE);
	HAL_I2C_Slave_SetAddress1(I2C0, I2CDEV_S_ADDR, DISABLE);
}

/**********************************************************************
  * @brief  Reports the name of the source file and the source line number
  *   where the check_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: check_param error line source number
  * @retval : None
 **********************************************************************/
void I2C_Error_Handler(void)
{
    while (1)
    {
    }
}

void I2C_Interrupt_Write_Data(uint8_t uDeviceId, uint8_t uAddr, uint8_t *uData, uint8_t uDataSize) //Master - I2C_TRANSFER_INTERRUPT
{
	I2C_M_SETUP_Type transferMCfg;
	uint8_t i;
	uint8_t Master_Buf[BUFFER_SIZE] = {0,}; //BUFFER_SIZE = 10;
#if defined(SOC_ERROR_ALARM) || defined(ESD_ERROR_RECOVERY)
	uint32_t uCount = 0;
#ifdef ESD_ERROR_RECOVERY
	Bool B_Error = FALSE;
#endif
#endif

#if defined(USEN_BT_SPK) && !defined(NOT_USE_POWER_DOWN_MUTE) //220217 //2022-10-04_1 : To enable I2C communication under power off mode when we use TAS5806MD due to "Deep Sleep + Mute"
	if(!Power_State())
	{
#ifdef _I2C_DEBUG_MSG
		_DBG("\n\rI2C_Interrupt_Write_Data !!! = ");
		_DBH(uAddr);
#endif
		return;
	}
#endif

#ifdef ESD_ERROR_RECOVERY
	do {
		if(B_Error)
			_DBG("\n\rI2C_Interrupt_Write_Data !!! = ");

		B_Error = FALSE;
#endif //ESD_ERROR_RECOVERY
		complete = FALSE;

		/* Make I2C Data */
		Master_Buf[0]=uAddr; //address

#ifdef _I2C_DEBUG_MSG
		_DBG("\n+++I2C uAddr : \n");
		_DBH(Master_Buf[0]);
		_DBG("\n+++I2C Write Data : \n");
#endif

#if defined(USEN_BAP) && defined(ADC_INPUT_ENABLE) && !defined(TI_AMP_DSP_VOLUME_CONTROL_ENABLE)
		if(uAddr == 0x4C && !Is_BAmp_Init()) //2022-11-22_1 //#define TAS5806MD_DAC_GAIN_CONTROL_REG					(0x4C)
		{
#ifdef ADC_INPUT_DEBUG_MSG
			_DBG("\n+++I2C Write Vol = \n");
			_DBH(uData[0]);
#endif
			if(uData[0] != 0xff)
			uData[0] = uData[0] + uAttenuator_Vol_Value();
#ifdef ADC_INPUT_DEBUG_MSG
			_DBG("\n===I2C Write Vol = \n");
			_DBH(uData[0]);
#endif
		}
#endif

		for(i=0;i<uDataSize;i++)
		{
			Master_Buf[i+1] = uData[i];

#ifdef _I2C_DEBUG_MSG
			_DBH(Master_Buf[i+1]);
			_DBG("\n\r ");
#endif
		}

		/* Set I2C condition */	
		transferMCfg.sl_addr7bit = uDeviceId;
		transferMCfg.tx_data = Master_Buf;
		transferMCfg.tx_length = uDataSize+1; //Need to add address byte

		/* Call I2C function */
		HAL_I2C_Master_Transmit(I2C0, &transferMCfg, I2C_TRANSFER_INTERRUPT);

		/* Wait until interrupt flag is set */
		while(!complete)
		{
#ifdef ESD_ERROR_RECOVERY
			uCount++;

			if(uCount == ESD_ERROR_RECOVERY_TIME) // 5000000 = 10sec (100000 = 200ms)
			{
				B_Error = TRUE;
				//I2C_Configure();
				//NVIC_ClearPendingIRQ(I2C0_IRQn);
				//NVIC_EnableIRQ(I2C0_IRQn);
				//HAL_I2C_DeInit(I2C0);
				I2C_Configure();
#ifdef ESD_ERROR_RECOVERY_DEBUG_MSG
				_DBG("\n\rSOC_ERROR - 4");
#endif

				break;
			}

#else //ESD_ERROR_RECOVERY
#ifdef SOC_ERROR_ALARM
			if(Get_Cur_Status_LED_Mode() == STATUS_SOC_ERROR_MODE)
			{
				return; 
			}

			uCount++;

			if(uCount == 5000000) // 5000000 = 10sec (100000 = 200ms)
			{
#ifdef TIMER21_LED_ENABLE
				Set_Status_LED_Mode(STATUS_SOC_ERROR_MODE);
#endif
#ifdef SOC_ERROR_ALARM_DEBUG_MSG
				_DBG("\n\rSOC_ERROR - 4");
#endif
				TIMER20_SoC_error_flag_Start();

				return;
			}		
#endif				
#endif //ESD_ERROR_RECOVERY
		};


#ifdef ESD_ERROR_RECOVERY
	}while(B_Error);
#endif
#ifdef ESD_ERROR_RECOVERY_DEBUG_MSG
	if(B_Error)
		_DBG("\n\rI2C_Interrupt_Write_Data - End");
#endif

	__NOP();

}

void I2C_Interrupt_Read_Data(uint8_t uDeviceId, uint8_t uAddr, uint8_t *uData, uint8_t uDataSize) //Master - I2C_TRANSFER_INTERRUPT
{
	I2C_M_SETUP_Type transferMCfg;
	uint8_t regAddr = 0, i;
	uint8_t Master_Buf[BUFFER_SIZE] = {0,}; //BUFFER_SIZE = 10;
#if defined(SOC_ERROR_ALARM) || defined(ESD_ERROR_RECOVERY)
	uint32_t uCount = 0;
#ifdef ESD_ERROR_RECOVERY
	Bool B_Error = FALSE;
#endif
#endif
#if defined(USEN_BT_SPK) && !defined(NOT_USE_POWER_DOWN_MUTE) //220217 2022-10-04_1 : To enable I2C communication under power off mode when we use TAS5806MD due to "Deep Sleep + Mute"
	if(!Power_State())
	{
#ifdef _I2C_DEBUG_MSG
		_DBG("\n\rI2C_Interrupt_Write_Data !!! = ");
		_DBH(uAddr);
#endif
		return;
	}
#endif

#ifdef ESD_ERROR_RECOVERY
	do{
		if(B_Error)
			_DBG("\n\rI2C_Interrupt_Read_Data !!! = ");

		B_Error = FALSE;
#endif

		complete = FALSE;

		regAddr = uAddr;
		transferMCfg.sl_addr7bit = uDeviceId;
		transferMCfg.tx_data = &regAddr;
		transferMCfg.tx_length = 1;
		transferMCfg.rx_data = Master_Buf;
		transferMCfg.rx_length = uDataSize;//5; //2023-03-10_1 : I2C read size should be depends on data size from calling function.
		transferMCfg.tx_count = 0;
		transferMCfg.rx_count = 0;
		
		HAL_I2C_MasterTransferData(I2C0, &transferMCfg, I2C_TRANSFER_INTERRUPT);

		while(!complete)
		{
#ifdef ESD_ERROR_RECOVERY
			uCount++;

			if(uCount == ESD_ERROR_RECOVERY_TIME) // 5000000 = 10sec (100000 = 200ms)
			{
				B_Error = TRUE;
				//I2C_Configure();
				//NVIC_ClearPendingIRQ(I2C0_IRQn);
				//NVIC_EnableIRQ(I2C0_IRQn);
				//HAL_I2C_DeInit(I2C0);
				I2C_Configure();
#ifdef ESD_ERROR_RECOVERY_DEBUG_MSG
				_DBG("\n\rSOC_ERROR - 4");
#endif
				
				break;
			}

#else //ESD_ERROR_RECOVERY
#ifdef SOC_ERROR_ALARM
			if(Get_Cur_Status_LED_Mode() == STATUS_SOC_ERROR_MODE)
			{
				return; 
			}
					
			uCount++;

			if(uCount == 5000000) // 5000000 = 10sec (100000 = 200ms)
			{
#ifdef TIMER21_LED_ENABLE
				Set_Status_LED_Mode(STATUS_SOC_ERROR_MODE);
#endif
#ifdef SOC_ERROR_ALARM_DEBUG_MSG
				_DBG("\n\rSOC_ERROR - 5");
#endif
				TIMER20_SoC_error_flag_Start();

				return;
			}
		
#endif
#endif //ESD_ERROR_RECOVERY
		};
#ifdef ESD_ERROR_RECOVERY
	}while(B_Error);
#endif
#ifdef ESD_ERROR_RECOVERY_DEBUG_MSG
	if(B_Error)
		_DBG("\n\rI2C_Interrupt_Read_Data - End");
#endif

	__NOP();
	
	for(i=0;i<uDataSize;i++)
	{
		uData[i] = Master_Buf[i];
#ifdef _I2C_DEBUG_MSG
		_DBG("\n+++I2C Read Data : \n");
		_DBH(uData[i]);
#endif		
	}
#if defined(USEN_BAP) && defined(ADC_INPUT_ENABLE) && !defined(TI_AMP_DSP_VOLUME_CONTROL_ENABLE)
	if(uAddr == 0x4C && !Is_BAmp_Init()) //2022-11-22_1 //#define TAS5806MD_DAC_GAIN_CONTROL_REG					(0x4C)
	{
#ifdef ADC_INPUT_DEBUG_MSG
		_DBG("\n+++I2C Read Vol = \n");
		_DBH(uData[0]);
#endif
		if(uData[0] != 0xff)
			uData[0] = uData[0] + uAttenuator_Vol_Value();
#ifdef ADC_INPUT_DEBUG_MSG
		_DBG("\n===I2C Read Vol = \n");
		_DBH(uData[0]);
#endif
	}
#endif
}

#endif //I2C_0_ENABLE


