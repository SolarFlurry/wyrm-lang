#pragma once

#include "utils/common.h"
#include "../token.h"
#include "../ast.h"
#include "../lexer/lexer.h"

typedef struct {
	int left;
	int right;
} BindingPower;

ASTNode* makeNode(ArenaAllocator* arena, NodeType type);

void consume(TokenType type, const char* message);
void next();
Token* lookahead(size_t offset);

ASTNode* parse(ArenaAllocator* arena);

ASTNode* parseStatement(ArenaAllocator* arena);
ASTNode* parseVarDecl(ArenaAllocator* arena);
ASTNode* parseFuncDecl(ArenaAllocator* arena);
void parseParamList(
	ArenaAllocator* arena,
	GrowableArray* names,
	GrowableArray* types,
	TokenType endSymbol,
	bool canBeAuto
);

ASTNode* parseType(ArenaAllocator* arena);

void parseExpressionList(
	ArenaAllocator* arena,
	GrowableArray* list,
	TokenType endSymbol
);
ASTNode* parseExpression(ArenaAllocator* arena);
ASTNode* parseBlock(ArenaAllocator* arena);
ASTNode* parseAtom(ArenaAllocator* arena);