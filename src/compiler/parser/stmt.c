#include "parser.h"

#include "../error/error.h"
#include <stdio.h>

AstNode* parseVarDecl(ArenaAllocator* arena) {
	AstNode* stmt = makeNode(arena, NODE_STMT_VARDEC);
	stmt->data.stmt.varDec.isPublic = false;

	if (lookahead(0)->type == TOK_KEYWORD_CONST) {
		stmt->data.stmt.varDec.varType = VAR_CONST;
		next();
	} else {
		next();
		if (lookahead(0)->type == TOK_KEYWORD_MUT) {
			stmt->data.stmt.varDec.varType = VAR_LOCAL_MUT;
			next();
		} else {
			stmt->data.stmt.varDec.varType = VAR_LOCAL;
		}
	}
	stmt->data.stmt.varDec.lvalue = lookahead(0);

	consume(TOK_IDENT, "Expected an identifier");

	if (lookahead(0)->type == TOK_COLON) {
		next();
		stmt->data.stmt.varDec.type = parseType(arena);
	} else {
		stmt->data.stmt.varDec.type = NULL;
	}

	consume(TOK_EQ, "Expected '='");
	stmt->data.stmt.varDec.initial = parseExpression(arena);
	return stmt;
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

AstNode* parseFuncDecl(ArenaAllocator* arena) {
	AstNode* stmt = makeNode(arena, NODE_STMT_FUNCDEC);
	stmt->data.stmt.funcDec.isPublic = false;
	next();

	stmt->data.stmt.funcDec.lvalue = lookahead(0);
	consume(TOK_IDENT, "Expected an identifier");
	consume(TOK_LPAREN, "Expected '('");
	GrowableArray paramNames = growableArrayCreate(arena, sizeof(Token*));
	GrowableArray paramTypes = growableArrayCreate(arena, sizeof(AstNode*));

	parseParamList(arena, &paramNames, &paramTypes, TOK_RPAREN, false);
	consume(TOK_RPAREN, "Expected ')'");

	if (lookahead(0)->type != TOK_LBRACE) {
		stmt->data.stmt.funcDec.returnType = parseType(arena);
	}
	
	stmt->data.stmt.funcDec.paramNames = (Token**)paramNames.data;
	stmt->data.stmt.funcDec.paramTypes = (AstNode**)paramTypes.data;
	stmt->data.stmt.funcDec.paramCount = paramNames.length;

	consume(TOK_LBRACE, "Expected '{'");

	GrowableArray stmts = growableArrayCreate(arena, sizeof(AstNode*));

	while (lookahead(0)->type != TOK_RBRACE) {
		AstNode* stmt = parseStatement(arena);
		AstNode** slot = (AstNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}
	next();
	stmt->data.stmt.funcDec.stmts = (AstNode**)stmts.data;
	stmt->data.stmt.funcDec.stmtCount = stmts.length;

	return stmt;
}

AstNode* parseStatement(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_KEYWORD_LET: case TOK_KEYWORD_CONST: {
			AstNode* stmt = parseVarDecl(arena);
			consume(TOK_SEMICOLON, "Expected ';'");
			return stmt;
		}
		case TOK_KEYWORD_PUB: {
			next();
			if (lookahead(0)->type == TOK_KEYWORD_FUNC) {
				AstNode* stmt = parseFuncDecl(arena);
				stmt->data.stmt.funcDec.isPublic = true;
				return stmt;
			}
			if (lookahead(0)->type == TOK_KEYWORD_CONST) {
				AstNode* stmt = parseVarDecl(arena);
				stmt->data.stmt.varDec.isPublic = true;
				return stmt;
			}
			errorFromCause("Expected a declaration", lookahead(0));
			next();
		}
		case TOK_KEYWORD_FUNC: {
			return parseFuncDecl(arena);
		}
		case TOK_KEYWORD_BREAK: case TOK_KEYWORD_RETURN: {
			AstNode* stmt = makeNode(arena, NODE_STMT_BLOCKEXIT);
			stmt->data.stmt.blockExit.isReturn = lookahead(0)->type == TOK_KEYWORD_RETURN;
			next();
			stmt->data.stmt.blockExit.exitExpr = parseExpression(arena);
			consume(TOK_SEMICOLON, "Expected ';'");
			return stmt;
		}
		default: {
			AstNode* stmt = parseExpression(arena);
			consume(TOK_SEMICOLON, "Expected ';'");
			return stmt;
		}
	}
}