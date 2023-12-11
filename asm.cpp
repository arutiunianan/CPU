#include "asm.h"

static void CreateBufferOfLines( Asm* ass )
{
    assert( ass != NULL );

    int line = 1;
    ass->linestr[0] = ass->code;
    for( int i = 0; i < ass->code_size - 1; i++ )
        if( ass->code[i] == '\n' )
        {
            ass->linestr[line++] = &( ass->code[i+1] );

            ass->code[i] = '\0';
        }
}

int IsLabel( Asm* ass, Com* command, char* label )
{
    strcat( label, ":" );
    for ( size_t i = 0; i < ass->labels_num; i++ )
	{        
		if( strcmp(label, ass->labels[i].label_name) == 0 )
		{
			command->CPUcmdarg.arg = ( double )ass->labels[i].label_address;
            
			return 1;
		}
	}
    return 0;
}

int IsReg( Com* command, char* reg )
{
    #define REG_DEF( reg_name, reg_cpu_code )     \
	    if( strcmp(#reg_name, reg) == 0 )          \
        {                                           \
            command->CPUcmdarg.arg = reg_cpu_code;   \
            return 1;                                 \
        }                                    
	#include "regs.h"
	#undef REG_DEF
    return 0;
}

int IsCommand( Com* command )
{
    #define DEF_CMD( name, cpu_code, args_num, ...)        \
	    if ( strcmp(command->cmdCode, #name) == 0 )         \
        {                                                    \
		    command->CPUcmdarg.cmd = ( CPUCommand )cpu_code;    \
            command->argNum = args_num;                        \
            return 1;                                           \
        }
            
    #include "cmds.h"
	#undef DEF_CMD
    return 0;
}

void def( Asm* ass, Com* command, argType argtype )
{
    if( IsCommand( command ) )
    {
        SetCommandBitCode( &command->CPUcmdarg.cmd, argtype ); 
        ass->cmds[ass->cmd_num].cmd = command->CPUcmdarg.cmd;
    
        if( !strcmp( command->cmdCode, "POP" ) && argtype == IMM )
            SetErrorBit( &ass->errors, ASM_POP_WITH_NUM );

        if( argtype != NOARG )
            if( command->argNum == 1 )
                ass->cmds[ass->cmd_num].arg = command->CPUcmdarg.arg;
            else
                SetErrorBit( &ass->errors, ASM_TOO_FEW_ARGS );
        else if( command->argNum != 0 )
            SetErrorBit( &ass->errors, ASM_TOO_MANY_ARGS );

        ass->cmd_num++;
    }
    else
        SetErrorBit( &ass->errors, INVALID_ASM_COMMAND );
}

int ReadLine( Asm* ass, char* curStr, Com* command )
{
    assert(ass != NULL | command != NULL | curStr != NULL );

    command->cmdCode = ( char* )calloc( strlen(curStr), sizeof( char ) );
    char* str = ( char* )calloc( strlen( curStr ), sizeof( char ) );

    if ( ass->errors > 0 )
		return 0;

    if ( sscanf(curStr,"%s %lf",command->cmdCode,&command->CPUcmdarg.arg) == 2 )
    {

        def(ass,command,IMM);
    }
    else if( sscanf(curStr,"%s %s",command->cmdCode,str) == 2 )
    {
        if( IsReg( command,str ) )
            def( ass,command,REG );
        else if( IsLabel( ass, command, str ) )
            def( ass,command,LAB );
        else
          SetErrorBit( &ass->errors, ASM_INVALID_REG_OR_LABEL_NAME );
    }
    else if( sscanf( curStr,"%s %s",command->cmdCode,str ) == 1  )
        if( command->cmdCode[strlen(command->cmdCode) - 1] != ':' )
            def( ass,command,NOARG );
            
    if ( ass->errors > 0 )
		return 0;
    else
        return 1;
}

int ProcessingASM( Asm* ass, const char* file_name )
{
    assert( ass != NULL );

    ERROR_PROCESSING( ass, ASMDump, ASMDtor, 0 )

    Com* command = &ass->cur_cmd;

    assert( command != NULL );

    FILE *file = fopen( file_name, "w" );

    if ( !file )
		SetErrorBit( &ass->errors, ASM_COMPILED_FILE_ERROR );

    ass->labels = ( Label* )calloc( ass->line_number, sizeof( Label ) );
    int blank_lines_counter = 0;
    char* trash; 

    for ( int line_num = 1; line_num < ass->line_number + 1; line_num++ )
	{
        char* curStr = ass->linestr[line_num - 1];
        int cmdSize = strlen(curStr);
		command->cmdCode = ( char* )calloc( cmdSize, sizeof( char ) );
        trash = ( char* )calloc( cmdSize, sizeof( char ) );
                        
        if ( sscanf(curStr,"%s %s",command->cmdCode,trash) == 1 )
        { 
            if( command->cmdCode[cmdSize - 1] == ':')
            {
                ass->labels[ass->labels_num].label_name = ( char* )calloc( cmdSize, sizeof( char ) );
                ass->labels[ass->labels_num].label_name = curStr;
                
                ass->labels[ass->labels_num].label_address = line_num - ass->labels_num - blank_lines_counter;
                ass->labels_num++;
            }
        }
        else if( sscanf( curStr,"%s %s",command->cmdCode,trash ) == -1 )
            blank_lines_counter++;

        ass->cur_cmd = {};
        Com* command = &ass->cur_cmd;
        ERROR_PROCESSING(ass, ASMDump, ASMDtor, line_num)
	}
    free( trash );

	for ( int line_num = 1; line_num < ass->line_number + 1; line_num++ )
	{
        char* curStr = ass->linestr[line_num - 1];
		ReadLine( ass, curStr, command );
        ERROR_PROCESSING(ass, ASMDump, ASMDtor, line_num)
        ass->cur_cmd = {};
        Com* command = &ass->cur_cmd;
	}

    fwrite( ass->cmds, sizeof(CPU), ass->cmd_num, file );
    fclose( file );
}

int ReadFile( Asm* ass, FILE* file )
{
    ass->code_size = GetFileSize( file, SEEK_SET );
    ass->code = ( char* )calloc( ass->code_size + 1, sizeof( char ) );
    if ( !ass->code )
        return 0;
    
    fread( ass->code, sizeof( char ), ass->code_size, file );
    ass->code[ass->code_size] = '\0';

    ass->line_number = GetLineNumber( ass->code, ass->code_size );
    ass->linestr = ( char** ) calloc( ass->line_number, sizeof( char* ) );
    if ( !ass->linestr )
        return 0;

    CreateBufferOfLines( ass );

    ass->cmd_num = 0;
    ass->cmds = ( CPU* )calloc( ass->line_number, sizeof( CPU ) );
    if ( !ass->cmds )
        return 0;

    return 1;
}

int ASMCtor( Asm* ass, const char* file_name )
{
    if ( !ass )
		return ASM_PTR_NULL;

    ass->errors = NO_ERRORS;

    FILE* file = fopen( file_name, "rb" );
    if ( !file )
		SetErrorBit( &ass->errors, ASM_COMPILED_FILE_ERROR );

    ass->log =  fopen( "asmlog.txt","wb" );
    if ( !ass->log )
		SetErrorBit(&ass->errors, ASM_LOGER_ERROR);

    if( !ReadFile( ass,file ) )
        SetErrorBit( &ass->errors, ASM_BAD_TEXT_INFO );

    fclose( file );
	ERROR_PROCESSING( ass, ASMDump, ASMDtor, 0 )
    return ass->errors;
}

void ASMDump( Asm* ass, size_t line_num, FILE* logger )
{
	assert( ass != NULL );
	assert( logger != NULL );

	static size_t num_of_call = 1;

	fprintf( logger, 
		"=======================================\n"
		"ASM DUMP CALL #%zu\n"
		"Line: ", num_of_call );
	if (line_num == 0)
		fprintf( logger, "Before processing file\n" );
	else
		fprintf( logger, "%zu\n", line_num );

	if ( ass->errors )
	{
		fprintf( logger, "-------------ERRORS------------\n" );
		if ( ass->errors & ASM_PTR_NULL )
		{
			fprintf( logger, "ASM POINTER IS NULL\n" );
			return;
		}
		if ( ass->errors & ASM_BAD_TEXT_INFO ) fprintf( logger, "SOME TROUBLES WITH TEXT INFO STRUCT\n" );
		if ( ass->errors & ASM_LOGER_ERROR ) fprintf( logger, "ASM LOGGER ERROR\n" );
	    if ( ass->errors & ASM_COMPILED_FILE_ERROR ) fprintf( logger, "COMPILED FILE ERROR\n");
	    if ( ass->errors & ASM_INVALID_REG_OR_LABEL_NAME ) fprintf( logger, "Invalid reg or label name!\n" );
	    if ( ass->errors & ASM_TOO_MANY_ARGS ) fprintf( logger, "Too MANY args in command!\n" );
	    if ( ass->errors & ASM_TOO_FEW_ARGS ) fprintf( logger, "Too FEW args in command!\n" );	   
	    if ( ass->errors & ASM_POP_WITH_NUM ) fprintf( logger, "You just did Pop with num, are you crazy????\n" );

		fprintf( logger, "----------END_OF_ERRORS--------\n" );
	}
	else
		fprintf( logger, "------------NO_ERRORS----------\n" );
	fprintf( logger, "=======================================\n\n" );
	num_of_call++;
}


int ASMDtor( Asm* ass )
{
    if ( !ass )
		return ASM_PTR_NULL;
	if ( !ass->code || !ass->linestr || !ass->cmds )
    {
		SetErrorBit( &ass->errors, ASM_BAD_TEXT_INFO );
        return ASM_BAD_TEXT_INFO;
    }
    else
    {
        free( ass->code) ;
        free( ass->linestr );
        free( ass->cmds );
        free( ass->labels );
        ass->code_size = 0xB0BA;
        ass->line_number = 0xB1BA;
        ass->cmd_num = 0xB0BA;
        fclose( ass->log );
    }
    return NO_ERRORS;
}

int main( int argc, const char* argv[] )
{
    Asm ass = {};

    if ( argc == 1 )
    {
        ASMCtor( &ass, "quadratic_equation.txt" );
        ProcessingASM( &ass, "ass.txt" );
        ASMDtor( &ass );

    }
    else if ( argc == 3 )
    {
        ASMCtor( &ass, argv[1] );
        ProcessingASM( &ass, argv[2] );
        ASMDtor( &ass );
    }
    else
    {
        printf( "Invalid number of args to program" );
    }

}
