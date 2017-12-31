#include <stdio.h>
#include <dirent.h>
#include <fstream>

#include "filehandling.h"

using namespace std;

bool checkFile(const char * filepath) {
	ifstream file(filepath);
	return static_cast<bool>(file);
}


bool createFile(const char * filepath) {
	ifstream file(filepath);
	bool check = !file;
	if (check) {
		ofstream newFile(filepath);
		newFile.close();
	}
	return check;
}