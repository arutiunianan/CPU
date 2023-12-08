#include "cpu.h"

void UnsetCommandBitCode(CPUCommand* command_cpu_code, argType arg_type)
{
	//assert(command_cpu_code != NULL);

	*(char*)command_cpu_code &= ~(char)arg_type;
}

void SetCommandTypeBitCode(argType* old_arg_type, argType new_arg_type)
{
	//assert(old_arg_type != NULL);

	*(char*)old_arg_type |= (char)new_arg_type;
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

Elem_t GetProperArgument(Cpu* cpu)
{
        if (cpu->curCmd.cmdArgType & IMM)			            
		    return cpu->curCmd.CPUcmdarg.arg;
	    /*else if (cpu->curCmd.cmdArgType & REG)	
            switch ((int)cpu->curCmd.CPUcmdarg.arg)
	        {
		    #define REG_DEF(reg_name, reg_value)       \
		        case reg_name:						 \
			    return cmd_arg;		     \

		    #include "regs.h"

		    #undef REG_DEF
	        }					*/					
	    //else
		    //assert(0);
}

int IsValidCommand(Com* command)
{

    //assert(command != NULL);

	//if (!command)
	//	return 0;
    if ( (char) command->CPUcmdarg.cmd & (IMM | REG) && command->CPUcmdarg.cmd != HLT)
    {

        if((char)command->CPUcmdarg.cmd & REG)
		{
			UnsetCommandBitCode(&command->CPUcmdarg.cmd, REG);
			SetCommandTypeBitCode(&command->cmdArgType, REG);

			if(!IsValidRegArg(command))
				return 0;

			command->argNum = 1;
		}
		else if((char)command->CPUcmdarg.cmd & IMM)
		{
			UnsetCommandBitCode(&command->CPUcmdarg.cmd, IMM);
			SetCommandTypeBitCode(&command->cmdArgType, IMM);
            printf("%d\n",command->CPUcmdarg.arg);
		
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
		
		#include "commands.h"

		default:
			return 0;
	}
	#undef DEF_CMD
	return 1;

}

int GetFileSize( FILE *text, int startOfCode )
{
    fseek( text, 0, SEEK_END );
    int fileSize = ftell( text ) - startOfCode;
    fseek( text, 0, startOfCode );
    return fileSize;

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
				
				#include "commands.h"

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
    /*for(int i = 0; i < cpu.stack.size;i++ )
        printf("%d\n",cpu.stack.data[i]);*/
    //CPUDtor( &cpu );

}