#pragma once

#include <string.h>

#include "utils/common.h"
#include "../token.h"
#include "../ast.h"
#include "../lexer/lexer.h"

#define MAKE_NODE(parser, type, nodeKind) \
	(type*)memcpy(ARENA_ALLOC(p->arena, type, 1), &(type){.kind = nodeKind, .token = p_lookahead(parser, 0)}, sizeof(type))

typedef struct Parser {
    Lexer lx;
    ArenaAllocator* arena;
} Parser;

typedef struct {
	int left;
	int right;
} BindingPower;

Parser p_init(Lexer lx, ArenaAllocator* arena);

void p_consume(Parser* p, TokenType type, const char* message);
void p_next(Parser* p);
Token p_lookahead(Parser* p, size_t offset);

AstNode* p_parse(Parser* p);

DeclNode* p_parseDecl(Parser* p);
AstNode* parseStatement(Parser* p);
DeclNode* parseVarDecl(Parser* p);
DeclNode* parseFuncDecl(Parser* p);
void parseParamList(
	Parser* p,
	GrowableArray* names,
	GrowableArray* types,
	TokenType endSymbol,
	bool canBeAuto
);

ExprNode* parseType(Parser* p);

void parseExpressionList(
	Parser* p,
	GrowableArray* list,
	TokenType endSymbol
);
ExprNode* parseExpression(Parser* p);
ExprNode* parseBlock(Parser* p);
ExprNode* parsePrimary(Parser* p);
