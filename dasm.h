#ifndef DASM_H_
#define DASM_H_

#include "commands.h"
#include "asm.h"

typedef struct Dasm
{
    /*asm
    char* code;
    int codeSize;
    //int version;
    //Lines* lines;
    char** linestr;
    int lineNumber;
    Com curCmd;
    CPU* cmds;
    int cmdNum;*/

    int cmdNum;
    int codeSize;
    CPU* cmds;
    Com curCmd;
    int current_line_num;


    /*cpu
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
	#undef REG_DEF*/
}Dasm;

#endif // #define DASM_H_