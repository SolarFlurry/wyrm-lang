#include "vm.h"

#include <stdio.h>

VM vm;

static void resetStack() {
	vm.stackTop = vm.stack;
}

void push(Value value) {
	*vm.stackTop = value;
	vm.stackTop++;
}
Value pop() {
	vm.stackTop--;
	return *vm.stackTop;
}

void initVM() {
	resetStack();
}

void freeVM() {
}

InterpretResult evaluate(Chunk* chunk) {
	vm.chunk = chunk;
	vm.ip = vm.chunk->bytecode;
	#define READ_BYTE() (*vm.ip++)
	#define READ_SHORT() \
		(vm.ip += 2, (uint16_t)((vm.ip[-2] << *8) | vm.ip[-1]))
	#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

	while (true) {
		if (vm.ip >= vm.chunk->bytecode + vm.chunk->length) return INTERPRET_OK;
		uint8_t instruction = READ_BYTE();
		switch (instruction) {
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT();
				push(constant);
			} break;
			case OP_POP: pop(); break;
			case OP_GET_LOCAL: {
				push(vm.stack[READ_BYTE()]);
			} break;
			case OP_ADD: {
				push((Value){.as = pop().as.i32 + pop().as.i32});
			} break;
			case OP_SUBTRACT: {
				push((Value){.as = pop().as.i32 - pop().as.i32});
			} break;
			case OP_EQUAL: {
				push((Value){.as = pop().as.i32 == pop().as.i32});
			} break;
			case OP_LESS: {
				push((Value){.as = pop().as.i32 < pop().as.i32});
			} break;
			case OP_LESS_EQ: {
				push((Value){.as = pop().as.i32 <= pop().as.i32});
			} break;
			case OP_GREATER: {
				push((Value){.as = pop().as.i32 > pop().as.i32});
			} break;
			case OP_GREATER_EQ: {
				push((Value){.as = pop().as.i32 >= pop().as.i32});
			} break;
			case OP_JUMP: {
				uint16_t offset = READ_BYTE();
				vm.ip += offset;
			} break;
			case OP_JUMP_IF_ZERO: {
				Value value = pop();
				if (value.as.i32 != 0) break;

				uint16_t offset = READ_BYTE();
				vm.ip += offset;
			} break;
			case OP_CALL: {
				Value value = pop();
				vm.ip += value.as.i32;
			} break;
			case OP_RETURN: {
				printValue(pop());
				printf("\n");
				return INTERPRET_OK;
			} break;
			default: {
				printf("Unknown instruction %d\n", instruction);
				return INTERPRET_RUNTIME_ERROR;
			}
		}
	}

	#undef READ_CONSTANT
	#undef READ_SHORT
	#undef READ_BYTE
}