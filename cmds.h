#define POP_STK(num) StackPop(&cpu_file->stack, &num)
#define PUSH_STK(num) StackPush(&cpu_file->stack, num)

DEF_CMD(HLT, 0, 0,
    {
        printf("\nend\n");
        return 0;
    }
)

DEF_CMD(PUSH, 1, 1,
    {
        PUSH_STK(GetProperArgument(cpu_file, arg_type, command));
    }
)

DEF_CMD(POP, 2, 1,
    {
        Elem_t num = 0;
        POP_STK(num);
        SetReg(cpu_file, command->arg, num);
    }
)
 
DEF_CMD(ADD, 3, 0,
    {
        Elem_t val1 = 0;
        Elem_t val2 = 0;
        POP_STK(val1);
        POP_STK(val2);
        PUSH_STK(val1 + val2);
    }
)

DEF_CMD(MUL, 4, 0,
    {
        Elem_t val1 = 0;
        Elem_t val2 = 0;
        POP_STK(val1);
        POP_STK(val2);
        PUSH_STK(val1 * val2);
    }
)

DEF_CMD(SUB, 5, 0,
    {
        Elem_t val1 = 0;
        Elem_t val2 = 0;
        POP_STK(val1);
        POP_STK(val2);
        PUSH_STK(val2 - val1);
    }
)

DEF_CMD(DIV, 6, 0,
    {
        Elem_t val1 = 0;
        Elem_t val2 = 0;
        POP_STK(val1);
        POP_STK(val2);
        PUSH_STK(val2 / val1);
    }
)

DEF_CMD(SQRT, 7, 0,
    {
        Elem_t val = 0;
        POP_STK(val);
        PUSH_STK(sqrt(val));
    }
)

DEF_CMD(OUT, 9, 0,
    {
        Elem_t num = 0;
        POP_STK(num);

        printf("%lf", num);
    }
)

DEF_CMD(IN, 10, 0,
    {
        Elem_t num = 0;
        printf("IN: ");
        if( scanf("%lf", &num) != 1)
            SetErrorBit(&cpu_file->errors, CPU_WRONG_INPUT);

        PUSH_STK(num);
    }
)

DEF_CMD(JMP, 12, 1,
	cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
)

DEF_CMD(JA, 13, 1,
    {
        Elem_t num1 = {};
	    Elem_t num2 = {};
	    POP_STK(num1);
	    POP_STK(num2);
	    if(num1 > num2)
		    cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
    }
)

DEF_CMD(JB, 14, 1, 
    {
        Elem_t num1 = {};       
	    Elem_t num2 = {};	    
	    POP_STK(num1);			    
	    POP_STK(num2);			    
	    if(num1 < num2)      
		    cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
    }
)

DEF_CMD(JAE, 15, 1,
    {
        Elem_t num1 = {};       
	    Elem_t num2 = {};	    
	    POP_STK(num1);			    
	    POP_STK(num2);			    
	    if(num1 >= num2)      
		    cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
    }
)

DEF_CMD(JBE, 16, 1,
    {
        Elem_t num1 = {};       
	    Elem_t num2 = {};	    
	    POP_STK(num1);			    
	    POP_STK(num2);			    
	    if(num1 <= num2)      
		    cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
    }
)

DEF_CMD(JE, 17, 1,
    {
        Elem_t num1 = {};       
	    Elem_t num2 = {};	    
	    POP_STK(num1);			    
	    POP_STK(num2);			    
	    if(num1 == num2)
		    cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
    }
)

DEF_CMD(JNE, 18, 1,
    {
        Elem_t num1 = {};       
	    Elem_t num2 = {};	    
	    POP_STK(num1);			    
	    POP_STK(num2);			    
	    if(num1 != num2)      
		    cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
    }
)

DEF_CMD(CALL, 19, 1,
    {
	    PUSH_STK(cpu_file->current_line_num);
	    cpu_file->current_line_num = GetProperArgument(cpu_file, arg_type, command) - 1;
    }
)

DEF_CMD(RET, 20, 0,
	if (cpu_file->stack.size >= 1)
	{
		Elem_t ret_address = 0;
		POP_STK(ret_address);
		cpu_file->current_line_num = (int)ret_address;
	}
)

DEF_CMD(OUTC, 21, 0,
	if (cpu_file->stack.size >= 1)
	{
        Elem_t num_to_output = 0;
		POP_STK(num_to_output);
		printf("%c", (char)num_to_output);
	}
)

#undef DEF_CMD

#undef POP_STK
#undef PUSH_STK