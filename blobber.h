#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

//basic non-compressed blobs
// -generate c header for blobs so you can just include them in ur project
//add compression
// -common lossless formats
// -common lossy formats
//mix compressed and non compressed files in a blob


namespace Gore {
	struct File {
		std::string path;
		std::string compression;
	};

	struct RetFile {
		void* dat;
		size_t size;
	};

	class Blob {
	private:
		std::vector<File> files;
	public:
		void addFile(File f) {
			files.push_back(f);
		}
		std::vector<File>& getFiles() {
			return files;
		}
	};

	class Blobber {
	private:
		static size_t getFileSize(std::string path) {
			std::ifstream f;
			f.open(path, std::ios::binary);
			if (f) {
				size_t s = 0;
				std::string str;
				while (std::getline(f, str)) {
					s += str.size();
					if (!f.eof() && !f.fail()) {
						s++;
					}
				}
				f.close();
				return s;
			}
			return 0;
		}
		static size_t countOccur(std::string str, char c) {
			size_t count = 0;
			for (auto& i : str) {
				if (i == c) {
					count++;
				}
			}
			return count;
		}
		static std::string getFileName(std::string path) {
			size_t pos = 0;
			size_t c = countOccur(path, '\\');
			for (pos; pos < path.size() && c > 0; pos++) {
				if (path[pos] == '\\') {
					c--;
				}
			}
			std::string str;
			for (pos; pos < path.size(); pos++) {
				str.push_back(path[pos]);
			}
			return str;
		}
		
		static size_t getFileOffsetHeader(std::string path, std::string name) {
			std::ifstream f;
			f.open(path, std::ios::binary);
			if (!f) {
				return 0;
			}
			std::string str;
			size_t p = 0;
			while (std::getline(f, str)) {
				p = str.find(name);
				if (p != std::string::npos) {
					p += name.size() + sizeof(size_t); //skipping file size
					break;
				}
				
			}
			f.seekg(p);
			size_t off = 0;
			f.read((char*)&off, sizeof(size_t));
			f.close();
			return off;
		}
		static size_t getFileSizeHeader(std::string path, std::string name) {
			std::ifstream f;
			f.open(path, std::ios::binary);
			if (!f) {
				return 0;
			}
			std::string str;
			size_t p = 0;
			while (std::getline(f, str)) {
				p = str.find(name);
				if (p != std::string::npos) {
					p += name.size(); //skipping name
					break;
				}

			}
			f.seekg(p);
			size_t siz = 0;
			f.read((char*)&siz, sizeof(size_t));
			f.close();
			return siz;
		}
		static size_t getFileHeaderSize(std::string path, std::string name) {
			size_t siz = 0;
			std::ifstream f;
			f.open(path, std::ios::binary);
			std::string str;
			while (std::getline(f, str)) {
				if (str.find("end of header") != std::string::npos) {
					f.close();
					return siz;
				}
				siz += str.size();
				if (!f.eof() && !f.fail()) {
					siz++;
				}
			}
			return siz;
			f.close();
		}
		static size_t getFileOffsetHeader(void* dst, size_t size, std::string name) {
			
			std::string str;
			size_t p = 0;
			while (true) {
				str = readNewLine((char*)dst, size, &p);
				p = str.find(name);
				if (p != std::string::npos) {
					p += name.size() + sizeof(size_t); //skipping file size
					break;
				}
			}

			
			size_t off = 0;
			readDataFromString(str, (char*)&off, sizeof(off), p);
			
			return off;
		}
		static size_t getFileSizeHeader(void* dst, size_t size, std::string name) {
			
			std::string str;
			size_t p = 0;
			while (true) {
				str = readNewLine((char*)dst, size, &p);
				p = str.find(name);
				if (p != std::string::npos) {
					p += name.size(); //skipping file name
					break;
				}
			}
			size_t siz = 0;
			readDataFromString(str, (char*)&siz, sizeof(siz), p);
			return siz;
		}
		static size_t getFileHeaderSize(void* dst, size_t size, std::string name) {
			size_t siz = 0;
			size_t p = 0;
			std::string str;
			while (true) {
				str = readNewLine((char*)dst, size, &p);
				if (str.find("end of header") != std::string::npos) {
					return siz;
				}
				siz += str.size() + 1;
				
			}
			return siz;
			
		}

		static void writeDataPos(char* data, char* cpy, size_t size, size_t* pos) {
			for (size_t i = 0; i < size; i++) {
				data[(*pos) + i] = cpy[i];
			}
			*pos += size;
		}
		static void readDataPos(char* data, char* src, size_t size, size_t* pos) {
			for (size_t i = 0; i < size; i++, (*pos)++) {
				data[i] = src[*pos];
			}
		}
		static std::string readNewLine(char* dst, size_t size, size_t* pos) {
			std::string str = "";
			for (size_t i = *pos; dst[i] != '\n' && i < size; i++) {
				str.push_back(dst[i]);
			}
			*pos += str.size() + 1;
			return str;
		}
		static void readDataFromString(std::string str, char* dst, size_t size, size_t pos) {
			size_t j = 0;
			for (size_t i = pos; i < str.size() && j < size; i++, j++) {
				dst[j] = str[i];
			}
		}
		
	public:
		//turns a list of files into blob
		static Blob* blobifiy(std::vector<File> files) {
			Blob* b = new Blob;
			for (auto& i : files) {
				b->addFile(i);
			}
			return b;
		}
		//blob to be written and the path you want to write the file
		static bool writeBlob(Blob* blob, std::string file_path) {
			std::ofstream f;
			f.open(file_path, std::ios::binary);
			if (!f) {
				return false;
			}
			//header will contain name of file then the size of the file, and then the offset in the file
			size_t cur_offset = 0; //offset after header
			size_t head_offset = 0;
			//write the header for each file
			for (auto& i : blob->getFiles()) {
				f << getFileName(i.path);
				size_t size = getFileSize(i.path);
				f.write((char*)&size, sizeof(size));
				//f << size;
				f.write((char*)&cur_offset, sizeof(cur_offset));
				//f << cur_offset;
				head_offset += (i.path.size()) + sizeof(size_t) * 2; //bytes for the size and offset
				cur_offset += size;
			}
			f << "\nend of header\n";
			//write the data based on location in blob
			for (auto& i : blob->getFiles()) {
				std::ifstream f2;
				f2.open(i.path, std::ios::binary);
				std::string str;
				while (std::getline(f2, str)) {
					f << str;
					if (!f2.eof() && !f2.fail()) {
						f << '\n';
					}
				}
				f2.close();
			}
			f.close();
			return true;
		}
		//for creating blobs in memory, generates the memory for you
		static RetFile writeBlob(Blob* blob) {
			//header will contain name of file then the size of the file, and then the offset in the file
			size_t cur_offset = 0; //offset after header
			size_t head_offset = 0;
			//get block size needed
			size_t block_size = 0;
			for (auto& i : blob->getFiles()) {
				block_size += getFileSize(i.path);
				block_size += getFileName(i.path).size() + (sizeof(size_t) * 2);
			}
			block_size += sizeof("\nend of header\n");
			std::cout << "size: " << block_size << "\n";
			void* dst = new char[block_size];
			//*dst = std::malloc(block_size);
			char* data = (char*)dst;
			//write the header for each file
			size_t pos = 0;
			for (auto& i : blob->getFiles()) {
				std::string str = getFileName(i.path);
				writeDataPos(data, (char*)str.c_str(), str.size(), &pos);
				size_t size2 = getFileSize(i.path);
				writeDataPos(data,(char*)&size2, sizeof(size2), &pos);
				//f << size;
				writeDataPos(data, (char*)&cur_offset, sizeof(cur_offset), &pos);
				//f << cur_offset;
				head_offset += (i.path.size()) + (sizeof(size_t) * 2); //bytes for the size and offset
				cur_offset += size2;
			}
			std::string temp = "\nend of header\n";
			writeDataPos(data, (char*)temp.c_str(), temp.size(), &pos);
			//write the data based on location in blob
			for (auto& i : blob->getFiles()) {
				std::ifstream f2;
				f2.open(i.path, std::ios::binary);
				std::string str;
				while (std::getline(f2, str)) {
					writeDataPos(data, (char*)str.c_str(), str.size(), &pos);
					if (!f2.eof() && !f2.fail()) {
						char c = '\n';
						writeDataPos(data, &c, 1, &pos);
					}
					//std::cout << pos << '\n';
				}
				f2.close();
			}


			return {dst, block_size};
		}
		//writes the blob as header file to the file path
		static bool writeAsHeader(Blob* blob, std::string file_path) {
			std::ofstream f;
			f.open(file_path, std::ios::binary);
			if (!f) {
				return false;
			}
			//writing the basic c header stuff
			

			//header will contain name of file then the size of the file, and then the offset in the file
			size_t cur_offset = 0; //offset after header
			size_t head_offset = 0;
			//write the header for each file
			for (auto& i : blob->getFiles()) {
				f << getFileName(i.path);
				size_t size = getFileSize(i.path);
				f.write((char*)&size, sizeof(size));
				//f << size;
				f.write((char*)&cur_offset, sizeof(cur_offset));
				//f << cur_offset;
				head_offset += (i.path.size()) + sizeof(size_t) * 2; //bytes for the size and offset
				cur_offset += size;
			}
			f << "\nend of header\n";
			//write the data based on location in blob
			for (auto& i : blob->getFiles()) {
				std::ifstream f2;
				f2.open(i.path, std::ios::binary);
				std::string str;
				while (std::getline(f2, str)) {
					f << str;
					if (!f2.eof() && !f2.fail()) {
						f << '\n';
					}
				}
				f2.close();
			}
			//writing the end of the char* and the final stuff needed for the rest of the header

			f.close();
			return true;
		}
		//reads file from blob, from disk
		static RetFile getFile(std::string path, std::string name, bool text = false) {
			size_t size = getFileSizeHeader(path, name);
			size_t offset = getFileOffsetHeader(path, name);
			size_t header_size = getFileHeaderSize(path, name);
			header_size += 14; //skipping the last \n
			std::ifstream f;
			if (text) {
				f.open(path);
			}
			else {
				f.open(path, std::ios::binary);
			}
			//have to skip past the header
			//now going to offset from there
			RetFile file;
			f.seekg(header_size + offset);
			void* dst = std::malloc(size);
			f.read((char*)dst, size);
			file.dat = dst;
			file.size = size;
			f.close();
			return file;
		}
		//reads file from blob, from memory
		static RetFile getFile(void* loc, size_t siz, std::string name) {
			size_t size = getFileSizeHeader(loc, siz, name);
			size_t offset = getFileOffsetHeader(loc, siz, name);
			size_t header_size = getFileHeaderSize(loc, siz, name);
			header_size += 14; //skipping the last \n
			//have to skip past the header
			//now going to offset from there
			RetFile file;
			void* dst = std::malloc(size);
			size_t pos = header_size + offset;
			readDataPos((char*)dst, (char*)loc, size, &pos);
			file.dat = dst;
			file.size = size;
			
			return file;
		}
		
		static void extractBlobAndWrite(std::string path, std::string name, std::string writepath) {
			size_t size = getFileSizeHeader(path, name);
			size_t offset = getFileOffsetHeader(path, name);
			size_t header_size = getFileHeaderSize(path, name);
			header_size += 14; //skipping the last \n
			std::ifstream f;
			f.open(path, std::ios::binary);
			//have to skip past the header
			//now going to offset from there
			f.seekg(header_size + offset);
			std::ofstream f2;
			f2.open(writepath, std::ios::binary);
			for (size_t i = 0; i < size; i++) {
				char c;
				f.read(&c, 1);
				f2 << c;
			}
			f2.close();
			f.close();
		}
	};


}

