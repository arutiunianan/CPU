#ifndef ISA_H_
#define ISA_H_

#include "stack/stack.h"

typedef enum Regs 
{
    #define REG_DEF(name, reg_code) name = reg_code,

    #include "regs.h"
    #undef REG_DEF
}Regs;

typedef enum Cmds 
{
    #define DEF_CMD(name, cpu_code, ...) name = cpu_code,

    #include "cmds.h"
    #undef DEF_CMD
}Cmds;

typedef struct Instruction
{
    Cmds cmd;
    Elem_t arg;
}Instruction;

#endif // #define ISA_H_