#pragma once

#include "utils/common.h"
#include "utils/memory.h"
#include "../token.h"

typedef struct Lexer {
	uint32_t index;
	uint32_t line;
	uint32_t col;
	const char* source;
} Lexer;

Lexer lx_init(const char* source);

Token lx_nextTok(Lexer* lx);
