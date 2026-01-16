#pragma once

#include "utils/common.h"
#include "token.h"

typedef struct ASTNode ASTNode;

typedef enum {
	NODE_EXPR_LITERAL,
	NODE_EXPR_IDENT,
	NODE_EXPR_BINOP,
	NODE_EXPR_UNOP,

	NODE_STMT_PROGRAM,
	NODE_STMT_VARDEC,
} NodeType;

typedef enum {
	LIT_INT,
	LIT_STRING,
	LIT_BOOL,
} LiteralType;

typedef enum {
	BINOP_ADD,
	BINOP_SUBTRACT,
} BinaryOp;

typedef enum {
	UNOP_NEGATE,
} UnaryOp;

typedef struct ASTNode {
	NodeType type;
	Token* token;
	union {
		union {
			struct {
				LiteralType type;
			} literal;
			struct {
				BinaryOp op;
				ASTNode* lhs;
				ASTNode* rhs;
			} binaryOp;
			struct {
				UnaryOp op;
				ASTNode* operand;
			} unaryOp;
		} expr;
		union {
			struct {
				ASTNode** stmts;
				size_t stmtCount;
			} program;
			struct {
				ASTNode* lvalue;
				ASTNode* type; // can be null
				ASTNode* initial;
			} varDec;
		} stmt;
	} data;
} ASTNode;

void printAST(ASTNode* ast, int indent);