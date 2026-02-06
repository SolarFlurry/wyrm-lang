#include "error.h"

#include "../compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ErrorInformation errors[MAX_ERRORS];
static ErrorInformation warnings[MAX_ERRORS];
static size_t errorCount = 0;
static size_t warningCount = 0;

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
void warn(const char* message, uint32_t line, uint32_t col) {
	if (warningCount >= MAX_ERRORS) {
		printf("exceeded max warnings");
		return;
	}
	warnings[warningCount++] = (ErrorInformation) { line, col, 1, message };
}
void warnFromCause(const char* message, Token* cause) {
	if (warningCount >= MAX_ERRORS) {
		printf("exceeded max errors");
		return;
	}
	warnings[warningCount++] = (ErrorInformation) { cause->line, cause->col, cause->length, message };
}

size_t errorsCount() {
	return errorCount;
}

static void printError(ErrorInformation info, bool isWarn) {
	char* line = getLine(info.line);
	printf(
		"%s\x1b[0m %s\n \x1b[34;1m-->\x1b[0;2;3m %s:%u:%u\x1b[0m\n\x1b[34;1m%4u |\x1b[0m %s\n       ",
		isWarn ? "\x1b[33;1m[Warning]" : "\x1b[31;1m[Error]",
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
	printf(" %s occured here\x1b[0m\n", isWarn ? "Warning" : "Error");
}

void printErrors() {
	putchar('\n');
	for (int i = 0; i < warningCount; i++) {
		printError(warnings[i], true);
		putchar('\n');
	}
	for (int i = 0; i < errorCount; i++) {
		printError(errors[i], false);
		putchar('\n');
	}
}