#include "error.h"

#include "../compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ErrorInformation errors[MAX_ERRORS];
static ErrorInformation warnings[MAX_ERRORS];
static size_t errorCount = 0;
static size_t warningCount = 0;

static uint32_t getLineStart(uint32_t targetIdx) {
    const char* source = getSource();
	uint32_t lineStart = 0;
    uint32_t index = 0;
	while (index < targetIdx) {
        if (source[index] == 0) break;
        if (source[index] == '\n') {
            lineStart = index + 1;
        }
        index += 1;
	}
    return lineStart;
}

void error(const char* message, uint32_t line, uint32_t col, uint32_t start) {
	if (errorCount >= MAX_ERRORS) {
		printf("exceeded max errors");
		return;
	}
	errors[errorCount++] = (ErrorInformation) { line, col, 1, start, message };
}
void errorFromCause(const char* message, Token cause) {
	if (errorCount >= MAX_ERRORS) {
		printf("exceeded max errors");
		return;
	}
	errors[errorCount++] = (ErrorInformation) { cause.line, cause.col, cause.length, cause.start, message };
}
void warn(const char* message, uint32_t line, uint32_t col, uint32_t start) {
	if (warningCount >= MAX_ERRORS) {
		printf("exceeded max warnings");
		return;
	}
	warnings[warningCount++] = (ErrorInformation) { line, col, 1, start, message };
}
void warnFromCause(const char* message, Token cause) {
	if (warningCount >= MAX_ERRORS) {
		printf("exceeded max errors");
		return;
	}
	warnings[warningCount++] = (ErrorInformation) { cause.line, cause.col, cause.length, cause.start, message };
}

size_t errorsCount() {
	return errorCount;
}

static void printError(ErrorInformation info, bool isWarn) {
	printf(
		"%s\x1b[0m %s\n \x1b[34;1m-->\x1b[0;3m %s:%u:%u:%u\x1b[0m\n\x1b[34;1m%4u | \x1b[0;2m",
		isWarn ? "\x1b[33;1m[Warning]" : "\x1b[31;1m[Error]",
		info.message,
		getFilename(),
		info.line+1,
		info.col + 1,
        info.start,
		info.line+1
	);

    const char* source = getSource();
    
	uint32_t index = getLineStart(info.start);
    while (index < info.start) {
        putchar(source[index]);
        index++;
    }
    printf("\x1b[22;%sm", isWarn ? "33" : "31");
    while (index < info.start + info.length) {
        putchar(source[index]);
        index++;
    }
    printf("\x1b[0;2m");
    for (uint32_t i = info.start + info.length; source[i] != '\n'; i++) {
        putchar(source[i]);
    }
    printf("\x1b[0m\n       ");

	for (int i = 0; i < info.col; i++) {
		putchar(' ');
	}
    printf("\x1b[1;%sm", isWarn ? "33" : "31");

	for (int i = 0; i < info.length; i++) {
		putchar('^');
	}
	printf("\x1b[92;1m %s occured here\x1b[0m\n", isWarn ? "Warning" : "Error");
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
