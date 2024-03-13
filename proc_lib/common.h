#ifndef COM_H_
#define COM_H_

#include <stdlib.h>
#include <ctype.h>
#include <math.h> 
#include <string.h>
#include "isa.h"
#include "io.h"
#include "label.h"

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

void SetErrorBit(int* error, int errorbit);
void UnsetErrorBit(int* error, int errorbit);
Cmds ClearInstrArgType(Cmds command_cpu_code, ArgType arg_type);

#endif // #define COM_H_