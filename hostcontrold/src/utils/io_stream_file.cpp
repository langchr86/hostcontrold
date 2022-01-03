#include "io_stream_file.h"

#include <sys/stat.h>

#include <cstring>

#include <fstream>

IOStreamFile::IOStreamFile()
    : logger_("IOStreamFile", {}) {}

bool IOStreamFile::CheckFileExists(const std::string& path) const {
  std::ifstream file(path);
  return file.is_open();
}

bool IOStreamFile::CreateEmptyFile(const std::string& path) {
  if (CheckFileExists(path)) {
    return true;
  }

  std::ofstream file(path);
  if (file.is_open() == false) {
    logger_.SdLogErr("Failed to create file: %s" , path.c_str());
    return false;
  }

  return true;
}

bool IOStreamFile::RemoveFile(const std::string& path) {
  if (CheckFileExists(path) == false) {
    return true;
  }

  const int status = std::remove(path.c_str());
  if (status != 0) {
    logger_.SdLogErr("Failed to delete file: %s: %s" , path.c_str(), std::strerror(errno));
    return false;
  }

  return true;
}

bool IOStreamFile::CreateDirectory(const std::string& path) {
  const int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  if (status != 0 && errno == EEXIST) {
    logger_.SdLogDebug("Folder already exists: %s" , path.c_str());
    return true;
  }

  if (status != 0) {
    logger_.SdLogErr("Failed to create folder: %s: %s" , path.c_str(), std::strerror(errno));
    return false;
  }

  return true;
}
