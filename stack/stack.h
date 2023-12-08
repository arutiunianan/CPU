#ifndef STACK_H
#define STACK_H

typedef double Elem_t;
#define ELEM_T_SPECIFIER "%d"

#ifdef _DEBUG

#include "hash.h"

enum StackErrorBits
{
	STACK_POINTER_NULL          = 1 << 0,
	STACK_DATA_POINTER_NULL     = 1 << 1,
	STACK_SIZE_GREATER_CAPACITY = 1 << 2,
	STACK_SIZE_LESS_ONE         = 1 << 3,
	STACK_LCANARY_DMG           = 1 << 4,
	STACK_RCANARY_DMG           = 1 << 5,
	STACK_DATA_LCANARY_DMG      = 1 << 6,
	STACK_DATA_RCANARY_DMG      = 1 << 7,
	STACK_HASH_MISMATCH         = 1 << 8,
	STACK_REALLOC_ERROR         = 1 << 9,
	STACK_BAD_CAPACITY          = 1 << 10,
	STACK_BAD_SIZE              = 1 << 11,
	STACK_DELETED               = 1 << 12
};

#define StackCtor(stack) _StackCtor ((stack), #stack, __FILE__, __LINE__, __func__)
const Elem_t POISON_ELEM = 666;

typedef unsigned long long Canary_t;

const Canary_t STACK_CANARY_NUM       = 0xAB0BA228;
const Canary_t STACK_DATA_CANARY_NUM  = 0xAB0BA322;

const size_t STACK_MAX_SIZE     = 10e15;
const size_t STACK_MAX_CAPACITY = 10e15;

struct StackCreationInfo
{
	size_t      line;
	const char* func;
	const char* var_name;
};
struct Stack
{
	Canary_t left_canary;
	
	size_t size;
	size_t capacity;
	Elem_t* data;

	unsigned errors;
	StackCreationInfo stack_creation_inf;
	Hash_t hash;

	Canary_t right_canary;
};

int StackCheckState(Stack* stk);

Hash_t StackHash(const Stack* stk);

int _StackCtor(Stack* stk, const char* var_name, const char* file_name, size_t line, const char* func_name);

#else
const int STK_ERROR = -1;

struct Stack
{
	int size;
	size_t capacity;

	Elem_t* data;
};

int StackCtor(Stack* stk);

#endif

int StackPop(Stack* stk, Elem_t* deleted_elem);

int StackPush(Stack* stk, Elem_t elem);

int StackDtor(Stack* stk);

#endif // STACK_H