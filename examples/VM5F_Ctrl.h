/*----------------------------------------------------------------------------------------------\
|																								|
| VM5F_Cmds																					|								
|																								|
\----------------------------------------------------------------------------------------------*/

#ifndef _VM5F_CTRL_H
#define _VM5F_CTRL_H

#include <stdint.h>
#include "Tag_Gen2.h"


/*----------------------------------------------------------------------------------------------\
|																								|
| Constantes																					|
|																								|
\----------------------------------------------------------------------------------------------*/

/**************** PASSWORD ********************/
#define GEN2_PASSWORD_SIZE				4
#define VM5F_ERRORCODE_PASSWORDERROR	0x40

/**************** MEM BANK ********************/
typedef enum {
	MEMBANK_RESERVED = 0,
	MEMBANK_EPC,
	MEMBANK_TID,
	MEMBANK_USER,
} GEN2_MEMBANK;

/**************** MEM BANK LOCK ****************/
typedef enum {
	LOCK_MEMBANK_USER = 1,
	LOCK_MEMBANK_TID,
	LOCK_MEMBANK_EPC,
	LOCK_MEMBANK_RESERVED_ACCESS,
	LOCK_MEMBANK_RESERVED_KILL,
} GEN2_LOCK_MEMBANK;

/**************** LOCK TYPES *******************/
typedef enum {
	LOCKTYPE_OPEN,
	LOCKTYPE_LOCK,
	LOCKTYPE_PERMANENT_OPEN,
	LOCKTYPE_PERMANENT_LOCK,
} GEN2_LOCKTYPES;

/**************** BAUDRATE ********************/
#define VM5F_BAUDRATEPOS_MAX	2
typedef enum {
	VM5F_BAUDRATE_38400 = 0x03,
	VM5F_BAUDRATE_115200 = 0x04,
} VM5F_BAUDRATES;

/*
 * Define qual baudrate sera utilizado pelo modulo
 * 0 = VM5F_BAUDRATE_38400
 * 1 = VM5F_BAUDRATE_115200
 */
#define VM5F_BAUDRATEPOS_DEFAULT	1

#if (VM5F_BAUDRATEPOS_DEFAULT >= VM5F_BAUDRATEPOS_MAX)
#error "Baudrate nao habilitado para este modulo"
#endif

/********* VALORES MAXIMO E MINIMOS *************/
#define VM5F_MAX_FREQQTTY		63
#define VM5F_FREQMIN			902000UL
#define VM5F_FREQMAX			928000UL

#define VM5F_POT_MAX_DBM		26

/*************** INVENTARIO *******************/
/* Each inventory round takes between 30~50ms */
#define VM5F_AUTH_ROUNDS		3
#define VM5F_AUTH_IDLE_TMR		30	//x10ms

/*************** TEMPORIZACOES ****************/
#define VM5F_INITRESET_TMROUT		20	//x10ms	
#define VM5F_OPERATING_RESET_TMR	40	//x250ms = 10s	

/*----------------------------------------------------------------------------------------------\
|																								|
| Estruturas																					|
|																								|
\----------------------------------------------------------------------------------------------*/
typedef union
{
	uint8_t bytes;
	struct
	{
		uint8_t lock	: 1;	
	} DATA;
} GEN2_WRITE_FLAGS;

typedef union
{
	uint8_t bytes[TAG_GEN2_SIZE + sizeof(uint16_t) + TAG_CODE_SIZE + sizeof(GEN2_WRITE_FLAGS) +sizeof(uint8_t)];
	struct
	{
		uint8_t				epc_origin[TAG_GEN2_SIZE];
		uint16_t			tid;
		uint8_t				serial[TAG_CODE_SIZE];
		GEN2_WRITE_FLAGS	flag;
		uint8_t				rfu;
	} DATA;
	
} GEN2_WRITE_DATA;

/*----------------------------------------------------------------------------------------------\
|																								|
| Externos																						|
|																								|
\----------------------------------------------------------------------------------------------*/
extern TAG_GEN2_SOLINV_DATA VM5F_received_tag;

void VM5F_Proc (void);
void VM5F_ProcTimming (TIMER_SCALES);
void VM5F_WriteTag (GEN2_WRITE_DATA *write_info);

uint8_t VM5F_Get_LastWriteResult (void);
uint8_t VM5F_Get_CommError (void);
uint8_t VM5F_Get_FreeToWrite (void);

#endif