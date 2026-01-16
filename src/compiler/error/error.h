#pragma once

#include "utils/common.h"

#define MAX_ERRORS 255

typedef struct {
	uint32_t line;
	const char* message;
} ErrorInformation;

void error(const char* message, uint32_t line);
void printErrors();
size_t errorsCount();