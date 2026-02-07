#include "cli_parser.hpp"

#include <cstdio>
#include <iostream>
#include <cstdarg>
#include <algorithm>
#include <format>

void cliError(const char* format, ...) {
	va_list args;
	va_start(args, format);

	std::printf("\x1b[31;1m[Error]\x1b[0m ");
	std::vprintf(format, args);
	std::putchar('\n');

	va_end(args);
}

Subcommand::Subcommand(string name, string description, int expectedArgs) {
	this->name = name;
	this->expectedArgs = expectedArgs;
	this->description = description;
}

Subcommand Subcommand::addFlag(string flag) {
	this->flags.push_back(flag);
	return *this;
}

ArgsParser::ArgsParser(int argc, const char** argv) {
	this->argc = argc;
	this->argv = argv;
}

void ArgsParser::printHelp() {
	std::cout << this->description << "\n\nUsage: " << this->argv[0] << " <COMMAND> [FLAGS]\n\nCommands:\n\n";

	for (auto& subcommand : this->subcommands) {
		std::cout << "    " << std::format("{:<10}", subcommand.name) << ' ' << subcommand.description << std::endl;
	}
}

ArgsParser ArgsParser::setDescription(string description) {
	this->description = description;
	return *this;
}

ArgsParser ArgsParser::setHelpMessage(string helpMessage) {
	this->helpMessage = helpMessage;
	return *this;
}

ArgsParser ArgsParser::addSubcommand(Subcommand subcommand) {
	this->subcommands.push_back(subcommand);
	return *this;
}

ArgsInfo ArgsParser::parse() {
	if (this->argc <= 1) {
		cliError("Expected a command");
		exit(1);
	}

	ArgsInfo info;

	Subcommand* currentSubcommand = NULL;

	for (auto& subcommand : this->subcommands) {
		if (subcommand.name == this->argv[1]) {
			currentSubcommand = &subcommand;
		}
	}

	if (currentSubcommand == NULL) {
		cliError("Unknown subcommand '%s'", this->argv[1]);
		exit(1);
	}

	info.subcommand = this->argv[1];

	for (int i = 2; i < this->argc; i++) {
		if (this->argv[i][0] == '-') {
			auto it = std::find(currentSubcommand->flags.begin(), currentSubcommand->flags.end(), this->argv[i]);
			if (it == currentSubcommand->flags.end()) {
				cliError("Unknown flag %s", this->argv[i]);
				exit(1);
			}
			info.flags.push_back(this->argv[i]);
		} else {
			info.args.push_back(this->argv[i]);
		}
	}

	if (currentSubcommand->expectedArgs != info.args.size()) {
		cliError("Expected %d arguments, got %d", currentSubcommand->expectedArgs, info.args.size());
		exit(1);
	}

	return info;
}