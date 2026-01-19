#include "parser.h"

#include <string.h>
#include "../error/error.h"

ASTNode* parseType(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_IDENT: {
			char* name = arenaAlloc(arena, sizeof(char) * (lookahead(0)->length + 1));
			memcpy(name, lookahead(0)->start, lookahead(0)->length);
			name[lookahead(0)->length] = '\0';
			next();
			ASTNode* ast = makeNode(arena, NODE_TYPE_BASIC);
			ast->data.type.basic.name = name;
			return ast;
		} break;
		case TOK_ASTERISK: {
			ASTNode* ast = makeNode(arena, NODE_TYPE_POINTER);
			next();
			ast->data.type.pointer.pointee = parseType(arena);
			return ast;
		} break;
		default: {
			errorFromCause("Unexpected token", lookahead(0));
			next();
		}
	}
}