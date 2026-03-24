#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdalign.h>

void* reallocate(void* ptr, size_t newSize) {
	if (newSize == 0) {
		free(ptr);
		return NULL;
	}
	void* result = realloc(ptr, newSize);
	if (result == NULL) {
		printf("reallocation failed, requested %zu bytes\n", newSize);
	}
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

void* arenaAlloc(ArenaAllocator* arena, ptrdiff_t s, ptrdiff_t align) {
	ptrdiff_t padding = -(uintptr_t)(arena->offset + arena->buffer->ptr) & (align - 1);
	arena->offset += padding;
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

char* createOwnedString(ArenaAllocator* arena, const char* str, size_t length) {
	char* c = ARENA_ALLOC(arena, char, length + 1);
	strncpy(c, str, length);
	c[length] = '\0';
	return c;
}

void arenaDestroy(ArenaAllocator* arena) {
	Buffer* current = arena->head;
	while (current != NULL) {
		void* ptr = current;
		current = current->next;
		free(ptr);
	}
}

GrowableArray growableArrayCreate(ArenaAllocator* arena, size_t itemSize, size_t alignment) {
	return (GrowableArray){
		.arena = arena,
		.capacity = 0,
		.length = 0,
		.itemSize = itemSize,
		.data = NULL,
		.alignment = alignment,
	};
}

void* growableArrayGet(GrowableArray* growableArray, size_t index) {
	if (index >= growableArray->length) {
		return NULL;
	}
	return growableArray->data + index*growableArray->itemSize;
}

void* growableArrayPush(GrowableArray* growableArray) {
	if (growableArray->length >= growableArray->capacity) {
		growableArray->capacity = GROW_CAPACITY(growableArray->capacity);
		void* ptr = arenaAlloc(growableArray->arena, growableArray->capacity * growableArray->itemSize, growableArray->alignment);
		memcpy(ptr, growableArray->data, growableArray->length * growableArray->itemSize);
		growableArray->data = ptr;
	}
	return growableArray->data + (growableArray->length++) * growableArray->itemSize;
}