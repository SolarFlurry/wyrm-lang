#include "parser.h"

#include "compiler/error/error.h"
#include <stdio.h>
#include <string.h>
#include <stdalign.h>

static BindingPower infixBp(TokenType op) {
	switch (op) {
		case TOK_EQ:
			return (BindingPower){2, 1};

		case TOK_AMP_AMP:
		case TOK_PIPE_PIPE:
			return (BindingPower){3, 4};
		
		case TOK_LARROW:
		case TOK_LARROW_EQ:
		case TOK_RARROW:
		case TOK_RARROW_EQ:
		case TOK_EQ_EQ:
		case TOK_BANG_EQ:
			return (BindingPower){5, 6};
		
		case TOK_PLUS:
		case TOK_MINUS:
			return (BindingPower){7, 8};
		
		case TOK_ASTERISK:
		case TOK_SLASH:
		case TOK_PERCENT:
			return (BindingPower){9, 10};
		
		case TOK_AMP:
		case TOK_PIPE:
			return (BindingPower){11, 12};
		
		case TOK_DOT:
		case TOK_COLON_COLON:
			return (BindingPower){17, 18};

		default: return (BindingPower){0, 0};
	}
}

static BindingPower prefixBp(TokenType op) {
	switch (op) {
		case TOK_BANG:
		case TOK_AMP:
		case TOK_MINUS:
		case TOK_PLUS:
			return (BindingPower){0, 15};
		
		default: return (BindingPower){0, 0};
	}
}

static BindingPower postfixBp(TokenType op) {
	switch (op) {
		case TOK_LPAREN:
		case TOK_LBRACK:
			return (BindingPower){17, 0};
		
		default: return (BindingPower){0, 0};
	}
}

static BinOpCategory getCategory(TokenType op) {
	switch (op) {
		case TOK_DOT:
		case TOK_COLON_COLON:
			return BINOP_ACCESS;
		case TOK_AMP:
		case TOK_PIPE:
			return BINOP_BITWISE;
		case TOK_PLUS:
		case TOK_MINUS:
		case TOK_ASTERISK:
		case TOK_SLASH:
		case TOK_PERCENT:
			return BINOP_ARITHMETIC;
		case TOK_LARROW:
		case TOK_RARROW:
		case TOK_LARROW_EQ:
		case TOK_RARROW_EQ:
		case TOK_EQ_EQ:
		case TOK_BANG_EQ:
			return BINOP_COMPARISON;
		case TOK_EQ:
			return BINOP_ASSIGNMENT;
		default: {
			return BINOP_ACCESS; // placeholder
		} // do nothing
	}
}

static inline bool isInfixOp(TokenType type) {
	BindingPower bp = infixBp(type);
	return bp.left != 0 || bp.right != 0;
}
static inline bool isPrefixOp(TokenType type) {
	BindingPower bp = prefixBp(type);
	return bp.right != 0;
}
static inline bool isPostfixOp(TokenType type) {
	BindingPower bp = postfixBp(type);
	return bp.left != 0;
}

static ExprNode* parseExprBP(ArenaAllocator* arena, int minBP) {
	ExprNode* lhs;
	if (isPrefixOp(lookahead(0)->type)) {
		lhs = MAKE_NODE(arena, ExprNode, NODE_EXPR_UNOP);
		BindingPower bp = prefixBp(lookahead(0)->type);
		next();
		lhs->data.unaryOp.operand = parseExprBP(arena, bp.right);
	} else {
		lhs = parsePrimary(arena);
	}

	while(true) {
		if (lookahead(0)->type == TOK_EOF) break;

		if (isPostfixOp(lookahead(0)->type)) {
			BindingPower bp = postfixBp(lookahead(0)->type);
			if (bp.left < minBP) {
				break;
			}
			ExprNode* unaryOp = MAKE_NODE(arena, ExprNode, NODE_EXPR_UNOP);
			if (lookahead(0)->type == TOK_LPAREN) {
				next();
				GrowableArray exprs = GROWABLE_ARRAY_NEW(AstNode*, arena);
				parseExpressionList(arena, &exprs, TOK_RPAREN);
				consume(TOK_RPAREN, "Expected matching ')'");
				unaryOp->kind = NODE_EXPR_CALL;
				unaryOp->data.funcCall.func = lhs;
				unaryOp->data.funcCall.args = (AstNode**)exprs.data;
				unaryOp->data.funcCall.argsCount = exprs.length;
				lhs = unaryOp;
			} else {
				next();
				unaryOp->data.unaryOp.operand = lhs;
				lhs = unaryOp;
			}
			continue;
		}

		if (!isInfixOp(lookahead(0)->type)) break;
		BindingPower bp = infixBp(lookahead(0)->type);

		if (bp.left < minBP) break;

		ExprNode* binOp = MAKE_NODE(arena, ExprNode, NODE_EXPR_BINOP);
		binOp->data.binaryOp.lhs = lhs;
		binOp->data.binaryOp.op = lookahead(0)->type;
		binOp->data.binaryOp.category = getCategory(lookahead(0)->type);

		next();

		ExprNode* rhs = parseExprBP(arena, bp.right);
		binOp->data.binaryOp.rhs = rhs;
		lhs = binOp;
	}
	return lhs;
}

ExprNode* parseExpression(ArenaAllocator* arena) {
	return parseExprBP(arena, 0);
}

void parseExpressionList(ArenaAllocator* arena, GrowableArray* list, TokenType endSymbol) {
	while (lookahead(0)->type != endSymbol) {
		AstNode** s = growableArrayPush(list);
		*s = parseExpression(arena);
		
		if (lookahead(0)->type == endSymbol) break;
		consume(TOK_COMMA, "Expected ','");
	}
}

ExprNode* parseBlock(ArenaAllocator* arena) {
	ExprNode* block = MAKE_NODE(arena, ExprNode, NODE_EXPR_BLOCK);
	consume(TOK_LBRACE, "Expected '{'");

	GrowableArray stmts = GROWABLE_ARRAY_NEW(AstNode*, arena);

	while (lookahead(0)->type != TOK_RBRACE) {
		AstNode* stmt = parseStatement(arena);
		AstNode** slot = (AstNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}
	next();
	block->data.block.stmts = (AstNode**)stmts.data;
	block->data.block.stmtCount = stmts.length;

	return block;
}

ExprNode* parsePrimary(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_LPAREN: {
			next();

			if (lookahead(0)->type == TOK_RPAREN) {
				ExprNode* unit = MAKE_NODE(arena,ExprNode, NODE_EXPR_TUPLE);
				unit->data.tuple.fields = NULL;
				unit->data.tuple.length = 0;
				return unit;
			}

			AstNode* expr = parseExpression(arena);

			if (lookahead(0)->type != TOK_COMMA) {
				consume(TOK_RPAREN, "Expected matching ')'");
				return expr;
			}

			GrowableArray tuple = GROWABLE_ARRAY_NEW(AstNode*, arena);
			AstNode** s = growableArrayPush(&tuple);
			*s = expr;
			next();

			parseExpressionList(arena, &tuple, TOK_RPAREN);
			next();

			expr = MAKE_NODE(arena, ExprNode, NODE_EXPR_TUPLE);
			expr->data.expr.tuple.fields = (AstNode**)tuple.data;
			expr->data.expr.tuple.length = tuple.length;
		}
		case TOK_AT: {
			ExprNode* expr = MAKE_NODE(arena, ExprNode, NODE_EXPR_BUILTIN);
			next();
			expr->data.builtinCall.builtin = lookahead(0);
			consume(TOK_IDENT, "Expected an identifier");
			consume(TOK_LPAREN, "Expected '('");
			GrowableArray args = GROWABLE_ARRAY_NEW(AstNode*, arena);

			parseExpressionList(arena, &args, TOK_RPAREN);
			next();
			expr->data.builtinCall.args = (AstNode**)args.data;
			expr->data.builtinCall.argsCount = args.length;

			return expr;
		}
		case TOK_LBRACE: {
			return parseBlock(arena);
		}
		case TOK_IDENT: {
			ExprNode* identifier = MAKE_NODE(arena, ExprNode, NODE_EXPR_IDENT);
			identifier->data.ident.name = createOwnedString(arena, lookahead(0)->start, lookahead(0)->length);
			next();
			return identifier;
		}
		case TOK_INT: {
			ExprNode* literal = MAKE_NODE(arena, ExprNode, NODE_EXPR_LITERAL);
			literal->data.literal.type = LIT_INT;
			next();
			return literal;
		}
		case TOK_STRING: {
			ExprNode* literal = MAKE_NODE(arena, ExprNode, NODE_EXPR_LITERAL);
			literal->data.literal.type = LIT_STRING;
			next();
			return literal;
		}
		case TOK_KEYWORD_IF: {
			ExprNode* ifExpr = MAKE_NODE(arena, ExprNode, NODE_EXPR_IF);
			next();
			ifExpr->data.ifExpr.condition = parseExpression(arena);
			if (lookahead(0)->type == TOK_KEYWORD_THEN) {
				Token* thenToken = lookahead(0);
				next();
				if (lookahead(0)->type == TOK_LBRACE) {
					warnFromCause("Remove the 'then'", thenToken);
				}
				ifExpr->data.ifExpr.trueBranch = parseExpression(arena);
			} else {
				ifExpr->data.ifExpr.trueBranch = parseBlock(arena);
			}
			if (lookahead(0)->type != TOK_KEYWORD_ELSE) {
				return ifExpr;
			}
			next();
			ifExpr->data.ifExpr.falseBranch = parseExpression(arena);
			return ifExpr;
		}
		case TOK_PIPE: case TOK_PIPE_PIPE: {
			ExprNode* lambda = MAKE_NODE(arena, ExprNode, NODE_EXPR_LAMBDA);

			GrowableArray paramNames = GROWABLE_ARRAY_NEW(Token*, arena);
			GrowableArray paramTypes = GROWABLE_ARRAY_NEW(ExprNode*, arena);

			if (lookahead(0)->type == TOK_PIPE) {
				next();
				parseParamList(arena, &paramNames, &paramTypes, TOK_PIPE, true);
			}
			next();

			if (lookahead(0)->type != TOK_MINUS_RARROW) {
				parseType(arena); // ignore result for now
			}


			lambda->data.lambda.paramNames = (Token**)paramNames.data;
			lambda->data.lambda.paramTypes = (ExprNode**)paramTypes.data;
			lambda->data.lambda.paramCount = paramNames.length;

			lambda->data.lambda.body = parseExpression(arena);

			return lambda;
		}
		case TOK_KEYWORD_FALSE: case TOK_KEYWORD_TRUE: {
			ExprNode* literal = MAKE_NODE(arena, ExprNode, NODE_EXPR_LITERAL);
			literal->data.literal.type = LIT_BOOL;
			next();
			return literal;
		}
		case TOK_KEYWORD_RETURN: case TOK_KEYWORD_BREAK: {
			ExprNode* exit = MAKE_NODE(arena, ExprNode, NODE_EXPR_EXIT);
			exit->data.exit.isReturn = lookahead(0)->type == TOK_KEYWORD_RETURN;
			next();
			exit->data.exit.exitExpr = parseExpression(arena);
			return exit;
		}
		default: {
			const char* base = "Unexpected ";
			const char* addition = describeTokenType(lookahead(0)->type);
			char* fmt = ARENA_ALLOC(arena, char, strlen(base) + strlen(addition) + 1);
			memcpy(fmt, base, strlen(base));
			strcat(fmt, addition);
			errorFromCause(fmt, lookahead(0));
			next();
			// not sure what this should return
		}
	}
}

AstNode* parseTypeExpr(ArenaAllocator* arena) {

}