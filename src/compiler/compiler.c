#include "compiler.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codegen/codegen.h"
#include "ast.h"
#include "error/error.h"
#include "utils/memory.h"

#include <stdio.h>
#include <stdlib.h>

Compiler compiler;

Chunk compile(const char* source, const char* filename) {
	ArenaAllocator allocator;
	arenaInit(&allocator, 1024);

	compiler.source = source;
	compiler.filename = filename;
	
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

const char* getSource() {
	return compiler.source;
}
const char* getFilename() {
	return compiler.filename;
}