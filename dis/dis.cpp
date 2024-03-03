#include "dis.h"

char* GetReg(DisFile* dis_file, Elem_t reg)
{
	assert(dis_file != NULL);

	switch((int)reg)
	{
		#define REG_DEF(reg_name, reg_value)  \
		case reg_value:						   \
			return #reg_name;				    \
			break;							     

		#include "../regs.h"
		#undef REG_DEF
	}
    SetErrorBit(&dis_file->errors, DIS_INVALID_REG_OR_LABEL_NAME);
}

int IsValidRegArg(CommandWithArg* command)
{
	assert(command != NULL);

	int is_reg_found = 0;

	switch((size_t)command->arg)
	{
		#define REG_DEF(name, cpu_code, ...)  \
		case cpu_code:                         \
			is_reg_found = 1;                   \
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

int IsValidCommand(DisFile* dis_file, CommandWithArg* command, ArgType* arg_type)
{
	assert(dis_file != NULL);
    assert(command != NULL);
	assert(arg_type != NULL);

	int arg_num = 0;
    if((char)command->cmd & (IMM | REG | LAB) && command->cmd != HLT)
    {
		if((char)command->cmd & REG)
		{
			UnsetCommandBitCode(&command->cmd, REG);

			*arg_type = REG;

			if(!IsValidRegArg(command))
			{
				SetErrorBit(&dis_file->errors, DIS_INVALID_REG_OR_LABEL_NAME);
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
                SetErrorBit(&dis_file->errors, DIS_POP_WITH_NUM);
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
		SetErrorBit(&dis_file->errors, DIS_TOO_MANY_ARGS);
	}
	else
    {
		arg_num = 0;
    }

    switch(command->cmd)
	{
		#define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)  \
		case cmd_name:											   \
			return (arg_num == cmd_n_args);
		
		#include "../cmds.h"

		default:
			return 0;
	}
	#undef DEF_CMD

}

int DISCtor(DisFile* dis_file, const char* file_name)
{	
    if(!dis_file)
	{
		return DIS_PTR_NULL;
	}

    dis_file->errors = NO_ERRORS;

    FILE *file = fopen(file_name, "rb");
    if(!file)
	{
		SetErrorBit(&dis_file->errors, DIS_COMPILED_FILE_ERROR);
	}

    dis_file->log =  fopen("dis/dislog.txt","wb");
    if(!dis_file->log)
	{
		SetErrorBit(&dis_file->errors, DIS_LOGER_ERROR);
	}
    
    int start_of_code = 0;
    int dis_code_size = GetFileSize(file, start_of_code);
    dis_file->cmd_num = dis_code_size / sizeof(CommandWithArg);

    dis_file->cmds = (CommandWithArg*)calloc(sizeof(char), dis_code_size);
    if(!dis_file->cmds)
	{
		SetErrorBit(&dis_file->errors, DIS_CMDS_PTR_NULL);
	}

    fread(dis_file->cmds, sizeof(char), dis_code_size, file);

    fclose(file);

    ERROR_PROCESSING(dis_file, DISDump, DISDtor, 0);
}

int DISProcess(DisFile* dis_file, const char* file_name)
{
    assert(dis_file != NULL);
	assert(file_name != NULL);

    if(!dis_file->cmds)
	{
		SetErrorBit(&dis_file->errors, DIS_CMDS_PTR_NULL);
	}

	ERROR_PROCESSING(dis_file, DISDump, DISDtor, 0);

    int* line_num = &dis_file->current_line_num;
    CommandWithArg* command = (CommandWithArg*)calloc(1, sizeof(CommandWithArg));
    FILE *file = fopen(file_name, "w");

    for(*line_num = 1; *line_num < dis_file->cmd_num + 1; (*line_num)++)
	{
        command->cmd = *(Cmds*) ((char*)dis_file->cmds + (*line_num-1) * sizeof(CommandWithArg));
		command->arg  = *(Elem_t*) ((char*)dis_file->cmds + (*line_num-1) * sizeof(CommandWithArg) + 8);

		ArgType arg_type = NOARG;
        if(IsValidCommand(dis_file, command, &arg_type))
        {
            switch(command->cmd)
			{
				#define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)                          \
                    case cmd_name:                                                                 \
                        if(arg_type == IMM)                                                         \
						{																		     \
                            fprintf(file, "%s %lf\n", #cmd_name, command->arg);                       \
						}																			   \
                        else if(arg_type == REG)                                                        \
						{																				 \
                            fprintf(file, "%s %s\n", #cmd_name, GetReg(dis_file, command->arg));          \
						}																				   \
                        else if(arg_type == LAB)                                                            \
						{																					 \
                            fprintf(file, "%s %lf\n", #cmd_name, command->arg);                          	  \
						}																					   \
                        else                                                                                    \
						{																						 \
                            fprintf(file, "%s\n", #cmd_name);                                              		  \
						}																						   \
                        break;
				
				#include "../cmds.h"
				#undef DEF_CMD
			}
        }
        else
			SetErrorBit(&dis_file->errors, INVALID_DIS_COMMAND);

        ERROR_PROCESSING(dis_file, DISDump, DISDtor, *line_num);
    }
}

void DISDump(DisFile* dis_file, size_t line_num, FILE* logger)
{
	assert(dis_file != NULL);
	assert(logger != NULL);

	static size_t num_of_call = 1;

	fprintf(logger, 
		"=======================================\n"
		"DIS DUMP CALL #%zu\n"
		"Line: ", num_of_call);
	if(line_num == 0)
	{
		fprintf(logger, "Before processing file\n");
	}
	else
	{
		fprintf(logger, "%zu\n", line_num);
	}

	if(dis_file->errors)
	{
		fprintf(logger, "-------------ERRORS------------\n");
		if(dis_file->errors & DIS_PTR_NULL)
		{
			fprintf(logger, "DIS POINTER IS NULL\n");
			return;
		}
		if(dis_file->errors & DIS_BAD_TEXT_INFO) 	   	    fprintf(logger, "SOME TROUBLES WITH TEXT INFO STRUCT\n");
		if(dis_file->errors & DIS_LOGER_ERROR) 		   	    fprintf(logger, "DIS LOGGER ERROR\n");
	    if(dis_file->errors & DIS_COMPILED_FILE_ERROR) 	    fprintf(logger, "COMPILED FILE ERROR\n");
	    if(dis_file->errors & DIS_INVALID_REG_OR_LABEL_NAME) fprintf(logger, "Invalid reg or label name!\n");
	    if(dis_file->errors & DIS_TOO_MANY_ARGS) 		    fprintf(logger, "Too MANY args in command!\n");
	    if(dis_file->errors & DIS_POP_WITH_NUM) 			    fprintf(logger, "You just did Pop with num, are you crazy????\n");

		fprintf(logger, "----------END_OF_ERRORS--------\n");
	}
	else
		fprintf(logger, "------------NO_ERRORS----------\n");
	fprintf(logger, "=======================================\n\n");
	num_of_call++;

}

int DISDtor(DisFile* dis_file)
{
    if(!dis_file)
	{
		return DIS_PTR_NULL;
	}

    fclose(dis_file->log);

    free(dis_file->cmds);
    dis_file->cmd_num = 0xB0BA;
    dis_file->current_line_num = 0xB1BA;
}

int main(int argc, const char* argv[])
{
    DisFile dis_file = {};

    if(argc == 1)
    {
        DISCtor(&dis_file, "ass.txt");
        DISProcess(&dis_file, "dis/dis.txt");
    }
    else if(argc == 3)
    {
        DISCtor(&dis_file, argv[1]);
        DISProcess(&dis_file, argv[2]);
    }
    else
    {
        printf("Invalid number of args to program");
		return 1;
    }
	DISDtor(&dis_file);
	return 0;
}