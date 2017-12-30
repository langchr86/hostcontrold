#include <sys/types.h>
#include <sys/stat.h>

int removeAllFiles(const char* path);
bool checkAndRemoveFile(const char * filepath);
bool checkFile(const char * filepath);
bool createFile(const char * filepath);
