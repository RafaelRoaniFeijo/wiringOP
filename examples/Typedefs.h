/*----------------------------------------------------------------------------------------------\
|																								|
| TYPEDEFS																						|
|																								|
\----------------------------------------------------------------------------------------------*/
#include <stdint.h>


/*----------------------------------------------------------------------------------------------\
|																								|
| Constantes																					|
|																								|
\----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------\
|																								|
| Macros																						|
|																								|
\----------------------------------------------------------------------------------------------*/

// bit manipulation
#define 		Bit_Get(p,m) 		((p) & (m)) //define nome endere�o
#define 		Bit_Set(p,m) 		((p) |= (m)) 
#define 		Bit_Clr(p,m) 		((p) &= ~(m)) 
#define 		Bit_Tog(p,m) 		((p) ^= (m)) 
#define 		Bit_Write(c,p,m) 	(c ? Bit_Set(p,m) : Bit_Clr(p,m)) 
#define 		BIT(x) 				(0x01 << (x)) 
#define 		LONGBIT(x) 			((uint32_t)0x00000001 << (x)) 

#define 		LOW_BYTE(x) 		(x & 0xFF) 
#define 		HIGH_BYTE(x) 		(LOW_BYTE(x >> 8))

#define			LOW_NIBLLE(x)		(x & 0xF)
#define			HIGH_NIBLLE(x)		(LOW_NIBLLE(x >> 4))

#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

/*----------------------------------------------------------------------------------------------\
|																								|
| Estruturas																					|
|																								|
\----------------------------------------------------------------------------------------------*/
typedef union 
{
	uint16_t Int;
	uint8_t	 Char[2];
	int16_t  SInt;
	struct 
	{
		uint8_t Lsb;
		uint8_t Msb;
	} BYTES;
} WORD;

typedef union 
{
	uint32_t	Long;
	int32_t		SLong;
	uint16_t	Int[2];
	int16_t		SInt[2];
	uint8_t		Char[4];
	struct 
	{
		uint8_t LLsb;
		uint8_t LMsb;
		uint8_t ULsb;
		uint8_t UMsb;
	} BYTES;
} DWORD;

#if (ANTENA_QTTY > 8)
	#error "Quantidade de antenas nao e suportada pelo array abaixo"
#endif
typedef union 
{
	uint8_t Value;
	struct 
	{
		uint8_t 	AntNr1: 	1;	// 0.0 
		uint8_t 	AntNr2: 	1;	// 0.1 
		uint8_t 	AntNr3: 	1; 	// 0.2
		uint8_t 	AntNr4:		1; 	// 0.3
		uint8_t 	AntNr5:		1; 	// 0.4
		uint8_t 	AntNr6:		1; 	// 0.5
		uint8_t 	AntNr7:		1;	// 0.6
		uint8_t		AntNr8: 	1;	// 0.7
	} BITS;
} ANTENNA_MAP;

/*----------------------------------------------------------------------------------------------\
|																								|
| Subdivisoes do timer																			|
|																								|
\----------------------------------------------------------------------------------------------*/
typedef enum 
{
	TIMER_10MS,
	TIMER_50MS,
	TIMER_250MS,
	TIMER_500MS,
	TIMER_1S,
} TIMER_SCALES;

/*----------------------------------------------------------------------------------------------\
|																								|
| Erros																							|
|																								|
\----------------------------------------------------------------------------------------------*/
typedef enum 
{
	ERR_NONE					= 0x00,
	ERR_GRP_NOT_FOUND			= 0x01,
	ERR_CMD_NOT_FOUND			= 0x02,
	ERR_NO_TAG					= 0x03,
	ERR_INVALID_SIZE			= 0x04,
	ERR_INVALID_VALUE			= 0x05,
	ERR_INVALID_ADDRESS			= 0x06,
	ERR_PARAM_NOT_FOUND			= 0x07,
	ERR_PARAM_INVALID_SIZE		= 0x08,
	ERR_MANUT_MODE_OFF			= 0x09,
	ERR_OPER_MODE_INCORRECT		= 0x0A,
	ERR_PARAM_SIGN_NOT_FOUND	= 0x0B,
	ERR_INVALID_TAG				= 0x0C,
	ERR_BUFFER_FULL				= 0x0D,
	ERR_OPERATION_ATLIMIT		= 0x0E,
	ERR_INVALID_INDEX			= 0x0F,
	ERR_INVALID_HANDLE			= 0x10,
	ERR_NOT_FOUND				= 0x11,
	ERR_REMOTE_DEV_COMM			= 0x12,
	ERR_MEM_FULL				= 0x13,
	ERR_TEST_ERROR				= 0x14,
} SI_ERRORS;

/*----------------------------------------------------------------------------------------------\
|																								|
| Ponteiros de funcoes																			|
|																								|
\----------------------------------------------------------------------------------------------*/
typedef void(*FncPtr_t)(void);


#endif
