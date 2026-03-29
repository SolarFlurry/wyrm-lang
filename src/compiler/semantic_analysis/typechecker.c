#include "semantic_analysis.h"

#include "../error/error.h"
#include <string.h>
#include <stdio.h>
#include <stdalign.h>

bool typeEquals(ExprNode* type, ExprNode* type2) {
	if (type == type2) {
		return true;
	}

	if (type == NULL || type2 == NULL) return true;

	if (type->kind != type2->kind) {
		return false;
	}

	switch (type->kind) {
		case NODE_EXPR_TYPE_NAMED: {
			return strcmp(type->data.type.named.name, type2->data.type.named.name) == 0;
		}
		case NODE_EXPR_TYPE_PTR: {
			return typeEquals(type->data.type.pointer.operand, type2->data.type.pointer.operand);
		}
		default: {
			// unreachable (assuming always pass in a valid type)
		}
	}

	return false;
}

void typecheckVarDec(ArenaAllocator* arena, DeclNode* decl, Scope* scope) {
	switch (decl->data.var.varType) {
		case VAR_LOCAL: case VAR_LOCAL_MUT: {
			Token* lvalue = decl->data.lvalue;

			char* name = createOwnedString(arena, lvalue->start, lvalue->length);

			AstNode* initialType = typecheckExpr(arena, decl->data.var.initial, scope);
			if (decl->data.var.type != NULL && !typeEquals(decl->data.var.type, initialType)) {
				errorFromCause("Specified type does not match type of initial", lvalue);
			}

			AstNode* type = initialType;
			
			Symbol* added = scopeAddSymbol(arena, scope, name, VAR_LOCAL, type, NULL);

			if (added == NULL) {
				errorFromCause("Redefinition of variable", lvalue);
				return;
			}
			added->funcIndex = scope->funcIndex++;
		} break;
		case VAR_CONST: {
			if (scope->isFuncScope) {
				errorFromCause("Cannot have a const binding in a function body", decl->token);
			}
		} break;
	}
}

void typecheckFuncDec(ArenaAllocator* arena, DeclNode* decl, Scope* scope) {
	if (scope->isFuncScope) {
		errorFromCause("Cannot have a func declaration in a function body", decl->token);
		return;
	}

	Scope* funcScope = ARENA_ALLOC(arena, Scope, 1);
	initScope(arena, funcScope, scope);
	funcScope->isFuncScope = true;

	for (int i = 0; i < decl->data.func.paramCount; i++) {
		Token* name = decl->data.func.paramNames[i];
		Symbol* added = scopeAddSymbol(
			arena,
			funcScope,
			createOwnedString(arena, name->start, name->length),
			VAR_LOCAL,
			decl->data.func.paramTypes[i],
			NULL
		);
	}

	decl->data.func.scope = funcScope;
	for (int i = 0; i < decl->data.func.stmtCount; i++) {
		typecheckStmt(arena, decl->data.func.stmts[i], funcScope);
	}
}

void typecheckStmt(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	switch (ast->kind) {
		case NODE_DECL_VAR: {
			typecheckVarDec(arena, ast, scope);
		} break;
		case NODE_DECL_FUNC: {
			typecheckFuncDec(arena, ast, scope);
		} break;
		default: {
			ExprNode* type = typecheckExpr(arena, ast, scope);
			ast->data.expr.unusedResult = false;
			if (type != NULL) {
				warn("Unused result of expression", ast->token->line, ast->token->col);
				ast->data.expr.unusedResult = true;
			}
		}
	}
}

void typeCheck(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	if (ast->kind != NODE_PROGRAM) {
		error("Type Checker: Expected a program node", 0, 0);
		return;
	}
	for (int i = 0; i < ast->data.program.declCount; i++) {
		typecheckStmt(arena, ast->data.program.decls[i], scope);
	}
}