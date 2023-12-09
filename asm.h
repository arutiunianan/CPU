#ifndef ASM_H_
#define ASM_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h> 
#include <string.h>
#include "cpu.h"

typedef struct Lines
{
    char* str;
    int length;

}Lines;

typedef struct Asm
{
    char* code;
    int codeSize;
    //int version;
    //Lines* lines;
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
enum Regs RegToNum( char c );

#endif // #define ASM_H_