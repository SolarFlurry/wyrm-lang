#pragma once

#include "utils/common.h"
#include "value.h"

typedef enum {
	OP_CONSTANT,
	OP_POP,
	OP_GET_LOCAL,
	OP_ADD,
	OP_SUBTRACT,
	OP_JUMP,
	OP_JUMP_IF_ZERO,
	OP_CALL,
	OP_RETURN,
} Opcode;

typedef struct Chunk {
	uint8_t* bytecode;
	ValueBuffer constants;
	int capacity;
	int length;
} Chunk;

#ifdef __cplusplus
extern "C" {
#endif

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);
uint8_t addConstant(Chunk* chunk, Value value);
void freeChunk(Chunk* chunk);

#ifdef __cplusplus
}
#endif