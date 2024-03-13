#include "cpu.h"


int main(int argc, const char* argv[])
{
    CpuLog cpu_log = {};

    if(argc == 1)
    {
        CPUCtor(&cpu_log, "ass.txt");
    }
    else if(argc == 2)
    {
        CPUCtor(&cpu_log, argv[1]);
    }
    else
    {
        printf("Invalid number of args to program\n");
        return 1;
    }
    CPUProcess(&cpu_log);
    CPUDtor(&cpu_log);
    return 0;
}