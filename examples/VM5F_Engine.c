/*----------------------------------------------------------------------------------------------\
|																								|
| VM5F_Engine_V1																				|
|																								|
\----------------------------------------------------------------------------------------------*/
#include <stdio.h>

#include "VM5F_Cmds.h"
#include "VM5F_Engine.h"

/*----------------------------------------------------------------------------------------------\
|																								|
| Registradores																					|
|																								|
\----------------------------------------------------------------------------------------------*/
uint8_t VM5F_Rx_Byte;
VM5FDRIVER_FLAGS VM5F_Flags; //esse registrador está
uint8_t VM5F_CommOfflineCnt;

// recepcao
uint8_t VM5F_RxState;
uint8_t VM5F_RxTimeout;
uint8_t *VM5F_RxBuffPtr;
WORD VM5F_RxAddress;

// transmissao
uint8_t VM5F_TxState;
uint8_t *VM5F_TxBuffPtr;
uint8_t VM5F_TxAttemptsCnt;
uint8_t VM5F_TxAttemptsTmr;

// buffer unificado
VM5F_BUFFER VM5F_Buffer; //pode acessar o rx ou tx
uint8_t *VM5F_TxWrBuffPtr;
uint8_t VM5F_TxWrPayloadSize;
uint8_t *VM5F_RxRdBuffPtr;

/*----------------------------------------------------------------------------------------------\
|																								|
| Funcoes e procedures																			|
|																								|
\----------------------------------------------------------------------------------------------*/
void VM5F_ProcComm(void);
void VM5F_TxProc(void);
void VM5F_RxProc(void);
void VM5F_RxReset(void);
void VM5F_Bypass_ResetPointers(void);
uint8_t VM5F_TxData_Dispach(void);
void VM5F_TxData_ReDispach(void);

void VM5F_TxWrPtr_Reset_ToPayload(void);
void VM5F_TxWrPtr_Byte(uint8_t Byte);
void VM5F_TxWrPtr_Word(WORD Word);
void VM5F_TxWrPtr_Buff(void *DataPtr, uint8_t Size);
void VM5F_TxWrPtr_BuffInv(void *DataPtr, uint8_t Size);

void VM5F_RxRdPtr_Reset_ToPayload(void);
uint8_t VM5F_RxRdPtr_Byte(void);
uint16_t VM5F_RxRdPtr_Word(void);
void VM5F_RxPayload_To_Buff(void *DestPtr, uint8_t Size);
void VM5F_RxPayload_To_BuffInv(void *DestPtr, uint8_t Size);
void VM5F_RxRdPtr_Shift(uint8_t size);

/*----------------------------------------------------------------------------------------------\
|																								|
| Loop Principal																				|
|																								|
\----------------------------------------------------------------------------------------------*/
void VM5F_ProcComm(void)
{
	VM5F_TxProc(); //manda sinal
	VM5F_RxProc(); //recebe sinal

	
	VM5F_RxData_Proc(); //Processa recebimento de dados
}

//----------------------------------------------------------------------------------------------
// Processa recepcao de dados
//----------------------------------------------------------------------------------------------
void VM5F_RxProc(void)
{

	if (VM5F_RxState && VM5F_RxTimeout >= VM5F_RXTIMEROUT)
	{
		VM5F_RxReset();
	}

	if (mVM5F_RxdByteOk())
	{

		// verifica se pode processar dado
		if (mVM5F_RxdByteErr())
		{
			VM5F_RxReset();
		}
		else
		{
			VM5F_Rx_Byte = mVM5F_GetRxByte();
			VM5F_Flags.DATA.RxProcInit = 1;
			VM5F_RxTimeout = 0;

			switch (VM5F_RxState++)
			{
			// start
			case 0:
			{
				if (VM5F_Rx_Byte == VM5F_RXINIT)
				{
					VM5F_RxBuffPtr = &VM5F_Buffer.DIR.Rx.DATA.Size;
					VM5F_Buffer.DIR.Rx.DATA.CheckSum = VM5F_Rx_Byte;
				}
				else
				{
					VM5F_RxReset();
				}
				break;
			}
			// recebe buffer
			default:
			{

				(*VM5F_RxBuffPtr--) = VM5F_Rx_Byte;
				if (VM5F_RxState > (VM5F_Buffer.DIR.Rx.DATA.Size + 1))
				{
					VM5F_Buffer.DIR.Rx.DATA.CheckSum = (~VM5F_Buffer.DIR.Rx.DATA.CheckSum) + 1;
					if (VM5F_Buffer.DIR.Rx.DATA.CheckSum == VM5F_Rx_Byte)
					{
						VM5F_Flags.DATA.RxValidPacket = 1;
						VM5F_CommOfflineCnt = 0;
						VM5F_Flags.DATA.CommOffline = 0;
					}
					VM5F_RxReset();
				}
				else
				{
					VM5F_Buffer.DIR.Rx.DATA.CheckSum += VM5F_Rx_Byte;
				}

				if (VM5F_RxState == 1)
				{
					if (VM5F_Rx_Byte > VM5F_RXPROT_PAYLOAD_SIZE)
					{
						VM5F_RxReset();
					}
				}
				break;
			}
			}
		}
	}

	if (VM5F_CommOfflineCnt >= VM5F_OFFLINEERROR_TMR)
	{
		VM5F_Flags.DATA.CommOffline = 1;
	}
}

//----------------------------------------------------------------------------------------------
// Reseta processamento
//----------------------------------------------------------------------------------------------
void VM5F_RxReset(void)
{
	while (mVM5F_RxdByteOk())
	{
		if (mVM5F_GetRxByte())
			;
	}
	VM5F_RxState = 0;
	VM5F_Flags.DATA.RxProcInit = 0;
}

//----------------------------------------------------------------------------------------------
// Processa envio de dados
//----------------------------------------------------------------------------------------------
void VM5F_TxProc(void)
{
	if (mVM5F_TxByteOk() && VM5F_Flags.DATA.RxProcInit == 0 && VM5F_Flags.DATA.TxDataInit == 1)
	{		//^
	//expande para o UART1_TxByteOk() regis 
	//caso não esteja recebendo, rx = 0
		uint8_t TxByte; //define como tipo 8 bits
		

		switch (VM5F_TxState++)
		{
		// start byte
		case 0:
			mVM5F_PutTxByte(VM5F_TXINIT);
			VM5F_TxBuffPtr = &VM5F_Buffer.DIR.Tx.DATA.Size;
			break;

		// buffer
		default:
			TxByte = (*VM5F_TxBuffPtr--);
			mVM5F_PutTxByte(TxByte);

			if (VM5F_TxState > (VM5F_Buffer.DIR.Tx.DATA.Size + 1))
			{
				VM5F_TxState = 0;
				VM5F_Flags.DATA.TxDataInit = 0;
				VM5F_Flags.DATA.ReaderFail = 0;
				VM5F_Flags.DATA.LastCmdOk = 0;
				VM5F_Flags.DATA.CheckCmdData = 0;
				VM5F_TxAttemptsTmr = 0;
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------
// Envia dados
//----------------------------------------------------------------------------------------------
uint8_t VM5F_TxData_Dispach(void)
{
	static enum {
		PREPARE,
		SENDING,
	} state = PREPARE;

	if (VM5F_Flags.DATA.TxDataInit == 0)
	{
		if (state == PREPARE)
		{
			uint8_t n, _checksum = 0;
			uint8_t *buf_pnt;

			VM5F_Buffer.DIR.Tx.DATA.Head = VM5F_TXINIT;
			VM5F_Buffer.DIR.Tx.DATA.Address = VM5F_GENERAL_ADDRESS;
			VM5F_Buffer.DIR.Tx.DATA.Size = VM5F_DEFAULT_PROT_TX_SIZE + VM5F_TxWrPayloadSize;

			buf_pnt = &VM5F_Buffer.DIR.Tx.DATA.Head;
			for (n = 0; n < (VM5F_TxWrPayloadSize + VM5F_DEFAULT_TX_SIZE); n++)
			{
				_checksum += *buf_pnt;
				buf_pnt--;
			}
			*buf_pnt = (~_checksum) + 1;

			VM5F_Flags.Value[0] = 0;
			VM5F_Flags.DATA.TxDataInit = 1;
			VM5F_Flags.DATA.RxWaitAnswer = VM5F_Buffer.DIR.Tx.DATA.Command != VM5F_CMD_RESET;

			VM5F_TxAttemptsCnt = 0;
			VM5F_TxAttemptsTmr = 0;

			state = SENDING;
		}
		else // SENDING
		{
			if (VM5F_Flags.DATA.RxWaitAnswer == 0)
			{
				VM5F_TxWrPtr_Reset_ToPayload();
				state = PREPARE;
				return 0xFF;
			}
			else if (VM5F_TxAttemptsTmr > VM5F_TXATTEMPTS_TMR)
			{
				if (VM5F_TxAttemptsCnt < VM5F_MAX_ATTEMPTS)
				{
					VM5F_TxData_ReDispach();
					VM5F_TxAttemptsCnt++;
				}
				else
				{
					VM5F_TxWrPtr_Reset_ToPayload();
					printf("\nErro 2\n");
					VM5F_Flags.DATA.ReaderFail = 1;
					state = PREPARE;
					return 0xFF;
				}
			}
		}
	}
	return 0;
}

//----------------------------------------------------------------------------------------------
// Re-envia dados
//----------------------------------------------------------------------------------------------
void VM5F_TxData_ReDispach(void)
{
	VM5F_Flags.DATA.TxDataInit = 1;
}

//----------------------------------------------------------------------------------------------
// Reseta ponteiro de escrita do payload de TX
//----------------------------------------------------------------------------------------------
void VM5F_TxWrPtr_Reset_ToPayload(void)
{
	VM5F_TxWrBuffPtr = &VM5F_Buffer.DIR.Tx.DATA.Payload[VM5F_TXPROT_PAYLOAD_SIZE - 1];
	VM5F_TxWrPayloadSize = 0;
}

//----------------------------------------------------------------------------------------------
// Escreve byte no buffer de transmissao
//----------------------------------------------------------------------------------------------
void VM5F_TxWrPtr_Byte(uint8_t Byte)
{
	/*	Caso nao esteja transmitindo ou esperando resposta */
	if (VM5F_Flags.DATA.TxDataInit == 0 && VM5F_Flags.DATA.RxWaitAnswer == 0)
	{
		*VM5F_TxWrBuffPtr-- = Byte;

		VM5F_TxWrPayloadSize++;
	}
}

//----------------------------------------------------------------------------------------------
// Escreve word no buffer de transmissao
//----------------------------------------------------------------------------------------------
void VM5F_TxWrPtr_Word(WORD Word)
{
	VM5F_TxWrPtr_Byte(Word.BYTES.Msb);
	VM5F_TxWrPtr_Byte(Word.BYTES.Lsb);
}

//----------------------------------------------------------------------------------------------
// Escreve buffer no buffer de transmissao
//----------------------------------------------------------------------------------------------
void VM5F_TxWrPtr_Buff(void *DataPtr, uint8_t Size)
{
	uint8_t TempCnt;

	for (TempCnt = 0; TempCnt < Size; TempCnt++)
	{
		VM5F_TxWrPtr_Byte(*(uint8_t *)DataPtr++);
	}
}

//----------------------------------------------------------------------------------------------
// Escreve Numero no buffer de transmissao (buffer em ordem reversa)
//----------------------------------------------------------------------------------------------
void VM5F_TxWrPtr_BuffInv(void *DataPtr, uint8_t Size)
{
	uint8_t TempCnt;

	DataPtr += Size;
	for (TempCnt = 0; TempCnt < Size; TempCnt++)
	{
		DataPtr--;
		VM5F_TxWrPtr_Byte(*(uint8_t *)DataPtr);
	}
}

//----------------------------------------------------------------------------------------------
// Reseta ponteiro de leitura do payload de RX
//----------------------------------------------------------------------------------------------
void VM5F_RxRdPtr_Reset_ToPayload(void)
{
	VM5F_RxRdBuffPtr = &VM5F_Buffer.DIR.Rx.DATA.Payload[VM5F_RXPROT_PAYLOAD_SIZE - 1];
}

//----------------------------------------------------------------------------------------------
// Escreve byte no buffer de transmissao
//----------------------------------------------------------------------------------------------
uint8_t VM5F_RxRdPtr_Byte(void)
{
	return *VM5F_RxRdBuffPtr--;
}

//----------------------------------------------------------------------------------------------
// Escreve word no buffer de transmissao
//----------------------------------------------------------------------------------------------
uint16_t VM5F_RxRdPtr_Word(void)
{
	WORD TempVal;

	TempVal.BYTES.Lsb = VM5F_RxRdPtr_Byte();
	TempVal.BYTES.Msb = VM5F_RxRdPtr_Byte();

	return TempVal.Int;
}

//----------------------------------------------------------------------------------------------
// Le payload do buffer de recepcao
//----------------------------------------------------------------------------------------------
void VM5F_RxPayload_To_Buff(void *DestPtr, uint8_t Size)
{
	uint8_t TempCnt;

	for (TempCnt = 0; TempCnt < Size; TempCnt++)
	{
		*(uint8_t *)DestPtr++ = VM5F_RxRdPtr_Byte();
	}
}

//----------------------------------------------------------------------------------------------
// Le payload do buffer de recepcao (buffer em ordem reversa)
//----------------------------------------------------------------------------------------------
void VM5F_RxPayload_To_BuffInv(void *DestPtr, uint8_t Size)
{
	uint8_t TempCnt;

	DestPtr += Size;
	for (TempCnt = 0; TempCnt < Size; TempCnt++)
	{
		DestPtr--;
		*(uint8_t *)DestPtr = VM5F_RxRdPtr_Byte();
	}
}

//----------------------------------------------------------------------------------------------
// Desloca ponteiro de leitura do buffer
//----------------------------------------------------------------------------------------------
void VM5F_RxRdPtr_Shift(uint8_t size)
{
	while (--size)
		VM5F_RxRdBuffPtr--;
}
