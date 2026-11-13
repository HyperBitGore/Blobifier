#pragma once
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <string>
#include <tuple>

// TODO:
// - add header function
// - add compression
// - cli blobbing program 

class blob {
	private:
		class member {
			public:
			std::string compression;
			std::string name;
			size_t size;
			member (std::string compression, size_t size, std::string name) {
				this->compression = compression;
				this->size = size;
				this->name = name;
			}
			virtual ~member() = default;
			virtual std::vector<uint8_t> read() = 0;
		};
		class file_member : public member {
			public:
			std::filesystem::path path;
			file_member (std::string compression, size_t size, std::string name, std::filesystem::path path) : member (compression, size, name) {
				this->path = path;
			} 
			std::vector<uint8_t> read() override {
				std::ifstream file (path, std::ios::binary);
				if (!file) {
					return {};
				}
				uint8_t* data = new uint8_t[size];
				file.read((char*)data, size);
				file.close();
				std::vector<uint8_t> dat(data, data + size);
				delete[] data;
				return dat;
			}
		};
		class data_member : public member {
			public:
			std::vector<uint8_t> dat;
			data_member (std::string compression, size_t size, std::string name, std::vector<uint8_t> dat) : member (compression, size, name) {
				this->dat = dat;
			}
			std::vector<uint8_t> read() override {
				return dat;
			}
		};
		class blob_member : public member {
			public:
			size_t offset;
			std::filesystem::path path;
			blob_member (std::string compression, size_t size, std::string name, size_t offset, std::filesystem::path path) : member (compression, size, name) {
				this->offset = offset;
				this->path = path;
			}
			std::vector<uint8_t> read() override {
				std::ifstream file (path, std::ios::binary);
				if (!file) {
					return {};
				}
				uint8_t* data = new uint8_t[size];
				file.seekg(offset);
				file.read((char*)data, size);
				file.close();
				std::vector<uint8_t> dat(data, data + size);
				delete[] data;
				return dat;
			}
		};
		const std::string headerEnd = "end of header";
		bool write (std::function<void(uint8_t*, size_t)> writeFunction) {
			size_t cur_offset = 0; //offset after header
			size_t head_offset = 0;
			size_t block_size = blob_size;
			block_size += sizeof(headerEnd) - 1;
			std::cout << "size: " << block_size << "\n";
			//write the header for each file
			for (auto& i : members) {
				size_t name_size = i->name.size();
				writeFunction((uint8_t*)&name_size, sizeof(name_size));
				writeFunction((uint8_t*)i->name.c_str(), i->name.size());
				writeFunction((uint8_t*)&(i->size), sizeof(i->size));
				writeFunction((uint8_t*)&cur_offset, sizeof(cur_offset));
				head_offset += (i->name.size() + 2) + sizeof(size_t) * 3; //bytes for the size and offset
				cur_offset += i->size;
			}
			size_t end_size = headerEnd.size();
			writeFunction((uint8_t*)&end_size, sizeof(end_size));
			writeFunction((uint8_t*)headerEnd.c_str(), headerEnd.size());
			// write data blocks
			for (auto& i : members) {
				auto data = i->read();
				writeFunction((uint8_t*)data.data(), data.size());
			}
			return true;
		}
		bool read (std::function<void(char*, size_t)> readFunction, std::function<void(std::string, size_t, std::string, size_t)> memberCreate) {
			blob_size = 0;
			members.clear();
			// read the header
			while (true) {
				std::array<char, 256> temp{};
				size_t string_length, size, offset;
				readFunction((char*)&string_length, sizeof(string_length));
				readFunction(temp.data(), string_length);
				std::string name(temp.data());
				if (name.compare("end of header") == 0) {
					break;
				}
				readFunction((char*)&size, sizeof(size));
				readFunction((char*)&offset, sizeof(offset));
				memberCreate("none", size, name, offset);
			}
			return true;
		}
		std::vector<std::unique_ptr<member>> members;
		size_t blob_size = 0;
	public:
		blob () = default;

		// move
		blob (blob&& b) {
			this->members = std::move(b.members);
			this->blob_size = b.blob_size;
		}
		blob& operator=(blob&& b) {
			this->members = std::move(b.members);
			this->blob_size = b.blob_size;
			return *this;
		}

		// blob manipulation
		void addFile (std::filesystem::path path, std::string name) {
			size_t size = std::filesystem::file_size(path);
			blob_size += size;
			std::unique_ptr<file_member> member = std::make_unique<file_member>("none", size, name, path);
			members.push_back(std::move(member));
		}
		void addFile (std::vector<uint8_t> data, std::string name) {
			blob_size += data.size();
			std::unique_ptr<data_member> member = std::make_unique<data_member>("none", data.size(), name, data);
			members.push_back(std::move(member));
		}
		void addFile (uint8_t* data, size_t size, std::string name) {
			std::vector<uint8_t> n_data(data, data + size);
			blob_size += size;
			std::unique_ptr<data_member> member = std::make_unique<data_member>("none", n_data.size(), name, n_data);
			members.push_back(std::move(member));
		}
		std::vector<uint8_t> getFile (const std::string& name) {
			for (auto& member : members) {
				if (member->name == name) {
					return member->read();
				}
			}
			return {};
		}
		// file loading/writing
		bool serialize (std::filesystem::path save_path) {
			std::ofstream file (save_path, std::ios::binary);
			if (!file) {
				return false;
			}
			auto writeFunction = [&](uint8_t* data, size_t size) {
				file.write((char*)data, size);
			};
			bool writeSucces = write(writeFunction);

			file.close();
			return writeSucces && static_cast<bool>(file);
		}
		std::vector<uint8_t> serialize () {
			std::vector<uint8_t> out;
			auto writeFunction = [&](uint8_t* data, size_t size) {
				out.insert(out.end(), data, data + size);
			};
			bool writeSucces = write(writeFunction);
			if (!writeSucces) {
				return {};
			}
			return out;
		}
		// loads from file shouldn't be loaded into memory
		bool load (std::filesystem::path blob_path) {
			std::ifstream file(blob_path, std::ios::binary);
			if (!file) {
				return false;
			}
			blob_size = 0;
			members.clear();
			auto readFunc = [&](char* data, size_t size) {
				file.read(data, size);
			};
			std::vector<blob_member*> loaded_members;
			auto memberCreate = [&](std::string, size_t size, std::string name, size_t offset) {
				std::unique_ptr<blob_member> member = std::make_unique<blob_member>("none", size, name, offset, blob_path);
				blob_size += size;
				loaded_members.push_back(member.get());
				members.push_back(std::move(member));
			};
			bool res = read(readFunc, memberCreate);
			const size_t data_start = static_cast<size_t>(file.tellg());
			for (auto* member : loaded_members) {
				member->offset += data_start;
			}
			return res;
		}
		// leave in memory since data already there
		bool load (const std::vector<char>& data) {
			blob_size = 0;
			members.clear();
			size_t data_offset = 0;
			auto readFunc = [&](char* n_data, size_t size) {
				for (size_t i = 0; i < size && data_offset < data.size(); i++, data_offset++) {
					n_data[i] = data[data_offset];
				}
			};
			std::vector<std::tuple<std::string, size_t, std::string, size_t>> pending;
			auto memberCreate = [&](std::string compression, size_t size, std::string name, size_t offset) {
				pending.emplace_back(compression, size, name, offset);
			};
			bool res = read(readFunc, memberCreate);
			for (auto& [compression, size, name, offset] : pending) {
				std::vector<uint8_t> n_data;
				for (size_t i = 0; i < size && data_offset + offset + i < data.size(); i++) {
					n_data.push_back(data[data_offset + offset + i]);
				}
				std::unique_ptr<data_member> member = std::make_unique<data_member>(compression, size, name, n_data);
				blob_size += size;
				members.push_back(std::move(member));
			}
			return res;
		}
		std::string serializeToHeader () {
			std::string header;

			return header;
		}
		bool serializeToHeader (std::filesystem::path save_path) {

			return true;
		}
};