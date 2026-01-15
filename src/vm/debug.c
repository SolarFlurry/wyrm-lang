#include "debug.h"

#include <stdio.h>

void disassembleChunk(Chunk* chunk, const char* name) {
	printf("== %s ==\n", name);

	for (int offset = 0; offset < chunk->length;) {
		offset = disassembleInstruction(chunk, offset);
	}
}

int simpleInstruction(const char* name, int offset) {
	printf("%s\n", name);
	return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset) {
	printf("%04d ", offset);
	if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
		printf("    |  ");
	} else {
		printf("[%4d] ", chunk->lines[offset]);
	}

	uint8_t instruction = chunk->bytecode[offset];
	switch (instruction) {
		case OP_CONSTANT: {
			printf("OP_CONSTANT  ");
			uint8_t constant = chunk->bytecode[offset + 1];
			printValue(chunk->constants.values[constant]);
			printf("\n");
			return offset + 2;
		}
		case OP_ADD:
			return simpleInstruction("OP_ADD", offset);
		case OP_SUBTRACT:
			return simpleInstruction("OP_SUBTRACT", offset);
		case OP_RETURN: 
			return simpleInstruction("OP_RETURN", offset);
		default: {
			printf("Unknown opcode %d\n", instruction);
			return offset + 1;
		}
	}
}