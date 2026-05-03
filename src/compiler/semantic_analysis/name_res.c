#include "semantic_analysis.h"

#include "../error/error.h"
#include "compiler/compiler.h"
#include <stdio.h>
#include <stdalign.h>

static void resolveNode(ArenaAllocator* arena, DeclNode* decl, Scope* scope) {
	switch (decl->kind) {
		case NODE_DECL_VAR: {
			if (decl->data.var.varType != VAR_CONST) {
				errorFromCause("Cannot have let bindings at the global scope", decl->token);
				return;
			}
			
			Token lvalue = decl->data.lvalue;
			char* s = createOwnedString(arena, &getSource()[lvalue.start], lvalue.length);

			AstNode* initialType = typecheckExpr(arena, decl->data.var.initial, scope);
			if (decl->data.var.type && !typeEquals(decl->data.var.type, initialType)) {
				errorFromCause("Specified type does not match type of initial", lvalue);
			}

			bool result = scopeAddSymbol(arena, scope, s, VAR_CONST, decl->data.var.type, NULL);
			if (!result) {
				errorFromCause("Redefinition of symbol", lvalue);
				return;
			}
		} break;
		case NODE_DECL_FUNC: {
			Token lvalue = decl->data.lvalue;
			char* s = createOwnedString(arena, &getSource()[lvalue.start], lvalue.length);
			ExprNode* type = ARENA_ALLOC(arena, ExprNode, 1);
			type->kind = NODE_EXPR_TYPE_FUNC;
			type->data.type.func.paramCount = decl->data.func.paramCount;
			type->data.type.func.paramTypes = decl->data.func.paramTypes;
			type->data.type.func.returnType = decl->data.func.returnType;

			Symbol* result = scopeAddSymbol(arena, scope, s, VAR_CONST, type, NULL);
			if (result == NULL) {
				errorFromCause("Redefinition of symbol", lvalue);
				return;
			}
		} break;
		default: {
			errorFromCause("Only declarations are allowed at the global scope", decl->token);
		}
	}
}

void resolveNames(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	if (ast->kind != NODE_PROGRAM) {
		error("name resolution: Expected a program node", 0, 0, 0);
		return;
	}
	for (int i = 0; i < ast->data.program.declCount; i++) {
		resolveNode(arena, ast->data.program.decls[i], scope);
	}
}
