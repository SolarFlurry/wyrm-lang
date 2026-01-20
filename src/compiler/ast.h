#pragma once

#include "utils/common.h"
#include "token.h"

typedef struct ASTNode ASTNode;
typedef struct Scope Scope;

typedef enum {
	NODE_EXPR_LITERAL,
	NODE_EXPR_IDENT,
	NODE_EXPR_BLOCK,
	NODE_EXPR_TUPLE,
	NODE_EXPR_BINOP,
	NODE_EXPR_UNOP,
	NODE_EXPR_LAMBDA,
	NODE_EXPR_IF,

	NODE_STMT_PROGRAM,
	NODE_STMT_VARDEC,
	NODE_STMT_FUNCDEC,
	NODE_STMT_BLOCKEXIT,

	NODE_TYPE_BASIC,
	NODE_TYPE_POINTER,
	NODE_TYPE_ARRAY,
	NODE_TYPE_TUPLE,
	NODE_TYPE_FUNCTION,
} NodeType;

typedef enum {
	VAR_CONST,
	VAR_LOCAL,
	VAR_LOCAL_MUT,
} VarDeclType;

typedef enum {
	LIT_INT,
	LIT_STRING,
	LIT_BOOL,
} LiteralType;

typedef enum {
	BINOP_CALL,
	BINOP_ADD,
	BINOP_SUBTRACT,
} BinaryOp;

typedef enum {
	UNOP_INDEX,
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
				ASTNode** stmts;
				Scope* scope;
			} block;
			struct {
				ASTNode** fields;
				size_t length;
			} tuple;
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
				Token** paramNames;
				ASTNode** paramTypes;
				size_t paramCount;
				ASTNode* body;
			} lambda;
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
				Token* lvalue;
				ASTNode* type; // can be null
				ASTNode* initial;
				bool isPublic;
			} varDec;
			struct {
				Token* lvalue;
				ASTNode** paramTypes;
				size_t paramCount;
				Token** paramNames;
				ASTNode* returnType;
				ASTNode* body;
				Scope* scope;
				bool isPublic;
			} funcDec;
			struct {
				bool isReturn;
				ASTNode* exitExpr; // can be null;
			} blockExit;
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