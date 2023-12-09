#include "asm.h"

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

void def( Asm* ass, Com* command, argType argtype, char* reg )
{

    #define DEF_CMD( name, cpu_code, args_num, ...)                      \
	if ( strcmp(command->cmdCode, #name) == 0 )  \
		command->CPUcmdarg.cmd = (CPUCommand)cpu_code;
    #include "cmds.h"
	#undef DEF_CMD

    SetCommandBitCode( &command->CPUcmdarg.cmd, argtype ); 
    ass->cmds[ass->cmdNum].cmd = command->CPUcmdarg.cmd;
    
    if( argtype != NOARG )
    {
        if( argtype == REG )
        {
            #define REG_DEF(reg_name, reg_cpu_code)                                \
	            if(strcmp(#reg_name, reg) == 0)     \
		        command->CPUcmdarg.arg = reg_cpu_code;                                       
	        #include "regs.h"
	        #undef REG_DEF
        }
        ass->cmds[ass->cmdNum].arg = command->CPUcmdarg.arg;
    }

    ass->cmdNum++;

}

int ReadLine( Asm* ass, char* curStr, Com* command )
{

    command->cmdCode = ( char* )calloc( strlen(curStr), sizeof( char ) );
    char* reg = ( char* )calloc( strlen(curStr), sizeof( char ) );
    if ( sscanf(curStr,"%s %lf",command->cmdCode,&command->CPUcmdarg.arg) == 2 )
    {
        //def(ass,command,IMM,reg);
        #define DEF_CMD( name, cpu_code, args_num, ...)                      \
	    if ( strcmp(command->cmdCode, #name) == 0 )  \
		    command->CPUcmdarg.cmd = (CPUCommand)cpu_code;
        #include "cmds.h"
		#undef DEF_CMD
        SetCommandBitCode( &command->CPUcmdarg.cmd, IMM ); 
        ass->cmds[ass->cmdNum].cmd = command->CPUcmdarg.cmd;
        ass->cmds[ass->cmdNum].arg = command->CPUcmdarg.arg;
        ass->cmdNum++;
    }
    else if( sscanf(curStr,"%s %s",command->cmdCode,reg) == 2 )
    {//def(ass,command,REG,reg);
        
        #define DEF_CMD( name, cpu_code, args_num, ...)                      \
	    if ( strcmp(command->cmdCode, #name) == 0 )  \
		    command->CPUcmdarg.cmd = (CPUCommand)cpu_code;
        #include "cmds.h"
		#undef DEF_CMD

        #define REG_DEF(reg_name, reg_cpu_code)                                \
	    if(strcmp(#reg_name, reg) == 0)     \
		    command->CPUcmdarg.arg = reg_cpu_code;                                       
	    #include "regs.h"
	    #undef REG_DEF

        SetCommandBitCode( &command->CPUcmdarg.cmd, REG );    
        ass->cmds[ass->cmdNum].cmd = command->CPUcmdarg.cmd;
        ass->cmds[ass->cmdNum].arg = command->CPUcmdarg.arg;
        ass->cmdNum++;
        
    }
    else
    {//def(ass,command,NOARG,reg);
        
        #define DEF_CMD( name, cpu_code, args_num, ...)                      \
	    if ( strcmp(command->cmdCode, #name) == 0 )  \
		    command->CPUcmdarg.cmd = (CPUCommand)cpu_code;
        #include "cmds.h"
		#undef DEF_CMD
        ass->cmds[ass->cmdNum].cmd = command->CPUcmdarg.cmd;
        ass->cmdNum++;
        
    }
}

int ProcessingASM( Asm* ass, const char* equation )
{
    Com* command = &ass->curCmd;
    FILE *file = fopen( equation, "w" );

	for ( int line_num = 1; line_num < ass->lineNumber + 1; line_num++ )
	{
        char* curStr = ass->linestr[line_num - 1];
		ReadLine( ass, curStr, command );
        ass->curCmd = {};
        Com* command = &ass->curCmd;
	}

    fwrite(ass->cmds, sizeof(CPU), ass->cmdNum, file);
    for( int i = 0; i < ass->cmdNum; i++)
        printf("%d  %lf\n",ass->cmds[i].cmd,ass->cmds[i].arg);

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
