#include <stdio.h>

#include "io.h"

FILE* g_log_file = {};
const char* g_log_file_path = "log.txt";

int LogFileCtor(FILE* log_file, const char* file_path)
{
#ifdef _DEBUG

    log_file = (fopen( file_path, "w" ));
    //fopen_s(log_file, file_path, "w");

    if (!log_file)
    {
        log_file = stdout;
        fprintf(log_file, "FILE OPENING ERROR, PRINTING TO STDOUT\n");
        return LOG_FILE_ERROR;
    }

#endif
    return 0;
}

int LogFileDtor(FILE* log_file)
{
#ifdef _DEBUG

    if (!log_file)
        return LOG_FILE_ERROR;

    fclose(log_file);

#endif
    return 0;
}

#ifdef _DEBUG

void PrintStackErrorsToFile(unsigned errors)
{
    if (errors == 0)
        fprintf(g_log_file, "-----------NO_ERRORS-----------\n");
    else
    {
        fprintf(g_log_file, "-------------ERRORS------------\n");
        if (errors & STACK_DATA_POINTER_NULL)     fprintf(g_log_file, "Stack DATA NULL\n");
        if (errors & STACK_LCANARY_DMG)           fprintf(g_log_file, "Stack Left Canary DAMAGED\n");
        if (errors & STACK_RCANARY_DMG)           fprintf(g_log_file, "Stack Right Canary DAMAGED\n");
        if (errors & STACK_SIZE_GREATER_CAPACITY) fprintf(g_log_file, "Stack Size GREATER Capacity\n");
        if (errors & STACK_SIZE_LESS_ONE)         fprintf(g_log_file, "Stack Size LESS One\n");
        if (errors & STACK_DATA_LCANARY_DMG)      fprintf(g_log_file, "Stack Data Left Canary DAMAGED\n");
        if (errors & STACK_DATA_RCANARY_DMG)      fprintf(g_log_file, "Stack Data Right Canary DAMAGED\n");
        if (errors & STACK_HASH_MISMATCH)         fprintf(g_log_file, "Stack Hash DIFFERENCE\n");
        if (errors & STACK_REALLOC_ERROR)         fprintf(g_log_file, "Stack REALLOC ERROR\n");
        if (errors & STACK_BAD_SIZE)              fprintf(g_log_file, "Stack Size TOO BIG\n");
        if (errors & STACK_BAD_CAPACITY)          fprintf(g_log_file, "Stack Capacity TOO BIG\n");
        fprintf(g_log_file, "----------END_OF_ERRORS--------\n");
    }
}

void StackDumpToFile(Stack* stk, const char* file_name, const size_t line, const char* func_name)
{
    static size_t num_dump_call = 1;

    fprintf(g_log_file, 
    "=======================================\n"
    "STACK DUMP CALL #%zu\n"
    "Called from: %s(%zu)\n", num_dump_call++, func_name, line);
    if (!stk)
    {
        fprintf(g_log_file,
            "Stack Pointer is NULL. No data in Stack\n"
            "=======================================\n\n");
        return;
    }
    PrintStackErrorsToFile(stk->errors);

    fprintf(g_log_file, "Stack [%p], name: \"%s\" from: %s(%zu)\n",
        stk,
        stk->stack_creation_inf.var_name,
        stk->stack_creation_inf.func,
        stk->stack_creation_inf.line);
    fprintf(g_log_file, 
    "{\n"
    "size = %zu\n"
    "capacity = %zu\n", stk->size, stk->capacity);
    if (!(stk->errors & STACK_DATA_POINTER_NULL))
    {
        fprintf(g_log_file, 
        "data[%p]\n"
        "  {\n", stk->data);
        for (size_t i = 0; i < stk->capacity; i++)
        {
            PRINT_DATA_TO_FILE(g_log_file, i, stk->data[i]);
            if (stk->data[i] == POISON_ELEM)
                fprintf(g_log_file, " ( POISON )");
            fprintf(g_log_file, "\n");
        }
        fprintf(g_log_file, 
        "  }\n");
    }
    fprintf(g_log_file, 
    "}\n"
    "=======================================\n\n");
}

#endif
void PrintStackElems(Stack* stk)
{
	for (size_t i = 0; i < stk->size; i++)
	{
		PRINT_DATA(i, stk->data[i]);
		printf("\n");
	}
}