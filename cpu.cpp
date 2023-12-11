#include "cpu.h"

int IsValidRegArg( Com* command )
{
	assert( command != NULL );

	int is_reg_found = 0;

	switch( ( size_t )command->CPUcmdarg.arg )
	{
		#define REG_DEF( name, cpu_code, ... )   \
		case cpu_code:                            \
			is_reg_found = 1;                      \
			break;

		#include "regs.h"
	}
	#undef REG_DEF
	
	if( !is_reg_found )
		return 0;

	return 1;
}

void SetReg( Cpu* cpu, Elem_t reg, Elem_t value )
{
	assert( cpu != NULL );

	switch ( ( int )reg )
	{
		#define REG_DEF( reg_name, reg_value )     \
		case reg_name:						        \
			cpu->reg_name = value;				     \
            return;                                   \
			break;							     

		#include "regs.h"
		#undef REG_DEF
	}
    SetErrorBit( &cpu->errors, CPU_INVALID_REG_OR_LABEL_NAME );
}

Elem_t GetProperArgument( Cpu* cpu )
{
        if ( cpu->cur_cmd.cmdArgType & IMM || cpu->cur_cmd.cmdArgType & LAB )
		    return cpu->cur_cmd.CPUcmdarg.arg;
	    else if ( cpu->cur_cmd.cmdArgType & REG )
            switch ( ( int )cpu->cur_cmd.CPUcmdarg.arg )
	        {
		        #define REG_DEF( reg_name, reg_value )       \
		            case reg_value:			       			  \
			        return cpu->reg_name;		               \

		        #include "regs.h"
		        #undef REG_DEF
	        }
	    else
		    SetErrorBit( &cpu->errors, CPU_INVALID_REG_OR_LABEL_NAME );
}

int IsValidCommand( Cpu* cpu, Com* command )
{
    assert( command != NULL );

	if ( !command )
		return 0;

    if ( ( char ) command->CPUcmdarg.cmd & ( IMM | REG | LAB ) && command->CPUcmdarg.cmd != HLT )
    {
        if( ( char )command->CPUcmdarg.cmd & REG )
		{
			UnsetCommandBitCode( &command->CPUcmdarg.cmd, REG );
			command->cmdArgType = REG;

			if( !IsValidRegArg( command ) )
            {
				SetErrorBit( &cpu->errors, CPU_INVALID_REG_OR_LABEL_NAME );
                return 0;
            }
			command->argNum = 1;
		}
		else if((char)command->CPUcmdarg.cmd & IMM)
		{
			UnsetCommandBitCode(&command->CPUcmdarg.cmd, IMM);
			command->cmdArgType = IMM;
            
            if( command->CPUcmdarg.cmd == POP )
            {
                SetErrorBit( &cpu->errors, CPU_POP_WITH_NUM) ;
                return 0;
            }
			command->argNum = 1;
        }
        else if (( char )command->CPUcmdarg.cmd & LAB )
		{
			UnsetCommandBitCode( &command->CPUcmdarg.cmd, LAB );
			command->cmdArgType = LAB;
			command->argNum = 1;
        }
    }
	else if( command->CPUcmdarg.arg != 0 )
		SetErrorBit( &cpu->errors, CPU_TOO_MANY_ARGS );
	else
	{
        command->cmdArgType = NOARG;
		command->argNum = 0;
    }

    switch ( command->CPUcmdarg.cmd )
	{
		#define DEF_CMD( cmd_name, cmd_num, cmd_n_args, cmd_code )	\
		case cmd_name:												 \
			return ( command->argNum == cmd_n_args );
		
		#include "cmds.h"

		default:
			return 0;
	}
	#undef DEF_CMD
}

int CPUCtor( Cpu* cpu, const char* binaryFile )
{

    if ( !cpu )
		return CPU_PTR_NULL;

    cpu->errors = NO_ERRORS;

    FILE *file = fopen( binaryFile, "rb" );
    if ( !file )
		SetErrorBit( &cpu->errors, CPU_COMPILED_FILE_ERROR );

    cpu->log =  fopen( "cpulog.txt","wb" );
    if ( !cpu->log )
		SetErrorBit(&cpu->errors, CPU_LOGER_ERROR );
    
    int startOfCode = 0;
    cpu->code_size = GetFileSize( file, startOfCode );
    cpu->cmd_num = cpu->code_size/sizeof( CPU );

    cpu->cmds = ( CPU* )calloc( sizeof( char ), cpu->code_size );
    if (!cpu->cmds)
		SetErrorBit( &cpu->errors, CPU_CMDS_PTR_NULL );

    fread( cpu->cmds, sizeof( char ), cpu->code_size, file );

    fclose( file );

    if (StackCtor( &cpu->stack ) )
		SetErrorBit( &cpu->errors, CPU_BAD_STACK );

	ERROR_PROCESSING( cpu, CPUDump, CPUDtor, 0 );

    return 0;
}

int ProcessingCPU( Cpu* cpu )
{
    assert( cpu != NULL );

    if ( !cpu->cmds )
		SetErrorBit( &cpu->errors, CPU_CMDS_PTR_NULL );

	ERROR_PROCESSING( cpu, CPUDump, CPUDtor, 0 );

    int* line_num = &cpu->current_line_num;
    Com* command = &cpu->cur_cmd;
    for ( *line_num = 1; *line_num < cpu->cmd_num + 1; (*line_num)++ )
	{
        command->CPUcmdarg.cmd = *( CPUCommand* ) ( ( char*) cpu->cmds + ( *line_num-1 ) * sizeof( CPU ) );
		command->CPUcmdarg.arg  = *( Elem_t* ) ( ( char* ) cpu->cmds + ( *line_num-1 ) * sizeof( CPU ) + 8 );

        if( IsValidCommand( cpu, command ) )
        {
            switch ( command->CPUcmdarg.cmd )
			{
				#define DEF_CMD( cmd_name, cmd_num, cmd_n_args, cmd_code ) case cmd_name: cmd_code; break;
				
				#include "cmds.h"
				#undef DEF_CMD
			}
        }
        else
			SetErrorBit( &cpu->errors, CPU_WRONG_INPUT );

		ERROR_PROCESSING( cpu, CPUDump, CPUDtor, *line_num );

    }
}

void CPUDump( Cpu* cpu, size_t num_of_line, FILE* logger )
{
	static size_t num_of_call = 1;
	fprintf( logger, "=======================================\nCPU DUMP CALL #%zu\nLine: ", num_of_call );
	if ( num_of_line == 0 )
		fprintf( logger, "Before processing file\n" );
	else
		fprintf( logger, "%zu\n", num_of_line );

	if ( cpu->errors )
	{
		fprintf( logger, "-------------ERRORS------------\n" );
		if ( cpu->errors & CPU_PTR_NULL )
		{
			fprintf( logger, "CPU POINTER IS NULL\n" );
			return;
		}
		if ( cpu->errors & CPU_BAD_STACK ) fprintf( logger, "SOME TROUBLES WITH STACK STRUCT\n" );
		if ( cpu->errors & CPU_WRONG_INPUT ) fprintf( logger, "WRONG INPUT\n" );
		if ( cpu->errors & CPU_LOGER_ERROR ) fprintf( logger, "CPU LOGER ERROR\n" );
		if ( cpu->errors & CPU_CMDS_PTR_NULL ) fprintf( logger, "CPU CS PTR NULL\n" );
		if ( cpu->errors & CPU_COMPILED_FILE_ERROR ) fprintf( logger, "CPU COMPILED FILE ERROR\n" );		
        if ( cpu->errors & CPU_INVALID_REG_OR_LABEL_NAME ) fprintf( logger, "Invalid reg or label name!\n" );
	    if ( cpu->errors & CPU_TOO_MANY_ARGS ) fprintf( logger, "Too MANY args in command!\n" );
	    if ( cpu->errors & CPU_POP_WITH_NUM ) fprintf( logger, "You just did Pop with num, are you crazy????\n" );

		fprintf( logger, "----------END_OF_ERRORS--------\n" );
	}
	else
		fprintf( logger, "------------NO_ERRORS----------\n" );

	if ( num_of_line > 0 )
	{
		for ( size_t current_line = 1; current_line <= cpu->cmd_num; current_line++ )
		{
			CPUCommand current_cmd_cpu_code = *( CPUCommand* ) ( ( char* ) cpu->cmds + ( current_line-1 ) * sizeof ( CPU ) );
			Elem_t current_cmd_cpu_arg_code = *( Elem_t* ) ( ( char* ) cpu->cmds + ( current_line-1) * sizeof( CPU ) + 8 );

			fprintf(logger, "(%2zu) %3d %lf" , current_line, current_cmd_cpu_code, current_cmd_cpu_arg_code);
			if ( current_line == cpu->current_line_num+1 )
				fprintf( logger, " <-----" );
			fprintf( logger, "\n" );
		}
	}
	#define REG_DEF( name, code, ... ) fprintf( logger, "%s:%lf"  ", " , #name, cpu->name );
	
	#include "regs.h"

	#undef REG_DEF
	fprintf( logger, "\n" );
	fprintf( logger, "=======================================\n\n" );
	
	num_of_call++;

}


int CPUDtor( Cpu* cpu )
{
    if ( !cpu )
		return CPU_PTR_NULL;

    fclose( cpu->log );
    if ( StackDtor( &cpu->stack ) )
			SetErrorBit( &cpu->errors, CPU_BAD_STACK );

    free( cpu->cmds );
    cpu->cmd_num = 0xB0BA;
    cpu->current_line_num = 0xB1BA;
    cpu->code_size = 0xB0BA;
}

int main( int argc, const char* argv[] )
{
    Cpu cpu = {};

    if ( argc == 1 )
    {
        CPUCtor( &cpu, "ass.txt" );
        ProcessingCPU( &cpu );
        CPUDtor( &cpu );
    }
    else if ( argc == 2 )
    {
        CPUCtor( &cpu, argv[1] );
        ProcessingCPU( &cpu );
        CPUDtor( &cpu );
    }
    else
        printf( "Invalid number of args to program" );
}