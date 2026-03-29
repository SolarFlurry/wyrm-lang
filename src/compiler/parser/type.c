#include "parser.h"

#include <string.h>
#include "../error/error.h"

ExprNode* parseType(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_IDENT: {
			char* name = createOwnedString(arena, lookahead(0)->start, lookahead(0)->length);
			next();
			ExprNode* ast = MAKE_NODE(arena, ExprNode, NODE_EXPR_TYPE_NAMED);
			ast->data.type.named.name = name;
			return ast;
		} break;
		case TOK_ASTERISK: {
			ExprNode* ast = MAKE_NODE(arena, ExprNode, NODE_EXPR_TYPE_PTR);
			next();
			ast->data.type.pointer.operand = parseType(arena);
			return ast;
		} break;
		default: {
			errorFromCause("Unexpected token", lookahead(0));
			next();
		}
	}
}