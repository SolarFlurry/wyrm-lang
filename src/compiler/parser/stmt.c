#include "parser.h"

ASTNode* parseVarDecl(ArenaAllocator* arena) {
	ASTNode* stmt = makeNode(arena, NODE_STMT_VARDEC);

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

	consume(TOK_EQ, "Expected a '='");
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
		ASTNode** type = (ASTNode**)growableArrayPush(types);
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

ASTNode* parseFuncDecl(ArenaAllocator* arena) {
	ASTNode* stmt = makeNode(arena, NODE_STMT_FUNCDEC);
	next();

	stmt->data.stmt.funcDec.lvalue = lookahead(0);
	consume(TOK_IDENT, "Expected an identifier");
	consume(TOK_LPAREN, "Expected '('");
	GrowableArray paramNames = growableArrayCreate(arena, sizeof(Token*));
	GrowableArray paramTypes = growableArrayCreate(arena, sizeof(ASTNode*));

	parseParamList(arena, &paramNames, &paramTypes, TOK_RPAREN, false);
	consume(TOK_RPAREN, "Expected ')'");
	
	stmt->data.stmt.funcDec.paramNames = (Token**)paramNames.data;
	stmt->data.stmt.funcDec.paramTypes = (ASTNode**)paramTypes.data;
	stmt->data.stmt.funcDec.paramCount = paramNames.length;

	stmt->data.stmt.funcDec.body = parseBlock(arena);

	return stmt;
}

ASTNode* parseStatement(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_KEYWORD_LET: case TOK_KEYWORD_CONST: {
			ASTNode* stmt = parseVarDecl(arena);
			consume(TOK_SEMICOLON, "Expected ';'");
			return stmt;
		}
		case TOK_KEYWORD_FUNC: {
			return parseFuncDecl(arena);
		}
		default: {
			ASTNode* stmt = parseExpression(arena);
			if (stmt->type != NODE_EXPR_BLOCK) consume(TOK_SEMICOLON, "Expected ';'");
			return stmt;
		}
	}
}