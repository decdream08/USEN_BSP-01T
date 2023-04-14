#ifndef _SPI_H_
#define _SPI_H_

#include "A31G21x.h"

void SPI_Init(SerialPort_t Port, uint32_t Baudrate);
void SPI_Open(SerialPort_t Port, Serial_Handle_t Handle);
void SPI_Close(SerialPort_t Port);
void SPI_PutChar(SerialPort_t Port, uint8_t Data);
void SPI_PutString(SerialPort_t Port, uint8_t* String);
void SPI_WriteByte(uint8_t Data);

void SPI_Send(SerialPort_t port, uint8_t *txbuf, uint8_t buflen);
void USART_SPI_SS_control(USART_Type *USART1n, char ch);
#endif


