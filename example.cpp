#include "blobber.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace {

std::vector<char> readFile(const std::filesystem::path& path) {
	std::ifstream file(path, std::ios::binary);
	return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void writeFile(const std::filesystem::path& path, const std::vector<char>& data) {
	std::ofstream file(path, std::ios::binary);
	file.write(data.data(), static_cast<std::streamsize>(data.size()));
}

void assertBlobDataEquals(const blob::BlobData& file, const std::vector<char>& expected) {
	assert(file.size == expected.size());
	assert(std::equal(expected.begin(), expected.end(), static_cast<const char*>(file.dat)));
}

void freeBlobData(blob::BlobData file) {
	delete[] static_cast<char*>(file.dat);
}

}

int main() {
	const auto dir = std::filesystem::temp_directory_path() / "blobifier-integrity-test";
	std::filesystem::create_directories(dir);
	const auto firstPath = dir / "first.bin";
	const auto secondPath = dir / "second.bin";
	const auto blobPath = dir / "test.blob";
	const auto extractedPath = dir / "extracted.bin";

	const std::vector<char> firstData = {'\x00', '\x01', '\x7f', '\x80', '\xff', '\n', '\r', '\x00'};
	const std::vector<char> secondData = {'a', '\n', 'b', '\r', '\x00', 'c', '\n'};
	writeFile(firstPath, firstData);
	writeFile(secondPath, secondData);

	auto* blob = blob::write::blobifiy({
		{firstPath.string(), ""},
		{secondPath.string(), ""},
	});
	assert(blob::write::writeBlob(blob, blobPath.string()));

	auto first = blob::load::getFile(blobPath.string(), firstPath.string(), false);
	assertBlobDataEquals(first, firstData);
	freeBlobData(first);
	std::cout << "[PASS] disk extraction preserves binary data\n";

	auto second = blob::load::getFile(blobPath.string(), secondPath.string(), false);
	assertBlobDataEquals(second, secondData);
	freeBlobData(second);
	std::cout << "[PASS] disk extraction preserves text delimiters and NULs\n";

	blob::load::extractBlobAndWrite(blobPath.string(), firstPath.string(), extractedPath.string());
	assert(readFile(extractedPath) == firstData);
	std::cout << "[PASS] extracted file matches original bytes\n";

	auto memoryBlob = blob::write::writeBlob(blob);
	auto memoryFile = blob::load::getFile(memoryBlob.dat, memoryBlob.size, secondPath.string());
	assertBlobDataEquals(memoryFile, secondData);
	freeBlobData(memoryFile);
	std::cout << "[PASS] memory blob extraction preserves data\n";
	delete[] static_cast<char*>(memoryBlob.dat);
	delete blob;

	std::filesystem::remove_all(dir);
	return 0;
}
