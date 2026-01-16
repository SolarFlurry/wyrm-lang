#include "error.h"

#include <stdio.h>

ErrorInformation errors[MAX_ERRORS];
size_t errorCount = 0;

void error(const char* message, uint32_t line) {
	if (errorCount >= MAX_ERRORS) {
		printf("exceeded max errors");
		return;
	}
	errors[errorCount++] = (ErrorInformation) { line, message };
}

size_t errorsCount() {
	return errorCount;
}

static void printError(ErrorInformation info) {
	printf("\x1b[31m[Error]\x1b[0m: %s\n%4u | Error occured here\n", info.message, info.line);
}

void printErrors() {
	for (int i = 0; i < errorCount; i++) {
		printError(errors[i]);
		printf("\n");
	}
}