#include "parser.h"

#include "compiler/error/error.h"
#include "compiler/compiler.h"
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

static ExprNode* parseExprBP(Parser* p, int minBP) {
	ExprNode* lhs;
	if (isPrefixOp(p_lookahead(p, 0).type)) {
		lhs = MAKE_NODE(p, ExprNode, NODE_EXPR_UNOP);
		BindingPower bp = prefixBp(p_lookahead(p, 0).type);
		lhs->data.unaryOp.op = getSource()[p_lookahead(p, 0).start];
		p_next(p);
		lhs->data.unaryOp.operand = parseExprBP(p, bp.right);
	} else {
		lhs = parsePrimary(p);
	}

	while(true) {
		if (p_lookahead(p, 0).type == TOK_EOF) break;

		if (isPostfixOp(p_lookahead(p, 0).type)) {
			BindingPower bp = postfixBp(p_lookahead(p, 0).type);
			if (bp.left < minBP) {
				break;
			}
			ExprNode* unaryOp = MAKE_NODE(p, ExprNode, NODE_EXPR_UNOP);
			if (p_lookahead(p, 0).type == TOK_LPAREN) {
				p_next(p);
				GrowableArray exprs = GROWABLE_ARRAY_NEW(AstNode*, &p->arena);
				parseExpressionList(p, &exprs, TOK_RPAREN);
				p_consume(p,TOK_RPAREN, "Expected matching ')'");
				unaryOp->kind = NODE_EXPR_CALL;
				unaryOp->data.funcCall.func = lhs;
				unaryOp->data.funcCall.args = (AstNode**)exprs.data;
				unaryOp->data.funcCall.argsCount = exprs.length;
				lhs = unaryOp;
			} else {
				lhs->data.unaryOp.op = getSource()[p_lookahead(p, 0).start];
				p_next(p);
				unaryOp->data.unaryOp.operand = lhs;
				lhs = unaryOp;
			}
			continue;
		}

		if (!isInfixOp(p_lookahead(p, 0).type)) break;
		BindingPower bp = infixBp(p_lookahead(p, 0).type);

		if (bp.left < minBP) break;

		ExprNode* binOp = MAKE_NODE(p, ExprNode, NODE_EXPR_BINOP);
		binOp->data.binaryOp.lhs = lhs;
		binOp->data.binaryOp.op = p_lookahead(p, 0).type;
		binOp->data.binaryOp.category = getCategory(p_lookahead(p, 0).type);

		p_next(p);

		ExprNode* rhs = parseExprBP(p, bp.right);
		binOp->data.binaryOp.rhs = rhs;
		lhs = binOp;
	}
	return lhs;
}

ExprNode* parseExpression(Parser* p) {
	return parseExprBP(p, 0);
}

void parseExpressionList(Parser* p, GrowableArray* list, TokenType endSymbol) {
	while (p_lookahead(p, 0).type != endSymbol) {
		AstNode** s = growableArrayPush(list);
		*s = parseExpression(p);
		
		if (p_lookahead(p, 0).type == endSymbol) break;
		p_consume(p,TOK_COMMA, "Expected ','");
	}
}

ExprNode* parseBlock(Parser* p) {
	ExprNode* block = MAKE_NODE(p, ExprNode, NODE_EXPR_BLOCK);
	p_consume(p,TOK_LBRACE, "Expected '{'");

	GrowableArray stmts = GROWABLE_ARRAY_NEW(AstNode*, &p->arena);

	while (p_lookahead(p, 0).type != TOK_RBRACE) {
		AstNode* stmt = parseStatement(p);
		AstNode** slot = (AstNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}
	p_next(p);
	block->data.block.stmts = (AstNode**)stmts.data;
	block->data.block.stmtCount = stmts.length;

	return block;
}

ExprNode* parsePrimary(Parser* p) {
	switch (p_lookahead(p, 0).type) {
		case TOK_LPAREN: {
			p_next(p);

			if (p_lookahead(p, 0).type == TOK_RPAREN) {
				ExprNode* unit = MAKE_NODE(p,ExprNode, NODE_EXPR_TUPLE);
				unit->data.tuple.fields = NULL;
				unit->data.tuple.length = 0;
				return unit;
			}

			AstNode* expr = parseExpression(p);

			if (p_lookahead(p, 0).type != TOK_COMMA) {
				p_consume(p,TOK_RPAREN, "Expected matching ')'");
				return expr;
			}

			GrowableArray tuple = GROWABLE_ARRAY_NEW(AstNode*, &p->arena);
			AstNode** s = growableArrayPush(&tuple);
			*s = expr;
			p_next(p);

			parseExpressionList(p, &tuple, TOK_RPAREN);
			p_next(p);

			expr = MAKE_NODE(p, ExprNode, NODE_EXPR_TUPLE);
			expr->data.expr.tuple.fields = (AstNode**)tuple.data;
			expr->data.expr.tuple.length = tuple.length;
		}
		case TOK_AT: {
			ExprNode* expr = MAKE_NODE(p, ExprNode, NODE_EXPR_BUILTIN);
			p_next(p);
			expr->data.builtinCall.builtin = p_lookahead(p, 0);
			p_consume(p,TOK_IDENT, "Expected an identifier");
			p_consume(p,TOK_LPAREN, "Expected '('");
			GrowableArray args = GROWABLE_ARRAY_NEW(AstNode*, &p->arena);

			parseExpressionList(p, &args, TOK_RPAREN);
			p_next(p);
			expr->data.builtinCall.args = (AstNode**)args.data;
			expr->data.builtinCall.argsCount = args.length;

			return expr;
		}
		case TOK_LBRACE: {
			return parseBlock(p);
		}
		case TOK_IDENT: {
			ExprNode* identifier = MAKE_NODE(p, ExprNode, NODE_EXPR_IDENT);
			identifier->data.ident.name = createOwnedString(&p->arena, &getSource()[p_lookahead(p, 0).start], p_lookahead(p, 0).length);
			p_next(p);
			return identifier;
		}
		case TOK_INT: {
			ExprNode* literal = MAKE_NODE(p, ExprNode, NODE_EXPR_LITERAL);
			literal->data.literal.type = LIT_INT;
			p_next(p);
			return literal;
		}
		case TOK_STRING: {
			ExprNode* literal = MAKE_NODE(p, ExprNode, NODE_EXPR_LITERAL);
			literal->data.literal.type = LIT_STRING;
			p_next(p);
			return literal;
		}
		case TOK_KEYWORD_IF: {
			ExprNode* ifExpr = MAKE_NODE(p, ExprNode, NODE_EXPR_IF);
			p_next(p);
			ifExpr->data.ifExpr.condition = parseExpression(p);
			if (p_lookahead(p, 0).type == TOK_KEYWORD_THEN) {
				Token thenToken = p_lookahead(p, 0);
				p_next(p);
				if (p_lookahead(p, 0).type == TOK_LBRACE) {
					warnFromCause("Remove the 'then'", thenToken);
				}
				ifExpr->data.ifExpr.trueBranch = parseExpression(p);
			} else {
				ifExpr->data.ifExpr.trueBranch = parseBlock(p);
			}
			if (p_lookahead(p, 0).type != TOK_KEYWORD_ELSE) {
				return ifExpr;
			}
			p_next(p);
			ifExpr->data.ifExpr.falseBranch = parseExpression(p);
			return ifExpr;
		}
		case TOK_PIPE: case TOK_PIPE_PIPE: {
			ExprNode* lambda = MAKE_NODE(p, ExprNode, NODE_EXPR_LAMBDA);

			GrowableArray paramNames = GROWABLE_ARRAY_NEW(Token*, &p->arena);
			GrowableArray paramTypes = GROWABLE_ARRAY_NEW(ExprNode*, &p->arena);

			if (p_lookahead(p, 0).type == TOK_PIPE) {
				p_next(p);
				parseParamList(p, &paramNames, &paramTypes, TOK_PIPE, true);
			}
			p_next(p);

			if (p_lookahead(p, 0).type != TOK_MINUS_RARROW) {
				parseType(p); // ignore result for now
			}


			lambda->data.lambda.paramNames = (Token**)paramNames.data;
			lambda->data.lambda.paramTypes = (ExprNode**)paramTypes.data;
			lambda->data.lambda.paramCount = paramNames.length;

			lambda->data.lambda.body = parseExpression(p);

			return lambda;
		}
		case TOK_KEYWORD_FALSE: case TOK_KEYWORD_TRUE: {
			ExprNode* literal = MAKE_NODE(p, ExprNode, NODE_EXPR_LITERAL);
			literal->data.literal.type = LIT_BOOL;
			p_next(p);
			return literal;
		}
		case TOK_KEYWORD_RETURN: case TOK_KEYWORD_BREAK: {
			ExprNode* exit = MAKE_NODE(p, ExprNode, NODE_EXPR_EXIT);
			exit->data.exit.isReturn = p_lookahead(p, 0).type == TOK_KEYWORD_RETURN;
			p_next(p);
			exit->data.exit.exitExpr = parseExpression(p);
			return exit;
		}
		default: {
			const char* base = "Unexpected ";
			const char* addition = describeTokenType(p_lookahead(p, 0).type);
			char* fmt = ARENA_ALLOC(&p->arena, char, strlen(base) + strlen(addition) + 1);
			memcpy(fmt, base, strlen(base));
			strcat(fmt, addition);
			errorFromCause(fmt, p_lookahead(p, 0));
			p_next(p);
			// not sure what this should return
		}
	}
}

AstNode* parseTypeExpr(ArenaAllocator* p) {

}
