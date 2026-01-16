#include "codegen.h"

#include "compiler/error/error.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Chunk generateBytecode(ASTNode* ast) {
	if (ast->type != NODE_STMT_PROGRAM) {
		error("Argument to bytecode not a program", 0);
	}

	Chunk bytecode;
	initChunk(&bytecode);

	for (int i = 0; i < ast->data.stmt.program.stmtCount; i++) {
		genExpression(&bytecode, ast->data.stmt.program.stmts[i]);
	}

	writeChunk(&bytecode, OP_RETURN, 0);

	return bytecode;
}

void genExpression(Chunk* chunk, ASTNode* expr) {
	switch (expr->type) {
		case NODE_EXPR_LITERAL: {
			switch (expr->data.expr.literal.type) {
				case LIT_INT: {
					char* string = (char*)malloc(expr->token->length + 1);
					memcpy(string, expr->token->start, expr->token->length);
					string[expr->token->length] = '\0';

					int value = atoi(string);

					free(string);

					uint8_t constant = addConstant(chunk, (Value){.as = {.i32 = value}});

					writeChunk(chunk, OP_CONSTANT, expr->token->line);
					writeChunk(chunk, constant, expr->token->line);
				} break;
				default: {
					error("this literal type is not supported yet", expr->token->line);
					return;
				}
			}
		} break;
		case NODE_EXPR_BINOP: {
			genExpression(chunk, expr->data.expr.binaryOp.rhs);
			genExpression(chunk, expr->data.expr.binaryOp.lhs);
			switch (expr->data.expr.binaryOp.op) {
				case BINOP_ADD: {
					writeChunk(chunk, OP_ADD, expr->token->line);
				} break;
				case BINOP_SUBTRACT: {
					writeChunk(chunk, OP_SUBTRACT, expr->token->line);
				} break;
				default: {
					error("this operator is not yet supported", expr->token->line);
				}
			}
		} break;
		case NODE_EXPR_IDENT: {
			error("Identifiers are not supported yet", expr->token->line);
			return;
		}
		default: {
			error("not an expression", expr->token->line);
			return;
		}
	}
}