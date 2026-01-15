#pragma once

#include "utils/common.h"
#include "utils/memory.h"
#include "../token.h"

typedef struct {
	char* current;
	char* start;
	uint32_t line;
	ArenaAllocator* arena;
} Lexer;

void initLexer(ArenaAllocator* arena, char* program);

bool isEnd();
char peek(size_t offset);
void advance();
void skipWhitespace();
Token* nextToken();