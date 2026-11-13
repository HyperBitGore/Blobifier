#include "blobber.hpp"
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

}

int main() {
	const auto dir = std::filesystem::temp_directory_path() / "blobifier-integrity-test";
	std::filesystem::create_directories(dir);
	const auto firstPath = dir / "first.bin";
	const auto secondPath = dir / "second.bin";
	const auto extractedPath = dir / "extracted.bin";
	const auto blobPath = dir / "blob.blob";
	const auto blobRoundTripPath = dir / "blob-round-trip.blob";
	const auto sourceBlobPath = dir / "source-files.blob";

	const std::vector<char> firstData = {'\x00', '\x01', '\x7f', '\x80', '\xff', '\n', '\r', '\x00'};
	const std::vector<char> secondData = {'a', '\n', 'b', '\r', '\x00', 'c', '\n'};
	writeFile(firstPath, firstData);
	writeFile(secondPath, secondData);

	blob memorySource;
	memorySource.addFile(std::vector<uint8_t>(firstData.begin(), firstData.end()), "first.bin");
	memorySource.addFile(std::vector<uint8_t>(secondData.begin(), secondData.end()), "second.bin");
	const auto serializedMemoryBlob = memorySource.serialize();
	assert(!serializedMemoryBlob.empty());
	const std::vector<char> serializedMemoryChars(serializedMemoryBlob.begin(), serializedMemoryBlob.end());

	blob memoryblob;
	assert(memoryblob.load(serializedMemoryChars));
	assert(memoryblob.serialize() == serializedMemoryBlob);
	std::cout << "[PASS] blob loads and serializes in memory\n";

	blob fileSource;
	fileSource.addFile(firstPath, "first.bin");
	fileSource.addFile(secondPath, "second.bin");
	assert(fileSource.serialize(blobPath));
	blob fileblob;
	assert(fileblob.load(blobPath));
	assert(fileblob.serialize(blobRoundTripPath));
	assert(readFile(blobRoundTripPath) == readFile(blobPath));
	std::cout << "[PASS] blob loads from and serializes to files\n";

	const auto sourceDir = std::filesystem::path(__FILE__).parent_path();
	blob sourceBlob;
	sourceBlob.addFile(sourceDir / "blobber.hpp", "blobber.hpp");
	sourceBlob.addFile(sourceDir / "example.cpp", "example.cpp");
	assert(sourceBlob.serialize(sourceBlobPath));

	blob loadedSourceBlob;
	assert(loadedSourceBlob.load(sourceBlobPath));
	const auto blobberSource = readFile(sourceDir / "blobber.hpp");
	const auto exampleSource = readFile(sourceDir / "example.cpp");
	assert(loadedSourceBlob.getFile("blobber.hpp") == std::vector<uint8_t>(blobberSource.begin(), blobberSource.end()));
	assert(loadedSourceBlob.getFile("example.cpp") == std::vector<uint8_t>(exampleSource.begin(), exampleSource.end()));
	std::cout << "[PASS] blob stores and loads source files\n";

	//std::filesystem::remove_all(dir);
	return 0;
}
