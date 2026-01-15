#include "chunk.h"

#include "utils/memory.h"

void initChunk(Chunk* chunk) {
	initValueBuffer(&chunk->constants);
	chunk->bytecode = NULL;
	chunk->lines = NULL;
	chunk->capacity = 0;
	chunk->length = 0;
}

uint8_t addConstant(Chunk* chunk, Value constant) {
	writeValueBuffer(&chunk->constants, constant);
	return (uint8_t)chunk->constants.length - 1;
}

void writeChunk(Chunk* chunk, uint8_t byte, uint32_t line) {
	if (chunk->capacity >= chunk->length) {
		chunk->capacity = GROW_CAPACITY(chunk->capacity);
		chunk->bytecode = GROW_BUFFER(uint8_t, chunk->bytecode, chunk->capacity);
		chunk->lines = GROW_BUFFER(uint32_t, chunk->lines, chunk->capacity);
	}
	chunk->lines[chunk->length] = line;
	chunk->bytecode[chunk->length++] = byte;
}

void freeChunk(Chunk* chunk) {
	FREE_BUFFER(uint8_t, chunk->bytecode);
	FREE_BUFFER(uint32_t, chunk->lines);
	freeValueBuffer(&chunk->constants);
}