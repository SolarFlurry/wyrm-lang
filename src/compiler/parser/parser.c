#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdalign.h>

#include "../error/error.h"

Token* parseTok;

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

	GrowableArray decls = GROWABLE_ARRAY_NEW(DeclNode*, arena);

	while (lookahead(0)->type != TOK_EOF) {
		DeclNode* decl = parseDecl(arena);
		
		DeclNode** slot = (DeclNode**)growableArrayPush(&decls);
		*slot = decl;
	}

	AstNode* program = ARENA_ALLOC(arena, AstNode, 1);

	program->kind = NODE_PROGRAM;
	program->data.program.decls = decls.data;
	program->data.program.declCount = decls.length;

	return program;
}