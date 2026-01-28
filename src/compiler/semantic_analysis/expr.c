#include "semantic_analysis.h"

#include "../error/error.h"
#include <string.h>

AstNode* typecheckExpr(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	switch (ast->type) {
		case NODE_EXPR_LITERAL: {
			AstNode* node = arenaAlloc(arena, sizeof(AstNode));
			node->type = NODE_TYPE_BASIC;
			switch (ast->data.expr.literal.type) {
				case LIT_INT: node->data.type.basic.name = "i32"; break;
				case LIT_BOOL: node->data.type.basic.name = "bool"; break;
				default: {} // do nothing
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
		case NODE_EXPR_BINOP: {
			AstNode* lhsType = typecheckExpr(arena, ast->data.expr.binaryOp.lhs, scope);
			AstNode* rhsType = typecheckExpr(arena, ast->data.expr.binaryOp.rhs, scope);

			switch (ast->data.expr.binaryOp.category) {
				case BINOP_ACCESS: {
					return NULL;
				}
				case BINOP_BITWISE: {
					return NULL;
				}
				case BINOP_ARITHMETIC: {
					if (!typeEquals(lhsType, rhsType)) {
						errorFromCause("Operand types mismatch", ast->token);
						return NULL;
					}
					return lhsType;
				}
			}
		}
		case NODE_EXPR_IF: {
			AstNode* condType = typecheckExpr(arena, ast->data.expr.ifExpr.condition, scope);
			if (condType == NULL || condType->type != NODE_TYPE_BASIC || strcmp(condType->data.type.basic.name, "bool") != 0) {
				errorFromCause("Not a boolean", ast->data.expr.ifExpr.condition->token);
			}
			AstNode* trueType = typecheckExpr(arena, ast->data.expr.ifExpr.trueBranch, scope);
			if (trueType != NULL && ast->data.expr.ifExpr.falseBranch == NULL) {
				errorFromCause("If expression must have an else clause", ast->token);
				return NULL;
			}
			if (ast->data.expr.ifExpr.falseBranch == NULL) {
				return NULL;
			}
			AstNode* falseType = typecheckExpr(arena, ast->data.expr.ifExpr.falseBranch, scope);
			if (!typeEquals(trueType, falseType)) {
				errorFromCause("Return types of true and false branches do not match", ast->token);
				return NULL;
			}
			return trueType;
		}
		case NODE_EXPR_BLOCK: {
			Scope* blockScope = (Scope*)arenaAlloc(arena, sizeof(Scope));
			initScope(arena, blockScope, scope);
			ast->data.expr.block.scope = blockScope;
			for (int i = 0; i < ast->data.expr.block.stmtCount; i++) {
				typecheckStmt(arena, ast->data.expr.block.stmts[i], blockScope);
			}
			return NULL;
		}
		default: errorFromCause("cannot typecheck", ast->token);
	}
}