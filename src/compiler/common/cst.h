#pragma once

#include "compiler/token.h"

typedef enum {
    CST_ERROR_TREE,
    CST_FILE,
} TreeKind;

typedef struct {
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