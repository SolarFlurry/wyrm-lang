#include "parser.h"

#include "../error/error.h"
#include <stdio.h>
#include <stdalign.h>

DeclNode* parseVarDecl(Parser* p) {
	DeclNode* decl = MAKE_NODE(p, DeclNode, NODE_DECL_VAR);
	decl->data.isPublic = false;

	if (p_lookahead(p, 0).type == TOK_KEYWORD_CONST) {
		decl->data.var.varType = VAR_CONST;
		p_next(p);
	} else {
		p_next(p);
		if (p_lookahead(p, 0).type == TOK_KEYWORD_MUT) {
			decl->data.var.varType = VAR_LOCAL_MUT;
			p_next(p);
		} else {
			decl->data.var.varType = VAR_LOCAL;
		}
	}
	decl->data.lvalue = p_lookahead(p, 0);

	p_consume(p, TOK_IDENT, "Expected an identifier");

	if (p_lookahead(p, 0).type == TOK_COLON) {
		p_next(p);
		decl->data.var.type = parseType(p);
	} else {
		decl->data.var.type = NULL;
	}

	p_consume(p, TOK_EQ, "Expected '='");
	decl->data.var.initial = parseExpression(p);
	return decl;
}

void parseParamList(
	Parser* p,
	GrowableArray* names,
	GrowableArray* types,
	TokenType endSymbol,
	bool canBeAuto
) {
	while (p_lookahead(p, 0).type != endSymbol) {
		Token* name = (Token*)growableArrayPush(names);
		AstNode** type = (AstNode**)growableArrayPush(types);
		*name = p_lookahead(p, 0);
		p_consume(p, TOK_IDENT, "Expected an identifier");
		if (!canBeAuto || p_lookahead(p, 0).type == TOK_COLON) {
			p_consume(p, TOK_COLON, "Expected ':'");
			*type = parseType(p);
		} else {
			*type = NULL;
		}

		if (p_lookahead(p, 0).type == endSymbol) break;
		p_consume(p, TOK_COMMA, "Expected ','");
	}
}

DeclNode* parseFuncDecl(Parser* p) {
	DeclNode* decl = MAKE_NODE(p, DeclNode, NODE_DECL_FUNC);
	decl->data.isPublic = false;
	p_next(p);

	decl->data.lvalue = p_lookahead(p, 0);
	p_consume(p, TOK_IDENT, "Expected an identifier");
	p_consume(p, TOK_LPAREN, "Expected '('");
	GrowableArray paramNames = GROWABLE_ARRAY_NEW(Token, &p->arena);
	GrowableArray paramTypes = GROWABLE_ARRAY_NEW(AstNode*, &p->arena);

	parseParamList(p, &paramNames, &paramTypes, TOK_RPAREN, false);
	p_consume(p, TOK_RPAREN, "Expected ')'");

	if (p_lookahead(p, 0).type != TOK_LBRACE) {
		decl->data.func.returnType = parseType(p);
	}
	
	decl->data.func.paramNames = (Token*)paramNames.data;
	decl->data.func.paramTypes = (AstNode**)paramTypes.data;
	decl->data.func.paramCount = paramNames.length;

	p_consume(p, TOK_LBRACE, "Expected '{'");

	GrowableArray stmts = GROWABLE_ARRAY_NEW(AstNode*, &p->arena);

	while (p_lookahead(p, 0).type != TOK_RBRACE) {
		AstNode* stmt = parseStatement(p);
		AstNode** slot = (AstNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}
	p_next(p);
	decl->data.func.stmts = (AstNode**)stmts.data;
	decl->data.func.stmtCount = stmts.length;

	return decl;
}

DeclNode* p_parseDecl(Parser* p) {
	switch (p_lookahead(p, 0).type) {
		case TOK_KEYWORD_LET: case TOK_KEYWORD_CONST: {
			DeclNode* decl = parseVarDecl(p);
			p_consume(p, TOK_SEMICOLON, "Expected ';'");
			return decl;
		}
		case TOK_KEYWORD_FUNC: {
			DeclNode* decl = parseFuncDecl(p);
			return decl;
		}
		case TOK_KEYWORD_PUB: {
			p_next(p);
			if (p_lookahead(p, 0).type == TOK_KEYWORD_FUNC) {
				DeclNode* decl = parseFuncDecl(p);
				decl->data.isPublic = true;
				return decl;
			}
			if (p_lookahead(p, 0).type == TOK_KEYWORD_CONST) {
				DeclNode* decl = parseVarDecl(p);
				decl->data.isPublic = true;
				return decl;
			}
			errorFromCause("Expected a declaration", p_lookahead(p, 0));
			p_next(p);
		}
		default: return NULL; // placeholder
	}
}

AstNode* parseStatement(Parser* p) {
	switch (p_lookahead(p, 0).type) {
		case TOK_KEYWORD_LET: case TOK_KEYWORD_CONST: {
			return declToAst(&p->arena, p_parseDecl(p));
		}
		default: {
			AstNode* stmt = parseExpression(p);
			p_consume(p, TOK_SEMICOLON, "Expected ';'");
			return stmt;
		}
	}
}
