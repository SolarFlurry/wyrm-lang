#include "lexer.h"

#include <ctype.h>
#include <string.h>

Lexer lx;

void initLexer(ArenaAllocator* arena, const char* program) {
	lx.current = program;
	lx.start = lx.current;
	lx.line = 0;
	lx.col = 0;
	lx.arena = arena;
}

static Token* makeToken(TokenType type) {
	Token* token = (Token*)arenaAlloc(lx.arena, sizeof(Token));
	token->type = type;
	token->start = lx.start;
	token->length = lx.current - lx.start;
	token->line = lx.line;
	token->col = lx.col - (lx.current - lx.start);
	return token;
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
	if (lx.current - lx.start == start + length &&
    	memcmp(lx.start + start, rest, length) == 0) {
    	return type;
	}

  	return TOK_IDENT;
}

static TokenType identifierType() {
	switch (lx.start[0]) {
		case 'l': return checkKeyword(1, 2, "et", TOK_KEYWORD_LET);
		case 'm': return checkKeyword(1, 2, "ut", TOK_KEYWORD_MUT);
		case 'i': return checkKeyword(1, 1, "f", TOK_KEYWORD_IF);
		case 'e': return checkKeyword(1, 3, "lse", TOK_KEYWORD_ELSE);
		case 'f': return checkKeyword(1, 3, "unc", TOK_KEYWORD_FUNC);
		case 's': return checkKeyword(1, 5, "truct", TOK_KEYWORD_STRUCT);
	}
	return TOK_IDENT;
}

bool match(char c) {
	bool equal = peek(0) == c;
	advance();
	return equal;
}

bool isDigit(char c) {
	return c >= '0' && c <= '9';
}
bool isAlpha(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}
bool isAlphaDigit(char c) {
	return isDigit(c) || isAlpha(c);
}

bool isEnd() {
	return *lx.current == '\0';
}
char peek(size_t offset) {
	return lx.current[offset];
}
void advance() {
	if (isEnd()) return;
	lx.current++;
	lx.col++;
}
void skipWhitespace() {
	while (true) {
		if (isEnd()) break;
		if (!isspace(peek(0))) break;
		if (peek(0) == '/' && peek(1) == '/') {
			while (peek(0) != '\n' && !isEnd()) advance();
		}
		if (peek(0) == '\n') {
			lx.line++;
			advance();
			lx.col = 0;
		} else {
			advance();
		}
	}
}
Token* nextToken() {
	skipWhitespace();
	if (isEnd()) {
		return makeToken(TOK_EOF);
	}
	lx.start = lx.current;
	if (isAlpha(peek(0))) {
		while (!isEnd() && isAlphaDigit(peek(0))) {
			advance();
		}
		return makeToken(identifierType());
	}
	if (isDigit(peek(0))) {
		while (!isEnd() && isDigit(peek(0))) {
			advance();
		}
		return makeToken(TOK_INT);
	}
	char c = peek(0);
	advance();
	switch (c) {
		case '+': return makeToken(TOK_PLUS);
		case '-': return makeToken(match('>') ? TOK_MINUS_RARROW : TOK_MINUS);
		case '*': return makeToken(TOK_ASTERISK);
		case '/': return makeToken(TOK_SLASH);
		case ';': return makeToken(TOK_SEMICOLON);
		case ':': return makeToken(match(':') ? TOK_COLON_COLON : TOK_COLON);
		case '.': return makeToken(match('.') ? TOK_DOT_DOT : TOK_DOT);
		case ',': return makeToken(TOK_COMMA);
		case '<': return makeToken(match('=') ? TOK_LARROW_EQ : TOK_LARROW);
		case '>': return makeToken(match('=') ? TOK_RARROW_EQ : TOK_RARROW);
		case '(': return makeToken(TOK_LPAREN);
		case ')': return makeToken(TOK_RPAREN);
		case '[': return makeToken(TOK_LBRACK);
		case ']': return makeToken(TOK_RBRACK);
		case '{': return makeToken(TOK_LBRACE);
		case '}': return makeToken(TOK_RBRACE);
		case '=': return makeToken(match('=') ? TOK_EQ_EQ : TOK_EQ);
		default: break;
	}
	return makeToken(TOK_UNKNOWN);
}