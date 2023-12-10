#include "dasm.h"

char* GetReg(Elem_t reg)
{
	//assert(cpu != NULL);

	switch ((int)reg)
	{
		#define REG_DEF(reg_name, reg_value)     \
		case reg_value:						     \
			return #reg_name;				 \
			break;							     

		#include "regs.h"
		
		#undef REG_DEF
	}
}

int IsValidRegArg( Com* command )
{
	//assert(command != NULL);

	int is_reg_found = 0;

	switch((size_t)command->CPUcmdarg.arg)
	{
		#define REG_DEF(name, cpu_code, ...) \
		case cpu_code:                       \
			is_reg_found = 1;             \
			break;

		#include "regs.h"
	}

	#undef REG_DEF
	
	if(!is_reg_found)
	{
		//command->error = INVALID_REG_OR_LABEL_NAME;
		return 0;
	}

	return 1;
}

int IsValidCommand(Com* command)
{

    //assert(command != NULL);

	//if (!command)
	//	return 0;
    if ( (char) command->CPUcmdarg.cmd & (IMM | REG | LAB) && command->CPUcmdarg.cmd != HLT)
    {

        if((char)command->CPUcmdarg.cmd & REG)
		{
			UnsetCommandBitCode(&command->CPUcmdarg.cmd, REG);

			command->cmdArgType = REG;

			if(!IsValidRegArg(command))
				return 0;

			command->argNum = 1;
		}
		else if((char)command->CPUcmdarg.cmd & IMM)
		{
			UnsetCommandBitCode(&command->CPUcmdarg.cmd, IMM);
			command->cmdArgType = IMM;
            		
            //if(command->CPUcmdarg.cmd == POP)
            //{
                //command->error = POP_WITH_NUM;
                //return 0;
            //}

			command->argNum = 1;
        }
        else if((char)command->CPUcmdarg.cmd & LAB)
        {
            UnsetCommandBitCode(&command->CPUcmdarg.cmd, LAB);
            command->cmdArgType = LAB;

            command->argNum = 1;
        }
    }
	//else if(command->CPUcmdarg.arg != 0)
		//command->error == INVALID_SYNTAX;
	else
    {
        command->cmdArgType = NOARG;
		command->argNum = 0;
    }
    switch (command->CPUcmdarg.cmd)
	{
		#define DEF_CMD( cmd_name, cmd_num, cmd_n_args, cmd_code )			\
		case cmd_name:												\
			return (command->argNum == cmd_n_args);
		
		#include "cmds.h"

		default:
			return 0;
	}
	#undef DEF_CMD
	

}



int DASMCtor( Dasm* dasm, const char* equation )
{
    FILE *file = fopen( equation, "rb" );
    //ошибка открытия

    //fread( &cpu->version, sizeof( char ), sizeof( cpu->version ), file );
    //ошибка файл маленького размера
    //int startOfCode = ftell( file );
    int startOfCode = 0;
    dasm->codeSize = GetFileSize( file, startOfCode );
    dasm->cmdNum = dasm->codeSize/sizeof(CPU);

    dasm->cmds = ( CPU* )calloc( sizeof( char ), dasm->codeSize);
    fread(dasm->cmds, sizeof( char ), dasm->codeSize, file );

    fclose( file );

}

void ProcessingDASM( Dasm* dasm, const char* equation )
{

    int* line_num = &dasm->current_line_num;
    Com* command = &dasm->curCmd;
    FILE *file = fopen( equation, "w" );
    for (*line_num = 1; *line_num < dasm->cmdNum + 1; (*line_num)++)
	{
        command->CPUcmdarg.cmd = *(CPUCommand*)((char*)dasm->cmds + (*line_num-1) * sizeof(CPU));
		command->CPUcmdarg.arg  = *(Elem_t*)((char*)dasm->cmds + (*line_num-1) * sizeof(CPU) + 8);

        if(IsValidCommand( command ) )
        {
            switch (command->CPUcmdarg.cmd)
			{
				#define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) \
                    case cmd_name: \
                        if(command->cmdArgType == IMM) \
                            fprintf(file,"%s %lf\n",#cmd_name,command->CPUcmdarg.arg); \
                        else if(command->cmdArgType == REG) \
                            fprintf(file,"%s %s\n",#cmd_name,GetReg(command->CPUcmdarg.arg)); \
                        else if(command->cmdArgType == LAB) \
                            fprintf(file,"%s %lf\n",#cmd_name,command->CPUcmdarg.arg); \
                        else  \
                            fprintf(file,"%s\n",#cmd_name); \
                        break;
				
				#include "cmds.h"

				#undef DEF_CMD
			}
        }
    }

}



int main( int argc, const char* argv[] )
{
    Dasm dasm = {};

    if ( argc == 1 )
    {
        DASMCtor( &dasm, "ass.txt" );
        ProcessingDASM( &dasm, "dass.txt" );
    }
    else if ( argc == 3 )
    {
        DASMCtor( &dasm, argv[1] );
    }
    else
    {
        printf( "Invalid number of args to program" );
    }

}