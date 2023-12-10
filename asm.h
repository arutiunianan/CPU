#ifndef ASM_H_
#define ASM_H_

#include "cpu.h"
#include "commands.h"

typedef struct Asm
{
    char* code;
    int codeSize;
    Label* labels;
    int labels_num;
    char** linestr;
    int lineNumber;
    Com curCmd;
    CPU* cmds;
    int cmdNum;
}Asm;

int ASMCtor( Asm* ass, const char* equation );
int ASMDtor( Asm* ass );
int ProcessingASM( Asm* ass, const char* equation );

int ReadLine( Asm* ass, char* curStr, Com* command );
int StrToNum(  Com* command );

#endif // #define ASM_H_