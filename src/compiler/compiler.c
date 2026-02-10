#include "compiler.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic_analysis/semantic_analysis.h"
#include "semantic_analysis/symtable.h"
#include "codegen/codegen.h"
#include "ast.h"
#include "error/error.h"
#include "utils/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Compiler compiler;

Chunk compile(const char* source, const char* filename) {
	ArenaAllocator allocator;
	arenaInit(&allocator, 65536);

	compiler.source = source;
	compiler.filename = filename;
	
	clock_t parseStart = clock();
	initLexer(&allocator, source);
	AstNode* ast = parse(&allocator);

	if (errorsCount() > 0) {
		printf("Arena allocated %lu bytes\n", allocator.totalAllocated);
		printErrors();
		arenaDestroy(&allocator);
		exit(1);
	}
	printf("Parsing finished in %f seconds\n", (double)(clock() - parseStart)/CLOCKS_PER_SEC);

	printAST(ast, 0, 0, 0);

	clock_t typecheckStart = clock();

	Scope topScope;
	initScope(&allocator, &topScope, NULL);

	resolveNames(&allocator, ast, &topScope);
	typeCheck(&allocator, ast, &topScope);

	printf("Semantic analysis finished in %f seconds\n", (double)(clock() - typecheckStart)/CLOCKS_PER_SEC);

	if (errorsCount() > 0) {
		printf("Arena allocated %lu bytes\n", allocator.totalAllocated);
		printErrors();
		arenaDestroy(&allocator);
		exit(1);
	}
	printf("Arena allocated %lu bytes\n", allocator.totalAllocated);
	
	Chunk bytecode = generateBytecode(ast, &topScope, &allocator);
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