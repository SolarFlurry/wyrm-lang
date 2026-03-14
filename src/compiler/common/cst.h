#pragma once

#include "compiler/token.h"

typedef enum {
    CST_ERROR_TREE,
    CST_FILE,
    CST_FUNC,
    CST_PARAM_LIST,
    CST_PARAM,
    CST_BLOCK,
} TreeKind;

typedef struct CstNode CstNode;

typedef struct CstNode {
    enum {
        CST_TYPE_TOKEN,
        CST_TYPE_NODE,
    } type;
    union {
        CstNode* tree;
        Token token;
    };
} CstNode;

void printCst(CstNode* cst);