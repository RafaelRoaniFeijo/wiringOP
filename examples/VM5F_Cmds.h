/*----------------------------------------------------------------------------------------------\
|																								|
| VM5F_Cmds																					|
|																								|
\----------------------------------------------------------------------------------------------*/

#ifndef _VM5F_CMDS_H
#define _VM5F_CMDS_H

#include "VM5F_Ctrl.h"

/*----------------------------------------------------------------------------------------------\
|																								|
| Constantes																					|
|																								|
\----------------------------------------------------------------------------------------------*/

/*********************** Comandos utilizados **************************/
#define VM5F_CMD_RESET 0x70
#define VM5F_CMD_SET_BAUDRATE 0x71
#define VM5F_CMD_GET_FWVERSION 0x72
#define VM5F_CMD_SET_OUTPUTPWR 0x76
#define VM5F_CMD_GET_OUTPUTPWR 0x77
#define VM5F_CMD_SET_FREQUENCY 0x78
#define VM5F_CMD_GET_FREQUENCY 0x79
#define VM5F_CMD_INVENTORY 0x80
#define VM5F_CMD_READ_TAG 0x81
#define VM5F_CMD_WRITE_TAG 0x82
#define VM5F_CMD_LOCK_TAG 0x83
#define VM5F_CMD_REALTIME_INVENTORY 0x89
#define VM5F_CMD_GET_CLR_TAGBUF 0x91

/******************* Configuracoes disponiveis ************************/
typedef enum
{
	VM5F_FREQREGION_1 = 0x01,
	VM5F_FREQREGION_2,
	VM5F_FREQREGION_3,
	VM5F_FREQREGION_4,
} VM5F_FREQ_REGION;

#define VM5F_FREQSPACE_OFFSET 10e3 // FreqSpace is x10kHz
typedef enum
{
	VM5F_FREQSPACE_100KHZ = 10,
	VM5F_FREQSPACE_200KHZ = 20,
	VM5F_FREQSPACE_300KHZ = 30,
	VM5F_FREQSPACE_400KHZ = 40,
	VM5F_FREQSPACE_500KHZ = 50,
} VM5F_FREQ_SPACE;

typedef enum
{
	VM5F_OUTPUTPWR_18DBM = 18,
	VM5F_OUTPUTPWR_19DBM,
	VM5F_OUTPUTPWR_20DBM,
	VM5F_OUTPUTPWR_21DBM,
	VM5F_OUTPUTPWR_22DBM,
	VM5F_OUTPUTPWR_23DBM,
	VM5F_OUTPUTPWR_24DBM,
	VM5F_OUTPUTPWR_25DBM,
	VM5F_OUTPUTPWR_26DBM,
} VM5F_OUTPUT_POWER;

/******************* Configuracoes padrao (fixa) ************************/
#define VM5F_FREQSPACE_DEF VM5F_FREQSPACE_200KHZ
#define VM5F_FREQREGION_DEF VM5F_FREQREGION_4

/********************* Respostas do comando *****************************/
#define VM5F_CMD_ERRORCODE_SIZE 0x04

#define VM5F_COMMAND_SUCESS 0x10
#define VM5F_BUFFER_EMPTY 0x38
//#define VM5F_WRITE_FLASH_ERROR		0x23 - Foi retirado
#define VM5F_FWVERSION_EXPECTED 0x0109

/*----------------------------------------------------------------------------------------------\
|																								|
| Externos																						|
|																								|
\----------------------------------------------------------------------------------------------*/
void VM5F_RxData_Proc(void);

uint8_t VM5F_TxCmd_ResetMCU(void);
uint8_t VM5F_TxCmd_GetFWversion(void);
uint8_t VM5F_TxCmd_SetBaudRate(VM5F_BAUDRATES baudrate); // - adicionado o parametro baudrate
uint8_t VM5F_TxCmd_SetOutputPwr(void);
uint8_t VM5F_TxCmd_GetOutputPwr(void);
uint8_t VM5F_TxCmd_SetFrequency(void);
uint8_t VM5F_TxCmd_GetFrequency(void);
uint8_t VM5F_TxCmd_Inventory(uint8_t rounds); // - adicionado o parametro rounds
uint8_t VM5F_TxCmd_RealTimeInventory(uint8_t rounds);// - adicionado o parametro rounds
uint8_t VM5F_TxCmd_GetAndClearTagBuffer(void);
uint8_t VM5F_TxCmd_WriteTag(uint8_t *password, GEN2_MEMBANK membank, uint8_t address, uint8_t *data, uint8_t size);// - adicionado o parametro password como ponteiro, membank(A palavra-chave é usada para nomear objetos definidos pelo usuário. Estruturas muitas vezes têm que ser declaradas várias vezes no código. Add a GEN2_MEMBANK Assim, o usuário pode declarar vários codinomes de nomes para tipos incorporados e, em seguida, acorrentar declarações adicionais usando codinomes já criados)
uint8_t VM5F_TxCmd_LockTag(uint8_t *password, GEN2_LOCK_MEMBANK membank, GEN2_LOCKTYPES locktype); //O locktype pode acessar os codinomes do GEN2_LOCKTYPES
uint8_t VM5F_TxCmd_ReadTag(uint8_t *password);

#endif