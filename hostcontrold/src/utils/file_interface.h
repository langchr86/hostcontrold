#pragma once

#include <string>

class FileInterface {
 public:
  virtual ~FileInterface() = default;
  virtual bool CheckFileExists(const std::string& path) const = 0;

  virtual bool CreateEmptyFile(const std::string& path) = 0;
  virtual bool RemoveFile(const std::string& path) = 0;

  virtual bool CreateDirectory(const std::string& path) = 0;
};
