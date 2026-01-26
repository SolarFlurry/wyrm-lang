#pragma once

#include "utils/common.h"
#include "token.h"

typedef struct AstNode AstNode;
typedef struct Scope Scope;

typedef enum {
	NODE_EXPR_LITERAL,
	NODE_EXPR_IDENT,
	NODE_EXPR_BLOCK,
	NODE_EXPR_TUPLE,
	NODE_EXPR_CALL,
	NODE_EXPR_BUILTIN,
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
	OPT_ADD,
	OPT_SUB,
	OPT_MUL,
	OPT_DIV,
	OPT_MOD,
	OPT_CONCAT,
	OPT_NEGATE,
} OpType;

typedef enum {
	BINOP_ACCESS,
	BINOP_CAST,
	BINOP_BITWISE,
	BINOP_ARITHMETIC,
	BINOP_COMPARISON,
	BINOP_LOGICAL,
	BINOP_ASSIGNMENT,
} BinOpCategory;

typedef struct AstNode {
	NodeType type;
	Token* token;
	union {
		union {
			struct {
				LiteralType type;
			} literal;
			struct {
				char* name;
			} ident;
			struct {
				AstNode** stmts;
				size_t stmtCount;
				Scope* scope;
			} block;
			struct {
				AstNode** fields;
				size_t length;
			} tuple;
			struct {
				AstNode* func;
				AstNode** args;
				size_t argsCount;
			} funcCall;
			struct {
				Token* builtin;
				AstNode** args;
				size_t argsCount;
			} builtinCall;
			struct {
				BinOpCategory category;
				OpType op;
				AstNode* lhs;
				AstNode* rhs;
			} binaryOp;
			struct {
				OpType op;
				AstNode* operand;
			} unaryOp;
			struct {
				Token** paramNames;
				AstNode** paramTypes;
				size_t paramCount;
				AstNode* body;
			} lambda;
			struct {
				AstNode* condition;
				AstNode* trueBranch;
				AstNode* falseBranch; // can be null
			} ifExpr;
		} expr;
		union {
			struct {
				AstNode** stmts;
				size_t stmtCount;
			} program;
			struct {
				VarDeclType varType;
				Token* lvalue;
				AstNode* type; // can be null
				AstNode* initial;
				bool isPublic;
			} varDec;
			struct {
				Token* lvalue;
				AstNode** paramTypes;
				size_t paramCount;
				Token** paramNames;
				AstNode* returnType;
				AstNode** stmts;
				size_t stmtCount;
				Scope* scope;
				bool isPublic;
			} funcDec;
			struct {
				bool isReturn;
				AstNode* exitExpr; // can be null;
			} blockExit;
		} stmt;
		union {
			struct {
				char* name;
			} basic;
			struct {
				AstNode* pointee;
			} pointer;
			struct {
				AstNode* type;
				AstNode* size;
			} array;
			struct {
				AstNode** fieldTypes;
				size_t fieldCount;
			} tuple;
			struct {
				AstNode** paramTypes;
				size_t paramCount;
				AstNode* returnType;
			} function;
		} type;
	} data;
} AstNode;

void printAST(AstNode* ast, int indent);