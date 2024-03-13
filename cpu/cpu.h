#ifndef CPU_H_
#define CPU_H_

#include "../proc_lib/common.h"


// C++ 20 / C++ 23
// import ModuleName.SomeClass 
// export

typedef enum CPUErrors
{
    NO_ERRORS                     = 0,
    CPU_PTR_NULL                  = 1 << 0,
    CPU_BAD_STACK                 = 1 << 1,
    CPU_WRONG_INPUT               = 1 << 2,
    CPU_LOGER_ERROR               = 1 << 3,
    CPU_CMDS_PTR_NULL             = 1 << 4,
    CPU_COMPILED_FILE_ERROR       = 1 << 5,
    CPU_TOO_MANY_ARGS             = 1 << 6,
    CPU_INVALID_REG_OR_LABEL_NAME = 1 << 7,
    CPU_POP_WITH_NUM              = 1 << 8
}CPUErrors;

typedef struct Cpu
{
    #define REG_DEF(name, ...) Elem_t name;

    #include "../proc_lib/regs.h"
    #undef REG_DEF

}Cpu;

typedef struct CpuLog
{
    int current_line_num;
    Instruction* cmds;
    int cmd_num;

    FILE* log;
    int errors;
}CpuLog;


int CPUExecute(CpuLog* cpu_log);
int CPUCtor(CpuLog* cpu_log, const char* file);
int CPUDtor(CpuLog* cpu_log);
void SetReg(CpuLog* cpu_log, Elem_t reg, Elem_t value);
void CPUDump(CpuLog* cpu_log, size_t num_of_line, FILE* logger);

#endif // #define CPU_H_