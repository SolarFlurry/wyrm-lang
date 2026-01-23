#include "ast.h"

#include <stdio.h>

void printAST(AstNode* ast, int indent) {
	for (int i = 0; i < indent; i++) { printf("  "); }
	printf("└ \x1b[36m");

	switch (ast->type) {
		case NODE_STMT_PROGRAM: {
			printf("Program\x1b[0m:\n");
			for (int i = 0; i < ast->data.stmt.program.stmtCount; i++) {
				printAST(ast->data.stmt.program.stmts[i], indent + 1);
			}
		} break;
		case NODE_STMT_VARDEC: {
			printf("VarDecl\x1b[0m => \x1b[35m'");
			for (int i = 0; i < ast->data.stmt.varDec.lvalue->length; i++) {
				putchar(ast->data.stmt.varDec.lvalue->start[i]);
			}
			puts("'\x1b[0m");
			if (ast->data.stmt.varDec.type != NULL) {
				printAST(ast->data.stmt.varDec.type, indent + 1);
			}
			printAST(ast->data.stmt.varDec.initial, indent + 1);
		} break;
		case NODE_EXPR_IDENT: {
			printf("Identifier\x1b[0m => \x1b[35m'");
			for (int i = 0; i < ast->token->length; i++) {
				putchar(ast->token->start[i]);
			}
			printf("'\x1b[0m\n");
		} break;
		case NODE_EXPR_LITERAL: {
			printf("Literal\x1b[0m => \x1b[31m");
			switch (ast->data.expr.literal.type) {
				case LIT_INT: {
					printf("i32");
				} break;
				default: {} // do nothing
			}
			printf("\x1b[0m \x1b[33m'");
			for (int i = 0; i < ast->token->length; i++) {
				putchar(ast->token->start[i]);
			}
			printf("'\x1b[0m\n");
		} break;
		case NODE_EXPR_BINOP: {
			printf("BinaryOp\x1b[0m '");
			for (int i = 0; i < ast->token->length; i++) {
				putchar(ast->token->start[i]);
			}
			printf("'\n");
			printAST(ast->data.expr.binaryOp.lhs, indent + 1);
			printAST(ast->data.expr.binaryOp.rhs, indent + 1);
		} break;
		default: {
			printf("Unknown ASTNode type...\x1b[0m\n");
		}
	}
}