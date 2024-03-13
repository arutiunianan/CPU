#include "dis.h"

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
        printf("Invalid number of args to program\n");
        return 1;
    }
    DISDtor(&dis_file);
    return 0;
}