#pragma once

#include "vm/chunk.h"
#include "compiler/ast.h"
#include "compiler/semantic_analysis/symtable.h"

void stackAllocate(Scope* scope, int* v);

Chunk generateBytecode(AstNode* ast);
void genExpression(Chunk* chunk, AstNode* expr);