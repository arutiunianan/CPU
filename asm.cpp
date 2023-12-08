#include "asm.h"

void SetCommandBitCode(CPUCommand* command_cpu_code, argType arg_type)
{
	*(char*)command_cpu_code |= (char)arg_type;
}

int TrySetArgCPUCode(Asm* ass, Com* command, int needed_args_num) 
{
    if( needed_args_num )
    {//printf("%d %c\n",strlen( command->cmdArg ),command->cmdArg[0]);
        if( StrToNum( command ) )
            SetCommandBitCode( &command->CPUcmdarg.cmd, IMM ); 
        else if( strlen( command->cmdArg ) == 2 && command->cmdArg[0] == 'r' && RegToNum( command->cmdArg[1] ) ) 
        {                           
            SetCommandBitCode( &command->CPUcmdarg.cmd, REG );    
            command->CPUcmdarg.arg = RegToNum( command->cmdArg[1] );
        }
        ass->cmds[ass->cmdNum].cmd = command->CPUcmdarg.cmd;
        ass->cmds[ass->cmdNum].arg = command->CPUcmdarg.arg;
    }
    else  
    {  
        ass->cmds[ass->cmdNum].cmd = command->CPUcmdarg.cmd;
    }  
	ass->cmdNum++;
	/*assert(assembler != NULL);
	assert(command != NULL);
	
	if (command->error > 0)
		return false;

	if (command->arguments_num > needed_args_num)
	{
		command->error = TOO_MANY_ARGS;
		return false;
	}
	else if (command->arguments_num < needed_args_num)
	{
		command->error = TOO_FEW_ARGS;
		return false;
	}

	if (command->arguments_num > 0)
		ResolveTypeOfCommandArg(assembler, command);

	return true;*/
}

enum Regs RegToNum( char c )
{
    switch ( c )
    {
        case 'a':
            return ra;
            break;
        case 'b':
            return rb;
            break;
        case 'c':
            return rc;
            break;
        case 'd':
            return rd;
            break;
        case 'x':
            return rx;
            break;
        
        default:
            return error;
    }
}

int GetFileSize(FILE *text, int start)
{
    fseek( text, 0, SEEK_END );
    int LetterNumber = ftell( text );
    fseek( text, 0, SEEK_SET );
    return LetterNumber;
}

static int GetLineNumber( char* code, int codeSize )
{
    int lineNumber = 1;
    for( size_t i = 0; i < codeSize; i++ )
    {
        if ( code[i] == '\n' )
            lineNumber++;

    }
    return lineNumber;

}

static void CreateBufferOfLines( Asm* ass )
{
    int line = 1;
    ass->linestr[0] = ass->code;
    for( int i = 0; i < ass->codeSize - 1; i++ )

        if( ass->code[i] == '\n' )
        {
            ass->linestr[line++] = &( ass->code[i+1] );

            ass->code[i] = '\0';
        }

}

int SkipNonSpaces(char* source_command_str, int begin)
{

	char tmp = source_command_str[begin];

	while ( tmp != ' ' && tmp != '\t' && tmp != '\0' )
	{
		begin++;
		tmp = source_command_str[begin];
	}

	return begin;
}

static int SkipSpaces( char* source_command_str, int begin )
{

	char tmp = source_command_str[begin];

	while ( ( tmp == ' ' || tmp == '\t' ) && tmp != '\0' ) 
	{ 
		begin++;
		tmp = source_command_str[begin];
	}
	
	return begin;
}

int ReadLine( char* curStr, Com* command )
{
    int begin = SkipSpaces(curStr, 0);
	int end = SkipNonSpaces(curStr, begin);
    command->cmdCode = ( char* )calloc( end - begin, sizeof( char ) );
    strncpy(command->cmdCode, curStr + begin, end - begin);

    //int arg = 0;
    //CPU cmd = {};
    //char* cmd = ( char* )calloc( 10000, sizeof( char ) );


    //printf("%s\n",curStr);
    //sscanf(curStr,"%s %a",cmd,&arg);
    //printf("%d\n",arg);


	while ( curStr[end] != '\0' )
	{
        
	    begin = SkipSpaces(curStr, end);
		end = SkipNonSpaces(curStr, begin);

        command->cmdArg = ( char* )calloc( end - begin, sizeof( char ) );
		strncat( command->cmdArg, curStr + begin, end - begin );
        //printf("%s\n",command->cmdArg);
        
	}
}

int StrToNum(  Com* command )
{

	int isNegative = 0;
	int charNum = 0;
	Elem_t num = 0;
	if ( command->cmdArg[0] == '-' )
	{
		isNegative = 1;
		charNum = 1;
	}

    int strLen = strlen( command->cmdArg );
    //printf("%d\n",strLen);
    for ( ;charNum < strLen; charNum++ )
	{
		if( !isdigit( command->cmdArg[charNum] ) )
		{
			return 0;
		}
		num += ( command->cmdArg[charNum] - '0' ) * pow( 10, strLen - charNum - 1 );  
        //printf(" %d\n",1 * pow( 10, strLen - charNum - 1 )  );
	}
	if( isNegative )
    {
        command->CPUcmdarg.arg = -num;
		return 1;
    }
	command->CPUcmdarg.arg = num;
    return 1;
}

int ProcessingASM( Asm* ass, const char* equation )
{
    Com* command = &ass->curCmd;
    FILE *file = fopen( equation, "w" );

	for ( int line_num = 1; line_num < ass->lineNumber + 1; line_num++ )
	{
        char* curStr = ass->linestr[line_num - 1];
		ReadLine( curStr, command );
        
        #define DEF_CMD( name, cpu_code, args_num, ...)                      \
	    if ( strcmp(command->cmdCode, #name) == 0 )  \
	    {                                                                      \
		    command->CPUcmdarg.cmd = (CPUCommand)cpu_code;                \
            if(TrySetArgCPUCode( ass,command, args_num )) \
                int is_command_args_valid = 1;\
	    }
        #include "commands.h"
		#undef DEF_CMD
        ass->curCmd = {};
        Com* command = &ass->curCmd;
	}

    fwrite(ass->cmds, sizeof(CPU), ass->cmdNum, file);

}

int ASMCtor( Asm* ass, const char* equation )
{
    FILE* file = fopen( equation, "rb" );
    //ошибка открытия

    //fread( &cpu->version, sizeof( char ), sizeof( cpu->version ), file );
    //ошибка файл маленького размера
    ass->codeSize = GetFileSize( file, SEEK_SET );
    ass->code = ( char* )calloc( ass->codeSize + 1, sizeof( char ) );
    fread( ass->code, sizeof( char ), ass->codeSize, file );
    ass->code[ass->codeSize] = '\0';

    ass->lineNumber = GetLineNumber( ass->code, ass->codeSize );
    ass->linestr = ( char** ) calloc( ass->lineNumber, sizeof( char* ) );
    CreateBufferOfLines( ass );
    

    ass->cmdNum = 0;
    ass->cmds = ( CPU* )calloc( ass->lineNumber, sizeof( CPU ) );

    return 0;

}

int ASMDtor( Asm* ass )
{/*
    ass->codeSize = 0xB0BA;
    ass->lineNumber = 0xB1BA;

    free(ass->code);
    ass->code = NULL;

    free(ass->lines);
    ass->lines = NULL;*/

    return 0;

}

int main( int argc, const char* argv[] )
{
    Asm ass = {};

    if ( argc == 1 )
    {
        ASMCtor( &ass, "quadratic_equation.txt" );
        ProcessingASM( &ass, "ass.txt" );

    }
    else if ( argc == 3 )
    {
        ASMCtor( &ass, argv[1] );
    }
    else
    {
        printf( "Invalid number of args to program" );
    }

}
