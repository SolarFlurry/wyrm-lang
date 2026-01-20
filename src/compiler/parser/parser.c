#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "../error/error.h"

Token* parseTok;

ASTNode* makeNode(ArenaAllocator* arena, NodeType type) {
	ASTNode* ast = arenaAlloc(arena, sizeof(ASTNode));
	ast->type = type;
	ast->token = parseTok;
	return ast;
}

void consume(TokenType type, const char* message) {
	if (parseTok->type != type) {
		if (parseTok->type == TOK_EOF) {
			printErrors();
			exit(1);
		}
		errorFromCause(message, parseTok);
	} else next();
}
void next() {
	parseTok = nextToken();
}

Token* lookahead(size_t offset) {
	return parseTok;
}

ASTNode* parse(ArenaAllocator* arena) {
	parseTok = nextToken();

	GrowableArray stmts = growableArrayCreate(arena, sizeof(ASTNode*));

	while (lookahead(0)->type != TOK_EOF) {
		ASTNode* stmt = parseStatement(arena);
		ASTNode** slot = (ASTNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}

	ASTNode* program = arenaAlloc(arena, sizeof(ASTNode));

	program->type = NODE_STMT_PROGRAM;
	program->data.stmt.program.stmts = stmts.data;
	program->data.stmt.program.stmtCount = stmts.length;

	return program;
}