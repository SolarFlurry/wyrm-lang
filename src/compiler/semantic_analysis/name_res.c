#include "semantic_analysis.h"

#include "../error/error.h"

static void resolveNode(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	switch (ast->type) {
		case NODE_STMT_VARDEC: {
			if (ast->data.stmt.varDec.varType != VAR_CONST) {
				errorFromCause("Cannot have let bindings at the global scope", ast->token);
				return;
			}
			
			Token* lvalue = ast->data.stmt.varDec.lvalue;
			char* s = createOwnedString(arena, lvalue->start, lvalue->length);
			bool result = scopeAddSymbol(arena, scope, s, VAR_CONST, ast->data.stmt.varDec.type, NULL);
			if (!result) {
				errorFromCause("Redefinition of symbol", lvalue);
				return;
			}
		} break;
		case NODE_STMT_FUNCDEC: {
			Token* lvalue = ast->data.stmt.funcDec.lvalue;
			char* s = createOwnedString(arena, lvalue->start, lvalue->length);
			AstNode* type = arenaAlloc(arena, sizeof(AstNode*));
			type->type = NODE_TYPE_FUNCTION;
			type->data.type.function.paramCount = ast->data.stmt.funcDec.paramCount;
			type->data.type.function.paramTypes = ast->data.stmt.funcDec.paramTypes;
			type->data.type.function.returnType = ast->data.stmt.funcDec.returnType;

			bool result = scopeAddSymbol(arena, scope, s, VAR_CONST, type, NULL);
			if (!result) {
				errorFromCause("Redefinition of symbol", lvalue);
				return;
			}
		} break;
		default: {
			errorFromCause("Only declarations are allowed at the global scope", ast->token);
		}
	}
}

void resolveNames(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	if (ast->type != NODE_STMT_PROGRAM) {
		error("name resolution: Expected a program node", 0, 0);
		return;
	}
	for (int i = 0; i < ast->data.stmt.program.stmtCount; i++) {
		resolveNode(arena, ast->data.stmt.program.stmts[i], scope);
	}
}