/*----------------------------------------------------------------------------------------------\
|																								|
| Tag_Gen2.c																					|
|																								|
\----------------------------------------------------------------------------------------------*/

#include "Tag_Gen2.h"
#include "Typedefs.h"


/*----------------------------------------------------------------------------------------------\
|																								|
| Registradores																					|
|																								|
\----------------------------------------------------------------------------------------------*/
TAG_GEN2_FLAGS			Tag_Gen2_Flags;
WORD					Tag_Gen2_ValidCnt[TAG_BUFFER_SIZE];

uint8_t					Tag_InPos;
uint8_t					Tag_OutPos;
uint8_t					Tag_Qtty;

TAG_VALID_TMR			Tag_TmrOk;
ANTENNA_MAP				Tag_EventOk;
TAG_GEN2_DATA			Tag_Buffer[TAG_BUFFER_SIZE];
TAG_GEN2_DATA			Tag_Temp;

uint8_t					Tag_NewTag_Flag;
TAG_GEN2_DATA			Tag_Last;
uint8_t					Tag_Last_ValidationTmrOut;

/*----------------------------------------------------------------------------------------------\
|																								|
| Funcoes e procedures																			|
|																								|
\----------------------------------------------------------------------------------------------*/
void Tag_Gen2_InsertTemp(void);
void Tag_Gen2_Proc(void);
void Tag_Gen2_Add(uint8_t* TagPtr, uint8_t TagSize);
void Tag_Gen2_ClrQtty(void);
void Tag_Gen2_GetCode(uint8_t* DestPtr, uint8_t TagSize);

uint8_t Tag_Gen2_GetAntNr(void);
uint8_t Tag_Gen2_NextCode(void);

static void Tag_Gen2_Addto_TagBuffer (void);

/*----------------------------------------------------------------------------------------------\
|																								|
| Loop de processamento																			|
|																								|
\----------------------------------------------------------------------------------------------*/
void Tag_Gen2_Proc(void)
{
	uint8_t n;

	for (n=0; n < TAG_BUFFER_SIZE; n++) {
		if(Tag_Gen2_ValidCnt[n].Int >= /*FncGrp_Setup_Int.FNC.TagOkTmr.Int*/20)
			Bit_Set(Tag_TmrOk.Value, BIT(n));
	}

	Tag_NewTag_Flag = 0; 	// nenhum tag adicionado
	if (Tag_Gen2_Flags.Tag_Added == 1) 
	{
		Tag_Gen2_Flags.Tag_Added = 0;
		Tag_Gen2_InsertTemp();
	}
}


//----------------------------------------------------------------------------------------------
// Processa tag temporario
//----------------------------------------------------------------------------------------------
void Tag_Gen2_InsertTemp(void)
{
	/*uint8_t		n;
	uint8_t*	TagTempPtr;	
	uint8_t*	TagLastPtr;
	WORD		TagTIDCrc;

	switch (FncGrp_Setup_Char.FNC.AuthSel)
	{
		case SI_TAG_AUTH_1:
			if (Tag_Temp.Code.DATA.Tid.Int != SOLIDINVENT_TID_DEFAULT)
			{
				return;
			}
			TagTIDCrc.Int = 0;

			Math_CRC16_CCITT(&TagTIDCrc.Int, Tag_Temp.Code.DATA.Tid.BYTES.Msb);
			Math_CRC16_CCITT(&TagTIDCrc.Int, Tag_Temp.Code.DATA.Tid.BYTES.Lsb);

			TagTempPtr = &Tag_Temp.Code.DATA.TagCode[TAG_GEN2_SIZE-5];
			for (n = 0; n < (TAG_GEN2_SIZE-4); n++)
			{
				Math_CRC16_CCITT(&TagTIDCrc.Int, *TagTempPtr--);
			}

			Math_CRC16_CCITT(&TagTIDCrc.Int, Tag_Temp.Code.DATA.TidCRC16.BYTES.Msb);
			Math_CRC16_CCITT(&TagTIDCrc.Int, Tag_Temp.Code.DATA.TidCRC16.BYTES.Lsb);

			if (TagTIDCrc.Int)
			{
				return;
			}
			Tag_Temp.Code.DATA.Tid.Int = 0;
			Tag_Temp.Code.DATA.TidCRC16.Int = 0;

			//	Tag ja esta no buffer dentro do tempo de tag valido 
			if (Tag_Gen2_TagOnBuffer(&Tag_Temp))
				return;
		break;
		
		case SI_TAG_AUTH_2:
			//	Ja foi verificado tempo de tag valido, somente remove bytes da SI_AUTH_1 
			Tag_Temp.Code.DATA.Tid.Int = 0;
			Tag_Temp.Code.DATA.TidCRC16.Int = 0;
		break;
		
		//	Nao possui verificacao 
		case SI_TAG_AUTH_DISABLED:
		default:
			break;
	}

	// Copia tag como sendo a ultima adicionada 
	TagTempPtr = &Tag_Temp.RFU;
	TagLastPtr = &Tag_Last.RFU;
	for (n = 0; n < sizeof(TAG_GEN2_DATA); n++)
	{
		*TagLastPtr++ = *TagTempPtr++;
	}
	*/
	/* Se nao ha necessidade de validacao pela entrada digital, 
	 * ja eh possivel adiciona-la ao buffer 
	 */
	//Tag_Gen2_Addto_TagBuffer();
}

//----------------------------------------------------------------------------------------------
// Adiciona tag ao buffer e indica novo tag
//----------------------------------------------------------------------------------------------
void Tag_Gen2_Addto_TagBuffer (void)
{
	uint8_t*	TagBuffPtr;
	uint8_t*	TagLastPtr;

	// ajusta tempo de tag valido
	Bit_Clr(Tag_TmrOk.Value, BIT(Tag_InPos));
	Tag_Gen2_ValidCnt[Tag_InPos].Int = 0;

	// copia ultimo tag adicionado ao buffer
	TagLastPtr = &Tag_Last.RFU;
	TagBuffPtr = &Tag_Buffer[Tag_InPos].RFU;

	for (uint8_t n = 0; n < sizeof(TAG_GEN2_DATA); n++)
	{
		*TagBuffPtr++ = *TagLastPtr++;
	}

	if (++Tag_InPos >= TAG_BUFFER_SIZE)
	{
		Tag_InPos = 0;
	}
	if (Tag_Qtty < TAG_BUFFER_SIZE)
	{
		Tag_Qtty++;
	}
	else
	{
		Tag_OutPos = Tag_InPos;
	}

	// indica que tag foi adicionado
	Tag_NewTag_Flag = 0xFF;
}

//----------------------------------------------------------------------------------------------
// Verifica tempo de tag valido do tag
// 1 - tag encontrado, nao pode ser adicionado / 0 - tag nao foi encontrado no buffer
//----------------------------------------------------------------------------------------------
uint8_t Tag_Gen2_TagOnBuffer (TAG_GEN2_DATA *tag)
{
	uint8_t		n, i;
	uint8_t		TagOk;
	uint8_t*	TagTempPtr;
	uint8_t*	TagLastPtr;

	// verifica se o tag e diferente de todos os outros tags do buffer
	TagOk = 0;
	
	for(i=0; i< TAG_BUFFER_SIZE; i++){
		if(Bit_Get(Tag_TmrOk.Value, BIT(i))) //verifica tempo de tag valido
			TagOk++;
		else {
			TagLastPtr = &Tag_Buffer[i].Code.DATA.TagCode[0];
			TagTempPtr = &tag->Code.DATA.TagCode[0];
			for (n = 0; n < (TAG_GEN2_SIZE-4); n++)
			{
				if (*TagLastPtr != *TagTempPtr)
				{
					TagOk++;
					n = TAG_GEN2_SIZE;			//check the next tag
				}
				TagLastPtr++;
				TagTempPtr++;
			}
		}
	}

	if (TagOk != TAG_BUFFER_SIZE)
		return 1;
	else
		return 0;
}

//----------------------------------------------------------------------------------------------
// Recupera codigo do tag do buffer
//----------------------------------------------------------------------------------------------
void Tag_Gen2_GetCode(uint8_t* DestPtr, uint8_t TagSize)
{
	uint8_t n;
	uint8_t* TagBuffPtr;

	TagBuffPtr = &Tag_Buffer[Tag_OutPos].Code.Bytes[0];
	for (n = 0; n < TagSize; n++) 
	{
		*DestPtr++ = *TagBuffPtr++;
	}
}

//----------------------------------------------------------------------------------------------
// Retorna o ponteiro do buffer para leitura
//----------------------------------------------------------------------------------------------
void* Tag_Gen2_GetRdPtr(void)
{
	return &Tag_Buffer[Tag_OutPos].RFU; // posicao inicial da estrutura
}

//----------------------------------------------------------------------------------------------
// Avanca leitura de tags
//----------------------------------------------------------------------------------------------
uint8_t Tag_Gen2_NextCode(void)
{
	if (Tag_Qtty == 0)
		return 0;

	if (++Tag_OutPos >= TAG_BUFFER_SIZE)
		Tag_OutPos = 0;

	return --Tag_Qtty;
}

//----------------------------------------------------------------------------------------------
// Limpa buffer
//----------------------------------------------------------------------------------------------
void Tag_Gen2_ClrQtty(void)
{
	Tag_OutPos = 0;
	Tag_InPos = 0;
	Tag_Qtty = 0;
}

//----------------------------------------------------------------------------------------------
// Recebe tag temporario
//----------------------------------------------------------------------------------------------
void Tag_Gen2_Add(uint8_t* TagPtr, uint8_t TagSize)
{
	uint8_t* TagTempPtr;
	uint8_t n;

	if (Tag_Gen2_Flags.Tag_Added == 0) 
	{
		TagTempPtr = &Tag_Temp.Functions.value;
		*TagTempPtr++ = FUNCTION_NONE;
		*TagTempPtr++ = TAGEXTRA_NONE;
		*TagTempPtr++ = UHF_READER;

		if (TagSize > TAG_GEN2_SIZE) 
		{
			TagSize = TAG_GEN2_SIZE;
		}

		// recebe tag
		for (n = 0; n < TagSize; n++) 
		{
			*TagTempPtr++ = *TagPtr++;
		}

		// insere zeros (caso necessario)
		for (n = 0; n < (TAG_GEN2_SIZE-TagSize); n++)
		{
			*TagTempPtr++ = 0;
		}

		Tag_Gen2_Flags.Tag_Added = 1;
	}
}


/*
 * Informa tags com a impressao do Facility Code errado
 *
 * Na impressao das tags inferiores ao numero serial 159596,
 * foram convertidos para decimal somente o IdCode, ao inves
 * do FacilityCode + IdCode
 * Resulta que nas tags de 100000 a 159596, quando convertidos,
 * o FacilityCode nao seria o mesmo do impresso na tag.
 */
uint8_t
Tag_Gen2_WrongFacility (uint8_t *tagptr)
{
#define TAG_WRONGFAC_MAX	0x159596UL
#define TAG_WRONGFAC_MIN	0x100000UL

	uint8_t i;
	uint32_t tag_temp;
	
	tag_temp = ( (uint32_t) tagptr[0]	)		\
			| ( (uint32_t) tagptr[1] << 8 )		\
			| ( (uint32_t) tagptr[2] << 16);

	if ((tag_temp < TAG_WRONGFAC_MIN) || (tag_temp > TAG_WRONGFAC_MAX))
	{
		return 0;
	}
	for (i=3; i < TAG_GEN2_SIZE; i++)
	{
		// verifica demais posicoes da tag
		if (tagptr[i] != 0)
			return 0;
	}
	return 6; // necessita correcao no facility
}

/* Temporizacao do timeout de validacao do tag */
void 
Tag_Gen2_ProcTimming (TIMER_SCALES scale)
{
	if (scale == TIMER_10MS)
	{
		// validacao de tag
		for (uint8_t n = 0; n < TAG_BUFFER_SIZE; n++)
		{
			Tag_Gen2_ValidCnt[n].Int++;
		}
	}
	else if (scale == TIMER_250MS)
	{
		if (Tag_Last_ValidationTmrOut)
			Tag_Last_ValidationTmrOut--;
	}
}
