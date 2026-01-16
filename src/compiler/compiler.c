#include "compiler.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codegen/codegen.h"
#include "ast.h"
#include "error/error.h"
#include "utils/memory.h"

#include <stdio.h>
#include <stdlib.h>

char* compileSource;

Chunk compile(char* source) {
	ArenaAllocator allocator;
	arenaInit(&allocator, 1024);

	compileSource = source;
	
	initLexer(&allocator, source);
	ASTNode* ast = parse(&allocator);
	printf("Arena allocated %lu bytes\n", allocator.totalAllocated);
	if (errorsCount() > 0) {
		printErrors();
		arenaDestroy(&allocator);
		exit(1);
	}
	printAST(ast, 0);
	
	Chunk bytecode = generateBytecode(ast);
	arenaDestroy(&allocator);

	if (errorsCount() > 0) {
		printErrors();
		exit(1);
	}

	return bytecode;
}

char* getSource() {
	return compileSource;
}