#include "codegen.h"

#include "compiler/error/error.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

CodegenContext ctx;

static void emitByte(uint8_t byte) {
		writeChunk(ctx.chunk, byte);
	}

static int emitJump(uint8_t instruction) {
	emitByte(instruction);
	emitByte(0xff);
	emitByte(0xff);
	return ctx.chunk->length - 2;
}

static void patchJump(int offset, int with) {
	int jump = with - offset - 2;
	ctx.chunk->bytecode[offset] = (jump >> 8) & 0xff;
	ctx.chunk->bytecode[offset + 1] = jump & 0xff;
}

static void genStmts(int stmtCount, AstNode** stmts, Scope* scope) {
	for (int i = 0; i < stmtCount; i++) {
		genStmt(stmts[i], scope);
			}
	for (int i = 0; i < scope->symbols.length; i++) {
		emitByte(OP_POP);
			}
}

static void initCodegen(CodegenContext* ctx, ArenaAllocator* arena, Chunk* chunk) {
	ctx->arena = arena;
	ctx->chunk = chunk;
	ctx->mainLocation = 0;
}

Chunk generateBytecode(AstNode* ast, Scope* scope, ArenaAllocator* arena) {
	if (ast->type != NODE_STMT_PROGRAM) {
		error("Argument to bytecode not a program", 0, 0);
	}

	Chunk bytecode;
	initChunk(&bytecode);

	initCodegen(&ctx, arena, &bytecode);

	int jump = emitJump(OP_CALL);

		for (int i = 0; i < ast->data.stmt.program.stmtCount; i++) {
		genStmt(ast->data.stmt.program.stmts[i], scope);
	}
	
	patchJump(jump, ctx.mainLocation);

	return bytecode;
}

void genStmt(AstNode* stmt, Scope* scope) {
	switch (stmt->type) {
		case NODE_STMT_FUNCDEC: {
			Token* lvalue = stmt->data.stmt.funcDec.lvalue;
			char* string = (char*)malloc(lvalue->length + 1);
			memcpy(string, lvalue->start, lvalue->length);
			string[lvalue->length] = '\0';

			if (strcmp(string, "main") == 0) {
				ctx.mainLocation = ctx.chunk->length;
			}

			free(string);

			genStmts(
				stmt->data.stmt.funcDec.stmtCount,
				stmt->data.stmt.funcDec.stmts,
				stmt->data.stmt.funcDec.scope
			);
			
			emitByte(OP_RETURN);
		} break;
		case NODE_STMT_VARDEC: {
			genExpression(stmt->data.stmt.varDec.initial, scope);
		} break;
		case NODE_STMT_BLOCKEXIT: {
			if (stmt->data.stmt.blockExit.isReturn) {
				emitByte(OP_RETURN);
			}
		} break;
		default: {
			genExpression(stmt, scope);
			if (stmt->data.expr.unusedResult) {
				emitByte(OP_POP);
			}
		} break;
	}
}

void genExpression(AstNode* expr, Scope* scope) {
	switch (expr->type) {
		case NODE_EXPR_LITERAL: {
			switch (expr->data.expr.literal.type) {
				case LIT_INT: {
					char* string = (char*)malloc(expr->token->length + 1);
					memcpy(string, expr->token->start, expr->token->length);
					string[expr->token->length] = '\0';

					int value = atoi(string);

					free(string);

					uint8_t constant = addConstant(ctx.chunk, (Value){.as = {.i32 = value}});

					writeChunk(ctx.chunk, OP_CONSTANT);
					writeChunk(ctx.chunk, constant);
				} break;
				case LIT_BOOL: {
					uint8_t constant = 0;
					if (expr->token->type == TOK_KEYWORD_TRUE) {
						constant = addConstant(ctx.chunk, (Value){.as = {.i32 = 1}});
					} else {
						constant = addConstant(ctx.chunk, (Value){.as = {.i32 = 1}});
					}

					writeChunk(ctx.chunk, OP_CONSTANT);
					writeChunk(ctx.chunk, constant);
				} break;
				default: {
					errorFromCause("this literal type is not supported yet", expr->token);
					return;
				}
			}
		} break;
		case NODE_EXPR_BINOP: {
			genExpression(expr->data.expr.binaryOp.rhs, scope);
			genExpression(expr->data.expr.binaryOp.lhs, scope);
			switch (expr->data.expr.binaryOp.op) {
				case TOK_PLUS: {
					writeChunk(ctx.chunk, OP_ADD);
				} break;
				case TOK_MINUS: {
					writeChunk(ctx.chunk, OP_SUBTRACT);
				} break;
				case TOK_EQ_EQ: {
					writeChunk(ctx.chunk, OP_EQUAL);
				} break;
				default: {
					errorFromCause("this operator is not yet supported", expr->token);
				}
			}
		} break;
		case NODE_EXPR_IDENT: {
			Symbol* symbol = scopeLookup(scope, expr->data.expr.ident.name);
			emitByte(OP_GET_LOCAL);
			emitByte(symbol->funcIndex);
		} break;
		case NODE_EXPR_BLOCK: {
			genStmts(
				expr->data.expr.block.stmtCount,
				expr->data.expr.block.stmts,
				expr->data.expr.block.scope
			);
		} break;
		case NODE_EXPR_IF: {
			genExpression(expr->data.expr.ifExpr.condition, scope);
			int elseJump = emitJump(OP_JUMP_IF_ZERO);
			genExpression(expr->data.expr.ifExpr.trueBranch, scope);
			int outJump = emitJump(OP_JUMP);
			patchJump(elseJump, ctx.chunk->length);
			if (expr->data.expr.ifExpr.falseBranch != NULL) {
				genExpression(expr->data.expr.ifExpr.falseBranch, scope);
			}
			patchJump(outJump, ctx.chunk->length);
		} break;
		default: {
			errorFromCause("not an expression", expr->token);
			return;
		}
	}
}