#pragma once

#include "utils/common.h"

typedef enum {
	TOK_EOF,
	TOK_UNKNOWN,

	TOK_INT,
	TOK_IDENT,

	TOK_PLUS,
	TOK_MINUS,
} TokenType;

typedef struct {
	TokenType type;
	char* start;
	size_t length;
	uint32_t line;
} Token;