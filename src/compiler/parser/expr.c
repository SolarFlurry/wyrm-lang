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
	ASTNode* lhs = parseAtom(arena);

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

void parseExpressionList(ArenaAllocator* arena, GrowableArray* list, TokenType endSymbol) {
	while (lookahead(0)->type != endSymbol) {
		ASTNode* s = growableArrayPush(list);
		s = parseExpression(arena);
		
		if (lookahead(0)->type == endSymbol) break;
		consume(TOK_COMMA, "Expected ','");
	}
}

ASTNode* parseBlock(ArenaAllocator* arena) {
	consume(TOK_LBRACE, "Expected '{'");

	ASTNode* block = makeNode(arena, NODE_EXPR_BLOCK);
	GrowableArray stmts = growableArrayCreate(arena, sizeof(ASTNode*));

	while (lookahead(0)->type != TOK_RBRACE) {
		ASTNode* stmt = parseStatement(arena);
		ASTNode** slot = (ASTNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}
	next();
	block->data.expr.block.stmts = (ASTNode**)stmts.data;

	return block;
}

ASTNode* parseAtom(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_LPAREN: {
			next();

			if (lookahead(0)->type == TOK_RPAREN) {
				ASTNode* unit = makeNode(arena, NODE_EXPR_TUPLE);
				unit->data.expr.tuple.fields = NULL;
				unit->data.expr.tuple.length = 0;
				return unit;
			}

			ASTNode* expr = parseExpression(arena);

			if (lookahead(0)->type != TOK_COMMA) {
				consume(TOK_RPAREN, "Expected matching parenthesis ')'");
				return expr;
			}

			GrowableArray tuple = growableArrayCreate(arena, sizeof(ASTNode*));
			ASTNode** s = growableArrayPush(&tuple);
			*s = expr;
			next();

			parseExpressionList(arena, &tuple, TOK_RPAREN);
			next();

			expr = makeNode(arena, NODE_EXPR_TUPLE);
			expr->data.expr.tuple.fields = (ASTNode**)tuple.data;
			expr->data.expr.tuple.length = tuple.length;
		}
		case TOK_LBRACE: {
			return parseBlock(arena);
		}
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
		case TOK_BACKSLASH: {
			ASTNode* lambda = makeNode(arena, NODE_EXPR_LAMBDA);
			next();
			GrowableArray paramNames = growableArrayCreate(arena, sizeof(Token*));
			GrowableArray paramTypes = growableArrayCreate(arena, sizeof(ASTNode*));

			parseParamList(arena, &paramNames, &paramTypes, TOK_MINUS_RARROW, true);

			lambda->data.expr.lambda.paramNames = (Token**)paramNames.data;
			lambda->data.expr.lambda.paramTypes = (ASTNode**)paramTypes.data;
			lambda->data.expr.lambda.paramCount = paramNames.length;

			lambda->data.expr.lambda.body = parseExpression(arena);

			return lambda;
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