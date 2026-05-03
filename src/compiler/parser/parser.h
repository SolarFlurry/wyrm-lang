#pragma once

#include <string.h>

#include "utils/common.h"
#include "../token.h"
#include "../ast.h"
#include "../lexer/lexer.h"

#define MAKE_NODE(arena, type, nodeKind) \
	(type*)memcpy(ARENA_ALLOC(arena, type, 1), &(type){.kind = nodeKind, .token = lookahead(0)}, sizeof(type))

typedef struct {
	int left;
	int right;
} BindingPower;

void consume(TokenType type, const char* message);
void next();
Token lookahead(size_t offset);

AstNode* parse(ArenaAllocator* arena);

DeclNode* parseDecl(ArenaAllocator* arena);
AstNode* parseStatement(ArenaAllocator* arena);
DeclNode* parseVarDecl(ArenaAllocator* arena);
DeclNode* parseFuncDecl(ArenaAllocator* arena);
void parseParamList(
	ArenaAllocator* arena,
	GrowableArray* names,
	GrowableArray* types,
	TokenType endSymbol,
	bool canBeAuto
);

ExprNode* parseType(ArenaAllocator* arena);

void parseExpressionList(
	ArenaAllocator* arena,
	GrowableArray* list,
	TokenType endSymbol
);
ExprNode* parseExpression(ArenaAllocator* arena);
ExprNode* parseBlock(ArenaAllocator* arena);
ExprNode* parsePrimary(ArenaAllocator* arena);
