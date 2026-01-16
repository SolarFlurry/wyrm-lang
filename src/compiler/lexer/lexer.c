#include "lexer.h"

#include <ctype.h>

Lexer lx;

void initLexer(ArenaAllocator* arena, char* program) {
	lx.current = program;
	lx.start = lx.current;
	lx.line = 0;
	lx.arena = arena;
}

static Token* makeToken(TokenType type) {
	Token* token = (Token*)arenaAlloc(lx.arena, sizeof(Token));
	token->type = type;
	token->start = lx.start;
	token->length = lx.current - lx.start;
	token->line = lx.line;
	return token;
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
}
void skipWhitespace() {
	while (true) {
		if (isEnd()) break;
		if (!isspace(peek(0))) break;
		if (peek(0) == '/' && peek(1) == '/') {
			while (peek(0) != '\n' && !isEnd()) advance();
		}
		if (peek(0) == '\n') lx.line++;
		advance();
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
		return makeToken(TOK_IDENT);
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
		case '-': return makeToken(TOK_MINUS);
		case '*': return makeToken(TOK_ASTERISK);
		case '/': return makeToken(TOK_SLASH);
		case '(': return makeToken(TOK_LPAREN);
		case ')': return makeToken(TOK_RPAREN);
		default: break;
	}
	return makeToken(TOK_UNKNOWN);
}