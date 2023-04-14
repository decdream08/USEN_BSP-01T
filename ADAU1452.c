
#include "main_conf.h"

#ifdef SPI_11_ENABLE
#ifdef ADAU1452_ENABLE
#include "ADAU1452.h"
#include "ADAU1452_IC_1.h" //Actual action define
#include "A31G21x_hal_usart1n.h"
#include "A31G21x_it.h"
#include "spi.h"

#define	SPI_ADAU1452_CHIP_ADDR		(0x00)
#define	SPI_ADAU1452_WR				(0x00)
#define	SPI_ADAU1452_RD				(0x01)

#define SD_DUMMY_BYTE 				(0x00)

typedef enum {
	SPI_SS_ENABLE = 0,
	SPI_SS_DISABLE = 1
}SPI_SS;

void	ADAU1451_SPI_SS_Toggle(void);
void	ADAU1451_SPI_WriteNoSubAddr(uint8_t dev_addr, uint8_t * buffer, uint16_t len);
void ADAU1452_DSP_Init(void)
{
#if 0//def SOUND_BAR_GPIO_ENABLE /* DSP reset */ //When we use DSP reset, DSP doesn't work... Why??? //To Do !!!
	//GPIO Low	
	DSP_Reset(TRUE);
	//Delay 2ms
	delay_ms(2);
	//GPIO High
	DSP_Reset(FALSE);
#endif

	ADAU1451_SPI_SS_Toggle();
	//Delay 1ms
	delay_ms(1);
	ADAU1451_SPI_SS_Toggle();
	//Delay 1ms
	delay_ms(1);
	ADAU1451_SPI_SS_Toggle();
	//Delay 1ms
	delay_ms(1);

	//then SPI slave download
	default_download_IC_1();
}

uint8_t ADAU1451_SPI_ReadByte(void)
{
	uint8_t Data = 0;

	// Wait until the transmit buffer is empty

	// Send the byte
	SPI_WriteByte(SD_DUMMY_BYTE);

	// Wait until a data is received
	// Get the received data


	// Return the shifted data
	return Data;
}

void	ADAU1451_SPI_WriteSubAddr(uint8_t dev_addr, uint16_t sub_addr, uint8_t * buffer, uint32_t len)
{
	uint32_t i;

	USART_SPI_SS_control(USART11, SPI_SS_ENABLE); //Clear(LOW) SS line

	SPI_WriteByte(dev_addr |SPI_ADAU1452_WR); 	//send device address and WR/RD first

	SPI_WriteByte(sub_addr >> 8);	//sub address high byte first

	SPI_WriteByte(sub_addr & 0xFF); //sub address low byte

	for(i=0; i < len; i++)
	{
		SPI_WriteByte(buffer[i]);		//data byte
	}

	USART_SPI_SS_control(USART11, SPI_SS_DISABLE); //Set(High) SS line

}


void	ADAU1451_SPI_ReadSubAddr(uint8_t dev_addr, uint16_t sub_addr, uint8_t * buffer, uint32_t len)
{
	uint32_t i;

  	USART_SPI_SS_control(USART11, SPI_SS_ENABLE); //Clear(LOW) SS line
	
	SPI_WriteByte(dev_addr |SPI_ADAU1452_RD); 	/* send device address and WR/RD first */

	SPI_WriteByte(sub_addr >> 8);	/* sub address high byte first */

	SPI_WriteByte(sub_addr & 0xFF);	/* sub address low byte */
	
	for(i=0; i < len; i++)
	{
		buffer[i] = ADAU1451_SPI_ReadByte();	/* read data byte */
	}

 	USART_SPI_SS_control(USART11, SPI_SS_DISABLE); //Set(High) SS line
}

void	ADAU1451_SPI_SS_Toggle(void)
{
	uint16_t i;


  	USART_SPI_SS_control(USART11, SPI_SS_ENABLE); //Clear(LOW) SS line
	
	SPI_WriteByte(SPI_ADAU1452_CHIP_ADDR |SPI_ADAU1452_WR); 	// send device address and WR/RD first

	SPI_WriteByte(0x4F); // send sub address (High Byte)
	SPI_WriteByte(0xFF); // send sub address (Low Byte)

	for(i=0; i < 4; i++) // 4Byte 0x00 data
	{
		SPI_WriteByte(0x00);
	}

	USART_SPI_SS_control(USART11, SPI_SS_DISABLE); //Set(High) SS line
}

void	ADAU1451_SPI_WriteNoSubAddr(uint8_t dev_addr, uint8_t * buffer, uint16_t len)
{
	uint16_t i;


  	USART_SPI_SS_control(USART11, SPI_SS_ENABLE); //Clear(LOW) SS line
	
	SPI_WriteByte(dev_addr |SPI_ADAU1452_WR); 	/* send device address and WR/RD first */

	for(i=0; i < len; i++)
	{
		SPI_WriteByte(buffer[i]);		/* data byte */
	}

	USART_SPI_SS_control(USART11, SPI_SS_DISABLE); //Set(High) SS line
}

void SIGMA_WRITE_REGISTER_BLOCK(uint8_t DEVICE_ADDR, uint16_t SUB_ADDR, uint32_t len, ADI_REG_TYPE* data )
{
	   ADAU1451_SPI_WriteSubAddr(DEVICE_ADDR, SUB_ADDR, (uint8_t*)data, len);
}

void SIGMA_WRITE_DELAY(uint8_t DEVICE_ADDR,  uint16_t len, ADI_REG_TYPE* data )
{
	   ADAU1451_SPI_WriteSubAddr(DEVICE_ADDR, 0x00, (uint8_t*)data, len);
}

#endif //ADAU1452_ENABLE
#endif //SPI_11_ENABLE


