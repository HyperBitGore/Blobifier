#include "blobber.h"
#pragma warning(push, 0)
// Some include(s) with unfixable warnings
#include "testblob.h"
#pragma warning(pop)



int main() {
	blob::Blob* b;
	b = blob::write::blobifiy({ {"test.txt", ""}, {"FinalProject.zip", ""}, {"test2.txt", ""}});
	blob::write::writeBlob(b, "testblob.dat");
	//testing file return, the text data can be a bit messed up, have to prune \r yourself if you don't want that
	blob::RetFile f1 =	blob::load::getFile("testblob.dat", "test2.txt", false);
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
	blob::load::extractBlobAndWrite("testblob.dat", "FinalProject.zip", "test.zip");
	blob::RetFile f3 = blob::write::writeBlob(b);
	char* d = (char*)f3.dat;
	f.open("testblob2.dat", std::ios::binary);
	for (size_t i = 0; i < f3.size; i++) {
		f << d[i];
	}
	//delete f3.dat;
	//std::free(dst);
	f.close();
	blob::load::extractBlobAndWrite("testblob2.dat", "FinalProject.zip", "test22.zip");
	if (f3.dat) {
		blob::RetFile f2 =	blob::load::getFile(f3.dat, f3.size, "test2.txt");
		f.open("test23.txt");
		for (size_t i = 0; i < f2.size; i++) {
			f << ((char*)f2.dat)[i];
		}
		f.close();
	}
	unsigned char* t = loadtest_blob();
	blob::RetFile r4 = blob::load::getFile(t, 300303, "FinalProject.zip");
	f.open("test24.zip", std::ios::binary);
	for (size_t i = 0; i < r4.size; i++) {
		f << ((char*)r4.dat)[i];
	}
	f.close();
	blob::write::writeFileHeader("imgtest.jpg", "imgt.h", "img_test");

	//int t = '\"';
	//std::cout << t << "\n";
	//Gore::Blob* b2 = Gore::Blobber::blobifiy({ {"test.txt", ""}, {"FinalProject.zip", ""}, {"test2.txt", ""}});
	//Gore::Blobber::writeAsHeader(b2, "testblob.h", "test_blob");
	//std::string tem = Gore::Blobber::checkHexArray("test.txt");
	//std::cout << tem << "\n";
	return 0;
}