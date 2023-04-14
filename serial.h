#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "A31G21x.h"

void Serial_Init(SerialPort_t Port, uint32_t Baudrate);
void Serial_Open(SerialPort_t Port, Serial_Handle_t Handle);
void Serial_Close(SerialPort_t Port);
void Serial_PutChar(SerialPort_t Port, uint8_t Data);
void Serial_PutString(SerialPort_t Port, uint8_t* String);

void Serial_Send(SerialPort_t port, uint8_t *txbuf, uint8_t buflen);

#endif


