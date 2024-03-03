#include "asm.h"

static void CreateBufferOfLines(AsmFile* ass_file, char* asm_code, int asm_code_size)
{
    assert(ass_file != NULL);
    assert(asm_code != NULL);

    int line = 1;
    ass_file->lines.lines_ptr[0] = asm_code;
    for(int i = 0; i < asm_code_size - 1; i++)
    {
        if(asm_code[i] == '\n')
        {
            ass_file->lines.lines_ptr[line++] = &(asm_code[i+1]);

            asm_code[i] = '\0';
        }
    }
}

int IsLabel(AsmFile* ass_file, CommandWithArg* command, char* label)
{
    assert(ass_file != NULL);
    assert(command != NULL);
    assert(label != NULL);

    strcat(label, ":");
    for(size_t i = 0; i < ass_file->labels.labels_num; i++)
    {        
        if(strcmp(label, ass_file->labels.label_name[i]) == 0)
        {
            command->arg = (double)ass_file->labels.label_address[i];
            return 1;
        }
    }
    return 0;
}

int IsReg(CommandWithArg* command, char* reg)
{
    assert(command != NULL);
    assert(reg != NULL);

    #define REG_DEF(reg_name, reg_cpu_code)     \
        if(strcmp(#reg_name, reg) == 0)          \
        {                                         \
            command->arg = reg_cpu_code;           \
            return 1;                               \
        }                                    
    #include "../regs.h"
    #undef REG_DEF
    return 0;
}

int IsCommand(CommandWithArg* command, char* cmd_name, int* arg_num)
{
    assert(command != NULL);
    assert(cmd_name != NULL);
    assert(arg_num != NULL);

    #define DEF_CMD(name, cpu_code, args_num, ...)       \
        if(strcmp(cmd_name, #name) == 0)                  \
        {                                                  \
            command->cmd = (Cmds)cpu_code;                  \
            *arg_num = args_num;                             \
            return 1;                                         \
        }
            
    #include "../cmds.h"
    #undef DEF_CMD
    return 0;
}

void def(AsmFile* ass_file, CommandWithArg* command, char* cmd_name, ArgType arg_type)
{
    assert(ass_file != NULL);
    assert(command != NULL);
    assert(cmd_name != NULL);

    int arg_num = 0;
    if(IsCommand(command, cmd_name, &arg_num))
    {
        SetCommandBitCode(&command->cmd, arg_type); 
        ass_file->cmds[ass_file->cmd_num].cmd = command->cmd;
    
        if(!strcmp(cmd_name, "POP") && arg_type == IMM)
        {
            SetErrorBit(&ass_file->errors, ASM_POP_WITH_NUM);
        }

        if(arg_type != NOARG)
        {
            if(arg_num == 1)
            {
                ass_file->cmds[ass_file->cmd_num].arg = command->arg;
            }
            else
            {
                SetErrorBit(&ass_file->errors, ASM_TOO_FEW_ARGS);
            }
        }
        else if(arg_num != 0)
        {
            SetErrorBit(&ass_file->errors, ASM_TOO_MANY_ARGS);
        }

        ass_file->cmd_num++;
    }
    else
        SetErrorBit(&ass_file->errors, INVALID_ASM_COMMAND);
}

int ReadLine(AsmFile* ass_file, char* cur_str, CommandWithArg* command)
{
    assert(ass_file != NULL);
    assert(cur_str != NULL);
    assert(command != NULL);
    
    char* cmd_name = (char*)calloc(strlen(cur_str), sizeof(char));
    char* str = (char*)calloc(strlen(cur_str), sizeof(char));

    if(ass_file->errors > 0)
    {
		return 0;
    }

    if(sscanf(cur_str, "%s %lf", cmd_name, &command->arg) == 2)
    {
        def(ass_file, command, cmd_name, IMM);
    }
    else if(sscanf(cur_str, "%s %s", cmd_name, str) == 2)
    {
        if(IsReg(command,str))
        {
            def(ass_file, command, cmd_name, REG);
        }
        else if(IsLabel(ass_file, command, str))
        {
            def(ass_file, command, cmd_name, LAB);
        }
        else
        {
          SetErrorBit(&ass_file->errors, ASM_INVALID_REG_OR_LABEL_NAME);
        }
    }
    else if(sscanf(cur_str, "%s %s", cmd_name, str)== 1)
    {
        if(cmd_name[strlen(cmd_name) - 1] != ':')
        {
            def(ass_file, command, cmd_name, NOARG);
        }
    }
            
    if(ass_file->errors > 0)
    {
		return 0;
    }
    else
    {
        return 1;
    }
}

int ASMProcess(AsmFile* ass_file, const char* file_name)
{
    assert(ass_file != NULL);
    assert(file_name != NULL);

    ERROR_PROCESSING(ass_file, ASMDump, ASMDtor, 0)

    CommandWithArg* command = (CommandWithArg*)calloc(1, sizeof(CommandWithArg));
    assert(command != NULL);

    FILE *file = fopen(file_name, "w");

    if(!file)
    {
        SetErrorBit(&ass_file->errors, ASM_COMPILED_FILE_ERROR);
    }

    ass_file->labels.label_address = (int*)calloc(ass_file->lines.lines_number, sizeof(int));
    ass_file->labels.label_name = (char**)calloc(ass_file->lines.lines_number, sizeof(char*));
    
    int blank_lines_counter = 0;
    char* trash; 

    for(int line_num = 1; line_num < ass_file->lines.lines_number + 1; line_num++)
	{
        char* cur_str = ass_file->lines.lines_ptr[line_num - 1];
        int cmd_size = strlen(cur_str);
		char* cmd_name = (char*)calloc(cmd_size, sizeof(char));
        trash = (char*)calloc(cmd_size, sizeof(char));
                        
        if(sscanf(cur_str, "%s %s", cmd_name, trash) == 1)
        { 
            if(cmd_name[cmd_size - 1] == ':')
            {
                ass_file->labels.label_name[ass_file->labels.labels_num] = (char*)calloc(cmd_size, sizeof(char));
                ass_file->labels.label_name[ass_file->labels.labels_num] = cur_str;
                
                ass_file->labels.label_address[ass_file->labels.labels_num] = line_num - ass_file->labels.labels_num - blank_lines_counter;
                ass_file->labels.labels_num++;
            }
        }
        else if(sscanf(cur_str, "%s %s", cmd_name, trash) == -1)
        {
            blank_lines_counter++;
        }

        ERROR_PROCESSING(ass_file, ASMDump, ASMDtor, line_num)
    }
    free(trash);

    for(int line_num = 1; line_num < ass_file->lines.lines_number + 1; line_num++)
    {
        char* cur_str = ass_file->lines.lines_ptr[line_num - 1];
        ReadLine(ass_file, cur_str, command);
        ERROR_PROCESSING(ass_file, ASMDump, ASMDtor, line_num)
        CommandWithArg* command = (CommandWithArg*)calloc(1, sizeof(CommandWithArg));
    }

    fwrite(ass_file->cmds, sizeof(CommandWithArg), ass_file->cmd_num, file);
    fclose(file);
}

int ReadFile(AsmFile* ass_file, FILE* file)
{
    assert(ass_file != NULL);
    assert(file != NULL);

    int asm_code_size = GetFileSize(file, SEEK_SET);
    char* asm_code = (char*)calloc(asm_code_size + 1, sizeof(char));
    if(!asm_code)
    {
        return 0;
    }
    
    fread(asm_code, sizeof(char), asm_code_size, file);
    asm_code[asm_code_size] = '\0';

    ass_file->lines.lines_number = GetLineNumber(asm_code, asm_code_size);
    ass_file->lines.lines_ptr = (char**)calloc(ass_file->lines.lines_number, sizeof(char*));
    if(!ass_file->lines.lines_ptr)
    {
        return 0;
    }

    CreateBufferOfLines(ass_file, asm_code, asm_code_size);

    ass_file->cmd_num = 0;
    ass_file->cmds = (CommandWithArg*)calloc(ass_file->lines.lines_number, sizeof(CommandWithArg));
    if(!ass_file->cmds)
    {
        return 0;
    }

    return 1;
}

int ASMCtor(AsmFile* ass_file, const char* file_name)
{
    if(!ass_file)
    {
        return ASM_PTR_NULL;
    }

    ass_file->errors = NO_ERRORS;
    ass_file->labels = {};
    ass_file->labels.labels_num = 0;

    FILE* file = fopen(file_name, "rb");
    if(!file)
    {
        SetErrorBit(&ass_file->errors, ASM_COMPILED_FILE_ERROR);
    }

    ass_file->log =  fopen("asm/asmlog.txt", "wb");
    if(!ass_file->log)
    {
        SetErrorBit(&ass_file->errors, ASM_LOGER_ERROR);
    }

    if(!ReadFile(ass_file, file))
    {
        SetErrorBit(&ass_file->errors, ASM_BAD_TEXT_INFO);
    }

    fclose(file);
    ERROR_PROCESSING(ass_file, ASMDump, ASMDtor, 0)
    return ass_file->errors;
}

void ASMDump(AsmFile* ass_file, size_t line_num, FILE* logger)
{
    assert(ass_file != NULL);
    assert(logger != NULL);

    static size_t num_of_call = 1;

	fprintf(logger, 
        "=======================================\n"
        "ASM DUMP CALL #%zu\n"
        "Line: ", num_of_call);
    if(line_num == 0)
    {
        fprintf(logger, "Before processing file\n");
    }
    else
    {
        fprintf(logger, "%zu\n", line_num);
    }

	if(ass_file->errors)
    {
        fprintf(logger, "-------------ERRORS------------\n");
        if(ass_file->errors & ASM_PTR_NULL)
        {
            fprintf(logger, "ASM POINTER IS NULL\n");
            return;
        }
        if(ass_file->errors & ASM_BAD_TEXT_INFO)             fprintf(logger, "SOME TROUBLES WITH TEXT INFO STRUCT\n");
        if(ass_file->errors & ASM_LOGER_ERROR)               fprintf(logger, "ASM LOGGER ERROR\n");
        if(ass_file->errors & ASM_COMPILED_FILE_ERROR)       fprintf(logger, "COMPILED FILE ERROR\n");
        if(ass_file->errors & ASM_INVALID_REG_OR_LABEL_NAME) fprintf(logger, "Invalid reg or label name!\n");
        if(ass_file->errors & ASM_TOO_MANY_ARGS)             fprintf(logger, "Too MANY args in command!\n");
        if(ass_file->errors & ASM_TOO_FEW_ARGS)              fprintf(logger, "Too FEW args in command!\n");	   
        if(ass_file->errors & ASM_POP_WITH_NUM)              fprintf(logger, "You just did Pop with num, are you crazy????\n");

        fprintf(logger, "----------END_OF_ERRORS--------\n");
    }
    else
        fprintf(logger, "------------NO_ERRORS----------\n");
    fprintf(logger, "=======================================\n\n");
    num_of_call++;
}


int ASMDtor(AsmFile* ass_file)
{
    if(!ass_file)
    {
        return ASM_PTR_NULL;
    }
	if(!ass_file->lines.lines_ptr || !ass_file->cmds)
    {
        SetErrorBit(&ass_file->errors, ASM_BAD_TEXT_INFO);
        return ASM_BAD_TEXT_INFO;
    }
    else
    {
        free(ass_file->lines.lines_ptr);
        free(ass_file->cmds);
        free(ass_file->labels.label_address);
        free(ass_file->labels.label_name);
        ass_file->cmd_num = 0xB1BA;
        fclose(ass_file->log);
    }
    return NO_ERRORS;
}

int main(int argc, const char* argv[])
{
    AsmFile ass_file = {};

    if(argc == 1)
    {
        ASMCtor(&ass_file, "quadratic_equation.txt");
        ASMProcess(&ass_file, "ass.txt");

    }
    else if(argc == 3)
    {
        ASMCtor(&ass_file, argv[1]);
        ASMProcess(&ass_file, argv[2]);
    }
    else
    {
        printf("Invalid number of args to program");
        return 1;
    }
    ASMDtor(&ass_file);
    return 0;
}
