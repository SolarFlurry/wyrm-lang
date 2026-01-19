#pragma once

#include "../ast.h"
#include "symtable.h"

void resolveNames(ArenaAllocator* arena, ASTNode* ast, Scope* scope);