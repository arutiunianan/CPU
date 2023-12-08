#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "io.h"

/*
*
* COMMON FUNCTIONS BLOCK
*
*/

int StackDataRealloc(Stack* stk, size_t new_capacity);

enum DataReallocAction
{
	INCREASE_DATA = 0,
	DECREASE_DATA = 1
};
int StackDataReallocIfNeeded(Stack* stk, DataReallocAction realloc_action)
{
	assert(stk != NULL);
	assert(stk->data != NULL);

	const int INCREASE_COEFF = 4;
	const int DECREASE_COEFF = 2;

	switch (realloc_action)
	{
	case INCREASE_DATA:
		if (stk->capacity <= stk->size)
			return StackDataRealloc(stk, stk->capacity * INCREASE_COEFF);
		break;
	case DECREASE_DATA:
		if (stk->capacity > (stk->size + 1) * DECREASE_COEFF)
			return StackDataRealloc(stk, stk->capacity / DECREASE_COEFF);
		break;
	}
	return 0;
}

int StackPushKernal(Stack* stk, Elem_t elem)
{
	assert(stk != NULL);
	assert(stk->data != NULL);

	int realloc_res = StackDataReallocIfNeeded(stk, INCREASE_DATA);

	stk->data[stk->size] = elem;
	stk->size++;

	return realloc_res;
}

int StackPopKernal(Stack* stk, Elem_t* deleted_elem)
{
	assert(stk != NULL);
	assert(stk->data != NULL);
	assert(deleted_elem != NULL);

	*deleted_elem = stk->data[stk->size - 1];
	stk->size--;

	int realloc_res = StackDataReallocIfNeeded(stk, DECREASE_DATA);

	return realloc_res;
}

int StackCtorKernal(Stack* stk)
{
	assert(stk != NULL);

	const size_t INITIAL_CAPACITY = 2;

	stk->data = NULL;
	stk->size = 0;
	stk->capacity = 0;

	int realloc_res = StackDataRealloc(stk, INITIAL_CAPACITY);

	return realloc_res;
}

void StackDtorKernal(Stack* stk)
{
	assert(stk != NULL);
	assert(stk->data != NULL);

	free(stk->data);

	stk->capacity = 0;
	stk->size = 0;
}

/*
*
* END OF COMMON FUNCTIONS BLOCK
*
*/


/*
* 
* DEBUG FUNCTIONS BLOCK
* 
*/
#ifdef _DEBUG

Hash_t StackHash(Stack* stk)
{
	if (!stk)
	{
		STACK_DUMP_TO_FILE(stk);
		return STACK_POINTER_NULL;
	}

	Hash_t old_hash = stk->hash;
	unsigned errors = stk->errors;

	stk->hash   = 0;
	stk->errors = 0;
	Hash_t new_hash = Hash(stk, sizeof(Stack)) + Hash(stk->data, stk->size * sizeof(Elem_t));
	
	stk->hash = old_hash;
	stk->errors = errors;

	return new_hash;
}

Canary_t* GetDataLeftCanaryPtr(Stack* stk)
{
	assert(stk       != NULL);
	assert(stk->data != NULL);
	return (Canary_t*)((char*)stk->data - sizeof(Canary_t));
}
Canary_t* GetDataRightCanaryPtr(Stack* stk)
{
	assert(stk       != NULL);
	assert(stk->data != NULL);
	return (Canary_t*)(stk->data + stk->capacity);
}

#define STACK_CHECK_ERROR(stk, condition, error)	\
	if(condition)									\
		SetErrorBit(&stk->errors, error);			\
	else											\
		UnsetErrorBit(&stk->errors, error);

int StackCheckState(Stack* stk)
{
	if (!stk)
		return STACK_POINTER_NULL;

	UnsetErrorBit(&stk->errors, STACK_SIZE_LESS_ONE); // NO NEED TO FIX THIS MISTAKE IF ITS NOT CALLED FROM POP

	STACK_CHECK_ERROR(stk, stk->size > stk->capacity, STACK_SIZE_GREATER_CAPACITY)
	STACK_CHECK_ERROR(stk, stk->left_canary  != STACK_CANARY_NUM, STACK_LCANARY_DMG)
	STACK_CHECK_ERROR(stk, stk->right_canary != STACK_CANARY_NUM, STACK_RCANARY_DMG)

	STACK_CHECK_ERROR(stk, stk->size >= STACK_MAX_SIZE, STACK_BAD_SIZE)
	STACK_CHECK_ERROR(stk, stk->capacity >= STACK_MAX_SIZE, STACK_BAD_CAPACITY)

	Hash_t old_hash = stk->hash;
	Hash_t new_hash = StackHash(stk);
	STACK_CHECK_ERROR(stk, old_hash != new_hash, STACK_HASH_MISMATCH)
		
	if (!stk->data) 
	{
		SetErrorBit(&stk->errors, STACK_DATA_POINTER_NULL);
		return stk->errors;
	}
	else 
		UnsetErrorBit(&stk->errors, STACK_DATA_POINTER_NULL);

	STACK_CHECK_ERROR(stk, *GetDataLeftCanaryPtr(stk) != STACK_DATA_CANARY_NUM, STACK_DATA_LCANARY_DMG)
	STACK_CHECK_ERROR(stk, *GetDataRightCanaryPtr(stk) != STACK_DATA_CANARY_NUM, STACK_DATA_RCANARY_DMG)

	return stk->errors;
}	


int StackCheckStateBeforePop(Stack* stk)
{
	if (StackCheckState(stk) && stk == NULL)
		return STACK_POINTER_NULL;

	if (stk->size < 1)
		SetErrorBit(&stk->errors, STACK_SIZE_LESS_ONE);

	return stk->errors;
}

int StackFillPoison(Stack* stk, size_t capacity)
{
	if (!stk)
	{
		STACK_DUMP_TO_FILE(stk);
		return STACK_POINTER_NULL;
	}

	for (size_t i = stk->size; i < capacity; i++)
		stk->data[i] = POISON_ELEM;

	return 0;
}

int StackDataRealloc(Stack* stk, size_t new_capacity)
{
	if (!stk)
	{
		STACK_DUMP_TO_FILE(stk);
		return STACK_POINTER_NULL;
	}

	if (stk->data != NULL) // IF NOT FIRST CALL
		stk->data = (Elem_t*)GetDataLeftCanaryPtr(stk);

	while ((new_capacity * sizeof(Elem_t)) % sizeof(Canary_t) != 0) // MEMORY ALIGNMENT
		new_capacity++;

	Elem_t* new_data = (Elem_t*)realloc(stk->data, new_capacity * sizeof(Elem_t) + 2 * sizeof(Canary_t));
	
	stk->capacity = new_capacity;

	if (!new_data)
	{
		UnsetErrorBit(&stk->errors, STACK_REALLOC_ERROR);
		return stk->errors; 
	}

	stk->data = new_data;
	stk->data = (Elem_t*)((char *)stk->data + sizeof(Canary_t)); // MOVING DATA PTR
	
	StackFillPoison(stk, new_capacity);

	*GetDataLeftCanaryPtr(stk)  = STACK_DATA_CANARY_NUM;
    *GetDataRightCanaryPtr(stk) = STACK_DATA_CANARY_NUM;

	return 0;
}

int StackPush(Stack* stk, Elem_t elem)
{
	int stack_state = StackCheckState(stk);
	STACK_DUMP_TO_FILE(stk);

	if (stk == NULL)
		return STACK_POINTER_NULL;
	else if (stack_state)
		return stk->errors;
	
	StackPushKernal(stk, elem);

	stk->hash = StackHash(stk);

	stack_state = StackCheckStateBeforePop(stk);
	STACK_DUMP_TO_FILE(stk);

	return stack_state;
}

int StackPop(Stack* stk, Elem_t* deleted_elem)
{
	int stack_state = StackCheckStateBeforePop(stk);
	STACK_DUMP_TO_FILE(stk);

	if (stk == NULL)
		return STACK_POINTER_NULL;
	else if (stack_state)
		return stk->errors;

	StackPopKernal(stk, deleted_elem);

	stk->data[stk->size] = POISON_ELEM;
	stk->hash = StackHash(stk);

	stack_state = StackCheckState(stk);
	STACK_DUMP_TO_FILE(stk);

	return stack_state;
}

extern FILE* g_log_file;

int _StackCtor(Stack* stk, const char* var_name, const char* file_name, size_t line, const char* func_name)
{
	if (!stk)
	{
		STACK_DUMP_TO_FILE(stk);
		return STACK_POINTER_NULL;
	}
	static char num_of_stk = '0';
	num_of_stk++;
	char logger_name[13] = {'s', 't', 'k', num_of_stk, '_', 'l', 'o', 'g', '.', 't', 'x', 't', '\0'};

	LogFileCtor(&g_log_file, logger_name);

	stk->stack_creation_inf.var_name = var_name;
	stk->stack_creation_inf.line = line;
	stk->stack_creation_inf.func = func_name;

	stk->left_canary  = STACK_CANARY_NUM;
	stk->right_canary = STACK_CANARY_NUM;

	StackCtorKernal(stk);
	stk->errors = 0;

	stk->hash = StackHash(stk);

	int stack_state = StackCheckState(stk);
	STACK_DUMP_TO_FILE(stk);

	return stack_state;
}

int StackDtor(Stack* stk)
{
	int stack_state = StackCheckState(stk);
	STACK_DUMP_TO_FILE(stk);

	LogFileDtor(g_log_file);

	if (stk == NULL)
		return STACK_POINTER_NULL;
	else if (stack_state)
		return stk->errors;

	stk->stack_creation_inf.line = 0;
	stk->stack_creation_inf.func = NULL;
	stk->stack_creation_inf.var_name = NULL;

	stk->data = (Elem_t*)GetDataLeftCanaryPtr(stk);
	stk->errors = STACK_POINTER_NULL | STACK_DATA_POINTER_NULL;

	StackDtorKernal(stk);
	return 0;
}

/*
*
* END OF DEBUG FUNCTIONS BLOCK
*
*/


/*
*
* RELEASE FUNCTIONS BLOCK
* 
*/

#else
int StackDataRealloc(Stack* stk, size_t new_capacity)
{
	Elem_t* new_data = (Elem_t*)realloc(stk->data, new_capacity * sizeof(Elem_t));

	stk->data = new_data;
	stk->capacity = new_capacity;

	return 0;
}

int StackPush(Stack* stk, Elem_t elem)
{
	StackPushKernal(stk, elem);

	return 0;
}

int StackPop(Stack* stk, Elem_t* deleted_elem)
{
	StackPopKernal(stk, deleted_elem);

	stk->data[stk->size] = 0;

	return 0;
}


int StackCtor(Stack* stk)
{
	StackCtorKernal(stk);

	return 0;
}


int StackDtor(Stack* stk)
{
	StackDtorKernal(stk);

	return 0;
}

/*
*
* END OF RELEASE FUNCTIONS BLOCK
*
*/
#endif