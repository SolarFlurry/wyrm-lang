#pragma once

#include "vm/chunk.h"
#include "compiler/ast.h"

Chunk generateBytecode(ASTNode* ast);
void genExpression(Chunk* chunk, ASTNode* expr);