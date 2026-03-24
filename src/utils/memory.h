#pragma once

#include "common.h"

#include <stdalign.h>

#define GROW_CAPACITY(oldCapacity) \
	(((oldCapacity) < 8 ? 8 : (oldCapacity) * 2))

#define GROW_BUFFER(type, ptr, count) \
	(type*)reallocate((ptr), sizeof(type) * (count))

#define FREE_BUFFER(type, ptr) \
	reallocate(ptr, 0)

#define GROWABLE_ARRAY_NEW(type, arena) \
	growableArrayCreate(arena, sizeof(type), alignof(type))

typedef struct Buffer {
	void* ptr;
	size_t capacity;
	struct Buffer* next;
} Buffer;

typedef struct {
	Buffer* head;
	Buffer* buffer;
	size_t offset;
	size_t totalAllocated;
} ArenaAllocator;

typedef struct {
	void* data;
	size_t capacity;
	size_t length;
	size_t itemSize;
	size_t alignment;
	ArenaAllocator* arena;
} GrowableArray;

void arenaInit(ArenaAllocator* arena, size_t initialSize);
void* arenaAlloc(ArenaAllocator* arena, ptrdiff_t s, ptrdiff_t align);
void arenaDestroy(ArenaAllocator* arena);
char* createOwnedString(ArenaAllocator* arena, const char* str, size_t length);

Buffer* bufferCreate(size_t initialSize);

GrowableArray growableArrayCreate(ArenaAllocator* arena, size_t itemSize, size_t alignment);
void* growableArrayGet(GrowableArray* growableArray, size_t index);
void* growableArrayPush(GrowableArray* growableArray);

void* reallocate(void* ptr, size_t newSize);