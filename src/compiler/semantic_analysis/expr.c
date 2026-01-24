#include "semantic_analysis.h"

#include "../error/error.h"

AstNode* typecheckExpr(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	switch (ast->type) {
		case NODE_EXPR_LITERAL: {
			AstNode* node = arenaAlloc(arena, sizeof(AstNode));
			node->type = NODE_TYPE_BASIC;
			switch (ast->data.expr.literal.type) {
				case LIT_INT: node->data.type.basic.name = "i32"; break;
				case LIT_BOOL: node->data.type.basic.name = "bool"; break;
				default: // do nothing
			}
			return node;
		}
		case NODE_EXPR_IDENT: {
			Symbol* symbol = scopeLookup(scope, ast->data.expr.ident.name);
			if (symbol == NULL) {
				errorFromCause("Use of undeclared identifier", ast->token);
				return NULL;
			}
			return symbol->type;
		}
	}
}