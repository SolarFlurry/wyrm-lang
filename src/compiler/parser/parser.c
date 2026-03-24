#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdalign.h>

#include "../error/error.h"

Token* parseTok;

AstNode* makeNode(ArenaAllocator* arena, NodeType type) {
	AstNode* ast = ARENA_ALLOC(arena, AstNode, 1);
	ast->type = type;
	ast->token = parseTok;
	return ast;
}

void consume(TokenType type, const char* message) {
	if (parseTok->type != type) {
		if (parseTok->type == TOK_UNKNOWN) return;
		errorFromCause(message, parseTok);
	} else next();
}
void consumeUntil(TokenType type) {
	while (lookahead(0)->type != type) {
		next();
	}
}
void next() {
	parseTok = nextToken();
}

Token* lookahead(size_t offset) {
	return parseTok;
}

AstNode* parse(ArenaAllocator* arena) {
	parseTok = nextToken();

	GrowableArray stmts = GROWABLE_ARRAY_NEW(AstNode*, arena);

	while (lookahead(0)->type != TOK_EOF) {
		AstNode* stmt = parseStatement(arena);
		
		AstNode** slot = (AstNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}

	AstNode* program = ARENA_ALLOC(arena, AstNode, 1);

	program->type = NODE_STMT_PROGRAM;
	program->data.stmt.program.stmts = stmts.data;
	program->data.stmt.program.stmtCount = stmts.length;

	return program;
}