#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

Token* parseTok;

void consume(TokenType type, const char* error) {
	if (parseTok->type != type) {
		printf("[Error]: %s\n%4u | Error occured here", error, parseTok->line);
		exit(1);
	}
	parseTok = nextToken();
}

Token* lookahead(size_t offset) {
	return parseTok;
}

ASTNode* parse(ArenaAllocator* arena) {
	parseTok = nextToken();

	GrowableArray stmts = growableArrayCreate(arena, sizeof(ASTNode*));

	while (lookahead(0)->type != TOK_EOF) {
		ASTNode* stmt = parseExpression(arena);
		ASTNode** slot = (ASTNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}

	ASTNode* program = arenaAlloc(arena, sizeof(ASTNode));

	program->type = NODE_STMT_PROGRAM;
	program->data.stmt.program.stmts = stmts.data;
	program->data.stmt.program.stmtCount = stmts.length;

	return program;
}