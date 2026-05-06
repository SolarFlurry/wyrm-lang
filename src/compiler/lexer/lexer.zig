const std = @import("std");
const c = @import("c");

const keywords: std.StaticStringMap(c_uint) = .initComptime(.{
    .{ "let", c.TOK_KEYWORD_LET },
    .{ "mut", c.TOK_KEYWORD_MUT },
    .{ "if", c.TOK_KEYWORD_IF },
    .{ "else", c.TOK_KEYWORD_ELSE },
    .{ "struct", c.TOK_KEYWORD_STRUCT },
    .{ "while", c.TOK_KEYWORD_WHILE },
    .{ "return", c.TOK_KEYWORD_RETURN },
    .{ "break", c.TOK_KEYWORD_BREAK },
    .{ "pub", c.TOK_KEYWORD_PUB },
    .{ "test", c.TOK_KEYWORD_TEST },
    .{ "then", c.TOK_KEYWORD_THEN },
    .{ "true", c.TOK_KEYWORD_TRUE },
    .{ "const", c.TOK_KEYWORD_CONST },
    .{ "continue", c.TOK_KEYWORD_CONTINUE },
    .{ "func", c.TOK_KEYWORD_FUNC },
    .{ "for", c.TOK_KEYWORD_FOR },
    .{ "false", c.TOK_KEYWORD_FALSE },
});

const Token = c.struct_Token;
const Lexer = c.struct_Lexer;

fn advance(self: *Lexer) void {
    if (self.source[self.index] == '\n') {
        self.line += 1;
        self.col = 0;
    } else {
        self.col += 1;
    }
    self.index += 1;
}

const State = enum {
    start,
    invalid,
    comment,
    identifier,
    integer,
    float,
    equal,
    plus,
    minus,
    asterisk,
    slash,
    l_angle,
    r_angle,
};

export fn lx_init(source: [*:0]const u8) callconv(.c) Lexer {
    return .{
        .index = 0,
        .line = 0,
        .col = 0,
        .source = source,
    };
}

export fn lx_nextTok(lx: *Lexer) callconv(.c) Token {
    var result: Token = .{
        .line = lx.line,
        .col = lx.col,
        .start = lx.index,
        .length = 0,
        .type = c.TOK_UNKNOWN,
    };
    state: switch (State.start) {
        .start => switch (lx.source[lx.index]) {
            0 => return .{
                .type = c.TOK_EOF,
                .start = lx.index,
                .length = 0,
                .line = lx.line,
                .col = lx.col,
            },
            ' ', '\t', '\n', '\r' => {
                advance(lx);
                result.start = lx.index;
                result.col = lx.col;
                result.line = lx.line;
                continue :state .start;
            },
            'a'...'z', 'A'...'Z', '_' => {
                result.type = c.TOK_IDENT;
                continue :state .identifier;
            },
            '0'...'9' => {
                result.type = c.TOK_INT;
                continue :state .integer;
            },
            '=' => continue :state .equal,
            '+' => continue :state .plus,
            '-' => continue :state .minus,
            '*' => continue :state .asterisk,
            '/' => continue :state .slash,
            '<' => continue :state .l_angle,
            '>' => continue :state .r_angle,
            '(' => {
                advance(lx);
                result.type = c.TOK_LPAREN;
            },
            ')' => {
                advance(lx);
                result.type = c.TOK_RPAREN;
            },
            '[' => {
                advance(lx);
                result.type = c.TOK_LBRACK;
            },
            ']' => {
                advance(lx);
                result.type = c.TOK_RBRACK;
            },
            '{' => {
                advance(lx);
                result.type = c.TOK_LBRACE;
            },
            '}' => {
                advance(lx);
                result.type = c.TOK_RBRACE;
            },
            ';' => {
                advance(lx);
                result.type = c.TOK_SEMICOLON;
            },
            ':' => {
                advance(lx);
                result.type = c.TOK_COLON;
            },
            '!' => {
                advance(lx);
                result.type = c.TOK_BANG;
            },
            else => continue :state .invalid,
        },
        .invalid => {
            advance(lx);
            switch (lx.source[lx.index]) {
                0, '\n' => result.type = c.TOK_UNKNOWN,
                else => continue :state .invalid,
            }
        },
        .comment => {
            advance(lx);
            switch (lx.source[lx.index]) {
                0, '\n' => continue :state .start,
                else => continue :state .comment,
            }
        },
        .identifier => {
            advance(lx);
            switch (lx.source[lx.index]) {
                'a'...'z', 'A'...'Z', '_', '0'...'9', '\'' => continue :state .identifier,
                else => {
                    const ident = lx.source[result.start..lx.index];
                    if (keywords.get(ident)) |tag| {
                        result.type = tag;
                    }
                },
            }
        },
        .integer => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '0'...'9', '_' => continue :state .integer,
                '.' => {
                    advance(lx);
                    result.type = c.TOK_FLOAT;
                },
                else => {},
            }
        },
        .float => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '0'...'9', '_' => continue :state .float,
                else => {},
            }
        },
        .equal => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '=' => {
                    advance(lx);
                    result.type = c.TOK_EQ_EQ;
                },
                else => result.type = c.TOK_EQ,
            }
        },
        .plus => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '=' => {
                    advance(lx);
                    result.type = c.TOK_PLUS_EQ;
                },
                else => result.type = c.TOK_PLUS,
            }
        },
        .minus => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '=' => {
                    advance(lx);
                    result.type = c.TOK_MINUS_EQ;
                },
                else => result.type = c.TOK_MINUS,
            }
        },
        .asterisk => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '=' => {
                    advance(lx);
                    result.type = c.TOK_ASTERISK_EQ;
                },
                else => result.type = c.TOK_ASTERISK,
            }
        },
        .slash => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '=' => {
                    advance(lx);
                    result.type = c.TOK_SLASH_EQ;
                },
                '/' => {
                    advance(lx);
                    continue :state .comment;
                },
                else => result.type = c.TOK_SLASH,
            }
        },
        .l_angle => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '=' => {
                    advance(lx);
                    result.type = c.TOK_LARROW_EQ;
                },
                else => result.type = c.TOK_LARROW,
            }
        },
        .r_angle => {
            advance(lx);
            switch (lx.source[lx.index]) {
                '=' => {
                    advance(lx);
                    result.type = c.TOK_RARROW_EQ;
                },
                else => result.type = c.TOK_RARROW,
            }
        },
    }

    result.length = lx.index - result.start;
    return result;
}
