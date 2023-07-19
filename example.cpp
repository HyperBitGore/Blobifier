#include "blobber.h"


int main() {
	Gore::Blob* b;
	b = Gore::Blobber::blobifiy({ {"test.txt", ""}, {"FinalProject.zip", ""}, {"test2.txt", ""}});
	Gore::Blobber::writeBlob(b, "testblob.dat");
	//testing file return, the text data can be a bit messed up, have to prune \r yourself if you don't want that
	Gore::RetFile f1 = Gore::Blobber::getFile("testblob.dat", "test2.txt", false);
	std::cout << f1.dat << "\n";
	std::ofstream f;
	f.open("testout.txt");
	std::string str;
	for (size_t i = 0; i < f1.size; i++) {
		if (((char*)f1.dat)[i] != '\r') {
			f << ((char*)f1.dat)[i];
		}
	}
	f.close();
	//testing writing a file straight from blob to disk
	Gore::Blobber::extractBlobAndWrite("testblob.dat", "FinalProject.zip", "test.zip");
	Gore::RetFile f3 = Gore::Blobber::writeBlob(b);
	char* d = (char*)f3.dat;
	f.open("testblob2.dat", std::ios::binary);
	for (size_t i = 0; i < f3.size; i++) {
		f << d[i];
	}
	//delete f3.dat;
	//std::free(dst);
	f.close();
	Gore::Blobber::extractBlobAndWrite("testblob2.dat", "FinalProject.zip", "test22.zip");
	if (f3.dat) {
		Gore::RetFile f2 = Gore::Blobber::getFile(f3.dat, f3.size, "test2.txt");
		f.open("test23.txt");
		for (size_t i = 0; i < f2.size; i++) {
			f << ((char*)f2.dat)[i];
		}
		f.close();
	}

	
	return 0;
}