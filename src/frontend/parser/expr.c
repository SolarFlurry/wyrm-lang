#include "parser.h"

ASTNode* parseExpression(ArenaAllocator* arena) {
	ASTNode* ast = arenaAlloc(arena, sizeof(ASTNode));
	ast->type = NODE_EXPR_IDENT;
	ast->token = lookahead(0);

	consume(lookahead(0)->type, "unreachable");

	return ast;
}