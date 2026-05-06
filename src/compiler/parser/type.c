#include "parser.h"

#include "compiler/compiler.h"
#include <string.h>
#include "../error/error.h"

ExprNode* parseType(Parser* p) {
	switch (p_lookahead(p, 0).type) {
		case TOK_IDENT: {
			char* name = createOwnedString(p->arena, &getSource()[p_lookahead(p, 0).start], p_lookahead(p, 0).length);
			p_next(p);
			ExprNode* ast = MAKE_NODE(p, ExprNode, NODE_EXPR_TYPE_NAMED);
			ast->data.type.named.name = name;
			return ast;
		} break;
		case TOK_ASTERISK: {
			ExprNode* ast = MAKE_NODE(p, ExprNode, NODE_EXPR_TYPE_PTR);
			p_next(p);
			ast->data.type.pointer.operand = parseType(p);
			return ast;
		} break;
		default: {
			errorFromCause("Unexpected token", p_lookahead(p, 0));
			p_next(p);
		}
	}
}
