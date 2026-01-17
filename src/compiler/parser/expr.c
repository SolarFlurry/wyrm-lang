#include "parser.h"

#include "compiler/error/error.h"
#include <stdio.h>
#include <string.h>

static BindingPower infixBP(TokenType op) {
	switch (op) {
		case TOK_PLUS: case TOK_MINUS: return (BindingPower){1, 2};
		case TOK_ASTERISK: case TOK_SLASH: return (BindingPower){3, 4};
		default: return (BindingPower){0, 0};
	}
}

static BinaryOp getOp(TokenType op) {
	switch (op) {
		case TOK_PLUS: return BINOP_ADD;
		case TOK_MINUS: return BINOP_SUBTRACT;
		default: return /* unreachable */ BINOP_ADD;
	}
}

static inline bool isInfixOp(TokenType type) {
	BindingPower bp = infixBP(type);
	return bp.left != 0 && bp.right != 0;
}

static ASTNode* parseExprBP(ArenaAllocator* arena, int minBP) {
	ASTNode* lhs;
	if (lookahead(0)->type == TOK_LPAREN) {
		next();
		lhs = parseExprBP(arena, 0);
		consume(TOK_RPAREN, "Expected matching parenthesis");
	} else {
		lhs = parseAtom(arena);
	}

	while(true) {
		if (lookahead(0)->type == TOK_EOF) break;
		if (!isInfixOp(lookahead(0)->type)) break;
		BindingPower bp = infixBP(lookahead(0)->type);

		if (bp.left < minBP) break;

		ASTNode* binOp = makeNode(arena, NODE_EXPR_BINOP);
		binOp->data.expr.binaryOp.lhs = lhs;
		binOp->data.expr.binaryOp.op = getOp(lookahead(0)->type);

		next();

		ASTNode* rhs = parseExprBP(arena, bp.right);
		binOp->data.expr.binaryOp.rhs = rhs;
		lhs = binOp;
	}
	return lhs;
}

ASTNode* parseExpression(ArenaAllocator* arena) {
	return parseExprBP(arena, 0);
}

ASTNode* parseAtom(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_IDENT: {
			ASTNode* identifier = makeNode(arena, NODE_EXPR_IDENT);
			next();
			return identifier;
		}
		case TOK_INT: {
			ASTNode* literal = makeNode(arena, NODE_EXPR_LITERAL);
			literal->data.expr.literal.type = LIT_INT;
			next();
			return literal;
		}
		default: {
			const char* base = "Unexpected ";
			const char* addition = describeTokenType(lookahead(0)->type);
			char* fmt = arenaAlloc(arena, strlen(base) + strlen(addition) + 1);
			memcpy(fmt, base, strlen(base));
			strcat(fmt, addition);
			errorFromCause(fmt, lookahead(0));
			next();
			// not sure what this should return
		}
	}
}