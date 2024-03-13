#ifndef IO_H_
#define IO_H_

#include <stdio.h>
#include <assert.h>

typedef struct Lines
{
    char** lines_ptr;
    int lines_number;
}Lines;

int GetFileSize(FILE* text, int start);
int GetLineNumber(char* code, int code_size);
void CreateBufferOfLines(Lines* lines, char* code, int code_size);
int LinesCtor(Lines* lines, char* code, int code_size);
int LinesDtor(Lines* lines);

#endif // #define IO_H_