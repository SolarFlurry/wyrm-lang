#pragma once

#include "utils/common.h"
#include "utils/memory.h"
#include "../ast.h"

typedef struct AstNode AstNode;

typedef struct {
	const char* name;
	AstNode* type;
	VarDeclType varType;
} Symbol;

typedef struct Scope {
	struct Scope* parent;
	uint32_t depth;
	GrowableArray symbols;
	GrowableArray children;
	bool isFuncScope;
} Scope;

void initScope(ArenaAllocator* arena, Scope* scope, Scope* parent);
Symbol* scopeLookup(Scope* scope, const char* name);
Symbol* scopeLookupCurrent(Scope* scope, const char* name);
bool scopeAddSymbol(ArenaAllocator* arena, Scope* scope, const char* name, VarDeclType varType, AstNode* type);