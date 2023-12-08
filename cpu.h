#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include "stack/stack.h"

enum argType
{
    IMM = 1 << 5, 
	REG = 1 << 6,
};

typedef int Command;

typedef enum Regs
{
    ra = 1,
    rb = 2,
    rc = 3,
    rd = 4,
    rx = 5,
    error = 0,
}Regs;

typedef enum CPUCommand 
{
	#define DEF_CMD(name, cpu_code, ...) name = cpu_code,

	#include "commands.h"

	#undef DEF_CMD
}CPUCommand;

typedef struct CPU
{
    CPUCommand cmd;
    Elem_t arg;
}CPU;

typedef struct Com
{
	char* cmdCode;
	char* cmdArg;
    CPU CPUcmdarg;
    //int CPUcmd;
    //int CPUarg;
    int argNum;
	argType cmdArgType;
}Com;

enum Commands
{
    #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) \
        CMD_ ## cmd_name = (cmd_num),

    //#define DEF_JMP(jmp_name, jmp_num, jmp_sign)                 \
    //    JMP_ ## jmp_name = (jmp_num),

    #include "commands.h"

    #undef DEF_CMD
    //#undef DEF_JMP

};


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
}Cpu;


int ProcessingCPU( Cpu* cpu );
int CPUCtor( Cpu* cpu, const char* file );
int CPUDtor( Cpu* cpu );
int GetFileSize( FILE *text, int startOfCode );

#endif // #define CPU_H_