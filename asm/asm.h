#ifndef ASM_H_
#define ASM_H_

#include "../commands.h"

typedef enum ASMErrors
{
    NO_ERRORS                     = 0,
	ASM_PTR_NULL                  = 1 << 1,
	ASM_BAD_TEXT_INFO             = 1 << 2,
	INVALID_ASM_COMMAND           = 1 << 3,
	ASM_LOGER_ERROR               = 1 << 4,
	ASM_COMPILED_FILE_ERROR       = 1 << 5,
    ASM_TOO_MANY_ARGS             = 1 << 6,
	ASM_TOO_FEW_ARGS              = 1 << 7,
	ASM_INVALID_REG_OR_LABEL_NAME = 1 << 8,
	ASM_POP_WITH_NUM              = 1 << 9
}ASMErrors;

typedef struct AsmFile
{
    Labels labels;
    Lines lines;

    CommandWithArg* cmds;
    int cmd_num;

    FILE* log;
    int errors;
}AsmFile;

int ASMCtor(AsmFile* ass_file, const char* equation);
int ASMDtor(AsmFile* ass_file);
int ASMProcess(AsmFile* ass_file, const char* equation);
void ASMDump(AsmFile* ass_file, size_t line_num, FILE* logger);


#endif // #define ASM_H_