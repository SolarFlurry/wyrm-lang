#include "error.h"

#include "../compiler.h"
#include <stdio.h>

ErrorInformation errors[MAX_ERRORS];
size_t errorCount = 0;

static const char* getLine(uint32_t lineNum) {
	const char* line = getSource();
	int currentLine = 0;
	while (true) {
		if (currentLine == lineNum) return line;
		if (line[0] == '\0') return NULL;
		if (line[0] == '\n') {
			currentLine++;
		}
		line++;
	}
}

void error(const char* message, uint32_t line, uint32_t col) {
	if (errorCount >= MAX_ERRORS) {
		printf("exceeded max errors");
		return;
	}
	errors[errorCount++] = (ErrorInformation) { line, col, 1, message };
}
void errorFromCause(const char* message, Token* cause) {
	if (errorCount >= MAX_ERRORS) {
		printf("exceeded max errors");
		return;
	}
	errors[errorCount++] = (ErrorInformation) { cause->line, cause->col, cause->length, message };
}

size_t errorsCount() {
	return errorCount;
}

static void printError(ErrorInformation info) {
	printf(
		"\x1b[1;31m[Error]\x1b[0m: %s\n--> %s:%u:%u\n%4u | %s\n       ",
		info.message,
		getFilename(),
		info.line+1,
		info.col,
		info.line+1,
		getLine(info.line)
	);
	for (int i = 0; i < info.col; i++) {
		putchar(' ');
	}
	printf("\x1b[1;32m");
	for (int i = 0; i < info.length; i++) {
		putchar('^');
	}
	printf("\x1b[0m Error occured here\n");
}

void printErrors() {
	putchar('\n');
	for (int i = 0; i < errorCount; i++) {
		printError(errors[i]);
		putchar('\n');
	}
}