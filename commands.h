#ifndef COM_H_
#define COM_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h> 
#include <assert.h>
#include <string.h>
#include "stack/stack.h"

enum argType
{
    NOARG = 0,
    IMM = 1 << 5, 
	REG = 1 << 6,
    LAB = 1 << 7,
};

struct Label 
{
	char* label_name;
	int  label_address;
};

typedef struct Lines
{
    char* str;
    int length;

}Lines;

typedef enum Regs
{
    ra = 1,
    rb = 2,
    rc = 3,
    rd = 4,
    rxf = 5,
    rxs = 6,
    rr = 7,
    error = 0,
}Regs;

typedef enum CPUCommand 
{
	#define DEF_CMD(name, cpu_code, ...) name = cpu_code,

	#include "cmds.h"

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

    #include "cmds.h"

    #undef DEF_CMD
    //#undef DEF_JMP

};

void SetCommandBitCode(CPUCommand* command_cpu_code, argType arg_type);
void UnsetCommandBitCode(CPUCommand* command_cpu_code, argType arg_type);
void SetCommandTypeBitCode(argType* old_arg_type, argType new_arg_type);
int GetFileSize(FILE *text, int start);
int GetLineNumber( char* code, int codeSize );

#endif // #define COM_H_