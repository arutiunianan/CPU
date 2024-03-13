#include "asm.h"


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
        printf("Invalid number of args to program!\n");
        return 1;
    }
    ASMDtor(&ass_file);
    return 0;
}
