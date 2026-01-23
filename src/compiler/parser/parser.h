#pragma once

#include "utils/common.h"
#include "../token.h"
#include "../ast.h"
#include "../lexer/lexer.h"

typedef struct {
	int left;
	int right;
} BindingPower;

AstNode* makeNode(ArenaAllocator* arena, NodeType type);

void consume(TokenType type, const char* message);
void next();
Token* lookahead(size_t offset);

AstNode* parse(ArenaAllocator* arena);

AstNode* parseStatement(ArenaAllocator* arena);
AstNode* parseVarDecl(ArenaAllocator* arena);
AstNode* parseFuncDecl(ArenaAllocator* arena);
void parseParamList(
	ArenaAllocator* arena,
	GrowableArray* names,
	GrowableArray* types,
	TokenType endSymbol,
	bool canBeAuto
);

AstNode* parseType(ArenaAllocator* arena);

void parseExpressionList(
	ArenaAllocator* arena,
	GrowableArray* list,
	TokenType endSymbol
);
AstNode* parseExpression(ArenaAllocator* arena);
AstNode* parseBlock(ArenaAllocator* arena);
AstNode* parseAtom(ArenaAllocator* arena);