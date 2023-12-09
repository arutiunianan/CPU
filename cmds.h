#define POP_STK( num ) StackPop( &cpu->stack, &num)
#define PUSH_STK( num ) StackPush( &cpu->stack, num)

//#define POP_STK ( arg ) StackPop( &cpu->stack, &arg)

DEF_CMD(HLT, 0, 0,
    {
        CPUDtor( cpu );
        printf( "end\n" );
        return 0;
    }
)

DEF_CMD(PUSH, 1, 1,
    {
        PUSH_STK(GetProperArgument( cpu ));
    }
)

DEF_CMD(POP, 2, 1,
    {
        Elem_t num = 0;
        POP_STK( num );
        SetReg( cpu, command->CPUcmdarg.arg, num );
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

DEF_CMD(OUT, 7, 0,
    {
        Elem_t num = 0;
        POP_STK(num);

        printf("%lf", num);
    }
)

DEF_CMD(IN, 9, 0,
    {
        Elem_t num = 0;
        printf("IN: ");
        scanf("%lf", &num);

        PUSH_STK(num);
    }
)

#undef DEF_CMD

#undef POP_STK
#undef PUSH_STK