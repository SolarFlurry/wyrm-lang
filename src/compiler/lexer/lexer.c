#include "lexer.h"

#include <ctype.h>
#include <string.h>
#include "../error/error.h"

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
		case 's': return checkKeyword(1, 5, "truct", TOK_KEYWORD_STRUCT);
		case 'w': return checkKeyword(1, 4, "hile", TOK_KEYWORD_WHILE);
		case 'r': return checkKeyword(1, 5, "eturn", TOK_KEYWORD_RETURN);
		case 'b': return checkKeyword(1, 4, "reak", TOK_KEYWORD_BREAK);
		case 'p': return checkKeyword(1, 2, "ub", TOK_KEYWORD_PUB);
		case 't': if (lx.current - lx.start > 1) {
			switch (lx.start[1]) {
				case 'e': return checkKeyword(2, 2, "st", TOK_KEYWORD_TEST);
				case 'h': return checkKeyword(2, 2, "en", TOK_KEYWORD_THEN);
				case 'r': return checkKeyword(2, 2, "ue", TOK_KEYWORD_TRUE);
			}
		} break;
		case 'c': if (lx.current - lx.start > 1 && lx.start[1] == 'o') {
			if (lx.current - lx.start > 2 && lx.start[2] == 'n') {
				if (lx.current - lx.start > 3) {
					switch (lx.start[3]) {
						case 's': return checkKeyword(4, 1, "t", TOK_KEYWORD_CONST);
						case 't': return checkKeyword(4, 4, "inue", TOK_KEYWORD_CONTINUE);
					}
				}
			}
		} break;
		case 'f': if (lx.current - lx.start > 1) {
			switch (lx.start[1]) {
				case 'u': return checkKeyword(2, 2, "nc", TOK_KEYWORD_FUNC);
				case 'o': return checkKeyword(2, 1, "r", TOK_KEYWORD_FOR);
				case 'a': return checkKeyword(2, 3, "lse", TOK_KEYWORD_FALSE);
			}
		} break;
	}
	return TOK_IDENT;
}

bool match(char c) {
	if (isEnd()) return false;
	if (*lx.current != c) return false;
	advance();
	return true;
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
	if (isEnd()) return '\0';
	return lx.current[offset];
}
void advance() {
	if (isEnd()) return;
	lx.current++;
	lx.col++;
}
void skipWhitespace() {
	while (true) {
		switch (peek(0)) {
			case ' ': case '\t': case '\r': {
				advance();
			} break;
			case '\n': {
				lx.line++;
				advance();
				lx.col = 0;
			} break;
			case '/': {
				if (peek(1) == '/') {
					while (peek(0) != '\n' && !isEnd()) advance();
				} else return;
			} break;
			default: return;
		}
	}
}
Token* nextToken() {
	skipWhitespace();
	if (isEnd()) {
		return makeToken(TOK_EOF);
	}
	lx.start = lx.current;
	char c = peek(0);
	if (isAlpha(c)) {
		while (!isEnd() && isAlphaDigit(peek(0))) {
			advance();
		}
		return makeToken(identifierType());
	}
	if (isDigit(c)) {
		while (!isEnd() && isDigit(peek(0))) {
			advance();
		}
		return makeToken(TOK_INT);
	}
	// if (c == '\'') {
	// 	advance();
	// 	if (peek(0) == '\'') {
	// 		error("Empty character literal", lx.line, lx.col);
	// 		return makeToken(TOK_CHAR);
	// 	}
	// 	char actualChar;
	// 	if (peek(0) == '\\') {
	// 		advance();
	// 		switch (peek(0)) {
	// 			case '0': 
	// 		}
	// 	} else {
	// 		actualChar = peek(0);
	// 	}
	// 	advance();
	// 	if (peek(0) != '\'') {
	// 		error("Unclosed character literal", lx.line, lx.col);
	// 	}
	// }
	if (c == '"') {
		advance();
		lx.start = lx.current;
		while (!isEnd() && peek(0) != '"') advance();
		Token* tok = makeToken(TOK_STRING);
		advance();
		return tok;
	}
	advance();
	switch (c) {
		case '+': return makeToken(TOK_PLUS);
		case '-': return makeToken(match('>') ? TOK_MINUS_RARROW : TOK_MINUS);
		case '*': return makeToken(TOK_ASTERISK);
		case '/': return makeToken(TOK_SLASH);
		case '\\': return makeToken(TOK_BACKSLASH);
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
		case '=': return makeToken(match('=') ? TOK_EQ_EQ : match('>') ? TOK_EQ_RARROW : TOK_EQ);
		case '@': return makeToken(TOK_AT);
		default: break;
	}
	error("Unexpected character", lx.line, lx.col-1);
	return makeToken(TOK_UNKNOWN);
}