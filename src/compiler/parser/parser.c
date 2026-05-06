#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdalign.h>

#include "../error/error.h"

Token parseTok;

Parser p_init(Lexer lx, ArenaAllocator* arena) {
    return (Parser) {
        .arena = arena,
        .lx = lx,
    };
}

void p_consume(Parser* p, TokenType type, const char* message) {
	if (parseTok.type != type) {
		if (parseTok.type == TOK_UNKNOWN) return;
		errorFromCause(message, parseTok);
	} else p_next(p);
}
void consumeUntil(Parser* p, TokenType type) {
	while (p_lookahead(p, 0).type != type) {
		p_next(p);
	}
}
void p_next(Parser* p) {
	parseTok = lx_nextTok(&p->lx);
}

Token p_lookahead(Parser* p, size_t offset) {
	return parseTok;
}

AstNode* p_parse(Parser* p) {
	p_next(p);

	GrowableArray decls = GROWABLE_ARRAY_NEW(DeclNode*, p->arena);

	while (p_lookahead(p, 0).type != TOK_EOF) {
		DeclNode* decl = p_parseDecl(p);
		
		DeclNode** slot = (DeclNode**)growableArrayPush(&decls);
		*slot = decl;
	}

	AstNode* program = ARENA_ALLOC(p->arena, AstNode, 1);

	program->kind = NODE_PROGRAM;
	program->data.program.decls = decls.data;
	program->data.program.declCount = decls.length;

	return program;
}
