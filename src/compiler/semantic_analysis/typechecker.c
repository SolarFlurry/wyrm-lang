#include "semantic_analysis.h"

#include "../error/error.h"
#include <string.h>
#include <stdio.h>

bool typeEquals(AstNode* type, AstNode* type2) {
	if (type == type2) {
		return true;
	}

	if (type->type != type2->type) {
		return false;
	}

	switch (type->type) {
		case NODE_TYPE_BASIC: {
			return strcmp(type->data.type.basic.name, type2->data.type.basic.name) == 0;
		}
		case NODE_TYPE_POINTER: {
			return typeEquals(type->data.type.pointer.pointee, type2->data.type.pointer.pointee);
		}
		default: {
			// unreachable (assuming always pass in a valid type)
		}
	}

	return false;
}

void typecheckVarDec(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	switch (ast->data.stmt.varDec.varType) {
		case VAR_LOCAL: case VAR_LOCAL_MUT: {
			Token* lvalue = ast->data.stmt.varDec.lvalue;

			char* name = createOwnedString(arena, lvalue->start, lvalue->length);

			AstNode* initialType = typecheckExpr(arena, ast->data.stmt.varDec.initial, scope);
			if (ast->data.stmt.varDec.type != NULL && !typeEquals(ast->data.stmt.varDec.type, initialType)) {
				errorFromCause("Specified type does not match type of initial", lvalue);
			}

			AstNode* type = initialType;
			
			bool added = scopeAddSymbol(arena, scope, name, VAR_LOCAL, type, NULL);

			if (!added) {
				errorFromCause("Redefinition of variable", lvalue);
				return;
			}
		} break;
		case VAR_CONST: {
			if (scope->isFuncScope) {
				errorFromCause("Cannot have a const binding in a function body", ast->token);
			}
		} break;
	}
}

void typecheckFuncDec(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	if (scope->isFuncScope) {
		errorFromCause("Cannot have a func declaration in a function body", ast->token);
		return;
	}

	Scope* funcScope = arenaAlloc(arena, sizeof(Scope));
	initScope(arena, funcScope, scope);
	funcScope->isFuncScope = true;
	ast->data.stmt.funcDec.scope = funcScope;
	for (int i = 0; i < ast->data.stmt.funcDec.stmtCount; i++) {
		typecheckStmt(arena, ast->data.stmt.funcDec.stmts[i], funcScope);
	}
}

void typecheckStmt(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	switch (ast->type) {
		case NODE_STMT_VARDEC: {
			typecheckVarDec(arena, ast, scope);
		} break;
		case NODE_STMT_FUNCDEC: {
			typecheckFuncDec(arena, ast, scope);
		} break;
		default: {
			AstNode* type = typecheckExpr(arena, ast, scope);
			if (type != NULL) {
				warn("Unused result of expression", ast->token->line, ast->token->col);
			}
		}
	}
}

void typeCheck(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	if (ast->type != NODE_STMT_PROGRAM) {
		error("Type Checker: Expected a program node", 0, 0);
		return;
	}
	for (int i = 0; i < ast->data.stmt.program.stmtCount; i++) {
		typecheckStmt(arena, ast->data.stmt.program.stmts[i], scope);
	}
}