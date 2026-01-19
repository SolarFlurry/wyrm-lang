#include "symtable.h"

#include "../error/error.h"
#include <string.h>

void initScope(ArenaAllocator* arena, Scope* scope, Scope* parent) {
	scope->parent = parent;
	scope->depth = parent == NULL ? 0 : parent->depth + 1;

	scope->isFuncScope = false;

	scope->children = growableArrayCreate(arena, sizeof(Scope));
	scope->symbols = growableArrayCreate(arena, sizeof(Symbol));
}

Symbol* scopeLookup(Scope* scope, const char* name) {
	Scope* current = scope;
	while (current != NULL) {
		Symbol* symbol = scopeLookupCurrent(current, name);
		if (symbol != NULL) {
			return symbol;
		}
		current = current->parent;
	}
	return NULL;
}

Symbol* scopeLookupCurrent(Scope* scope, const char* name) {
	for (int i = 0; i < scope->symbols.length; i++) {
		Symbol* symbol = (Symbol*)scope->symbols.data + i * sizeof(Symbol);
		if (strcmp(name, symbol->name) == 0) {
			return symbol;
		}
	}
	return NULL;
}

bool scopeAddSymbol(ArenaAllocator* arena, Scope* scope, const char* name, VarDeclType varType, ASTNode* type) {
	Symbol* existing = scopeLookupCurrent(scope, name);
	if (existing != NULL) {
		return false;
	}

	Symbol* symbol = (Symbol*)growableArrayPush(&scope->symbols);

	symbol->name = name;
	symbol->type = type;
	symbol->varType = varType;

	return true;
}