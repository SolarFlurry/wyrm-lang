#include "vm/chunk.h"
#include "vm/debug.h"
#include "vm/vm.h"
#include "frontend/lexer/lexer.h"
#include "frontend/parser/parser.h"
#include "utils/memory.h"

#include <stdio.h>

int main () {
	ArenaAllocator allocator;
	arenaInit(&allocator, 1024);
	
	initLexer(&allocator, "1 + 2 - hello");
	parse(&allocator);
	arenaDestroy(&allocator);

	initVM();

	Chunk chunk;
	initChunk(&chunk);

	uint8_t constant = addConstant(&chunk, (Value){.as = {.i32 = 10}});
	writeChunk(&chunk, OP_CONSTANT, 7);
	writeChunk(&chunk, constant, 7);
	constant = addConstant(&chunk, (Value){.as = {.i32 = 11}});
	writeChunk(&chunk, OP_CONSTANT, 7);
	writeChunk(&chunk, constant, 7);
	writeChunk(&chunk, OP_ADD, 8);
	writeChunk(&chunk, OP_RETURN, 8);

	disassembleChunk(&chunk, "test chunk");

	evaluate(&chunk);

	freeVM();
	freeChunk(&chunk);

	return 0;
}