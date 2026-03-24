#pragma once

#include "vm/chunk.h"

typedef struct {
	const char* source;
	const char* filename;
} Compiler;

void compileCst(const char* source, const char* filename);
uint8_t compile(const char* source, const char* filename, Chunk* result);
const char* getSource();
const char* getFilename();