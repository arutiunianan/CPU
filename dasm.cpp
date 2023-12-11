#include "dasm.h"

char* GetReg( Dasm* dasm,Elem_t reg )
{
	switch ( ( int )reg )
	{
		#define REG_DEF(reg_name, reg_value)  \
		case reg_value:						   \
			return #reg_name;				    \
			break;							     

		#include "regs.h"
		#undef REG_DEF
	}
    SetErrorBit( &dasm->errors, DASM_INVALID_REG_OR_LABEL_NAME );
}

int IsValidRegArg( Com* command )
{
	assert( command != NULL );

	int is_reg_found = 0;

	switch( ( size_t )command->CPUcmdarg.arg )
	{
		#define REG_DEF( name, cpu_code, ... )  \
		case cpu_code:                         \
			is_reg_found = 1;                   \
			break;

		#include "regs.h"
	}

	#undef REG_DEF
	
	if( !is_reg_found )
		return 0;

	return 1;
}

int IsValidCommand( Dasm* dasm, Com* command )
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
				SetErrorBit( &dasm->errors, DASM_INVALID_REG_OR_LABEL_NAME );
                return 0;
            }

			command->argNum = 1;
		}
		else if ( ( char )command->CPUcmdarg.cmd & IMM )
		{
			UnsetCommandBitCode( &command->CPUcmdarg.cmd, IMM );
			command->cmdArgType = IMM;
            		
            if( command->CPUcmdarg.cmd == POP )
            {
                SetErrorBit( &dasm->errors, DASM_POP_WITH_NUM );
                return 0;
            }

			command->argNum = 1;
        }
        else if ( ( char )command->CPUcmdarg.cmd & LAB )
        {
            UnsetCommandBitCode( &command->CPUcmdarg.cmd, LAB );
            command->cmdArgType = LAB;

            command->argNum = 1;
        }
    }
	else if( command->CPUcmdarg.arg != 0 )
		SetErrorBit( &dasm->errors, DASM_TOO_MANY_ARGS );
	else
    {
        command->cmdArgType = NOARG;
		command->argNum = 0;
    }

    switch ( command->CPUcmdarg.cmd )
	{
		#define DEF_CMD( cmd_name, cmd_num, cmd_n_args, cmd_code )  \
		case cmd_name:												 \
			return ( command->argNum == cmd_n_args );
		
		#include "cmds.h"

		default:
			return 0;
	}
	#undef DEF_CMD

}

int DASMCtor( Dasm* dasm, const char* file_name )
{
    if ( !dasm )
		return DASM_PTR_NULL;

    dasm->errors = NO_ERRORS;

    FILE *file = fopen( file_name, "rb" );
    if ( !file )
		SetErrorBit( &dasm->errors, DASM_COMPILED_FILE_ERROR );

    dasm->log =  fopen( "dasmlog.txt","wb" );
    if ( !dasm->log )
		SetErrorBit( &dasm->errors, DASM_LOGER_ERROR );
    
    int startOfCode = 0;
    dasm->code_size = GetFileSize( file, startOfCode );
    dasm->cmd_num = dasm->code_size/sizeof( CPU );

    dasm->cmds = ( CPU* )calloc( sizeof( char ), dasm->code_size );
    if ( !dasm->cmds )
		SetErrorBit( &dasm->errors, DASM_CMDS_PTR_NULL );

    fread( dasm->cmds, sizeof( char ), dasm->code_size, file );

    fclose( file );

    ERROR_PROCESSING(dasm, DASMDump, DASMDtor, 0);
}

int ProcessingDASM( Dasm* dasm, const char* file_name )
{
    assert( dasm != NULL) ;

    if ( !dasm->cmds )
		SetErrorBit( &dasm->errors, DASM_CMDS_PTR_NULL) ;

	ERROR_PROCESSING( dasm, DASMDump, DASMDtor, 0 );

    int* line_num = &dasm->current_line_num;
    Com* command = &dasm->cur_cmd;
    FILE *file = fopen( file_name, "w" );

    for (*line_num = 1; *line_num < dasm->cmd_num + 1; (*line_num)++)
	{
        command->CPUcmdarg.cmd = *( CPUCommand* ) ( ( char* )dasm->cmds + ( *line_num-1 ) * sizeof( CPU ) );
		command->CPUcmdarg.arg  = *( Elem_t* ) ( ( char* )dasm->cmds + ( *line_num-1 ) * sizeof( CPU ) + 8 );

        if( IsValidCommand( dasm, command ) )
        {
            switch ( command->CPUcmdarg.cmd )
			{
				#define DEF_CMD( cmd_name, cmd_num, cmd_n_args, cmd_code )                          \
                    case cmd_name:                                                                   \
                        if( command->cmdArgType == IMM )                                              \
                            fprintf( file,"%s %lf\n",#cmd_name,command->CPUcmdarg.arg );               \
                        else if( command->cmdArgType == REG )                                           \
                            fprintf( file,"%s %s\n",#cmd_name,GetReg(dasm,command->CPUcmdarg.arg) );     \
                        else if( command->cmdArgType == LAB )                                             \
                            fprintf( file,"%s %lf\n",#cmd_name,command->CPUcmdarg.arg );                   \
                        else                                                                                \
                            fprintf( file,"%s\n",#cmd_name );                                                \
                        break;
				
				#include "cmds.h"
				#undef DEF_CMD
			}
        }
        else
			SetErrorBit( &dasm->errors, INVALID_DASM_COMMAND );

        ERROR_PROCESSING( dasm, DASMDump, DASMDtor, *line_num) ;
    }
}

void DASMDump( Dasm* dasm, size_t line_num, FILE* logger )
{
	assert( dasm != NULL );
	assert( logger != NULL );

	static size_t num_of_call = 1;

	fprintf( logger, 
		"=======================================\n"
		"DASM DUMP CALL #%zu\n"
		"Line: ", num_of_call );
	if ( line_num == 0 )
		fprintf( logger, "Before processing file\n" );
	else
		fprintf( logger, "%zu\n", line_num );

	if ( dasm->errors )
	{
		fprintf( logger, "-------------ERRORS------------\n" );
		if ( dasm->errors & DASM_PTR_NULL )
		{
			fprintf( logger, "DASM POINTER IS NULL\n" );
			return;
		}
		if ( dasm->errors & DASM_BAD_TEXT_INFO ) fprintf( logger, "SOME TROUBLES WITH TEXT INFO STRUCT\n" );
		if ( dasm->errors & DASM_LOGER_ERROR ) fprintf( logger, "DASM LOGGER ERROR\n" );
	    if ( dasm->errors & DASM_COMPILED_FILE_ERROR ) fprintf( logger, "COMPILED FILE ERROR\n" );
	    if ( dasm->errors & DASM_INVALID_REG_OR_LABEL_NAME ) fprintf( logger, "Invalid reg or label name!\n" );
	    if ( dasm->errors & DASM_TOO_MANY_ARGS ) fprintf( logger, "Too MANY args in command!\n" );
	    if ( dasm->errors & DASM_POP_WITH_NUM ) fprintf( logger, "You just did Pop with num, are you crazy????\n" );

		fprintf( logger, "----------END_OF_ERRORS--------\n" );
	}
	else
		fprintf( logger,"------------NO_ERRORS----------\n" );
	fprintf( logger, "=======================================\n\n" );
	num_of_call++;

}

int DASMDtor( Dasm* dasm )
{
    if ( !dasm )
		return DASM_PTR_NULL;

    fclose( dasm->log );

    free( dasm->cmds );
    dasm->cmd_num = 0xB0BA;
    dasm->current_line_num = 0xB1BA;
    dasm->code_size = 0xB0BA;
}

int main( int argc, const char* argv[] )
{
    Dasm dasm = {};

    if ( argc == 1 )
    {
        DASMCtor( &dasm, "ass.txt" );
        ProcessingDASM( &dasm, "dass.txt" );
        DASMDtor( &dasm );
    }
    else if ( argc == 3 )
    {
        DASMCtor( &dasm, argv[1] );
        ProcessingDASM( &dasm, argv[2] );
        DASMDtor( &dasm );
    }
    else
    {
        printf( "Invalid number of args to program" );
    }
}