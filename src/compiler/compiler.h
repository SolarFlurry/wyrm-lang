#pragma once

#include "vm/chunk.h"

typedef struct {
	const char* source;
	const char* filename;
} Compiler;

Chunk compile(const char* source, const char* filename);
const char* getSource();
const char* getFilename();