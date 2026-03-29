#include "parser.h"

#include "../error/error.h"
#include <stdio.h>
#include <stdalign.h>

DeclNode* parseVarDecl(ArenaAllocator* arena) {
	DeclNode* decl = MAKE_NODE(arena, DeclNode, NODE_DECL_VAR);
	decl->data.isPublic = false;

	if (lookahead(0)->type == TOK_KEYWORD_CONST) {
		decl->data.var.varType = VAR_CONST;
		next();
	} else {
		next();
		if (lookahead(0)->type == TOK_KEYWORD_MUT) {
			decl->data.var.varType = VAR_LOCAL_MUT;
			next();
		} else {
			decl->data.var.varType = VAR_LOCAL;
		}
	}
	decl->data.lvalue = lookahead(0);

	consume(TOK_IDENT, "Expected an identifier");

	if (lookahead(0)->type == TOK_COLON) {
		next();
		decl->data.var.type = parseType(arena);
	} else {
		decl->data.var.type = NULL;
	}

	consume(TOK_EQ, "Expected '='");
	decl->data.var.initial = parseExpression(arena);
	return decl;
}

void parseParamList(
	ArenaAllocator* arena,
	GrowableArray* names,
	GrowableArray* types,
	TokenType endSymbol,
	bool canBeAuto
) {
	while (lookahead(0)->type != endSymbol) {
		Token** name = (Token**)growableArrayPush(names);
		AstNode** type = (AstNode**)growableArrayPush(types);
		*name = lookahead(0);
		consume(TOK_IDENT, "Expected an identifier");
		if (!canBeAuto || lookahead(0)->type == TOK_COLON) {
			consume(TOK_COLON, "Expected ':'");
			*type = parseType(arena);
		} else {
			*type = NULL;
		}

		if (lookahead(0)->type == endSymbol) break;
		consume(TOK_COMMA, "Expected ','");
	}
}

DeclNode* parseFuncDecl(ArenaAllocator* arena) {
	DeclNode* decl = MAKE_NODE(arena, DeclNode, NODE_DECL_FUNC);
	decl->data.isPublic = false;
	next();

	decl->data.lvalue = lookahead(0);
	consume(TOK_IDENT, "Expected an identifier");
	consume(TOK_LPAREN, "Expected '('");
	GrowableArray paramNames = GROWABLE_ARRAY_NEW(Token*, arena);
	GrowableArray paramTypes = GROWABLE_ARRAY_NEW(AstNode*, arena);

	parseParamList(arena, &paramNames, &paramTypes, TOK_RPAREN, false);
	consume(TOK_RPAREN, "Expected ')'");

	if (lookahead(0)->type != TOK_LBRACE) {
		decl->data.func.returnType = parseType(arena);
	}
	
	decl->data.func.paramNames = (Token**)paramNames.data;
	decl->data.func.paramTypes = (AstNode**)paramTypes.data;
	decl->data.func.paramCount = paramNames.length;

	consume(TOK_LBRACE, "Expected '{'");

	GrowableArray stmts = GROWABLE_ARRAY_NEW(AstNode*, arena);

	while (lookahead(0)->type != TOK_RBRACE) {
		AstNode* stmt = parseStatement(arena);
		AstNode** slot = (AstNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}
	next();
	decl->data.func.stmts = (AstNode**)stmts.data;
	decl->data.func.stmtCount = stmts.length;

	return decl;
}

DeclNode* parseDecl(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_KEYWORD_LET: case TOK_KEYWORD_CONST: {
			DeclNode* decl = parseVarDecl(arena);
			consume(TOK_SEMICOLON, "Expected ';'");
			return decl;
		}
		case TOK_KEYWORD_FUNC: {
			DeclNode* decl = parseFuncDecl(arena);
			return decl;
		}
		case TOK_KEYWORD_PUB: {
			next();
			if (lookahead(0)->type == TOK_KEYWORD_FUNC) {
				DeclNode* decl = parseFuncDecl(arena);
				decl->data.isPublic = true;
				return decl;
			}
			if (lookahead(0)->type == TOK_KEYWORD_CONST) {
				DeclNode* decl = parseVarDecl(arena);
				decl->data.isPublic = true;
				return decl;
			}
			errorFromCause("Expected a declaration", lookahead(0));
			next();
		}
		default: return NULL; // placeholder
	}
}

AstNode* parseStatement(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_KEYWORD_LET: case TOK_KEYWORD_CONST: {
			return declToAst(arena, parseDecl(arena));
		}
		default: {
			AstNode* stmt = parseExpression(arena);
			consume(TOK_SEMICOLON, "Expected ';'");
			return stmt;
		}
	}
}