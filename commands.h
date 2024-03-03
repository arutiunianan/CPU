#ifndef COM_H_
#define COM_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h> 
#include <assert.h>
#include <string.h>
#include "stack/stack.h"

#define CHECK_ERROR(obj, condition, error)  \
    if(condition)                            \
        SetErrorBit(&obj->errors, error);     \
    else                                       \
        UnsetErrorBit(&obj->errors, error);

#define ERROR_PROCESSING(structure, StructDump, StructDtor, line_of_file) \
    if (structure->errors)                                                 \
    {                                                                       \
        StructDump(structure, line_of_file, stderr);                         \
        StructDtor(structure);                                                \
        return structure->errors;                                              \
    }                                                                           \
    StructDump(structure, line_of_file, structure->log);                                  

typedef enum ArgType
{
    NOARG = 0,
    IMM   = 1 << 5, 
    REG   = 1 << 6,
    LAB   = 1 << 7,
}ArgType;

typedef struct Labels
{
    char** label_name;
    int*  label_address;
    int labels_num;
}Labels;

typedef struct Lines
{
    char** lines_ptr;
    int lines_number;
}Lines;

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

typedef struct CommandWithArg
{
    Cmds cmd;
    Elem_t arg;
}CommandWithArg;

void SetErrorBit(int* error, int errorbit);
void UnsetErrorBit(int* error, int errorbit);
void SetCommandBitCode(Cmds* command_cpu_code, ArgType arg_type);
void UnsetCommandBitCode(Cmds* command_cpu_code, ArgType arg_type);
void SetCommandTypeBitCode(ArgType* old_arg_type, ArgType new_arg_type);
int GetFileSize(FILE *text, int start);
int GetLineNumber( char* code, int codeSize );

#endif // #define COM_H_