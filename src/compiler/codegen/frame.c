#include "codegen.h"

#include "utils/memory.h"

void stackAllocate(Scope* scope, int* v) {
	if (scope->isFuncScope) {
		for (int i = 0; i < scope->symbols.length; i++) {
			Symbol* symbol = (Symbol*)growableArrayGet(&scope->symbols, i);
			symbol->funcIndex = *v++ + i;
		}
	}
	for (int i = 0; i < scope->children.length; i++) {
		Scope** child = (Scope**)growableArrayGet(&scope->children, i);
		if ((*child)->isFuncScope) {
			int locals = 0;
			stackAllocate(*child, &locals);
		} else if (scope->isFuncScope) stackAllocate(*child, v);
	}
}