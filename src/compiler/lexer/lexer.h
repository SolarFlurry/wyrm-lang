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

void initLexer(const char* source);

Token nextToken();
