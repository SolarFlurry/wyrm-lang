#include "cli_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "utils/memory.h"

void cliError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	printf("\x1b[31;1m[Error]\x1b[0m ");
	vprintf(format, args);
	putchar('\n');
	va_end(args);
}

static void vectorInit(StringVector* flags) {
	flags->data = NULL;
	flags->length = 0;
	flags->capacity = 0;
}

static void vectorPush(StringVector* flags, const char* flag) {
	if (flags->capacity >= flags->length) {
		flags->capacity = GROW_CAPACITY(flags->capacity);
		flags->data = GROW_BUFFER(const char*, flags->data, flags->capacity);
	}
	flags->data[flags->length++] = flag;
}

void vectorFree(StringVector* flags) {
	FREE_BUFFER(const char*, flags->data);
}

const char* vectorGet(StringVector* vector, int i) {
	if (i >= vector->length) {
		printf("oh no\n");
		exit(1);
	}
	return vector->data[i];
}

bool vectorHas(StringVector* flags, const char* flag) {
	for (int i = 0; i < flags->length; i++) {
		if (strcmp(flag, flags->data[i]) == 0) {
			return true;
		}
	}
	return false;
}

void argsParserFree(ArgsParser* parser) {
	for (int i = 0; i < parser->subcommandsLength; i++) {
		vectorFree(&parser->subcommands[i].flags);
	}
}

void argsParserInit(ArgsParser* parser, const char* description, const char* helpMessage, int argc, const char** argv) {
	parser->subcommandsLength = 0;
	parser->description = description;
	parser->helpMessage = helpMessage;
	parser->argc = argc;
	parser->argv = argv;
}

void addSubcommand(ArgsParser* parser, const char* subcommand, const char* help, int expectedArgs, int numFlags,...) {
	va_list args;

	StringVector flags;
	vectorInit(&flags);

	va_start(args, numFlags);

	for (int i = 0; i < numFlags; i++) {
		vectorPush(&flags, va_arg(args, const char*));
	}

	va_end(args);

	parser->subcommands[parser->subcommandsLength++] = (Subcommand) {
		.flags = flags,
		.help = help,
		.name = subcommand,
		.expectedArgs = expectedArgs,
	};
}

void printHelp(ArgsParser* parser) {
	printf("%s\n\nUsage: %s <COMMAND> [FLAGS]\n\nCommands:\n", parser->description, parser->argv[0]);
	for (int i = 0; i < parser->subcommandsLength; i++) {
		Subcommand subcommand = parser->subcommands[i];
		printf("    %-10s %s\n", subcommand.name, subcommand.help);
	}
}

ArgsInfo parseArgs(ArgsParser* parser) {
	ArgsInfo info;
	info.subcommand = NULL;
	vectorInit(&info.flags);
	vectorInit(&info.args);

	if (parser->argc <= 1) {
		printf("%s\n", parser->helpMessage);
		exit(1);
	}

	Subcommand* subcommand = NULL;

	for (int i = 0; i < parser->subcommandsLength; i++) {
		if (strcmp(parser->subcommands[i].name, parser->argv[1]) == 0) {
			info.subcommand = parser->subcommands[i].name;
			subcommand = &parser->subcommands[i];
			break;
		}
	}
	if (info.subcommand == NULL) {
		cliError("Unknown command '%s'", parser->argv[1]);
		exit(1);
	}
	for (int i = 2; i < parser->argc; i++) {
		if (parser->argv[i][0] == '-') {
			if (!vectorHas(&subcommand->flags, parser->argv[i])) {
				cliError("Unknown flag '%s'", parser->argv[i]);
				exit(1);
			}
			vectorPush(&info.flags, parser->argv[i]);
		} else {
			vectorPush(&info.args, parser->argv[i]);
		}
	}

	if (subcommand->expectedArgs != info.args.length) {
		cliError("Expected %d arguments, got %d", subcommand->expectedArgs, info.args.length);
		exit(1);
	}

	return info;
}