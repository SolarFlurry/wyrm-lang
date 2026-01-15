#pragma once

#include "utils/common.h"
#include "../token.h"
#include "../ast.h"
#include "../lexer/lexer.h"

void consume(TokenType type, const char* error);
Token* lookahead(size_t offset);

ASTNode* parse(ArenaAllocator* arena);
ASTNode* parseExpression(ArenaAllocator* arena);