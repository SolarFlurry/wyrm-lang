#pragma once

#include "vm/chunk.h"

typedef struct {
	const char* source;
	const char* filename;
} Compiler;

#ifdef __cplusplus
extern "C" {
#endif

Chunk compile(const char* source, const char* filename);
const char* getSource();
const char* getFilename();

#ifdef __cplusplus
}
#endif