/*----------------------------------------------------------------------------------------------\
|																								|
| Tag_Gen2.h																					|
|																								|
\----------------------------------------------------------------------------------------------*/

#ifndef _TAG_GEN2_H
#define _TAG_GEN2_H

#include "Typedefs.h"

/*----------------------------------------------------------------------------------------------\
|																								|
| Macros																						|
|																								|
\----------------------------------------------------------------------------------------------*/
#define mTag_NewAdded() (Tag_NewTag_Flag == 0xFF)
#define Tag_Gen2_NewCode() (Tag_Qtty)
#define Tag_Gen2_TagQtty() (Tag_Qtty)

/*----------------------------------------------------------------------------------------------\
|																								|
| Constantes																					|
|																								|
\----------------------------------------------------------------------------------------------*/
#define TAG_BUFFER_SIZE 5
#define SOLIDINVENT_TID_DEFAULT 0x0001

/*----------------------------------------------------------------------------------------------\
|																								|
| Informacao sobre o leitor																			|
|																								|
\----------------------------------------------------------------------------------------------*/
/*
 * ATENTION: Should matches this values with software
 */
typedef enum
{
	FUNCTION_NONE = (0 << 0),
	FUNCTION_KEY_1 = (1 << 0),
	FUNCTION_KEY_2 = (1 << 1),
	FUNCTION_KEY_3 = (1 << 2),
	FUNCTION_KEY_4 = (1 << 3),
} KEYS_FUNCTIONS;

typedef enum
{
	TAGEXTRA_NONE = (0 << 0),
	TAGEXTRA_CRYPTO = (1 << 0),
	TAGEXTRA_PANIC = (1 << 1),
} TAG_EXTRA_INFO;

typedef enum
{
	NONE_READER,
	UHF_READER,
	MIFARE_READER,
	BIO_READER,
	PASSWORD_READER,
	_125KHZ_READER,
} READERS_ORIGIN;

/* Set cryptography by device - CriptoEnableSel */
typedef enum
{
	CRIPTO_DISABLE_ALL = 0,
	CRIPTO_ENABLE_KEYBOARD,
	CRIPTO_ENABLE_MIFARE,
	CRIPTO_ENABLE_ALL, // (CRIPTO_ENABLE_KEYBOARD | CRIPTO_ENABLE_MIFARE)
} CRIPTO_DEVICE_SEL;

/*----------------------------------------------------------------------------------------------\
|																								|
| Estruturas																					|
|																								|
\----------------------------------------------------------------------------------------------*/
#define TAG_GEN2_SIZE 12
#define TAG_CODE_SIZE TAG_GEN2_SIZE - 4

typedef union
{
	uint8_t Bytes[TAG_GEN2_SIZE];
	struct
	{
		uint8_t TagCode[TAG_CODE_SIZE];
		WORD TidCRC16;
		WORD Tid;
	} DATA;
} TAG_GEN2_SOLINV_DATA;

/*
 *	Functions keys
 */
typedef union
{
	uint8_t value;
	struct
	{
		uint8_t F1 : 1; // 0.0
		uint8_t F2 : 1; // 0.1
		uint8_t F3 : 1; // 0.2
		uint8_t F4 : 1; // 0.3
	} bits;
} TAGCTRL_FUNCTIONS;

/*
 *	Tag extras information
 */
typedef union
{
	uint8_t value;
	struct
	{
		uint8_t cripto : 1; // 0.0
		uint8_t panic : 1;	// 0.1
	} bits;
} TAGCTRL_EXTRAS;

/*
 *	All tag information
 */
typedef struct
{
	uint8_t RFU;
	TAGCTRL_FUNCTIONS Functions;
	TAGCTRL_EXTRAS Extras;
	READERS_ORIGIN Origin;
	TAG_GEN2_SOLINV_DATA Code;
} TAG_GEN2_DATA;

typedef union
{
	uint8_t Value;
	struct
	{
		uint8_t pos0 : 1;
		uint8_t pos1 : 1;
		uint8_t pos2 : 1;
		uint8_t pos3 : 1;
		uint8_t pos4 : 1;
		uint8_t vago5 : 1;
		uint8_t vago6 : 1;
		uint8_t vago7 : 1;
	} BYTE;
} TAG_VALID_TMR;

typedef struct
{
	uint8_t Tag_Added : 1; // 0.0
	uint8_t Vago01 : 1;	   // 0.1
	uint8_t Vago02 : 1;	   // 0.2
	uint8_t Vago03 : 1;	   // 0.3
	uint8_t Vago04 : 1;	   // 0.4
	uint8_t Vago05 : 1;	   // 0.5
	uint8_t Vago06 : 1;	   // 0.6
	uint8_t Vago07 : 1;	   // 0.7
} TAG_GEN2_FLAGS;

typedef struct
{
	uint8_t Counter;
	TAG_GEN2_DATA Tag;
} TAG_GEN2_BACKDATA;

/*----------------------------------------------------------------------------------------------\
|																								|
| Esternos																						|
|																								|
\----------------------------------------------------------------------------------------------*/
extern TAG_GEN2_FLAGS Tag_Gen2_Flags;
extern WORD Tag_Gen2_ValidCnt[TAG_BUFFER_SIZE];
extern uint8_t Tag_Qtty;

extern uint8_t Tag_NewTag_Flag;
TAG_GEN2_DATA Tag_Last;

extern void Tag_Gen2_Proc(void);
extern void Tag_Gen2_ClrQtty(void);
extern void Tag_Gen2_GetCode(uint8_t *DestPtr, uint8_t TagSize);
extern void Tag_Gen2_ProcTimming(TIMER_SCALES);
extern void *Tag_Gen2_GetRdPtr(void);
extern void Tag_Gen2_Add(uint8_t *TagPtr, uint8_t TagSize);

extern uint8_t Tag_Gen2_NextCode(void);
extern uint8_t Tag_Gen2_WrongFacility(uint8_t *tagptr);
extern uint8_t Tag_Gen2_TagOnBuffer(TAG_GEN2_DATA *tag);
#endif
