#include "commands.h"

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

void SetCommandBitCode(Cmds* command_cpu_code, ArgType arg_type)
{
    assert(command_cpu_code != NULL);

    *(char*)command_cpu_code |= (char)arg_type;
}

void UnsetCommandBitCode(Cmds* command_cpu_code, ArgType arg_type)
{
    assert(command_cpu_code != NULL);

    *(char*)command_cpu_code &= ~(char)arg_type;
}

void SetCommandTypeBitCode(ArgType* old_arg_type, ArgType new_arg_type)
{
    assert(old_arg_type != NULL);

    *(char*)old_arg_type |= (char)new_arg_type;
}

int GetFileSize(FILE* text, int start)
{
    assert(text != NULL);

    fseek(text, 0, SEEK_END);
    int fileSize = ftell(text) - start;
    fseek(text, 0, start);
    return fileSize;
}

int GetLineNumber(char* code, int codeSize)
{
    assert( code != NULL );

    int lineNumber = 1;
    for(size_t i = 0; i < codeSize; i++)
    {
        if (code[i] == '\n')
        {
            lineNumber++;
        }
    }
    return lineNumber;
}