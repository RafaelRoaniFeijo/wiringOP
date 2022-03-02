#ifndef _SERIAL_TEST_2_H
#define _SERIAL_TEST_2_H

#include <stdint.h>

//criado a biblioteca para definição de funções/proc utilizados no serialTest2.c


void PutTxByte(uint8_t byte);
uint8_t GetRxByte(void);
uint8_t RxByteOk(void);

//macros utilizadas 
#define UART1_RxdByteOk()       RxByteOk()
#define UART1_RxdByteErr()      0
#define UART1_GetRxByte()       GetRxByte()
#define UART1_TxByteOk()        1
#define UART1_PutTxByte(val)    PutTxByte(val)

#endif