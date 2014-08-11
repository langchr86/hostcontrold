#include <stdio.h>
#include <dirent.h>
#include <fstream>

using namespace std;


int removeAllFiles(const char* path) {
    // These are data types defined in the "dirent" header
    struct dirent *next_file;
    DIR *dir;

    char filepath[256];

    dir = opendir(path);

    while ( next_file = readdir(dir) )
    {
        // build the full path for each file in the folder
        sprintf(filepath, "%s/%s", path, next_file->d_name);
        remove(filepath);
    }
    return 0;
}


bool checkAndRemoveFile(const char * filepath) {
	ifstream file(filepath);
	bool check = file;
	if (check) {
		remove(filepath);
	}
	return check;
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