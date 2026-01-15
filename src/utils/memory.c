#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void* reallocate(void* ptr, size_t newSize) {
	if (newSize == 0) {
		free(ptr);
		return NULL;
	}
	void* result = realloc(ptr, newSize);
	if (result == NULL) exit(1);
	return result;
}

Buffer* bufferCreate(size_t initialSize) {
	void* raw = malloc(initialSize + sizeof(Buffer));

	Buffer* buffer = (Buffer*)raw;
	buffer->capacity = initialSize;
	buffer->next = NULL;
	buffer->ptr = raw + sizeof(Buffer);

	return buffer;
}

void arenaInit(ArenaAllocator* arena, size_t initialSize) {
	arena->buffer = bufferCreate(initialSize);
	arena->head = arena->buffer;
	arena->offset = 0;
	arena->totalAllocated = 0;
}

void* arenaAlloc(ArenaAllocator* arena, size_t s) {
	if (s > arena->buffer->capacity - arena->offset) {
		Buffer* buffer = bufferCreate(arena->buffer->capacity);
		arena->buffer->next = buffer;
		arena->buffer = buffer;
		arena->offset = 0;
	}
	void* ptr = arena->buffer->ptr + arena->offset;
	arena->offset += s;
	arena->totalAllocated += s;
	return ptr;
}

void arenaDestroy(ArenaAllocator* arena) {
	Buffer* current = arena->head;
	while (current != NULL) {
		void* ptr = current;
		current = current->next;
		free(ptr);
	}
}

GrowableArray growableArrayCreate(ArenaAllocator* arena, size_t itemSize) {
	GrowableArray growableArray = {0};
	growableArray.arena = arena;
	growableArray.capacity = 0;
	growableArray.length = 0;
	growableArray.itemSize = itemSize;
	growableArray.data = NULL;

	return growableArray;
}

void* growableArrayPush(GrowableArray* growableArray) {
	if (growableArray->length >= growableArray->capacity) {
		growableArray->capacity = GROW_CAPACITY(growableArray->capacity);
		void* ptr = arenaAlloc(growableArray->arena, growableArray->capacity * growableArray->itemSize);
		memcpy(ptr, growableArray->data, growableArray->length * growableArray->itemSize);
		growableArray->data = ptr;
	}
	return growableArray->data + (growableArray->length++) * growableArray->itemSize;
}