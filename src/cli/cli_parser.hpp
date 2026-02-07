#pragma once

#include "utils/memory.h"
#include <vector>
#include <string>
#include <string_view>

using std::vector, std::string, std::string_view;

struct Subcommand {
	string name;
	string description;
	vector<string> flags;
	int expectedArgs;

	Subcommand(string name, string description, int expectedArgs);
	Subcommand addFlag(string flag);
};

struct ArgsInfo {
	string_view subcommand;
	vector<const char*> flags;
	vector<const char*> args;
};

struct ArgsParser {
private:
	int argc;
	const char** argv;
	vector<Subcommand> subcommands;
	string description;
	string helpMessage;
	
public:
	ArgsParser(int argc, const char** argv);
	void printHelp();
	ArgsParser setDescription(string description);
	ArgsParser setHelpMessage(string helpMessage);
	ArgsParser addSubcommand(Subcommand commmand);
	ArgsInfo parse();
};

void cliError(const char* format, ...);