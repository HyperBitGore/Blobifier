#include "blobber.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "usage: blob [-o output.blob] file...\n";
		return 1;
	}

	std::filesystem::path output = "out.blob";
	std::vector<std::filesystem::path> files;
	for (int i = 1; i < argc; ++i) {
		const std::string argument = argv[i];
		if (argument == "-o") {
			if (i + 1 >= argc) {
				std::cerr << "blob: -o requires an output filename\n";
				return 1;
			}
			output = argv[++i];
		} else {
			files.emplace_back(argument);
		}
	}

	if (files.empty()) {
		std::cerr << "blob: no input files\n";
		return 1;
	}

	try {
		blob result;
		for (const auto& file : files) {
			result.addFile(file, file.filename().string());
		}
		if (!result.serialize(output)) {
			std::cerr << "blob: could not write " << output << '\n';
			return 1;
		}
	} catch (const std::exception& error) {
		std::cerr << "blob: " << error.what() << '\n';
		return 1;
	}

	return 0;
}