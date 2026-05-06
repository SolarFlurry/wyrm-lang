#pragma once

#include "../ast.h"
#include "symtable.h"

bool typeEquals(ExprNode* type, ExprNode* type2);

void resolveNames(ArenaAllocator* arena, AstNode* ast, Scope* scope);
void typeCheck(ArenaAllocator* arena, AstNode* ast, Scope* scope);
void typecheckStmt(ArenaAllocator* arena, AstNode* ast, Scope* scope);
ExprNode* typecheckExpr(ArenaAllocator* arena, ExprNode* ast, Scope* scope);
