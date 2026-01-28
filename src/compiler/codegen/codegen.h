#pragma once

#include "vm/chunk.h"
#include "compiler/ast.h"
#include "compiler/semantic_analysis/symtable.h"
#include "utils/memory.h"

typedef struct {
	Chunk* chunk;
	ArenaAllocator* arena;
	size_t mainLocation;
} CodegenContext;

void stackAllocate(Scope* scope, int* v);

Chunk generateBytecode(AstNode* ast, Scope* scope, ArenaAllocator* arena);
void genExpression(AstNode* expr, Scope* scope);
void genStmt(AstNode* stmt, Scope* scope);