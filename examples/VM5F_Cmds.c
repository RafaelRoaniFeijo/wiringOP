/*
 * VM5F Cmds
 */

#include <stdio.h>
#include <string.h>
#include "VM5F_Ctrl.h"
#include "VM5F_Engine.h"
#include "VM5F_Cmds.h"
#include "WriteTag.h"
#include <stdlib.h>
/*
 * Sobre o protocolo adotado:
 *
 * LastCmdOk = Obteve resposta do comando enviado anteriormente
 * CheckCmdData = Payload recebido nao confere com o esperado
 * RxWaitAnswer = Obrigatorio recebimento de resposta. Faz tres \
				  tentativas enquanto nao recebe resposta.
 * CheckErrorCode = indica se o comando possui codigo de erro
 * ReaderFail = OU o leitor na recebeu resposta dentro das tres \
				tentativas, OU leitor acusou erro (pelo codigo)
 *
 */
/*
 * Registers
 */

/*
 * Procedures
 */
void VM5F_RxData_Proc(void);

void VM5F_RxCmd_FWversion(void);
void VM5F_RxCmd_GetOutputPwr(void);
void VM5F_RxCmd_GetFrequency(void);
void VM5F_RxCmd_Inventory(void);
void VM5F_RxCmd_GetAndClearTagBuffer(void);
void VM5F_RxCmd_RealTimeInventory(void);
void VM5F_RxCmd_GetDataFrameIntv(void);
void VM5F_RxCmd_ReadTag(void);
void VM5F_RxCmd_WriteTag(void);
void VM5F_RxCmd_LockTag(void);
void VM5F_RxCmd_CheckErrorCode(void);

uint8_t VM5F_TxCmd_ResetMCU(void);
uint8_t VM5F_TxCmd_GetFWversion(void);
uint8_t VM5F_TxCmd_SetBaudRate(VM5F_BAUDRATES baudrate);
uint8_t VM5F_TxCmd_SetOutputPwr(void);
uint8_t VM5F_TxCmd_GetOutputPwr(void);
uint8_t VM5F_TxCmd_SetFrequency(void);
uint8_t VM5F_TxCmd_GetFrequency(void);
uint8_t VM5F_TxCmd_Inventory(uint8_t rounds);
uint8_t VM5F_TxCmd_RealTimeInventory(uint8_t rounds);
uint8_t VM5F_TxCmd_GetAndClearTagBuffer(void);
uint8_t VM5F_TxCmd_WriteTag(uint8_t *password, GEN2_MEMBANK membank, uint8_t address, uint8_t *data, uint8_t size);
uint8_t VM5F_TxCmd_LockTag(uint8_t *password, GEN2_LOCK_MEMBANK membank, GEN2_LOCKTYPES locktype);
uint8_t VM5F_TxCmd_ReadTag(uint8_t *password);

/*
 * Tabela de comandos recebidos
 */
typedef struct
{
	uint8_t CmdSize;
	uint8_t CmdCode;
	void (*VM5F_CmdProc)(void);
} VM5F_CMDS;

const VM5F_CMDS VM5F_Cmds_Rx[] =
	{
		{VM5F_CMD_ERRORCODE_SIZE, VM5F_CMD_SET_BAUDRATE, &VM5F_RxCmd_CheckErrorCode},
		{0x05, VM5F_CMD_GET_FWVERSION, &VM5F_RxCmd_FWversion},
		{VM5F_CMD_ERRORCODE_SIZE, VM5F_CMD_SET_OUTPUTPWR, &VM5F_RxCmd_CheckErrorCode},
		{0x04, VM5F_CMD_GET_OUTPUTPWR, &VM5F_RxCmd_GetOutputPwr},
		{VM5F_CMD_ERRORCODE_SIZE, VM5F_CMD_SET_FREQUENCY, &VM5F_RxCmd_CheckErrorCode},
		{0x09, VM5F_CMD_GET_FREQUENCY, &VM5F_RxCmd_GetFrequency}, // Parameters for region 4
		{0x0C, VM5F_CMD_INVENTORY, &VM5F_RxCmd_Inventory},
		{0xFF, VM5F_CMD_GET_CLR_TAGBUF, &VM5F_RxCmd_GetAndClearTagBuffer},
		{0xFF, VM5F_CMD_READ_TAG, &VM5F_RxCmd_ReadTag},
		{0xFF, VM5F_CMD_WRITE_TAG, &VM5F_RxCmd_WriteTag},
		{0xFF, VM5F_CMD_LOCK_TAG, &VM5F_RxCmd_LockTag},
};

/*
 * Processa recebimento de dados
 */
void VM5F_RxData_Proc(void)
{
	if (VM5F_Flags.DATA.RxValidPacket == 1 && VM5F_Flags.DATA.TxDataInit == 0)
	{
		VM5F_CMDS Temp_RxCmds;
		uint8_t CmdIdx;

		VM5F_Flags.DATA.RxValidPacket = 0;
		VM5F_RxRdPtr_Reset_ToPayload();

		/* Para parametros com tamanho variavel, trata dados recebidos separadamente */
		for (CmdIdx = 0; CmdIdx < sizeof(VM5F_Cmds_Rx); CmdIdx++)
		{
			memcpy(&Temp_RxCmds, &VM5F_Cmds_Rx[CmdIdx], sizeof(VM5F_CMDS));
			if (VM5F_Buffer.DIR.Rx.DATA.Command == Temp_RxCmds.CmdCode)
			{
				/*	Check error code */
				if (VM5F_Buffer.DIR.Rx.DATA.Size == VM5F_CMD_ERRORCODE_SIZE &&
					VM5F_Buffer.DIR.Rx.DATA.Size != Temp_RxCmds.CmdSize)
				{
					VM5F_Flags.DATA.RxWaitAnswer = 0;
					VM5F_RxCmd_CheckErrorCode();
				}
				/*	Routine related to command */
				else if ((VM5F_Buffer.DIR.Rx.DATA.Size == Temp_RxCmds.CmdSize) || (Temp_RxCmds.CmdSize == 0xFF))
				{
					VM5F_Flags.DATA.RxWaitAnswer = 0;
					VM5F_Flags.DATA.LastCmdOk = 1;
					Temp_RxCmds.VM5F_CmdProc();
					return;
				}
			}
		}
	}
}

/************************************************************************/
/*   COMANDOS RECEBIDOS                                                 */
/************************************************************************/
/*
 *	Verifica resultado do comando baseado no codigo de erro
 */
void VM5F_RxCmd_CheckErrorCode(void)
{
	if ((VM5F_Buffer.DIR.Rx.DATA.ErrorCode == VM5F_COMMAND_SUCESS) || (VM5F_Buffer.DIR.Rx.DATA.ErrorCode == VM5F_BUFFER_EMPTY))
	{
		VM5F_Flags.DATA.LastCmdOk = 1;
	}
	else
	{
		printf("\nComando: %02X - Erro: %02X\n", VM5F_Buffer.DIR.Rx.DATA.Command, VM5F_Buffer.DIR.Rx.DATA.ErrorCode);
		VM5F_Flags.DATA.ReaderFail = 1;
	}
}

/*
 * Recebe versao de firmware
 */
void VM5F_RxCmd_FWversion(void)
{
	WORD _fw_version;

	// como nao possui retorno de erro parte do payload esta presente no ErrorCode
	_fw_version.BYTES.Msb = VM5F_Buffer.DIR.Rx.DATA.ErrorCode;
	_fw_version.BYTES.Lsb = VM5F_RxRdPtr_Byte();
	printf("Versão %d.%d \n", _fw_version.BYTES.Msb, _fw_version.BYTES.Lsb);
	fflush(stdout);

	if (_fw_version.Int < VM5F_FWVERSION_EXPECTED)
	{
		VM5F_RxRdPtr_Reset_ToPayload();
		VM5F_Flags.DATA.CheckCmdData = 1;
	}
}

/*
 * Tabela com as configuracoes de potencia
 */
const uint8_t OutPwr_dbm_Table[] =
	{
		26, 25, 24, 22, 19 // values in dBm
};

/*
 * Recebe potencia
 */
void VM5F_RxCmd_GetOutputPwr(void)
{
	uint8_t power, power_fnc;

	// como o comando eh pequeno, o valor esta presente no ErrorCode
	power = VM5F_Buffer.DIR.Rx.DATA.ErrorCode;
	memcpy(&power_fnc, &OutPwr_dbm_Table[3], 1);

	if (power != power_fnc)
		VM5F_Flags.DATA.CheckCmdData = 1;
}

/*
 * Indica recebimento da frequencia
 */
void VM5F_RxCmd_GetFrequency(void)
{
	uint32_t frequency, freq_fnc;
	uint8_t freq_buf[3], spacing, spacing_fnc, qtty, qtty_fnc;

	uint16_t freq_final = 925;	 // FncGrp_Setup_Int.FNC.EndFreq.Int;
	uint16_t freq_inicial = 918; // FncGrp_Setup_Int.FNC.InitialFreq.Int;

	/* Get configuration expected */

	/* Fixed frequency */
	if (freq_final <= freq_inicial)
	{
		spacing_fnc = 0;
		qtty_fnc = 1;
	}
	/* Range of frequencies */
	else
	{
		spacing_fnc = VM5F_FREQSPACE_DEF;

		qtty_fnc = (uint8_t)(((freq_final - freq_inicial) * 100) / VM5F_FREQSPACE_DEF); // Xe6/20e4 --> 6 - 4 = 2 --> 10^2 = 100

		if (qtty_fnc > VM5F_MAX_FREQQTTY)
			qtty_fnc = VM5F_MAX_FREQQTTY;
	}
	freq_fnc = (uint32_t)(freq_inicial * 1e3); // Initial frequency

	/* Get module configuration */
	spacing = VM5F_RxRdPtr_Byte();
	qtty = VM5F_RxRdPtr_Byte();

	VM5F_RxPayload_To_Buff(freq_buf, 3);
	frequency = (uint32_t)(freq_buf[2]);
	frequency |= ((uint32_t)(freq_buf[1]) << 8);
	frequency |= ((uint32_t)(freq_buf[0]) << 16);

	/* Check with value expected */
	if (spacing != spacing_fnc)
		goto not_equal;

	if (qtty != qtty_fnc)
		goto not_equal;

	if (frequency == freq_fnc)
		return;

not_equal:
	VM5F_Flags.DATA.CheckCmdData = 1;
}

/*
 * Recebe resultado do inventario
 */
void VM5F_RxCmd_Inventory(void)
{
	WORD _tagcount;

	VM5F_RxPayload_To_BuffInv(_tagcount.Char, sizeof(WORD));

	printf("Tag Count %d\n", _tagcount.Int);
	fflush(stdout);

	if (_tagcount.Int >= 1)
		VM5F_Flags.DATA.CheckCmdData = 1;
}

/*
 * Recebe tags armazenados no buffer interno do VM5F
 */
void VM5F_RxCmd_GetAndClearTagBuffer(void)
{
	/*	Obtem o numero de tags que seram recebidos */
	uint8_t _tagcount = VM5F_RxRdPtr_Byte();

	if (_tagcount == 1)
	{
		TAG_GEN2_DATA tag;

		/*	Obtem tag */
		VM5F_RxRdPtr_Shift(4); // ignora posicoes desnecessarias
		VM5F_RxPayload_To_BuffInv(tag.Code.Bytes, TAG_GEN2_SIZE);

		fflush(stdout);

		// todo print da tag;
		printf("Li um tag: ");

		/*->*/ file_permanente = fopen("permanente.txt", "w");
		if (file_permanente == 0)
		{
			printf("erro");
			exit(1);
		}
		fseek(file_permanente, 0, SEEK_SET);

		for (int i = 0; i < TAG_GEN2_SIZE; i++)
		{
			// tag.Code.Bytes[i] = file_permanente;
			//char *str = malloc(sizeof(tag.Code.Bytes[i]));
			//memcpy(str, (tag.Code.Bytes[i]), sizeof(tag.Code.Bytes[i]));
			// printf("str: %d %d", str, sizeof(tag.Code.Bytes[i]));

			//free(str);
			//str = malloc(sizeof file_permanente);
			//memcpy(str, file_permanente, sizeof file_permanente);
			//free(str);

			//fscanf(file_permanente, "%d", &quantidade);
			//quantidade++;
			
			fprintf(file_permanente, "%02X", tag.Code.Bytes[i]);
			
			/*->*/ //fprintf(file_permanente, "\n%s", adicionar_nova_palavra);

			printf("%d-", tag.Code.Bytes[i]);
			// fwrite(tag.Code.Bytes[i], 1, sizeof(tag.Code.Bytes[i]), file_permanente);
			// tag.Code.Bytes[i] = Linha_permanente;
		}
		fclose(file_permanente);

		uint8_t i;
		for (i = 0; i < TAG_GEN2_SIZE; i++)
			VM5F_received_tag.Bytes[TAG_GEN2_SIZE - 1 - i] = tag.Code.Bytes[i];

		tag.Code.DATA.Tid.Int = 0;
		tag.Code.DATA.TidCRC16.Int = 0;

		VM5F_Flags.DATA.CheckCmdData = 1; // signalize tag found
	}
}

/*
 *	Recebe resultado da escrita na memoria do tag
 */
void VM5F_RxCmd_WriteTag(void)
{
	printf("VM5F_CMD_ERRORCODE_SIZE: %d\nVM5F_Buffer.DIR.Rx.DATA.Size: %d\n", VM5F_CMD_ERRORCODE_SIZE, VM5F_Buffer.DIR.Rx.DATA.Size);

	if (VM5F_Buffer.DIR.Rx.DATA.Size != VM5F_CMD_ERRORCODE_SIZE) // não entra
	{

		uint8_t tag[12];

		/*	Obtem tag */
		VM5F_RxRdPtr_Shift(5); // ignora posicoes desnecessarias
		VM5F_RxPayload_To_Buff(tag, 12);

		for (uint8_t i = 0; i < 12; i++)
		{ // printf("Valor tag:%d e VM5F_received_tag.Bytes[i]:%d\n", tag[i], VM5F_received_tag.Bytes[i]);
			if (tag[i] != VM5F_received_tag.Bytes[i])
				return;
		}
		VM5F_RxRdPtr_Shift(3);
		uint8_t error_code = VM5F_RxRdPtr_Byte();
		if (error_code == VM5F_COMMAND_SUCESS)
			VM5F_Flags.DATA.CheckCmdData = 1;
	}
}

/*
 *	Recebe resultado da escrita de lixo na memoria de usuario do tag
 */
void VM5F_RxCmd_LockTag(void)
{
	printf("LockTag\n\nVM5F_CMD_ERRORCODE_SIZE: %d\nVM5F_Buffer.DIR.Rx.DATA.Size: %d\n", VM5F_CMD_ERRORCODE_SIZE, VM5F_Buffer.DIR.Rx.DATA.Size);

	if (VM5F_Buffer.DIR.Rx.DATA.Size != VM5F_CMD_ERRORCODE_SIZE)
	{
		uint8_t tag[12];

		/*	Obtem tag */
		VM5F_RxRdPtr_Shift(5); // ignora posicoes desnecessarias
		VM5F_RxPayload_To_BuffInv(tag, TAG_GEN2_SIZE);

		for (uint8_t i = 0; i < TAG_GEN2_SIZE; i++)
		{
			if (tag[i] != VM5F_received_tag.Bytes[TAG_GEN2_SIZE - 1 - i])
				return;
		}
		VM5F_RxRdPtr_Shift(3);
		uint8_t error_code = VM5F_RxRdPtr_Byte();
		if (error_code == VM5F_COMMAND_SUCESS)
			VM5F_Flags.DATA.CheckCmdData = 1;
	}
}

/*
 *	Recebe resultado da escrita de lixo na memoria de usuario do tag
 */
void VM5F_RxCmd_ReadTag(void)
{
	uint8_t tag[TAG_GEN2_SIZE];

	if (VM5F_Buffer.DIR.Rx.DATA.Size != VM5F_CMD_ERRORCODE_SIZE)
	{
		/*	Obtem tag */
		VM5F_RxRdPtr_Shift(5); // ignora posicoes desnecessarias
		VM5F_RxPayload_To_BuffInv(tag, TAG_GEN2_SIZE);

		for (uint8_t i = 0; i < TAG_GEN2_SIZE; i++)
		{
			if (tag[i] != VM5F_received_tag.Bytes[TAG_GEN2_SIZE - 1 - i])
				return;
		}

		printf("Li um tag 2 \n");
		fflush(stdout);
		// Tag_Gen2_Add(tag, TAG_GEN2_SIZE);
		VM5F_Flags.DATA.CheckCmdData = 1;
	}
}

/************************************************************************/
/*    COMANDOS PARA ENVIAR									            */
/************************************************************************/

/*
 * Comandos para resetar modulo VM5F - nao retorna nada se ok
 */
uint8_t VM5F_TxCmd_ResetMCU(void)
{
	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_RESET;
	return VM5F_TxData_Dispach();
}

/*
 * Requisita a versao de firmware
 */
uint8_t VM5F_TxCmd_GetFWversion(void)
{
	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_GET_FWVERSION;
	return VM5F_TxData_Dispach();
}

/*
 *Comando para configurar potencia de saida (em dbm)
 */
uint8_t VM5F_TxCmd_SetOutputPwr(void)
{
	uint8_t power;
	memcpy(&power, &OutPwr_dbm_Table[/*FncGrp_Setup_Char.FNC.PowerOut*/ 3], 1);

	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_SET_OUTPUTPWR;
	VM5F_TxWrPtr_Byte(power);
	return VM5F_TxData_Dispach();
}

/*
 * Requisita potencia de saida (em dbm)
 */
uint8_t VM5F_TxCmd_GetOutputPwr(void)
{
	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_GET_OUTPUTPWR;
	return VM5F_TxData_Dispach();
}

/*
 * Comando para configurar a frequencia de operacao
 * ATENCAO: Modulo para de funcionar com valores inesperados
 */
uint8_t VM5F_TxCmd_SetFrequency(void)
{
	uint8_t _freq_buf[3], qtty, spacing = VM5F_FREQSPACE_DEF;
	uint32_t _frequency;

	uint16_t freq_final = 928;	 // FncGrp_Setup_Int.FNC.EndFreq.Int;
	uint16_t freq_inicial = 915; // FncGrp_Setup_Int.FNC.InitialFreq.Int;

	/* Configura numero de canais */
	if (freq_final <= freq_inicial)
	{
		qtty = 1;
		spacing = 0;
	}
	else
		qtty = (uint8_t)((((freq_final - freq_inicial) * 100) / VM5F_FREQSPACE_DEF) + 0.5);

	if (qtty > VM5F_MAX_FREQQTTY)
		qtty = VM5F_MAX_FREQQTTY;

	/* Configura frequencia inicial */
	_frequency = (uint32_t)(/*FncGrp_Setup_Int.FNC.InitialFreq.Int*/ 915 * 1e3);

	_freq_buf[2] = (uint8_t)(_frequency & 0xFF);
	_freq_buf[1] = (uint8_t)((_frequency & 0xFF00) >> 8);
	_freq_buf[0] = (uint8_t)((_frequency & 0xFF0000) >> 16);

	/* Seta parametros do comando */
	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_SET_FREQUENCY;
	VM5F_TxWrPtr_Byte(VM5F_FREQREGION_DEF);
	VM5F_TxWrPtr_Byte(spacing);
	VM5F_TxWrPtr_Byte(qtty);
	VM5F_TxWrPtr_Buff(_freq_buf, 3);
	return VM5F_TxData_Dispach();
}

/* Comando para obter frequencia de operacao */
uint8_t VM5F_TxCmd_GetFrequency(void)
{
	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_GET_FREQUENCY;
	return VM5F_TxData_Dispach();
}

/*
 * Requisita inventarios, conforme parametro
 */
uint8_t VM5F_TxCmd_Inventory(uint8_t rounds) // recebeu o rounds
{

	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_INVENTORY;
	VM5F_TxWrPtr_Byte(rounds);
	/*VM5F_TxData_Dispach();
	VM5F_TxResetAttempts();
	VM5F_Flags.DATA.CheckErrorCode = 1; retirado*/

	return VM5F_TxData_Dispach(); // retorno do dispatch
}

/*
 * Comando para obter tags detectadas e limpar o buffer
 */
uint8_t VM5F_TxCmd_GetAndClearTagBuffer(void)
{
	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_GET_CLR_TAGBUF;
	/*VM5F_TxData_Dispach();
	VM5F_TxResetAttempts();
	VM5F_Flags.DATA.CheckErrorCode = 1; retirado*/
	return VM5F_TxData_Dispach(); // retorno do dispatch
}

/*
 *	Comando para escrever em qualquer memoria do tag
 */
uint8_t VM5F_TxCmd_WriteTag(uint8_t *password, GEN2_MEMBANK membank, uint8_t address, uint8_t *data, uint8_t size)
{
	if (size > 0 && (size & 0x01) == 0) // tamanho deve ser proporcional a WORD
	{
		uint8_t word_size = size >> 1;

		VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_WRITE_TAG;
		VM5F_TxWrPtr_Buff(password, GEN2_PASSWORD_SIZE);
		VM5F_TxWrPtr_Byte(membank);
		VM5F_TxWrPtr_Byte(address);
		VM5F_TxWrPtr_Byte(word_size);
		VM5F_TxWrPtr_Buff(data, size);

		return VM5F_TxData_Dispach();
	}
	return 1; // NOTHING TO DO
}

/*
 *	Comando para bloquear um tag
 */
uint8_t VM5F_TxCmd_LockTag(uint8_t *password, GEN2_LOCK_MEMBANK membank, GEN2_LOCKTYPES locktype)
{
	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_LOCK_TAG;
	VM5F_TxWrPtr_Buff(password, GEN2_PASSWORD_SIZE);
	VM5F_TxWrPtr_Byte(membank);
	VM5F_TxWrPtr_Byte(locktype);
	return VM5F_TxData_Dispach();
}

/*
 *	Comando para ler qualquer parte da memoria de usuario do tag
 *	Motivo: Sera utilizado para validar a utilizacao do 'access password'
 */
uint8_t VM5F_TxCmd_ReadTag(uint8_t *password)
{
	uint8_t word_add = 0, word_cnt = 1;

	VM5F_Buffer.DIR.Tx.DATA.Command = VM5F_CMD_READ_TAG;
	VM5F_TxWrPtr_Byte(MEMBANK_TID);
	VM5F_TxWrPtr_Byte(word_add);
	VM5F_TxWrPtr_Byte(word_cnt);
	VM5F_TxWrPtr_Buff(password, GEN2_PASSWORD_SIZE);
	return VM5F_TxData_Dispach();
}