#include "parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/token.h"

#include <stdio.h>

static Token currentToken;
static GrowableArray events;

void printEvents() {
    size_t indent = 0;

    for (int i = 0; i < events.length; i++) {
        CstEvent event = *(CstEvent*)growableArrayGet(&events, i);
        switch (event.kind) {
            case CST_EVENT_OPEN:
                for (int j = 0; j < indent; j++) putchar('  ');
                printf("Kind %d:\n", event.open.kind);
                indent++;
                break;
            case CST_EVENT_CLOSE:
                indent--;
                break;
            case CST_EVENT_ADVANCE:
                for (int j = 0; j < indent; j++) putchar('  ');
                printf("Token '");
                printTokData(&event.advance.tok);
                printf("'\n");
                break;
        }
    }
    putchar('\n');
}

static size_t open(ArenaAllocator* arena) {
    size_t mark = events.length;
    CstEvent* slot = growableArrayPush(&events);
    slot->kind = CST_EVENT_OPEN;
    return mark;
}

static void close(ArenaAllocator* arena, size_t mark, TreeKind kind) {
    CstEvent* open = growableArrayGet(&events, mark);
    open->open.kind = kind;
    CstEvent* slot = growableArrayPush(&events);
    slot->kind = CST_EVENT_CLOSE;
}

static inline void quantumLeap() {
    CstEvent* slot = growableArrayPush(&events);
    slot->kind = CST_EVENT_ADVANCE;
    slot->advance.tok = currentToken;
    
    currentToken = *nextToken();
}

static inline Token tok() {
    return currentToken;
}

static inline TokenType current() {
    return tok().type;
}

static inline bool at(TokenType type) {
    return current() == type;
}

static inline bool eof() {
    return at(TOK_EOF);
}

static bool eat(TokenType type) {
    if (at(type)) {
        quantumLeap();
        return true;
    }
    return false;
}

static void expect(TokenType type) {
    if (eat(type)) return;
    // else error
}

static void advanceWithError(ArenaAllocator* arena) {
    size_t mark = open(arena);
    quantumLeap();
    close(arena, mark, CST_ERROR_TREE);
}

void parseCst(ArenaAllocator* arena) {
    events = growableArrayCreate(arena, sizeof(CstEvent));
    currentToken = *nextToken();
    size_t mark = open(arena);

    while (!eof()) {
        parseCstDec(arena);
    }

    close(arena, mark, CST_FILE);
}

void parseCstDec(ArenaAllocator* arena) {
    size_t mark = open(arena);
    eat(TOK_KEYWORD_PUB);

    switch (current()) {
        case TOK_KEYWORD_FUNC: {
            quantumLeap();
            expect(TOK_IDENT);
            if (at(TOK_LPAREN)) {
                parseCstParamList(arena);
            }
            if (at(TOK_LBRACE)) {
                parseCstBlock(arena);
            }
            close(arena, mark, CST_FUNC);
        } break;
        //case TOK_KEYWORD_CONST: {} break;
        default: {
            quantumLeap();
            close(arena, mark, CST_ERROR_TREE);
        } break;
    }
}

void parseCstParamList(ArenaAllocator* arena) {
    size_t mark = open(arena);
    expect(TOK_LPAREN);
    while (!at(TOK_RPAREN) && !eof()) {
        if (at(TOK_IDENT)) {
            parseCstParam(arena);
        } else break;
    }
    expect(TOK_RPAREN);

    close(arena, mark, CST_PARAM_LIST);
}

void parseCstParam(ArenaAllocator* arena) {
    size_t mark = open(arena);
    expect(TOK_IDENT);
    if (!at(TOK_RPAREN)) {
        expect(TOK_COMMA);
    }

    close(arena, mark, CST_PARAM);
}

void parseCstBlock(ArenaAllocator* arena) {
    size_t mark = open(arena);
    expect(TOK_LBRACE);
    expect(TOK_RBRACE);
    close(arena, mark, CST_BLOCK);
}