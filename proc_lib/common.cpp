#include "common.h"

void SetErrorBit(int* error, int errorbit)
{
    assert(error != NULL);
    
    *error |= errorbit;
}

void UnsetErrorBit(int* error, int errorbit)
{
    assert(error != NULL);

    *error &= ~(errorbit);
}

Cmds ClearInstrArgType(Cmds command_cpu_code, ArgType arg_type)
{
    assert(arg_type >= 0);

    return (Cmds)(command_cpu_code & ~arg_type);
}
