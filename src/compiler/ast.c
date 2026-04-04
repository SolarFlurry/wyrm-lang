#include "ast.h"

#include <stdio.h>

AstNode* exprToAst(ArenaAllocator* arena, ExprNode* expr) {

}
AstNode* declToAst(ArenaAllocator* arena, DeclNode* decl) {
    AstNode* ast = ARENA_ALLOC(arena, AstNode, 1);
    *ast = (AstNode){
        .data.decl = decl->data,
        .kind = decl->kind,
        .token = decl->token,
    };
    return ast;
}

static void printIndent(uint32_t indent, uint64_t has_lines, int indent_type) {
    for (int i = 0; i < indent; i++) {
        if ((1 << (indent - i) & has_lines) > 0) {
            printf("│  ");
        } else {
            printf("   ");
        }
    }
    switch (indent_type) {
        case 0: printf("   "); break;
        case 1: printf("├─ "); break;
        case 2: printf("╰─ "); break;
    }
}

void printAST(AstNode* ast, uint32_t indent, int indent_type, uint64_t has_lines) {
    printIndent(indent, has_lines, indent_type);
    printf("\x1b[36m");

    if (ast == NULL) {
        printf("NULL pointer\x1b[0m\n");
        return;
    }

    switch (ast->kind) {
        case NODE_PROGRAM: {
            printf("Program\x1b[0m:\n");
            for (int i = 0; i < ast->data.program.declCount; i++) {
                if (i == ast->data.program.declCount - 1) {
                    printAST(ast->data.program.decls[i], indent + 1, 2, has_lines << 1);
                    continue;
                }
                printAST(ast->data.program.decls[i], indent + 1, 1, has_lines << 1 | 1);
            }
        } break;
        case NODE_DECL_VAR: {
            printf("VarDecl\x1b[0m => \x1b[35m'");
            for (int i = 0; i < ast->data.decl.lvalue->length; i++) {
                putchar(ast->data.decl.lvalue->start[i]);
            }
            puts("'\x1b[0m");
            if (ast->data.decl.var.type != NULL) {
                printAST(ast->data.decl.var.type, indent + 1, 1, has_lines << 1);
            }
            printAST(ast->data.decl.var.initial, indent + 1, 2, has_lines << 1);
        } break;
        case NODE_DECL_FUNC: {
            printf("FuncDecl\x1b[0m => \x1b[35m'");
            for (int i = 0; i < ast->data.decl.lvalue->length; i++) {
                putchar(ast->data.decl.lvalue->start[i]);
            }
            puts("'\x1b[0m");
            for (int i = 0; i < ast->data.decl.func.stmtCount; i++) {
                if (i == ast->data.decl.func.stmtCount - 1) {
                    printAST(ast->data.decl.func.stmts[i], indent + 1, 2, has_lines << 1);
                    continue;
                }
                printAST(ast->data.decl.func.stmts[i], indent + 1, 1, has_lines << 1 | 1);
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
            printAST(ast->data.expr.binaryOp.lhs, indent + 1, 1, has_lines << 1 | 1);
            printAST(ast->data.expr.binaryOp.rhs, indent + 1, 2, has_lines << 1);
        } break;
        case NODE_EXPR_UNOP: {
            printf("UnaryOp\x1b[0m '%c'\n", ast->data.expr.unaryOp.op);
        } break;
        case NODE_EXPR_CALL: {
            printf("FuncCall\x1b[0m (%zu args):\n", ast->data.expr.funcCall.argsCount);
            if (ast->data.expr.funcCall.argsCount == 0)
                printAST(ast->data.expr.funcCall.func, indent + 1, 2, has_lines << 1);
            else
                printAST(ast->data.expr.funcCall.func, indent + 1, 1, has_lines << 1 | 1);
            for (int i = 0; i < ast->data.expr.funcCall.argsCount; i++) {
                if (i == ast->data.expr.funcCall.argsCount - 1) {
                    printAST(ast->data.expr.funcCall.args[i], indent + 1, 2, has_lines << 1);
                    continue;
                }
                printAST(ast->data.expr.funcCall.args[i], indent + 1, 1, has_lines << 1 | 1);
            }
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
            printf("IfExpr\x1b[0m:\n");
            printAST(ast->data.expr.ifExpr.condition, indent + 1, 1, has_lines << 1 | 1);
            if (ast->data.expr.ifExpr.falseBranch != NULL) {
                printAST(ast->data.expr.ifExpr.trueBranch, indent + 1, 1, has_lines << 1 | 1);
                printAST(ast->data.expr.ifExpr.falseBranch, indent + 1, 2, has_lines << 1);
            } else {
                printAST(ast->data.expr.ifExpr.trueBranch, indent + 1, 2, has_lines << 1);
            }
        } break;
        case NODE_EXPR_TYPE_NAMED: {
            printf("BasicType\x1b[0m => '%s'\n", ast->data.expr.type.named.name);
        } break;
        case NODE_EXPR_TYPE_FUNC: {
            printf("FuncType\x1b[0m (%lu args):\n", ast->data.expr.type.func.paramCount);

            if (ast->data.expr.type.func.paramCount == 0)
                printAST(ast->data.expr.type.func.returnType, indent + 1, 2, has_lines << 1);
            else
                printAST(ast->data.expr.type.func.returnType, indent + 1, 1, has_lines << 1 | 1);

            for (int i = 0; i < ast->data.expr.type.func.paramCount; i++) {
                if (i == ast->data.expr.type.func.paramCount - 1) {
                    printAST(ast->data.expr.type.func.paramTypes[i], indent + 1, 2, has_lines << 1);
                    continue;
                }
                printAST(ast->data.expr.type.func.paramTypes[i], indent + 1, 1, has_lines << 1 | 1);
            }
        } break;
        case NODE_EXPR_EXIT: {
            printf("ExitExpr\x1b[0m '%s'\n", ast->data.expr.exit.isReturn ? "return" : "break");
            printAST(ast->data.expr.exit.exitExpr, indent + 1, 2, has_lines << 1);
        } break;
        default: {
            printf("Unknown ASTNode type...\x1b[0m\n");
        }
    }
}