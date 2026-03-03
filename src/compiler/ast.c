#include "ast.h"

#include <stdio.h>

static void printIndent(uint32_t indent, uint64_t has_lines) {
    for (int i = 0; i < indent; i++) {
        if ((1 << (indent - i) & has_lines) > 0) {
            printf("│  ");
        } else {
            printf("   ");
        }
    }
}

void printAST(AstNode* ast, uint32_t indent, int indent_type, uint64_t has_lines) {
    printIndent(indent, has_lines);
    switch (indent_type) {
        case 0: printf("   "); break;
        case 1: printf("├─ "); break;
        case 2: printf("╰─ "); break;
    }
    printf("\x1b[36m");

    switch (ast->type) {
        case NODE_STMT_PROGRAM: {
            printf("Program\x1b[0m:\n");
            for (int i = 0; i < ast->data.stmt.program.stmtCount; i++) {
                if (i == ast->data.stmt.program.stmtCount - 1) {
                    printAST(ast->data.stmt.program.stmts[i], indent + 1, 2, has_lines << 1);
                    continue;
                }
                printAST(ast->data.stmt.program.stmts[i], indent + 1, 1, has_lines << 1 | 1);
            }
        } break;
        case NODE_STMT_VARDEC: {
            printf("VarDecl\x1b[0m => \x1b[35m'");
            for (int i = 0; i < ast->data.stmt.varDec.lvalue->length; i++) {
                putchar(ast->data.stmt.varDec.lvalue->start[i]);
            }
            puts("'\x1b[0m");
            if (ast->data.stmt.varDec.type != NULL) {
                printAST(ast->data.stmt.varDec.type, indent + 1, 1, has_lines << 1);
            }
            printAST(ast->data.stmt.varDec.initial, indent + 1, 2, has_lines << 1);
        } break;
        case NODE_STMT_FUNCDEC: {
            printf("FuncDecl\x1b[0m => \x1b[35m'");
            for (int i = 0; i < ast->data.stmt.funcDec.lvalue->length; i++) {
                putchar(ast->data.stmt.funcDec.lvalue->start[i]);
            }
            puts("'\x1b[0m");
            for (int i = 0; i < ast->data.stmt.funcDec.stmtCount; i++) {
                if (i == ast->data.stmt.funcDec.stmtCount - 1) {
                    printAST(ast->data.stmt.funcDec.stmts[i], indent + 1, 2, has_lines << 1);
                    continue;
                }
                printAST(ast->data.stmt.funcDec.stmts[i], indent + 1, 1, has_lines << 1 | 1);
            }
        } break;
        case NODE_EXPR_IDENT: {
            printf("Identifier\x1b[0m => \x1b[35m'");
            for (int i = 0; i < ast->token->length; i++) {
                putchar(ast->token->start[i]);
            }
            printf("'\x1b[0m\n");
        } break;
        case NODE_EXPR_LITERAL: {
            printf("Literal\x1b[0m => \x1b[31m");
            switch (ast->data.expr.literal.type) {
                case LIT_INT: {
                    printf("i32");
                } break;
                case LIT_BOOL: {
                    printf("bool");
                }
                default: {} // do nothing
            }
            printf("\x1b[0m \x1b[33m'");
            for (int i = 0; i < ast->token->length; i++) {
                putchar(ast->token->start[i]);
            }
            printf("'\x1b[0m\n");
        } break;
        case NODE_EXPR_BINOP: {
            printf("BinaryOp\x1b[0m '");
            for (int i = 0; i < ast->token->length; i++) {
                putchar(ast->token->start[i]);
            }
            printf("'\n");
            printAST(ast->data.expr.binaryOp.lhs, indent + 1, 1, has_lines << 1);
            printAST(ast->data.expr.binaryOp.rhs, indent + 1, 2, has_lines << 1);
        } break;
        case NODE_EXPR_BLOCK: {
            printf("Block\x1b[0m\n");
            for (int i = 0; i < ast->data.expr.block.stmtCount; i++) {
                if (i == ast->data.expr.block.stmtCount - 1) {
                    printAST(ast->data.expr.block.stmts[i], indent + 1, 2, has_lines << 1);
                    continue;
                }
                printAST(ast->data.expr.block.stmts[i], indent + 1, 1, has_lines << 1 | 1);
            }
        } break;
        case NODE_EXPR_IF: {
            printf("IfExpr\x1b[0m\n");
            printAST(ast->data.expr.ifExpr.condition, indent + 1, 1, has_lines << 1 | 1);
            if (ast->data.expr.ifExpr.falseBranch != NULL) {
                printAST(ast->data.expr.ifExpr.trueBranch, indent + 1, 1, has_lines << 1 | 1);
                printAST(ast->data.expr.ifExpr.falseBranch, indent + 1, 2, has_lines << 1);
            } else {
                printAST(ast->data.expr.ifExpr.trueBranch, indent + 1, 2, has_lines << 1);
            }
        } break;
        default: {
            printf("Unknown ASTNode type...\x1b[0m\n");
        }
    }
}