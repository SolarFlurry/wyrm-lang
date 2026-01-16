#include "parser.h"

ASTNode* parseStatement(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_KEYWORD_LET: {
			ASTNode* stmt = makeNode(arena, NODE_STMT_VARDEC);
			next();
			stmt->data.stmt.varDec.lvalue = parseExpression(arena);
			if (lookahead(0)->type == TOK_COLON) {
				next();
				stmt->data.stmt.varDec.type = parseExpression(arena);
			} else {
				stmt->data.stmt.varDec.type = NULL;
			}
			consume(TOK_EQ, "Expected a '='");
			stmt->data.stmt.varDec.initial = parseExpression(arena);
			return stmt;
		} break;
		default: {
			return parseExpression(arena);
		}
	}
}