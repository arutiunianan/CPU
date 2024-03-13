#include "cpu.h"

int IsValidRegArg(Instruction* command)
{
    assert(command != NULL);

    int is_reg_found = 0;

    switch((size_t)command->arg)
    {
        #define REG_DEF(name, cpu_code, ...)   \
        case cpu_code:                          \
            is_reg_found = 1;                    \
            break;

        #include "../proc_lib/regs.h"
    }
    #undef REG_DEF
    
    if(!is_reg_found)
    {
        return 0;
    }

    return 1;
}

void SetReg(CpuLog* cpu_log, Cpu* cpu, Elem_t reg, Elem_t value)
{
    assert(cpu_log != NULL);

    switch((int)reg)
    {
        #define REG_DEF(reg_name, reg_value)     \
        case reg_name:                            \
            cpu->reg_name = value;                 \
            return;                                 \
            break;                                 

        #include "../proc_lib/regs.h"
        #undef REG_DEF
    }
    SetErrorBit(&cpu_log->errors, CPU_INVALID_REG_OR_LABEL_NAME);
}

Elem_t GetValidArgument(CpuLog* cpu_log, Cpu* cpu, ArgType arg_type, Instruction* command)
{
    assert(cpu_log != NULL);
    assert(command != NULL);

    if(arg_type == IMM || arg_type == LAB)
    {
        return command->arg;
    }
    else if(arg_type == REG)
    {
        switch((int)command->arg)
        {
            #define REG_DEF(reg_name, reg_value)       \
                case reg_value:                         \
                    return cpu->reg_name;                \

            #include "../proc_lib/regs.h"
            #undef REG_DEF
        }
    }
    else
    {
        SetErrorBit(&cpu_log->errors, CPU_INVALID_REG_OR_LABEL_NAME);
    }
}

int IsValidCommand(CpuLog* cpu_log, Instruction* command, ArgType* arg_type)
{
    assert(cpu_log != NULL);
    assert(arg_type != NULL);
    assert(command  != NULL);

    int arg_num = 0;

    if((char)command->cmd & REG && command->cmd != HLT)
    {
        *arg_type = REG;

        if(!IsValidRegArg(command))
        {
            SetErrorBit(&cpu_log->errors, CPU_INVALID_REG_OR_LABEL_NAME);
            return 0;
        }
        arg_num = 1;
    }
    else if((char)command->cmd & IMM && command->cmd != HLT)
    {
        *arg_type = IMM;
            
        if(command->cmd == POP)
        {
            SetErrorBit(&cpu_log->errors, CPU_POP_WITH_NUM);
            return 0;
        }
        arg_num = 1;
    }
    else if((char)command->cmd & LAB && command->cmd != HLT)
    {
        *arg_type = LAB;

        arg_num = 1;
    }
    else if(command->arg != 0)
    {
        SetErrorBit(&cpu_log->errors, CPU_TOO_MANY_ARGS);
        return 0;
    }

    switch(ClearInstrArgType(command->cmd, *arg_type))
    {
        #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)    \
        case cmd_name:                                               \
            return (arg_num == cmd_n_args);
        
        #include "../proc_lib/cmds.h"

        default:
            return 0;
    }
    #undef DEF_CMD
}

int CPUCtor(CpuLog* cpu_log, const char* binary_file)
{
    if(!cpu_log)
    {
        return CPU_PTR_NULL;
    }

    cpu_log->errors = NO_ERRORS;

    FILE *file = fopen(binary_file, "rb");
    if(!file)
    {
        SetErrorBit(&cpu_log->errors, CPU_COMPILED_FILE_ERROR);
    }

    cpu_log->log =  fopen("cpu/cpulog.txt","wb");
    if(!cpu_log->log)
    {
        SetErrorBit(&cpu_log->errors, CPU_LOGER_ERROR);
    }
    
    int start_of_code = 0;
    int cpu_code_size = GetFileSize(file, start_of_code);
    cpu_log->cmd_num = cpu_code_size / sizeof(Instruction);

    cpu_log->cmds = (Instruction*)calloc(sizeof(char), cpu_code_size);
    if(!cpu_log->cmds)
    {
        SetErrorBit(&cpu_log->errors, CPU_CMDS_PTR_NULL);
    }

    fread(cpu_log->cmds, sizeof(char), cpu_code_size, file);

    fclose(file);

    ERROR_PROCESSING(cpu_log, CPUDump, CPUDtor, 0);

    return 0;
}

int CPUProcess(CpuLog* cpu_log)
{
    assert(cpu_log != NULL);

    Cpu cpu = {};
    Stack stack = {};

    if(!cpu_log->cmds)
    {
        SetErrorBit(&cpu_log->errors, CPU_CMDS_PTR_NULL);
    }
    if(StackCtor(&stack))
    {
        SetErrorBit(&cpu_log->errors, CPU_BAD_STACK);
    }

    ERROR_PROCESSING(cpu_log, CPUDump, CPUDtor, 0);

    int* line_num = &cpu_log->current_line_num;
    for(*line_num = 1; *line_num < cpu_log->cmd_num + 1; (*line_num)++)
    {
        Instruction command = cpu_log->cmds[*line_num - 1];

        ArgType arg_type = NOARG;

        if(IsValidCommand(cpu_log, &command, &arg_type))
        {
            switch(ClearInstrArgType(command.cmd, arg_type))
            {
                #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) case cmd_name: cmd_code; break;
                
                #include "../proc_lib/cmds.h"
                #undef DEF_CMD
            }
        }
        else
            SetErrorBit(&cpu_log->errors, CPU_WRONG_INPUT);

        ERROR_PROCESSING(cpu_log, CPUDump, CPUDtor, *line_num);

    }

    if(StackDtor(&stack))
    {
        SetErrorBit(&cpu_log->errors, CPU_BAD_STACK);
    }
}

void CPUDump(CpuLog* cpu_log, size_t num_of_line, FILE* logger)
{
    static size_t num_of_call = 1;
    fprintf(logger, "=======================================\nCPU DUMP CALL #%zu\nLine: ", num_of_call);
    if(num_of_line == 0)
    {
        fprintf(logger, "Before processing file\n");
    }
    else
    {
        fprintf(logger, "%zu\n", num_of_line);
    }

    if(cpu_log->errors)
    {
        fprintf(logger, "-------------ERRORS------------\n");
        if(cpu_log->errors & CPU_PTR_NULL)
        {
            fprintf(logger, "CPU POINTER IS NULL\n");
            return;
        }
        if(cpu_log->errors & CPU_BAD_STACK)                 fprintf(logger, "SOME TROUBLES WITH STACK STRUCT\n");
        if(cpu_log->errors & CPU_WRONG_INPUT)               fprintf(logger, "WRONG INPUT\n");
        if(cpu_log->errors & CPU_LOGER_ERROR)               fprintf(logger, "CPU LOGER ERROR\n");
        if(cpu_log->errors & CPU_CMDS_PTR_NULL)             fprintf(logger, "CPU CS PTR NULL\n");
        if(cpu_log->errors & CPU_COMPILED_FILE_ERROR)       fprintf(logger, "CPU COMPILED FILE ERROR\n");        
        if(cpu_log->errors & CPU_INVALID_REG_OR_LABEL_NAME) fprintf(logger, "Invalid reg or label name!\n");
        if(cpu_log->errors & CPU_TOO_MANY_ARGS)             fprintf(logger, "Too MANY args in command!\n");
        if(cpu_log->errors & CPU_POP_WITH_NUM)              fprintf(logger, "You just did Pop with num, are you crazy????\n");

        fprintf(logger, "----------END_OF_ERRORS--------\n");
    }
    else
        fprintf(logger, "------------NO_ERRORS----------\n");

    if(num_of_line > 0)
    {
        for(size_t current_line = 1; current_line <= cpu_log->cmd_num; current_line++)
        {
            Instruction current_сmd_with_arg = cpu_log->cmds[current_line - 1];
            
            fprintf(logger, "(%2zu) %3d %lf" , current_line, current_сmd_with_arg.cmd, current_сmd_with_arg.arg);
            if(current_line == cpu_log->current_line_num + 1)
            {
                fprintf(logger, " <-----");
            }
            fprintf(logger, "\n");
        }
    }
    fprintf(logger, "\n");
    fprintf(logger, "=======================================\n\n");
    
    num_of_call++;
}

int CPUDtor(CpuLog* cpu_log)
{
    if(!cpu_log)
    {
        return CPU_PTR_NULL;
    }

    fclose(cpu_log->log);
    

    free(cpu_log->cmds);
    cpu_log->cmd_num = 0xB0BA;
    cpu_log->current_line_num = 0xB1BA;
}
