#ifndef DASM_H_
#define DASM_H_

#include "commands.h"
#include "asm.h"

typedef struct Dasm
{
    int cmdNum;
    int codeSize;
    CPU* cmds;
    Com curCmd;
    int current_line_num;
}Dasm;

#endif // #define DASM_H_