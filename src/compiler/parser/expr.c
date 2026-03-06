#include "parser.h"

#include "compiler/error/error.h"
#include <stdio.h>
#include <string.h>

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
		default: {} // do nothing
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

static AstNode* parseExprBP(ArenaAllocator* arena, int minBP) {
	AstNode* lhs;
	if (isPrefixOp(lookahead(0)->type)) {
		lhs = makeNode(arena, NODE_EXPR_UNOP);
		BindingPower bp = prefixBp(lookahead(0)->type);
		next();
		lhs->data.expr.unaryOp.operand = parseExprBP(arena, bp.right);
	} else {
		lhs = parseAtom(arena);
	}

	while(true) {
		if (lookahead(0)->type == TOK_EOF) break;

		if (isPostfixOp(lookahead(0)->type)) {
			BindingPower bp = postfixBp(lookahead(0)->type);
			if (bp.left < minBP) {
				break;
			}
			AstNode* unaryOp = makeNode(arena, NODE_EXPR_UNOP);
			if (lookahead(0)->type == TOK_LPAREN) {
				next();
				GrowableArray exprs = growableArrayCreate(arena, sizeof(AstNode*));
				parseExpressionList(arena, &exprs, TOK_RPAREN);
				consume(TOK_RPAREN, "Expected matching ')'");
				unaryOp->type = NODE_EXPR_CALL;
				unaryOp->data.expr.funcCall.func = lhs;
				unaryOp->data.expr.funcCall.args = (AstNode**)exprs.data;
				unaryOp->data.expr.funcCall.argsCount = exprs.length;
				lhs = unaryOp;
			} else {
				next();
				unaryOp->data.expr.unaryOp.operand = lhs;
				lhs = unaryOp;
			}
			continue;
		}

		if (!isInfixOp(lookahead(0)->type)) break;
		BindingPower bp = infixBp(lookahead(0)->type);

		if (bp.left < minBP) break;

		AstNode* binOp = makeNode(arena, NODE_EXPR_BINOP);
		binOp->data.expr.binaryOp.lhs = lhs;
		binOp->data.expr.binaryOp.op = lookahead(0)->type;
		binOp->data.expr.binaryOp.category = getCategory(lookahead(0)->type);

		next();

		AstNode* rhs = parseExprBP(arena, bp.right);
		binOp->data.expr.binaryOp.rhs = rhs;
		lhs = binOp;
	}
	return lhs;
}

AstNode* parseExpression(ArenaAllocator* arena) {
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

AstNode* parseBlock(ArenaAllocator* arena) {
	AstNode* block = makeNode(arena, NODE_EXPR_BLOCK);
	consume(TOK_LBRACE, "Expected '{'");

	GrowableArray stmts = growableArrayCreate(arena, sizeof(AstNode*));

	while (lookahead(0)->type != TOK_RBRACE) {
		AstNode* stmt = parseStatement(arena);
		AstNode** slot = (AstNode**)growableArrayPush(&stmts);
		*slot = stmt;
	}
	next();
	block->data.expr.block.stmts = (AstNode**)stmts.data;
	block->data.expr.block.stmtCount = stmts.length;

	return block;
}

AstNode* parseAtom(ArenaAllocator* arena) {
	switch (lookahead(0)->type) {
		case TOK_LPAREN: {
			next();

			if (lookahead(0)->type == TOK_RPAREN) {
				AstNode* unit = makeNode(arena, NODE_EXPR_TUPLE);
				unit->data.expr.tuple.fields = NULL;
				unit->data.expr.tuple.length = 0;
				return unit;
			}

			AstNode* expr = parseExpression(arena);

			if (lookahead(0)->type != TOK_COMMA) {
				consume(TOK_RPAREN, "Expected matching ')'");
				return expr;
			}

			GrowableArray tuple = growableArrayCreate(arena, sizeof(AstNode*));
			AstNode** s = growableArrayPush(&tuple);
			*s = expr;
			next();

			parseExpressionList(arena, &tuple, TOK_RPAREN);
			next();

			expr = makeNode(arena, NODE_EXPR_TUPLE);
			expr->data.expr.tuple.fields = (AstNode**)tuple.data;
			expr->data.expr.tuple.length = tuple.length;
		}
		case TOK_AT: {
			AstNode* expr = makeNode(arena, NODE_EXPR_BUILTIN);
			next();
			expr->data.expr.builtinCall.builtin = lookahead(0);
			consume(TOK_IDENT, "Expected an identifier");
			consume(TOK_LPAREN, "Expected '('");
			GrowableArray args = growableArrayCreate(arena, sizeof(AstNode*));

			parseExpressionList(arena, &args, TOK_RPAREN);
			next();
			expr->data.expr.builtinCall.args = (AstNode**)args.data;
			expr->data.expr.builtinCall.argsCount = args.length;

			return expr;
		}
		case TOK_LBRACE: {
			return parseBlock(arena);
		}
		case TOK_IDENT: {
			AstNode* identifier = makeNode(arena, NODE_EXPR_IDENT);
			identifier->data.expr.ident.name = createOwnedString(arena, lookahead(0)->start, lookahead(0)->length);
			next();
			return identifier;
		}
		case TOK_INT: {
			AstNode* literal = makeNode(arena, NODE_EXPR_LITERAL);
			literal->data.expr.literal.type = LIT_INT;
			next();
			return literal;
		}
		case TOK_STRING: {
			AstNode* literal = makeNode(arena, NODE_EXPR_LITERAL);
			literal->data.expr.literal.type = LIT_STRING;
			next();
			return literal;
		}
		case TOK_KEYWORD_IF: {
			AstNode* ifExpr = makeNode(arena, NODE_EXPR_IF);
			next();
			ifExpr->data.expr.ifExpr.condition = parseExpression(arena);
			if (lookahead(0)->type == TOK_KEYWORD_THEN) {
				Token* thenToken = lookahead(0);
				next();
				if (lookahead(0)->type == TOK_LBRACE) {
					warnFromCause("Remove the 'then'", thenToken);
				}
				ifExpr->data.expr.ifExpr.trueBranch = parseExpression(arena);
			} else {
				ifExpr->data.expr.ifExpr.trueBranch = parseBlock(arena);
			}
			if (lookahead(0)->type != TOK_KEYWORD_ELSE) {
				return ifExpr;
			}
			next();
			ifExpr->data.expr.ifExpr.falseBranch = parseExpression(arena);
			return ifExpr;
		}
		case TOK_BACKSLASH: {
			AstNode* lambda = makeNode(arena, NODE_EXPR_LAMBDA);
			next();
			GrowableArray paramNames = growableArrayCreate(arena, sizeof(Token*));
			GrowableArray paramTypes = growableArrayCreate(arena, sizeof(AstNode*));

			parseParamList(arena, &paramNames, &paramTypes, TOK_MINUS_RARROW, true);

			lambda->data.expr.lambda.paramNames = (Token**)paramNames.data;
			lambda->data.expr.lambda.paramTypes = (AstNode**)paramTypes.data;
			lambda->data.expr.lambda.paramCount = paramNames.length;

			lambda->data.expr.lambda.body = parseExpression(arena);

			return lambda;
		}
		case TOK_KEYWORD_FALSE: case TOK_KEYWORD_TRUE: {
			AstNode* literal = makeNode(arena, NODE_EXPR_LITERAL);
			literal->data.expr.literal.type = LIT_BOOL;
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