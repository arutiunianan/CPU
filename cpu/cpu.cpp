#include "cpu.h"

int IsValidRegArg(CommandWithArg* command)
{
	assert(command != NULL);

	int is_reg_found = 0;

	switch((size_t)command->arg)
	{
		#define REG_DEF(name, cpu_code, ...)   \
		case cpu_code:                          \
			is_reg_found = 1;                    \
			break;

		#include "../regs.h"
	}
	#undef REG_DEF
	
	if(!is_reg_found)
	{
		return 0;
	}

	return 1;
}

void SetReg(CpuFile* cpu_file, Elem_t reg, Elem_t value)
{
	assert(cpu_file != NULL);

	switch((int)reg)
	{
		#define REG_DEF(reg_name, reg_value)     \
		case reg_name:						      \
			cpu_file->cpu.reg_name = value;	       \
            return;                                 \
			break;							     

		#include "../regs.h"
		#undef REG_DEF
	}
    SetErrorBit(&cpu_file->errors, CPU_INVALID_REG_OR_LABEL_NAME);
}

Elem_t GetProperArgument(CpuFile* cpu_file, ArgType arg_type, CommandWithArg* command)
{
	assert(cpu_file != NULL);
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
		        case reg_value:			       		    \
		        return cpu_file->cpu.reg_name;		     \

		    #include "../regs.h"
		    #undef REG_DEF
	    }
	}
    else
	{
	    SetErrorBit(&cpu_file->errors, CPU_INVALID_REG_OR_LABEL_NAME);
	}
}

int IsValidCommand(CpuFile* cpu_file, CommandWithArg* command, ArgType* arg_type)
{
	assert(cpu_file != NULL);
	assert(arg_type != NULL);

	if(!command)
	{
		return 0;
	}

	int arg_num = 0;
    if((char)command->cmd & (IMM | REG | LAB) && command->cmd != HLT)
    {
        if((char)command->cmd & REG)
		{
			UnsetCommandBitCode(&command->cmd, REG);
			*arg_type = REG;

			if(!IsValidRegArg(command))
            {
				SetErrorBit(&cpu_file->errors, CPU_INVALID_REG_OR_LABEL_NAME);
                return 0;
            }
			arg_num = 1;
		}
		else if((char)command->cmd & IMM)
		{
			UnsetCommandBitCode(&command->cmd, IMM);
			*arg_type = IMM;
            
            if(command->cmd == POP)
            {
                SetErrorBit(&cpu_file->errors, CPU_POP_WITH_NUM);
                return 0;
            }
			arg_num = 1;
        }
        else if((char)command->cmd & LAB)
		{
			UnsetCommandBitCode(&command->cmd, LAB);
			*arg_type = LAB;

			arg_num = 1;
        }
    }
	else if(command->arg != 0)
	{
		SetErrorBit(&cpu_file->errors, CPU_TOO_MANY_ARGS);
	}
	else
	{
		arg_num = 0;
    }

    switch(command->cmd)
	{
		#define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)	\
		case cmd_name:												 \
			return (arg_num == cmd_n_args);
		
		#include "../cmds.h"

		default:
			return 0;
	}
	#undef DEF_CMD
}

int CPUCtor(CpuFile* cpu_file, const char* binary_file)
{
    if(!cpu_file)
	{
		return CPU_PTR_NULL;
	}

    cpu_file->errors = NO_ERRORS;

    FILE *file = fopen(binary_file, "rb");
    if(!file)
	{
		SetErrorBit(&cpu_file->errors, CPU_COMPILED_FILE_ERROR);
	}

    cpu_file->log =  fopen("cpu/cpulog.txt","wb");
    if(!cpu_file->log)
	{
		SetErrorBit(&cpu_file->errors, CPU_LOGER_ERROR);
	}
    
    int start_of_code = 0;
    int cpu_code_size = GetFileSize(file, start_of_code);
    cpu_file->cmd_num = cpu_code_size / sizeof(CommandWithArg);

    cpu_file->cmds = (CommandWithArg*)calloc(sizeof(char), cpu_code_size);
    if(!cpu_file->cmds)
	{
		SetErrorBit(&cpu_file->errors, CPU_CMDS_PTR_NULL);
	}

    fread(cpu_file->cmds, sizeof(char), cpu_code_size, file);

    fclose(file);

    if(StackCtor(&cpu_file->stack))
	{
		SetErrorBit(&cpu_file->errors, CPU_BAD_STACK);
	}

	ERROR_PROCESSING(cpu_file, CPUDump, CPUDtor, 0);

    return 0;
}

int CPUProcess(CpuFile* cpu_file)
{
    assert(cpu_file != NULL);

    if(!cpu_file->cmds)
	{
		SetErrorBit(&cpu_file->errors, CPU_CMDS_PTR_NULL);
	}

	ERROR_PROCESSING(cpu_file, CPUDump, CPUDtor, 0);

    int* line_num = &cpu_file->current_line_num;
    CommandWithArg* command = (CommandWithArg*)calloc(1, sizeof(CommandWithArg));
    for(*line_num = 1; *line_num < cpu_file->cmd_num + 1; (*line_num)++)
	{
        command->cmd = *(Cmds*) ((char*)cpu_file->cmds + (*line_num - 1) * sizeof(CommandWithArg));
		command->arg  = *(Elem_t*) ((char*)cpu_file->cmds + (*line_num - 1) * sizeof(CommandWithArg) + 8);

		ArgType arg_type = NOARG;
        if(IsValidCommand(cpu_file, command, &arg_type))
        {
            switch(command->cmd)
			{
				#define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) case cmd_name: cmd_code; break;
				
				#include "../cmds.h"
				#undef DEF_CMD
			}
        }
        else
			SetErrorBit(&cpu_file->errors, CPU_WRONG_INPUT);

		ERROR_PROCESSING(cpu_file, CPUDump, CPUDtor, *line_num);

    }
}

void CPUDump(CpuFile* cpu_file, size_t num_of_line, FILE* logger)
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

	if(cpu_file->errors)
	{
		fprintf(logger, "-------------ERRORS------------\n");
		if(cpu_file->errors & CPU_PTR_NULL)
		{
			fprintf(logger, "CPU POINTER IS NULL\n");
			return;
		}
		if(cpu_file->errors & CPU_BAD_STACK)                 fprintf(logger, "SOME TROUBLES WITH STACK STRUCT\n");
		if(cpu_file->errors & CPU_WRONG_INPUT)               fprintf(logger, "WRONG INPUT\n");
		if(cpu_file->errors & CPU_LOGER_ERROR)               fprintf(logger, "CPU LOGER ERROR\n");
		if(cpu_file->errors & CPU_CMDS_PTR_NULL)             fprintf(logger, "CPU CS PTR NULL\n");
		if(cpu_file->errors & CPU_COMPILED_FILE_ERROR)       fprintf(logger, "CPU COMPILED FILE ERROR\n");		
        if(cpu_file->errors & CPU_INVALID_REG_OR_LABEL_NAME) fprintf(logger, "Invalid reg or label name!\n");
	    if(cpu_file->errors & CPU_TOO_MANY_ARGS)             fprintf(logger, "Too MANY args in command!\n");
	    if(cpu_file->errors & CPU_POP_WITH_NUM)              fprintf(logger, "You just did Pop with num, are you crazy????\n");

		fprintf(logger, "----------END_OF_ERRORS--------\n");
	}
	else
		fprintf(logger, "------------NO_ERRORS----------\n");

	if(num_of_line > 0)
	{
		for(size_t current_line = 1; current_line <= cpu_file->cmd_num; current_line++)
		{
			Cmds current_cmd_cpu_code = *(Cmds*) ((char*)cpu_file->cmds + (current_line - 1) * sizeof(CommandWithArg));
			Elem_t current_cmd_cpu_arg_code = *(Elem_t*) ((char*)cpu_file->cmds + (current_line - 1) * sizeof(CommandWithArg) + 8);

			fprintf(logger, "(%2zu) %3d %lf" , current_line, current_cmd_cpu_code, current_cmd_cpu_arg_code);
			if(current_line == cpu_file->current_line_num + 1)
			{
				fprintf(logger, " <-----");
			}
			fprintf(logger, "\n");
		}
	}
	#define REG_DEF(name, code, ...) fprintf(logger, "%s:%lf"  ", " , #name, cpu_file->cpu.name);
	
	#include "../regs.h"

	#undef REG_DEF
	fprintf(logger, "\n");
	fprintf(logger, "=======================================\n\n");
	
	num_of_call++;
}


int CPUDtor(CpuFile* cpu_file)
{
    if(!cpu_file)
	{
		return CPU_PTR_NULL;
	}

    fclose(cpu_file->log);
    if(StackDtor(&cpu_file->stack))
	{
		SetErrorBit(&cpu_file->errors, CPU_BAD_STACK);
	}

    free(cpu_file->cmds);
    cpu_file->cmd_num = 0xB0BA;
    cpu_file->current_line_num = 0xB1BA;
}

int main(int argc, const char* argv[])
{
    CpuFile cpu_file = {};

    if(argc == 1)
    {
        CPUCtor(&cpu_file, "ass.txt");
    }
    else if(argc == 2)
    {
        CPUCtor(&cpu_file, argv[1]);
    }
    else
	{
        printf("Invalid number of args to program");
		return 1;
	}
	CPUProcess(&cpu_file);
    CPUDtor(&cpu_file);
	return 0;
}