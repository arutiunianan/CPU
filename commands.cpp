#include "commands.h"

void SetCommandBitCode(CPUCommand* command_cpu_code, argType arg_type)
{
	*(char*)command_cpu_code |= (char)arg_type;
}

void UnsetCommandBitCode(CPUCommand* command_cpu_code, argType arg_type)
{
	//assert(command_cpu_code != NULL);

	*(char*)command_cpu_code &= ~(char)arg_type;
}

void SetCommandTypeBitCode(argType* old_arg_type, argType new_arg_type)
{
	//assert(old_arg_type != NULL);

	*(char*)old_arg_type |= (char)new_arg_type;
}

int GetFileSize(FILE *text, int start)
{
    fseek( text, 0, SEEK_END );
    int fileSize = ftell( text ) - start;
    fseek( text, 0, start );
    return fileSize;
}

int GetLineNumber( char* code, int codeSize )
{
    int lineNumber = 1;
    for( size_t i = 0; i < codeSize; i++ )
    {
        if ( code[i] == '\n' )
            lineNumber++;

    }
    return lineNumber;

}