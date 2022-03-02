/*----------------------------------------------------------------------------------------------\
|																								|
| VM5F_Engine.h																						|
|																								|
\----------------------------------------------------------------------------------------------*/

#ifndef _VM5F_ENGINE_H_
#define _VM5F_ENGINE_H_

#include "Typedefs.h"
#include "serialTest2.h"

/*----------------------------------------------------------------------------------------------\
|																								|
| Protocolo VM5F																				|
|																								|
+---------------------------------------+-------------------------------------------------------/
| Parametros                            |
\--------------------------------------*/
#define VM5F_DEFAULT_ADDRESS 0x01
#define VM5F_GENERAL_ADDRESS 0xFF

#define VM5F_DEFAULT_PROT_TX_SIZE 0x03 // size = byte(size) + byte(Address) + byte(command) + PayloadSize
#define VM5F_DEFAULT_TX_SIZE 0x04	   // size + byte(head) // byte(checksum) nao conta

#define VM5F_RXINIT 0xA0 // inicializador do pacote de recepcao
#define VM5F_TXINIT 0xA0 // inicializador do pacote de transmissao

/*--------------------------------------\
| Buffer de recepcao                    |
\--------------------------------------*/
#define VM5F_RXPROT_PAYLOAD_SIZE 45
#define VM5F_RXPROT_SIZE (5 + VM5F_RXPROT_PAYLOAD_SIZE)

typedef union
{
	uint8_t Char[VM5F_RXPROT_SIZE];
	struct
	{
		uint8_t CheckSum;
		uint8_t Payload[VM5F_RXPROT_PAYLOAD_SIZE];
		uint8_t ErrorCode;
		uint8_t Command;
		uint8_t Address;
		uint8_t Size;
	} DATA;
} VM5F_RXPROT;

/*--------------------------------------\
| Buffer de transmissao                 |
\--------------------------------------*/
#define VM5F_TXPROT_PAYLOAD_SIZE 45
#define VM5F_TXPROT_SIZE (5 + VM5F_TXPROT_PAYLOAD_SIZE)
typedef union
{
	uint8_t Char[VM5F_TXPROT_SIZE];
	struct
	{
		uint8_t Checksum;
		uint8_t Payload[VM5F_TXPROT_PAYLOAD_SIZE];
		uint8_t Command;
		uint8_t Address;
		uint8_t Size;
		uint8_t Head;
	} DATA;
} VM5F_TXPROT;

/*--------------------------------------\
| Buffer unificado		                |
\--------------------------------------*/
#define VM5F_BUFF_SIZE (VM5F_RXPROT_SIZE + VM5F_TXPROT_SIZE)
typedef union
{
	uint8_t Char[VM5F_BUFF_SIZE];
	struct
	{
		VM5F_RXPROT Rx;
		VM5F_TXPROT Tx;
	} DIR;
} VM5F_BUFFER;

/*--------------------------------------\
| Controle				                |
\--------------------------------------*/
#define VM5F_OFFLINEERROR_TMR 12 // x250ms
#define VM5F_RXTIMEROUT 4		 // x10ms
#define VM5F_TXATTEMPTS_TMR 100	 // x10ms

#define VM5F_MAX_ATTEMPTS 3

/*----------------------------------------------------------------------------------------------\
|																								|
| Macros																						|
|																								|
\----------------------------------------------------------------------------------------------*/
#define VM5F_UART_MODULE UART_MODULE_1

#define mVM5F_RxdByteOk() UART1_RxdByteOk()
#define mVM5F_RxdByteErr() UART1_RxdByteErr()
#define mVM5F_GetRxByte() UART1_GetRxByte()
#define mVM5F_TxByteOk() UART1_TxByteOk()
#define mVM5F_PutTxByte(val) UART1_PutTxByte(val)

/*----------------------------------------------------------------------------------------------\
|																								|
| Estruturas																					|
|																								|
\----------------------------------------------------------------------------------------------*/
typedef enum
{
	VM5F_BYPASS,
	VM5F_NORMAL,
} VM5F_OPERMODE;

typedef union
{
	uint8_t Value[2];
	WORD Value_int;
	struct
	{
		/* Communication control */
		uint8_t TxDataInit : 1;	   // 0.0
		uint8_t RxProcInit : 1;	   // 0.1
		uint8_t RxValidPacket : 1; // 0.2
		uint8_t RxWaitAnswer : 1;  // 0.3
		uint8_t Vago04 : 1;		   // 0.4
		uint8_t Vago05 : 1;		   // 0.5
		uint8_t Vago06 : 1;		   // 0.6
		uint8_t LastCmdOk : 1;	   // 0.7

		/* Device control */
		uint8_t CheckCmdData : 1; // 1.0
		uint8_t Write : 1;		  // 1.1
		uint8_t Lock : 1;		  // 1.2
		uint8_t WriteSucceed : 1; // 1.3
		uint8_t Vago14 : 1;		  // 1.4
		uint8_t Vago15 : 1;		  // 1.5
		uint8_t CommOffline : 1;  // 1.6
		uint8_t ReaderFail : 1;	  // 1.7
	} DATA; //acessa a estrutura
} VM5FDRIVER_FLAGS;	// E está acessand o typedef union para write sucess

/*----------------------------------------------------------------------------------------------\
|																								|
| Externos																						|
|																								|
\----------------------------------------------------------------------------------------------*/
extern VM5FDRIVER_FLAGS VM5F_Flags;
extern VM5F_BUFFER VM5F_Buffer;
extern uint8_t VM5F_CommOfflineCnt;
extern uint8_t VM5F_RxTimeout;
extern uint8_t VM5F_TxAttemptsTmr;

void VM5F_ProcComm(void);
uint8_t VM5F_TxData_Dispach(void);

void VM5F_TxWrPtr_Reset_ToPayload(void);
void VM5F_TxWrPtr_Byte(uint8_t Byte);
void VM5F_TxWrPtr_Word(WORD Word);
void VM5F_TxWrPtr_Buff(void *DataPtr, uint8_t Size);
void VM5F_TxWrPtr_BuffInv(void *DataPtr, uint8_t Size);

uint8_t VM5F_RxRdPtr_Byte(void);
uint16_t VM5F_RxRdPtr_Word(void);
void VM5F_RxRdPtr_Reset_ToPayload(void);
void VM5F_RxPayload_To_Buff(void *DestPtr, uint8_t Size);
void VM5F_RxPayload_To_BuffInv(void *DestPtr, uint8_t Size);
void VM5F_RxRdPtr_Shift(uint8_t size);
#endif
