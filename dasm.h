#ifndef DASM_H_
#define DASM_H_

#include "commands.h"
//#include "asm.h"

typedef enum DASMErrors
{
    NO_ERRORS = 0,
	DASM_PTR_NULL = 1 << 1,
	DASM_BAD_TEXT_INFO = 1 << 2,
	INVALID_DASM_COMMAND = 1 << 3,
	DASM_LOGER_ERROR = 1 << 4,
    DASM_CMDS_PTR_NULL = 1 << 5,
	DASM_COMPILED_FILE_ERROR = 1 << 6,
    DASM_TOO_MANY_ARGS = 1 << 7,
	DASM_INVALID_REG_OR_LABEL_NAME = 1 << 8,
	DASM_POP_WITH_NUM = 1 << 9
}DASMErrors;

typedef struct Dasm
{
    FILE* log;
    int cmd_num;
    int code_size;
    CPU* cmds;
    Com cur_cmd;
    int current_line_num;
    int errors;
}Dasm;

int DASMDtor( Dasm* dasm );
void DASMDump(Dasm* dasm, size_t line_num, FILE* logger);
int ProcessingDASM( Dasm* dasm, const char* file_name );
int DASMCtor( Dasm* dasm, const char* file_name );

#endif // #define DASM_H_