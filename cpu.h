#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include "commands.h"

typedef struct Cpu
{
    Stack stack;
    int current_line_num;
    //char* code;
    int codeSize;
    //int version;
    Com curCmd;
    CPU* cmds;
    int cmdNum;
    #define REG_DEF(name, ...) Elem_t name;

	#include "regs.h"

	#undef REG_DEF
}Cpu;


int ProcessingCPU( Cpu* cpu );
int CPUCtor( Cpu* cpu, const char* file );
int CPUDtor( Cpu* cpu );
int GetFileSize( FILE *text, int startOfCode );
void SetReg(Cpu* cpu, Elem_t reg, Elem_t value);

#endif // #define CPU_H_