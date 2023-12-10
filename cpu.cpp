#include "cpu.h"

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

void SetReg(Cpu* cpu, Elem_t reg, Elem_t value)
{
	//assert(cpu != NULL);

	switch ((int)reg)
	{
		#define REG_DEF(reg_name, reg_value)     \
		case reg_name:						     \
			cpu->reg_name = value;				 \
			break;							     

		#include "regs.h"
		
		#undef REG_DEF
	}
}

Elem_t GetProperArgument(Cpu* cpu)
{
        if (cpu->curCmd.cmdArgType & IMM || cpu->curCmd.cmdArgType & LAB)
		    return cpu->curCmd.CPUcmdarg.arg;
	    else if (cpu->curCmd.cmdArgType & REG)
            switch ((int)cpu->curCmd.CPUcmdarg.arg)
	        {
		        #define REG_DEF(reg_name, reg_value)       \
		            case reg_value:						 \
                    \
			        return cpu->reg_name;		     \

		        #include "regs.h"

		        #undef REG_DEF
	        }
	    //else
		    //assert(0);
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
            
            //printf("%d\n",command->CPUcmdarg.cmd);
		
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
            
            //printf("%d\n",command->CPUcmdarg.cmd);
		
            //if(command->CPUcmdarg.cmd == POP)
            //{
                //command->error = POP_WITH_NUM;
                //return 0;
            //}

			command->argNum = 1;
        }
    }
	//else if(command->CPUcmdarg.arg != 0)
		//command->error == INVALID_SYNTAX;
	else
		command->argNum = 0;

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

int CPUCtor( Cpu* cpu, const char* binaryFile )
{

    FILE *file = fopen( binaryFile, "rb" );
    //ошибка открытия

    //fread( &cpu->version, sizeof( char ), sizeof( cpu->version ), file );
    //ошибка файл маленького размера
    //int startOfCode = ftell( file );
    int startOfCode = 0;
    cpu->codeSize = GetFileSize( file, startOfCode );
    cpu->cmdNum = cpu->codeSize/sizeof(CPU);

    cpu->cmds = ( CPU* )calloc( sizeof( char ), cpu->codeSize);
    fread(cpu->cmds, sizeof( char ), cpu->codeSize, file );

    fclose( file );
    
    StackCtor( &cpu->stack );

    return 0;
}

int ProcessingCPU( Cpu* cpu )
{

    int* line_num = &cpu->current_line_num;
    Com* command = &cpu->curCmd;
    for (*line_num = 1; *line_num < cpu->cmdNum + 1; (*line_num)++)
	{
        command->CPUcmdarg.cmd = *(CPUCommand*)((char*)cpu->cmds + (*line_num-1) * sizeof(CPU));
		command->CPUcmdarg.arg  = *(Elem_t*)((char*)cpu->cmds + (*line_num-1) * sizeof(CPU) + 8);

        if(IsValidCommand( command ) )
        {
            switch (command->CPUcmdarg.cmd)
			{
				#define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) case cmd_name: cmd_code; break;
				
				#include "cmds.h"

				#undef DEF_CMD
			}
        }
    }
}

int CPUDtor( Cpu* cpu )
{
    StackDtor( &cpu->stack );
    //cpu->code = "";
    cpu->codeSize = -1;
    //cpu->version = -1;
    cpu = NULL;

}

int main( int argc, const char* argv[] )
{
    Cpu cpu = {};

    if ( argc == 1 )
    {
        CPUCtor( &cpu, "ass.txt" );
    }
    else if ( argc == 2 )
    {
        CPUCtor( &cpu, argv[1] );

    }
    else
    {
        printf( "Invalid number of args to program" );
        return 1;
    }

    ProcessingCPU( &cpu );
    //for(int i = 0; i < cpu.stack.size; i++)
    //    printf("%lf\n",cpu.stack.data[i]);
    //CPUDtor( &cpu );

}