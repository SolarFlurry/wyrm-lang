#include "semantic_analysis.h"

void typeCheck(ArenaAllocator* arena, AstNode* ast, Scope* scope) {
	if (ast->type != NODE_STMT_PROGRAM) {
		error("name resolution: Expected a program node", 0, 0);
		return;
	}
	for (int i = 0; i < ast->data.stmt.program.stmtCount; i++) {
		// resolve node
	}
}