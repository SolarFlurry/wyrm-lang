#pragma once

#include "utils/common.h"
#include "utils/memory.h"
#include "../token.h"

typedef struct {
	const char* current;
	const char* start;
	uint32_t line;
	uint32_t col;
	ArenaAllocator* arena;
} Lexer;

void initLexer(ArenaAllocator* arena, const char* program);

bool isEnd();
char peek(size_t offset);
void advance();
void skipWhitespace();
Token* nextToken();