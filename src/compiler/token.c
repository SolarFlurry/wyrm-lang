#include "token.h"

const char* describeTokenType(TokenType type) {
	switch (type) {
		case TOK_EOF: return "EOF";
		case TOK_INT: return "integer literal";
		case TOK_IDENT: return "identifier";
		case TOK_KEYWORD_LET: return "keyword 'let'";
		case TOK_KEYWORD_MUT: return "keyword 'mut'";
		case TOK_KEYWORD_FUNC: return "keyword 'func'";
		case TOK_KEYWORD_STRUCT: return "keyword 'struct'";
		default: return "token";
	}
}