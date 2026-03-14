#pragma once

#include "compiler/common/cst.h"
#include "utils/memory.h"

typedef struct CstEvent {
    enum {
        CST_EVENT_OPEN,
        CST_EVENT_CLOSE,
        CST_EVENT_ADVANCE,
    } kind;
    union {
        struct { TreeKind kind; } open;
        struct { Token tok } advance;
    };
} CstEvent;

#ifdef __cplusplus
extern "C" {
#endif

//CstNode* makeTree(ArenaAllocator* arena);
void printEvents();

void parseCst(ArenaAllocator* arena);
void parseCstDec(ArenaAllocator* arena);
void parseCstParamList(ArenaAllocator* arena);
void parseCstParam(ArenaAllocator* arena);
void parseCstBlock(ArenaAllocator* arena);

#ifdef __cplusplus
}
#endif