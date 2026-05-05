#include "semantic_analysis.h"

#include "../error/error.h"
#include <string.h>
#include <stdio.h>
#include <stdalign.h>

ExprNode* typecheckExpr(ArenaAllocator* arena, ExprNode* expr, Scope* scope) {
	switch (expr->kind) {
		case NODE_EXPR_LITERAL: {
			ExprNode* node = ARENA_ALLOC(arena, ExprNode, 1);
			node->kind = NODE_EXPR_TYPE_NAMED;
			switch (expr->data.literal.type) {
				case LIT_INT: node->data.type.named.name = "i32"; break;
				case LIT_BOOL: node->data.type.named.name = "bool"; break;
				default: {} // do nothing
			}
			return node;
		}
		case NODE_EXPR_IDENT: {
			Symbol* symbol = scopeLookup(scope, expr->data.ident.name);
			if (symbol == NULL) {
				errorFromCause("Use of undeclared identifier", expr->token);
				return NULL;
			}
			return symbol->type;
		}
		case NODE_EXPR_BINOP: {
			AstNode* lhsType = typecheckExpr(arena, expr->data.binaryOp.lhs, scope);
			AstNode* rhsType = typecheckExpr(arena, expr->data.binaryOp.rhs, scope);

			switch (expr->data.binaryOp.category) {
				case BINOP_ACCESS: {
					return NULL;
				}
				case BINOP_BITWISE: {
					return NULL;
				}
				case BINOP_ARITHMETIC: {
					if (!typeEquals(lhsType, rhsType)) {
						errorFromCause("Operand types mismatch", expr->token);
						return NULL;
					}
					return lhsType;
				}
				case BINOP_COMPARISON: {
					if (!typeEquals(lhsType, rhsType)) {
						errorFromCause("Operand types mismatch", expr->token);
						return NULL;
					}
					ExprNode* type = ARENA_ALLOC(arena, ExprNode, 1);
					type->kind = NODE_EXPR_TYPE_NAMED;
					type->data.type.named.name = "bool";
					return type;
				}
				default: {
					errorFromCause("Unknown binop category", expr->token);
					return NULL;
				}
			}
		}
		case NODE_EXPR_UNOP: {
			ExprNode* operandType = typecheckExpr(arena, expr->data.unaryOp.operand, scope);

			ExprNode* type = ARENA_ALLOC(arena, ExprNode, 1);

			switch (expr->data.unaryOp.op) {
				case '+': case '-': {
					type->kind = NODE_EXPR_TYPE_NAMED;
					type->data.type.named.name = "i32";
					if (!typeEquals(operandType, type)) {
						errorFromCause("Expected an integer", expr->token);
						return NULL;
					}
					return operandType;
				}
				case '!': {
					type->kind = NODE_EXPR_TYPE_NAMED;
					type->data.type.named.name = "bool";
					if (!typeEquals(operandType, type)) {
						errorFromCause("Expected a boolean", expr->token);
						return NULL;
					}
					return  operandType;
				}
				case '*': {
					type->kind = NODE_EXPR_TYPE_PTR;
					type->data = (Expr){
						.type.pointer.isMut = false,
						.type.pointer.isSlice = false,
						.type.pointer.operand = operandType,
					};
					return type;
				}
				default: {
					errorFromCause("Unknown unary operator", expr->token);
					return NULL;
				}
			}
		}
		case NODE_EXPR_IF: {
			ExprNode* condType = typecheckExpr(arena, expr->data.ifExpr.condition, scope);
			if (condType == NULL || condType->kind != NODE_EXPR_TYPE_NAMED || strcmp(condType->data.type.named.name, "bool") != 0) {
				errorFromCause("Not a boolean", expr->data.ifExpr.condition->token);
			}
			ExprNode* trueType = typecheckExpr(arena, expr->data.ifExpr.trueBranch, scope);
			if (trueType != NULL && expr->data.ifExpr.falseBranch == NULL) {
				errorFromCause("If expression must have an else clause", expr->token);
				return NULL;
			}
			if (expr->data.ifExpr.falseBranch == NULL) {
				return NULL;
			}
			ExprNode* falseType = typecheckExpr(arena, expr->data.ifExpr.falseBranch, scope);
			if (!typeEquals(trueType, falseType)) {
				errorFromCause("Return types of true and false branches do not match", expr->token);
				return NULL;
			}
			return trueType;
		}
		case NODE_EXPR_BLOCK: {
			Scope* blockScope = ARENA_ALLOC(arena, Scope, 1);
			initScope(arena, blockScope, scope);
			expr->data.block.scope = blockScope;
			for (int i = 0; i < expr->data.block.stmtCount; i++) {
				typecheckStmt(arena, expr->data.block.stmts[i], blockScope);
			}
			return NULL;
		}
		case NODE_EXPR_CALL: {
			ExprNode* funcType = typecheckExpr(arena, expr->data.funcCall.func, scope);
			if (funcType == NULL) return NULL;
			if (funcType->kind != NODE_EXPR_TYPE_FUNC) {
				errorFromCause("Cannot call on a non-function value", expr->token);
				return NULL;
			}
			if (funcType->data.type.func.paramCount != expr->data.funcCall.argsCount) {
				errorFromCause("Unexpected number of arguments", expr->token);
				return NULL;
			}
			for (int i = 0; i < expr->data.funcCall.argsCount; i++) {
				ExprNode* arg = expr->data.funcCall.args[i];
				ExprNode* paramType = typecheckExpr(arena, arg, scope);
				if (!typeEquals(paramType, funcType->data.type.func.paramTypes[i])) {
					errorFromCause("Type mismatch", arg->token);
				}
			}

			return funcType->data.type.func.returnType;
		}
		case NODE_EXPR_EXIT: {
			typecheckExpr(arena, expr->data.exit.exitExpr, scope);
			return NULL;
		}

		default: {
            printf("cannot typecheck error! node type %d, %u %u %u\n", expr->kind, expr->token.start, expr->token.length, expr->token.type);
			errorFromCause("cannot typecheck", expr->token);
		}
	}
}
