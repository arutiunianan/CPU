#include "io.h"

int GetFileSize(FILE* text, int start)
{
    assert(text != NULL);

    fseek(text, 0, SEEK_END);
    int fileSize = ftell(text) - start;
    fseek(text, 0, start);
    return fileSize;
}

int GetLineNumber(char* code, int code_size)
{
    assert( code != NULL );

    int lineNumber = 1;
    for(size_t i = 0; i < code_size; i++)
    {
        if (code[i] == '\n')
        {
            lineNumber++;
        }
    }
    return lineNumber;
}

void CreateBufferOfLines(Lines* lines, char* code, int code_size)
{
    assert(lines != NULL);
    assert(code  != NULL);

    int line = 1;
    lines->lines_ptr[0] = code;
    for(int i = 0; i < code_size - 1; i++)
    {
        if(code[i] == '\n')
        {
            lines->lines_ptr[line++] = &code[i+1];

            code[i] = '\0';
        }
    }
}

int LinesCtor(Lines* lines, char* code, int code_size)
{
    assert(lines != NULL);
    assert(code  != NULL);

    lines->lines_number = GetLineNumber(code, code_size);
    lines->lines_ptr = (char**)calloc(lines->lines_number, sizeof(char*));
    if(!lines->lines_ptr)
    {
        return 1;
    }

    CreateBufferOfLines(lines, code, code_size);
    return 0;
}

int LinesDtor(Lines* lines)
{
    if(lines == NULL && lines->lines_ptr == NULL)
    {
        return 0;
    }
    free(lines->lines_ptr);
    lines->lines_number = 0xB1BA;
    free(lines);
    return 1;
}