#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <format>
#include <cstdlib>
#include <utility>
#include <regex>

using std::string;
using std::vector;
namespace fs = std::filesystem;

std::pair<string, int> executeCommand(const char* cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen failed");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    int status = pclose(pipe);
    return {result, status};
}

string stripAnsi(const string& str) {
    const std::regex regex("(\\x9B|\\x1B\\[)[0-?]*[ -\\/]*[@-~]");

    return std::regex_replace(str, regex, "");
}

void getAllFromDir(vector<fs::path> &files, fs::path path, string extension) {
	for (const auto& file : fs::directory_iterator(path)) {
		if (fs::is_regular_file(file) && file.path().extension() == extension) {
			files.push_back(file);
		} else if (fs::is_directory(file)) {
			getAllFromDir(files, file, extension);
		}
	}
}

std::optional<vector<fs::path>> fetchTestFiles(fs::path testPath) {
	vector<fs::path> files;

	if (!fs::is_directory(testPath)) {
		return std::nullopt;
	}

	getAllFromDir(files, testPath, ".wr");
	return std::make_optional(files);
}

int main (int argc, char** argv) {
	if (argc != 2) {
		std::cout << "Expected 2 arguments, got " << argc << std::endl;
		return 1;
	}
	std::cout << "Fetching test files..." << std::endl;
	std::optional<vector<fs::path>> testFiles = fetchTestFiles(argv[1]);
	if (!testFiles) {
		std::cout << "Could not fetch test files from '" << argv[1] << "'\n";
		return 1;
	}

	int testsPassed = 0;
	for (auto file : testFiles.value()) {

		string command = std::format("./bin/wyrm build {}", file.string());
		auto [output, status] = executeCommand(command.c_str());

		std::ofstream outputFile(file.replace_extension(".txt"));

		if (outputFile.is_open()) {
			outputFile << stripAnsi(output);

			outputFile.close();
		} else {
			std::cout << "oh no\n";
		}
		file.replace_extension(".wr");

		int expected = file.string().find("error") == string::npos ? 0 : 1;

		if (status == expected) {
			testsPassed += 1;
		} else {
			std::cout << "\x1b[31m\uf467 Test Failed:\x1b[0m " << file << std::endl
				<< "Expected " << (expected ? "no error" : "error") << std::endl;
		}
	}
	std::cout << std::endl;
	std::cout << "\x1b[32mTests passed: " << testsPassed << "\x1b[0m\n"
		<< "\x1b[31mTests failed: " << testFiles->size() - testsPassed << "\x1b[0m\n";
}