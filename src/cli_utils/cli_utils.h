#pragma once

#include "utils/memory.h"

typedef struct {
	const char** data;
	int length;
	int capacity;
} StringVector;

typedef struct {
	const char* name;
	const char* help;
	int expectedArgs;
	StringVector flags;
} Subcommand;

typedef struct {
	const char* helpMessage;
	const char* description;
	Subcommand subcommands[16];
	int subcommandsLength;
	int argc;
	const char** argv;
} ArgsParser;

typedef struct {
	const char* subcommand;
	StringVector flags;
	StringVector args;
} ArgsInfo;

void vectorFree(StringVector* flags);
void cliError(const char* format, ...);
bool vectorHas(StringVector* flags, const char* flag);
const char* vectorGet(StringVector* vector, int i);
void argsParserFree(ArgsParser* parser);
void argsParserInit(ArgsParser* parser, const char* description, const char* helpMessage, int argc, const char** argv);
void addSubcommand(ArgsParser* parser, const char* subcommand, const char* help, int expectedArgs, int numFlags,...);
void printHelp(ArgsParser* parser);
ArgsInfo parseArgs(ArgsParser* parser);