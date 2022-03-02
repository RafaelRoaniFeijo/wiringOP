/*
 * VM5F Ctrl
 */

#include <string.h>
#include <stdio.h>
#include "VM5F_Cmds.h"
#include "VM5F_Engine.h"

#include "VM5F_Ctrl.h"

/*
 * Registers
 */
static uint8_t connect_tmr;
static uint8_t ctrl_tmr;

#define reset_tmr connect_tmr

TAG_GEN2_SOLINV_DATA VM5F_received_tag;
static uint8_t tag_to_transmit[TAG_GEN2_SIZE];
static uint8_t access_password[sizeof(DWORD)];

static enum {
	WAIT,
	INIT,
	READ,
	WRITE,
} main_state = WAIT;

/*
 * Procedures
 */
void VM5F_Proc(void);
void VM5F_ProcTimming(TIMER_SCALES);

uint8_t VM5F_Initialize_Proc(void);
uint8_t VM5F_Read_Proc(void);
uint8_t VM5F_Write_Proc(void);

static DWORD get_accesspassword(uint8_t *tag);
uint8_t check_sitag(TAG_GEN2_SOLINV_DATA *tag);

/*
 * Processo principal /////
 Funcionamento: O main_state é uma variável estática
 */
void VM5F_Proc(void)
{
	switch (main_state)
	{
	case WAIT:
		if (ctrl_tmr == 0)
		{
			//se o registrador não leu 
			if (VM5F_Flags.DATA.ReaderFail || connect_tmr == 0)
			{
				VM5F_Flags.DATA.ReaderFail = 0;
				main_state = INIT; //muda de estado
			}
			else if (VM5F_Flags.DATA.Write)
				main_state = WRITE;
			else
				main_state = READ;
			
	//	printf("State %d", main_state);
		}
		break;

	case INIT:
		if (VM5F_Initialize_Proc())
		{
			connect_tmr = VM5F_OPERATING_RESET_TMR;
			ctrl_tmr = VM5F_INITRESET_TMROUT;
			main_state = WAIT;
		}
		break;

	case READ:
		if (VM5F_Read_Proc())
		{
			ctrl_tmr = VM5F_AUTH_IDLE_TMR;
			main_state = WAIT;
		}
		break;

	case WRITE:
		if (VM5F_Write_Proc())
			main_state = WAIT;
		break;
	}
}

/*
 * Processo de inicializacao do modulo
 */
uint8_t
VM5F_Initialize_Proc(void)
{
	static enum {
		RESET,
		WAIT,
		GET_OUTPUT_PWR,
		SET_OUTPUT_PWR,
		GET_FREQUENCY,
		SET_FREQUENCY,
		DONE,
	} state = RESET;

	if (VM5F_Flags.DATA.ReaderFail)
		state = DONE;

	switch (state)
	{
	case RESET:
		if (VM5F_TxCmd_ResetMCU())
		{
			ctrl_tmr = VM5F_INITRESET_TMROUT;
			state++;
		}
		break;

	case WAIT:
		if (ctrl_tmr == 0)
			state++;
		break;

	case GET_OUTPUT_PWR:
		if (VM5F_TxCmd_GetOutputPwr())
		{
			if (VM5F_Flags.DATA.CheckCmdData)
				state = SET_OUTPUT_PWR;
			else
				state = GET_FREQUENCY;
		}
		break;

	case SET_OUTPUT_PWR:
		if (VM5F_TxCmd_SetOutputPwr())
			state++;
		break;

	case GET_FREQUENCY:
		if (VM5F_TxCmd_GetFrequency())
		{
			if (VM5F_Flags.DATA.CheckCmdData)
				state = SET_FREQUENCY;
			else
				state = DONE;
		}
		break;

	case SET_FREQUENCY:
		if (VM5F_TxCmd_SetFrequency())
			state++;
		break;

	case DONE:
		state = RESET;
		return 1;
		break;
	}
	return 0;
}

/*
 * Processo de leitura de tags
 */
uint8_t
VM5F_Read_Proc(void)
{
	static enum {
		INVENTORY,
		GET_TAG,
		ACCESS,
		END
	} state = INVENTORY;

	if (VM5F_Flags.DATA.ReaderFail)
		state = END;

	switch (state)
	{
	case INVENTORY:
		if (VM5F_TxCmd_Inventory(VM5F_AUTH_ROUNDS))
		{
			if (VM5F_Flags.DATA.CheckCmdData)
				state++;
			else
				state = END;
		}
		break;

	case GET_TAG:
		if (VM5F_TxCmd_GetAndClearTagBuffer())
		{
			if (VM5F_Flags.DATA.CheckCmdData)
			{
				reset_tmr = VM5F_OPERATING_RESET_TMR;

				//if (FncGrp_Setup_Char.FNC.AuthSel == SI_TAG_AUTH_2)
				{
					DWORD password;

					//	Obtem senha de acesso ao tag - rotina identica a da gravacao
					password = get_accesspassword (VM5F_received_tag.Bytes);
					access_password[0] = password.Char[1];
					access_password[1] = password.Char[0];
					access_password[2] = password.Char[3];
					access_password[3] = password.Char[2];

					state = ACCESS;
					break;
				}
			}
			state = END;
		}
		break;

	case ACCESS:
		if (VM5F_TxCmd_ReadTag(access_password))
			state++;
		break;

	case END:
		state = INVENTORY;
		return 1;
		break;
	}
	return 0;
}

/*
 * Processo de gravacao de tags
 */
uint8_t
VM5F_Write_Proc(void)
{
	static enum {
		WRITE_EPC,
		WRITE_RESERVED,
		LOCK_EPC,
		LOCK_ACCESS_PW,
		LOCK_KILL_PW,
		END
	} state = WRITE_EPC;

	if (VM5F_Flags.DATA.ReaderFail)
	{
		printf("\nreader Fail\n");
		state = END;
	}

	switch (state)
	{
	case WRITE_EPC: //quando desaproximo vem aqui    aproximo, apresenta o numero e depois retorna aquijk
	
	{
		uint8_t def_pw[GEN2_PASSWORD_SIZE] = {0, 0, 0, 0};
		uint8_t addr = 2; // inicia apos PC + CRC

		if (VM5F_TxCmd_WriteTag(def_pw, MEMBANK_EPC, addr, tag_to_transmit, TAG_GEN2_SIZE))
		{
			
			VM5F_Flags.DATA.WriteSucceed = 0; //libera buffer para gravar outro

			if (VM5F_Flags.DATA.CheckCmdData)
			{
				
				DWORD password;

				memcpy(VM5F_received_tag.Bytes, tag_to_transmit, TAG_GEN2_SIZE);

				/*	Obtem senha de acesso ao tag - rotina identica a da gravacao */
				password = get_accesspassword(VM5F_received_tag.Bytes);
				access_password[0] = password.Char[1];
				access_password[1] = password.Char[0];
				access_password[2] = password.Char[3];
				access_password[3] = password.Char[2];
				printf("\ncheguei aqui 1\n");

				state++;
			}
			else
				state = END;
		}
	}
	break;

	case WRITE_RESERVED:
	{
		
		uint8_t def_pw[GEN2_PASSWORD_SIZE] = {0, 0, 0, 0};
		uint8_t kill_pw[GEN2_PASSWORD_SIZE] = {0, 0, 0, 0};
		uint8_t addr = 0;
		uint8_t reserved_mem[GEN2_PASSWORD_SIZE * 2];
		memcpy(&reserved_mem[0], kill_pw, GEN2_PASSWORD_SIZE);
		memcpy(&reserved_mem[4], access_password, GEN2_PASSWORD_SIZE);

		if (VM5F_TxCmd_WriteTag(def_pw, MEMBANK_RESERVED, addr, reserved_mem, sizeof(reserved_mem)))
		{
			
			if (VM5F_Flags.DATA.CheckCmdData)
			{
				if (VM5F_Flags.DATA.Lock)
				{
					printf("\ncheguei aqui 2\n");
					state++;
					break;
				}
				VM5F_Flags.DATA.WriteSucceed = 1; //Como o lock = 1, está contando mais um no estado e sai desse case
			}
		
			state = END;
		}
			
	}
	//printf("aqui no CheckCmdData deu %d", VM5F_Flags.DATA.WriteSucceed);
	break;

	case LOCK_EPC:
	
		if (VM5F_TxCmd_LockTag(access_password, LOCK_MEMBANK_EPC, LOCKTYPE_LOCK))
		{
			if (VM5F_Flags.DATA.CheckCmdData)
			{
				printf("\ncheguei aqui 3\n");
				state++;
			}
			else
				state = END;
		}
		break;

	case LOCK_ACCESS_PW:
		if (VM5F_TxCmd_LockTag(access_password, LOCK_MEMBANK_RESERVED_ACCESS, LOCKTYPE_PERMANENT_LOCK))
		{
			if (VM5F_Flags.DATA.CheckCmdData)
			{
				printf("\ncheguei aqui 4\n");
				state++;
			}
			else
				state = END;
		}
		break;

	case LOCK_KILL_PW:
	
		if (VM5F_TxCmd_LockTag(access_password, LOCK_MEMBANK_RESERVED_KILL, LOCKTYPE_PERMANENT_LOCK))
		{
			if (VM5F_Flags.DATA.CheckCmdData)
			{
				printf("\ncheguei aqui 5\n");
				VM5F_Flags.DATA.WriteSucceed = 1; //verificar
				state++;
			}
			else
				state = END;
		}
		break;

	case END:
		printf("\ncheguei aqui 6\n");
		VM5F_Flags.DATA.Write = 0;
		state = WRITE_EPC;
		return 1;
		break;
	}
	return 0;
}

/*
 *	Adiciona dados a serem escritos no tag
 */
void VM5F_WriteTag(GEN2_WRITE_DATA *write_info)
{
	TAG_GEN2_SOLINV_DATA tag_transmit_compose;

	/*	Original Tag data */
	memcpy(VM5F_received_tag.Bytes, write_info->DATA.epc_origin, TAG_GEN2_SIZE);

	/*	Copy data to transmit */
	tag_transmit_compose.DATA.Tid.Int = write_info->DATA.tid;
	memcpy(tag_transmit_compose.DATA.TagCode, write_info->DATA.serial, TAG_CODE_SIZE);

	/*	Compose TID CRC */
	uint8_t temp_tag[TAG_CODE_SIZE + sizeof(tag_transmit_compose.DATA.Tid)];
	temp_tag[0] = tag_transmit_compose.DATA.Tid.Char[1];
	temp_tag[1] = tag_transmit_compose.DATA.Tid.Char[0];
	for (uint8_t i = 0; i < TAG_CODE_SIZE; i++)
		temp_tag[i + 2] = tag_transmit_compose.DATA.TagCode[TAG_CODE_SIZE - 1 - i];

	DWORD temp_tidcrc = {.Long = 0};
	for (uint8_t i = 0; i < sizeof(temp_tag); i++)
	{
		temp_tidcrc.Long ^= temp_tag[i];

		for (int j = 0; j < 8; j++)
		{
			if ((temp_tidcrc.Long & 0x0001) == 0x0001)
			{
				uint32_t aux = (temp_tidcrc.Long >> 1);
				temp_tidcrc.Long = (aux ^ 0x8408);
			}
			else
				temp_tidcrc.Long = (temp_tidcrc.Long >> 1);
		}
	}
	tag_transmit_compose.DATA.TidCRC16.Char[0] = temp_tidcrc.Char[1];
	tag_transmit_compose.DATA.TidCRC16.Char[1] = temp_tidcrc.Char[0];

	/*	Copy data */
	for (uint8_t i = 0; i < TAG_GEN2_SIZE; i++)
		tag_to_transmit[i] = tag_transmit_compose.Bytes[TAG_GEN2_SIZE - 1 - i];

	/*printf("Tag to write");
	for (uint8_t i = 0; i < TAG_GEN2_SIZE; i++) //retorna os valores lidos
		printf("%d-", tag_to_transmit[i]);
	fflush(stdout);*/

	/*	If tag it should be locked */
	VM5F_Flags.DATA.Lock = write_info->DATA.flag.DATA.lock;

	/*	Indicates to start write */
	VM5F_Flags.DATA.Write = 1;
}

/*
 *	Obtem resultado da ultima gravacao de tag. E limpa buffer
 */
uint8_t
VM5F_Get_LastWriteResult()
{
	uint8_t result = VM5F_Flags.DATA.WriteSucceed; //recebe sempre 0
	
	VM5F_Flags.DATA.WriteSucceed = 0;
	//printf("%d", result);
	return result;

	
}

/*
 *	Retorna se o leitor esta em erro de comunicao
 */
uint8_t
VM5F_Get_CommError(void)
{
	return VM5F_Flags.DATA.CommOffline;
}

/*
 *	Retorna se o leitor esta disponivel para gravacao
 */
uint8_t
VM5F_Get_FreeToWrite(void)
{
	return main_state == WAIT && VM5F_Get_CommError() == 0;
}

/*
 * Processo de temporizacao
 */
void VM5F_ProcTimming(TIMER_SCALES timer)
{
	if (timer == TIMER_10MS)
	{
		if (ctrl_tmr)
			ctrl_tmr--;

		VM5F_RxTimeout++;	  // tempo de reset do recebimento USART
		VM5F_TxAttemptsTmr++; // tempo entre tentativas de envio
	}
	else if (timer == TIMER_250MS)
	{
		if (connect_tmr)
			connect_tmr--;

		VM5F_CommOfflineCnt++; // tempo para informar de sistema offiline
	}
}

/*
 *	Verifica se eh um tag solid invent
 *	1 - ok, 0 - nok
 */
uint8_t
check_sitag(TAG_GEN2_SOLINV_DATA *tag)
{
	WORD TagTIDCrc;
	uint8_t *TagTempPtr;

	if (tag->DATA.Tid.Int != SOLIDINVENT_TID_DEFAULT)
	{
		return 0;
	}
	TagTIDCrc.Int = 0;

	// Math_CRC16_CCITT(&TagTIDCrc.Int, tag->DATA.Tid.BYTES.Msb);
	// Math_CRC16_CCITT(&TagTIDCrc.Int, tag->DATA.Tid.BYTES.Lsb);

	TagTempPtr = &tag->DATA.TagCode[TAG_GEN2_SIZE - 5];
	for (uint8_t n = 0; n < (TAG_GEN2_SIZE - 4); n++)
	{
		// Math_CRC16_CCITT(&TagTIDCrc.Int, *TagTempPtr--);
	}

	// Math_CRC16_CCITT(&TagTIDCrc.Int, tag->DATA.TidCRC16.BYTES.Msb);
	// Math_CRC16_CCITT(&TagTIDCrc.Int, tag->DATA.TidCRC16.BYTES.Lsb);

	if (TagTIDCrc.Int)
	{
		return 0;
	}

	tag->DATA.Tid.Int = 0;
	tag->DATA.TidCRC16.Int = 0;

	return 1;
}

/*
 *	Gera a senha de acesso para o tag Solid Invent
 */
static DWORD
get_accesspassword(uint8_t *tag)
{
	uint32_t pass = 0;
	DWORD ret;

	for (int i = 0; i < TAG_GEN2_SIZE; i++)
	{
		pass = pass ^ tag[i];

		for (int j = 0; j < 8; j++)
		{
			if ((pass & 0x0001) == 0x0001)
			{
				uint64_t aux = 0x84088408;
				uint64_t aux2 = pass >> 1;
				pass = (aux2 ^ aux);
			}
			else
				pass = pass >> 1;
		}
	}
	ret.Long = pass;
	return ret;
}
