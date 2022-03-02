#ifndef _WRITE_TAG_H
#define _WRITE_TAG_H
#define WAIT_INIT_TMR  500

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

FILE *file;
FILE *file_permanente;

unsigned int valoresTag;
char Linha[100];
char *linha_ptr; // ponteiro
char *result;


char adicionar_nova_palavra[100];
unsigned int quantidade;

void WriteTagProc(void);
void WriteTagTmr(void);
void LerArquivoTag(void);

typedef enum {
    WRITE_TAG_INIT,
    WRITE_TAG_WAIT_WRITE_INIT, 
    WRITE_TAG_WAIT_WRITE_END
} WRITE_TAG_STATES;

typedef union {
    uint8_t value;
    struct{
        uint8_t writeTag : 1;
        uint8_t writing : 1;
        uint8_t rfu01 : 1;//reservado futuro
    } FLAGS;
}WRITE_TAG_FLAGS;

typedef struct
{
    WRITE_TAG_STATES state;
    WRITE_TAG_FLAGS flags;
    uint16_t tmr;
} WRITE_TAG_DATA;


#endif