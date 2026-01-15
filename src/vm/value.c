#include "value.h"

#include "utils/memory.h"
#include <stdio.h>

void initValueBuffer(ValueBuffer* buffer) {
	buffer->values = NULL;
	buffer->capacity = 0;
	buffer->length = 0;
}

void writeValueBuffer(ValueBuffer* buffer, Value value) {
	if (buffer->length >= buffer->capacity) {
		buffer->capacity = GROW_CAPACITY(buffer->capacity);
		buffer->values = GROW_BUFFER(Value, buffer->values, buffer->capacity);
	}
	buffer->values[buffer->length++] = value;
}

void freeValueBuffer(ValueBuffer* buffer) {
	FREE_BUFFER(Value, buffer->values);
}

void printValue(Value value) {
	printf("%i", value.as.i32);
}