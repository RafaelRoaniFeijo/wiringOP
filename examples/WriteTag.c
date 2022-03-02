#include <stdio.h>
#include <stdint.h>
#include "WriteTag.h"
#include "VM5F_Ctrl.h"
#include "VM5F_Cmds.h"
#include "stdlib.h"
#include <string.h>


int i;
GEN2_WRITE_DATA write_data;

WRITE_TAG_DATA writeTagData;

void LerArquivoTag(void)
{
    if (writeTagData.flags.FLAGS.writing == 0) // se não está processando, consegue ler
    {
        file = fopen("teste.txt", "r"); // w+ read/write
        result = fgets(Linha, 18, file); // o 'fgets' lê até 99 caracteres ou até o '\n
        linha_ptr = &Linha[0];           // aponta pra primeira posição e pula de 2 em 2
                                         // linha_ptr = Linha;
        write_data.DATA.flag.DATA.lock = 0; //futuramente da pra fazer com que seja setado no arquivo
        write_data.DATA.tid = 1;


        if (result)
        { // Se foi possível ler
            int j;

            for (j = 0; j < (strlen(Linha)) / 2; j++)
            {
                sscanf(linha_ptr, "%2X", &valoresTag);

                write_data.DATA.serial[j] = valoresTag; //limitar para ser apenas 3 valores preenchidos somente no front
                
                // printf("%d\n", write_data.DATA.serial[j]);
                linha_ptr += 2;
            } // preciso que o valor seja separado de 2 em 2, juntar todos e exibir
              // e apagar a lista depois de usar
              // completar com zeros j
            for(; j < TAG_CODE_SIZE; j++) //não seta o primeiro indice pois já está sendo preenchido TAG_CODE_SIZE - os 8 valores seriais
            {
                write_data.DATA.serial[j] = 0;
            }
            writeTagData.flags.FLAGS.writeTag = 1; //seta e consegue iniciar o writeTag
        }
    }
    file = fopen("teste.txt", "w");
    fclose(file);
}
void WriteTagProc(void)
{
    switch (writeTagData.state)
    {
    case WRITE_TAG_INIT:
        if (writeTagData.flags.FLAGS.writeTag == 1) // se está liberado para ler
        {
            writeTagData.flags.FLAGS.writing = 1; // seta para não pegar leitura durante o processo
            // writeTagData.flags.value = 0; //zera todas flags
            if (VM5F_Get_FreeToWrite())
            {
                /*
                write_data.DATA.serial[0] = 1;
                write_data.DATA.serial[1] = 4;
                write_data.DATA.serial[2] = 8;
                write_data.DATA.serial[3] = 10;
                write_data.DATA.serial[4] = 0;
                write_data.DATA.serial[5] = 0;
                write_data.DATA.serial[6] = 0;
                write_data.DATA.serial[7] = 0;
                */
                for (uint8_t i = 0; i < 12; i++)
                {
                    write_data.DATA.epc_origin[i] = VM5F_received_tag.Bytes[i];
                }

                VM5F_WriteTag(&write_data);

                writeTagData.state++; // volta ao inicio
            }
        }
        break;

    case WRITE_TAG_WAIT_WRITE_INIT:
        if (!VM5F_Get_FreeToWrite())
        {
            writeTagData.state++;
        }
        break;

    case WRITE_TAG_WAIT_WRITE_END:
        if (VM5F_Get_FreeToWrite()) // verifica se esta livre p escrever
        {
            uint8_t result = VM5F_Get_LastWriteResult();
            // printf("\nresult %d\n", result);
            if (result) // Obtem resultado da ultima gravacao de tag. E limpa buffer
            {
                printf("Deu certo\n");
                writeTagData.flags.FLAGS.writeTag = 0; // zera depois de ler
                fflush(stdout);
            }
            else
            {
                printf("Deu errado\n");
                fflush(stdout);
            }

            writeTagData.flags.FLAGS.writing = 0;
            writeTagData.state = WRITE_TAG_INIT;
        }
        break;

    default:
        break;
    }
}

void WriteTagTmr(void) // x10ms
{
    if (writeTagData.tmr)
        writeTagData.tmr--;
}