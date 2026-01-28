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

int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset) {
	uint16_t jump = (uint16_t)(chunk->bytecode[offset + 1] << 8);
	jump |= chunk->bytecode[offset + 2];
	printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
	return offset + 3;
}

int disassembleInstruction(Chunk* chunk, int offset) {
	printf("%04d ", offset);

	uint8_t instruction = chunk->bytecode[offset];
	switch (instruction) {
		case OP_CONSTANT: {
			uint8_t constant = chunk->bytecode[offset + 1];
			printf("%-16s %4d '", "OP_CONSTANT", constant);
			printValue(chunk->constants.values[constant]);
			printf("'\n");
			return offset + 2;
		}
		case OP_POP:
			return simpleInstruction("OP_POP", offset);
		case OP_GET_LOCAL: {
			printf("%-16s %4d\n", "OP_GET_LOCAL", chunk->bytecode[offset + 1]);
			return offset + 2;
		}
		case OP_ADD:
			return simpleInstruction("OP_ADD", offset);
		case OP_SUBTRACT:
			return simpleInstruction("OP_SUBTRACT", offset);
		case OP_CALL:
			return jumpInstruction("OP_CALL", 1, chunk, offset);
		case OP_RETURN: 
			return simpleInstruction("OP_RETURN", offset);
		default: {
			printf("Unknown opcode 0x%x\n", instruction);
			return offset + 1;
		}
	}
}