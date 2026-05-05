#pragma once

#include "utils/common.h"
#include "token.h"
#include "utils/memory.h"

typedef struct ExprNode ExprNode;
typedef struct DeclNode DeclNode;
typedef struct AstNode AstNode;
typedef struct Scope Scope;

typedef enum {
	NODE_PROGRAM,

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
	NODE_EXPR_EXIT,

	NODE_EXPR_TYPE_NAMED,
	NODE_EXPR_TYPE_PTR,
	NODE_EXPR_TYPE_ARRAY,
	NODE_EXPR_TYPE_TUPLE,
	NODE_EXPR_TYPE_FUNC,

	NODE_DECL_VAR,
	NODE_DECL_FUNC,
} NodeKind;

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
	BINOP_ACCESS,
	BINOP_CAST,
	BINOP_BITWISE,
	BINOP_ARITHMETIC,
	BINOP_COMPARISON,
	BINOP_LOGICAL,
	BINOP_ASSIGNMENT,
} BinOpCategory;

typedef struct Expr {
	bool unusedResult;
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
			Token builtin;
			ExprNode** args;
			size_t argsCount;
		} builtinCall;
		struct {
			BinOpCategory category;
			TokenType op;
			ExprNode* lhs;
			ExprNode* rhs;
		} binaryOp;
		struct {
			char op;
			ExprNode* operand;
		} unaryOp;
		struct {
			Token* paramNames;
			ExprNode** paramTypes;
			size_t paramCount;
			AstNode* body;
		} lambda;
		struct {
			ExprNode* condition;
			ExprNode* trueBranch;
			ExprNode* falseBranch; // can be null
		} ifExpr;
		struct {
			bool isReturn;
			ExprNode* exitExpr;
		} exit;
		union {
			enum {
				TYPE_BUILTIN_ERROR,
				TYPE_BUILTIN_NORETURN,
			} builtin;
			struct {
				char* name;
			} named;
			struct {
				bool isMut;
				bool isSlice;
				ExprNode* operand;
			} pointer;
			struct {
				ExprNode* type;
				size_t len;
			} array;
			struct {
				ExprNode** fieldTypes;
				size_t fieldCount;
			} tuple;
			struct {
				ExprNode** paramTypes;
				size_t paramCount;
				ExprNode* returnType;
			} func;
		} type;
	};
} Expr;

typedef struct Decl {
	Token lvalue;
	bool isPublic;
	struct {
		VarDeclType varType;
		ExprNode* type; // can be null
		ExprNode* initial;
	} var;
	struct {
		ExprNode** paramTypes;
		size_t paramCount;
		Token* paramNames;
		ExprNode* returnType;
		AstNode** stmts;
		size_t stmtCount;
		Scope* scope;
	} func;
} Decl;

typedef struct ExprNode {
	NodeKind kind;
	Token token;
	Expr data;
} ExprNode;

typedef struct DeclNode {
	NodeKind kind;
	Token token;
	Decl data;
} DeclNode;

typedef struct AstNode {
	NodeKind kind;
	Token token;
	union {
		struct {
			DeclNode** decls;
			size_t declCount;
		} program;

		Expr expr;
		Decl decl;
	} data;
} AstNode;

void printAST(AstNode* ast, uint32_t indent, int indent_type, uint64_t has_lines);
AstNode* exprToAst(ArenaAllocator* arena, ExprNode* expr);
AstNode* declToAst(ArenaAllocator* arena, DeclNode* expr);
