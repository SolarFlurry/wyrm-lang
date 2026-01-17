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

	NODE_TYPE_BASIC,
	NODE_TYPE_POINTER,
	NODE_TYPE_ARRAY,
	NODE_TYPE_FUNCTION,
} NodeType;

typedef enum {
	VAR_LOCAL,
	VAR_LOCAL_MUT,
} VarDeclType;

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
			struct {
				ASTNode* condition;
				ASTNode* trueBranch;
				ASTNode* falseBranch; // can be null
			} ifExpr;
		} expr;
		union {
			struct {
				ASTNode** stmts;
				size_t stmtCount;
			} program;
			struct {
				VarDeclType varType;
				ASTNode* lvalue;
				ASTNode* type; // can be null
				ASTNode* initial;
			} varDec;
		} stmt;
		union {
			struct {
				char* name;
			} basic;
			struct {
				ASTNode* pointee;
			} pointer;
			struct {
				ASTNode* type;
				ASTNode* size;
			} array;
			struct {
				ASTNode** fieldTypes;
				size_t fieldCount;
			} tuple;
			struct {
				ASTNode** paramTypes;
				size_t paramCount;
				ASTNode* returnType;
			} function;
		} type;
	} data;
} ASTNode;

void printAST(ASTNode* ast, int indent);