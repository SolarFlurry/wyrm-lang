#pragma once

#include "../ast.h"
#include "symtable.h"

void resolveNames(ArenaAllocator* arena, AstNode* ast, Scope* scope);
void typeCheck(ArenaAllocator* arena, AstNode* ast, Scope* scope);