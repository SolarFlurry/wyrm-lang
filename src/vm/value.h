#pragma once

#include "utils/common.h"

typedef struct {
	union {
		int32_t i32;
	} as;
} Value;

typedef struct {
	enum {
		TYPE_I32,
	} type;
	Value value;
} DynValue;

typedef struct {
	Value* values;
	size_t capacity;
	size_t length;
} ValueBuffer;

void initValueBuffer(ValueBuffer* buffer);
void writeValueBuffer(ValueBuffer* buffer, Value value);
void freeValueBuffer(ValueBuffer* buffer);

void printValue(Value value);