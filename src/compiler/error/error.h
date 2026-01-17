#pragma once

#include "utils/common.h"
#include "compiler/token.h"

#define MAX_ERRORS 255

typedef struct {
	uint32_t line;
	uint32_t col;
	uint32_t length;
	const char* message;
} ErrorInformation;

void error(const char* message, uint32_t line, uint32_t col);
void errorFromCause(const char* message, Token* cause);
void printErrors();
size_t errorsCount();