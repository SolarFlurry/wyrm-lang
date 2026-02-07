#pragma once

#include "utils/common.h"
#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct VM {
	Chunk* chunk;
	uint8_t* ip;
	Value stack[STACK_MAX];
	Value* stackTop;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

#ifdef __cplusplus
extern "C" {
#endif

void push(Value value);
Value pop();

void initVM();
void freeVM();
InterpretResult evaluate(Chunk* chunk);

#ifdef __cplusplus
}
#endif