#ifndef __SERIAL_H
#define __SERIAL_H
#include "stdio.h"
/*
USART1  TXD  RXD  | USART2  TXD  RXD   RTS  CK   |  USART3  TXD  RXD  CK   CTS   RTS
        A9   A10  |         A2   A3    A1   A4   |          B10  B11  B12  B13   B14 
*/
extern char Serial_RxPacket[];
//大道至简!
void Serial_Init(USART_TypeDef* USARTx,uint32_t BaudRate);
//串口发送使用printf();
//底层函数
//USART_SendData(USART1, Byte);//发送单个字节
//USART_ReceiveData(USART1);

#endif
