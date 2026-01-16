#pragma once

#include "utils/common.h"

typedef enum {
	TOK_EOF,
	TOK_UNKNOWN,

	TOK_INT,
	TOK_IDENT,

	TOK_KEYWORD_LET,

	TOK_PLUS,
	TOK_MINUS,
	TOK_ASTERISK,
	TOK_SLASH,

	TOK_EQ,
	TOK_EQ_EQ,

	TOK_COLON,
	TOK_SEMICOLON,
	TOK_COMMA,
	TOK_DOT,

	TOK_LPAREN,
	TOK_RPAREN,
	TOK_LBRACK,
	TOK_RBRACK,
	TOK_LBRACE,
	TOK_RBRACE,
} TokenType;

typedef struct {
	TokenType type;
	char* start;
	size_t length;
	uint32_t line;
	uint32_t col;
} Token;