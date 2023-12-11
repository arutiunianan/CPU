#ifndef CPU_H_
#define CPU_H_

#include "commands.h"

typedef enum CPUErrors
{
    NO_ERRORS = 0,
	CPU_PTR_NULL = 1 << 0,
	CPU_BAD_STACK = 1 << 1,
	CPU_WRONG_INPUT = 1 << 2,
	CPU_LOGER_ERROR = 1 << 3,
	CPU_CMDS_PTR_NULL = 1 << 4,
	CPU_COMPILED_FILE_ERROR = 1 << 5,
    CPU_TOO_MANY_ARGS = 1 << 6,
	CPU_INVALID_REG_OR_LABEL_NAME = 1 << 7,
	CPU_POP_WITH_NUM = 1 << 8
}CPUErrors;

typedef struct Cpu
{
    FILE* log;
    Stack stack;
    int current_line_num;
    int code_size;
    Com cur_cmd;
    CPU* cmds;
    int cmd_num;
    int errors;
    #define REG_DEF(name, ...) Elem_t name;

	#include "regs.h"

	#undef REG_DEF
}Cpu;


int ProcessingCPU( Cpu* cpu );
int CPUCtor( Cpu* cpu, const char* file );
int CPUDtor( Cpu* cpu );
int GetFileSize( FILE *text, int startOfCode );
void SetReg(Cpu* cpu, Elem_t reg, Elem_t value);
void CPUDump(Cpu* cpu, size_t num_of_line, FILE* logger);

#endif // #define CPU_H_