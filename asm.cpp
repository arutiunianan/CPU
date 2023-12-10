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

int IsLabel( Asm* ass, Com* command, char* label )
{
    strcat(label, ":");
    for (size_t i = 0; i < ass->labels_num; i++)
	{        
		if(strcmp(label, ass->labels[i].label_name) == 0)
		{
			command->CPUcmdarg.arg = (double)ass->labels[i].label_address;
            
			return 1;
		}
	}
    return 0;
}

int IsReg( Com* command, char* reg )
{

    #define REG_DEF(reg_name, reg_cpu_code)                                \
	    if(strcmp(#reg_name, reg) == 0)     \
        {   \
            command->CPUcmdarg.arg = reg_cpu_code;   \
            return 1;   \
        }                                    
	#include "regs.h"
	#undef REG_DEF
    return 0;

}

int IsCommand( Com* command )
{
    #define DEF_CMD( name, cpu_code, args_num, ...)                      \
	    if ( strcmp(command->cmdCode, #name) == 0 )  \
        {  \
		    command->CPUcmdarg.cmd = (CPUCommand)cpu_code; \
            \
            return 1;  \
        }
            
    #include "cmds.h"
	#undef DEF_CMD
    //printf("%s\n",command->cmdCode);
    return 0;
}

void def( Asm* ass, Com* command, argType argtype )
{

    if(IsCommand(command))
    {
        SetCommandBitCode( &command->CPUcmdarg.cmd, argtype ); 
        //printf("%d %s\n\n",command->CPUcmdarg.cmd,command->cmdCode);
        ass->cmds[ass->cmdNum].cmd = command->CPUcmdarg.cmd;
    
        if( argtype != NOARG )
            ass->cmds[ass->cmdNum].arg = command->CPUcmdarg.arg;

        ass->cmdNum++;
    }

}

int ReadLine( Asm* ass, char* curStr, Com* command )
{

    command->cmdCode = ( char* )calloc( strlen(curStr), sizeof( char ) );
    char* str = ( char* )calloc( strlen(curStr), sizeof( char ) );
    if ( sscanf(curStr,"%s %lf",command->cmdCode,&command->CPUcmdarg.arg) == 2 )
        def(ass,command,IMM);
    else if( sscanf(curStr,"%s %s",command->cmdCode,str) == 2 )
    {
        
        if(IsReg(command,str)){
            def(ass,command,REG);}
        else if( IsLabel( ass, command, str ) )
            def(ass,command,LAB);
        //else
        //printf("    %s \n",command->cmdCode);
        //  error = Invalid arg
    }
    else if( sscanf(curStr,"%s %s",command->cmdCode,str) == 1  )
    {//printf("%s\n",command->cmdCode);
        
        if( command->cmdCode[strlen(command->cmdCode) - 1] != ':' )
            def(ass,command,NOARG);
    }
    else
    {
        //printf("%s\n",command->cmdCode);
    }
}

int ProcessingASM( Asm* ass, const char* equation )
{
    Com* command = &ass->curCmd;
    FILE *file = fopen( equation, "w" );

    ass->labels = ( Label* )calloc( ass->lineNumber, sizeof( Label ) );
    int blank_lines_counter = 0;
    char* p; 

    for ( int line_num = 1; line_num < ass->lineNumber + 1; line_num++ )
	{
        char* curStr = ass->linestr[line_num - 1];
        int cmdSize = strlen(curStr);
		command->cmdCode = ( char* )calloc( cmdSize, sizeof( char ) );
        char* trash = ( char* )calloc( cmdSize, sizeof( char ) );
                        
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
        else if(sscanf(curStr,"%s %s",command->cmdCode,trash) == -1)
            blank_lines_counter++;

        ass->curCmd = {};
        Com* command = &ass->curCmd;
	}

	for ( int line_num = 1; line_num < ass->lineNumber + 1; line_num++ )
	{
        
        char* curStr = ass->linestr[line_num - 1];
		ReadLine( ass, curStr, command );
        ass->curCmd = {};
        Com* command = &ass->curCmd;
	}

    fwrite(ass->cmds, sizeof(CPU), ass->cmdNum, file);
    //for(int i = 0; i < ass->labels_num; i++)
    //    printf("%s\n",ass->labels[i].label_name);

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
