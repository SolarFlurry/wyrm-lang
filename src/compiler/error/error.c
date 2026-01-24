#include "error.h"

#include "../compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ErrorInformation errors[MAX_ERRORS];
size_t errorCount = 0;

static char* getLine(uint32_t lineNum) {
	const char* lineStart = getSource();
	int currentLine = 0;
	while (currentLine < lineNum) {
		lineStart = strchr(lineStart, '\n');
		if (lineStart == NULL) {
			return NULL;
		}
		lineStart += 1;
		currentLine += 1;
	}
	int lineSize;
	const char* lineEnd = strchr(lineStart, '\n');
	if (lineEnd == NULL) {
		lineSize = strlen(lineStart);
	} else {
		lineSize = lineEnd - lineStart;
	}
	char* line = (char*)malloc(sizeof(char) * (lineSize + 1));
	memcpy(line, lineStart, lineSize);
	line[lineSize] = '\0';

	return line;
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
	char* line = getLine(info.line);
	printf(
		"\x1b[1;31m[Error]\x1b[0m: %s\n--> %s:%u:%u\n%4u | %s\n       ",
		info.message,
		getFilename(),
		info.line+1,
		info.col,
		info.line+1,
		line
	);
	free(line);
	for (int i = 0; i < info.col; i++) {
		putchar(' ');
	}
	printf("\x1b[1;92m");
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