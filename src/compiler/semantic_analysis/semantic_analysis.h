#pragma once

#include "../ast.h"
#include "symtable.h"

void resolveNames(ArenaAllocator* arena, AstNode* ast, Scope* scope);
void typeCheck(ArenaAllocator* arena, AstNode* ast, Scope* scope);
void typecheckStmt(ArenaAllocator* arena, AstNode* ast, Scope* scope);
AstNode* typecheckExpr(ArenaAllocator* arena, AstNode* ast, Scope* scope);