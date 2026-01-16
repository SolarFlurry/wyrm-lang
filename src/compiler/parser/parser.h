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
ASTNode* parseExpression(ArenaAllocator* arena);
ASTNode* parseAtom(ArenaAllocator* arena);