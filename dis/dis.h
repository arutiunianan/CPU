#ifndef DIS_H_
#define DIS_H_

#include "../commands.h"

typedef enum DISErrors
{
    NO_ERRORS                     = 0,
    DIS_PTR_NULL                  = 1 << 1,
    DIS_BAD_TEXT_INFO             = 1 << 2,
    INVALID_DIS_COMMAND           = 1 << 3,
    DIS_LOGER_ERROR               = 1 << 4,
    DIS_CMDS_PTR_NULL             = 1 << 5,
    DIS_COMPILED_FILE_ERROR       = 1 << 6,
    DIS_TOO_MANY_ARGS             = 1 << 7,
    DIS_INVALID_REG_OR_LABEL_NAME = 1 << 8,
    DIS_POP_WITH_NUM              = 1 << 9
}DISErrors;

typedef struct DisFile
{
    int cmd_num;
    CommandWithArg* cmds;
    int current_line_num;
    
    FILE* log;
    int errors;
}DisFile;

int DISDtor(DisFile* dis_file);
void DISDump(DisFile* dis_file, size_t line_num, FILE* logger);
int DISProcess(DisFile* dis_file, const char* file_name);
int DISCtor(DisFile* dis_file, const char* file_name);

#endif // #define DIS_H_