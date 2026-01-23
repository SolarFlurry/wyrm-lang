#pragma once

#include "vm/chunk.h"
#include "compiler/ast.h"

Chunk generateBytecode(AstNode* ast);
void genExpression(Chunk* chunk, AstNode* expr);