#pragma once

#include "utils/common.h"
#include "value.h"

typedef enum {
	OP_CONSTANT,
	OP_ADD,
	OP_SUBTRACT,
	OP_RETURN,
} Opcode;

typedef struct Chunk {
	uint8_t* bytecode;
	ValueBuffer constants;
	uint32_t* lines;
	int capacity;
	int length;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, uint32_t line);
uint8_t addConstant(Chunk* chunk, Value value);
void freeChunk(Chunk* chunk);