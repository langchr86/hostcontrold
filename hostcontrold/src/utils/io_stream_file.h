#pragma once

#include "utils/file_interface.h"
#include "utils/logger.hpp"

class IOStreamFile : public FileInterface {
 public:
  IOStreamFile();

  bool CheckFileExists(const std::string& path) const override;

  bool CreateEmptyFile(const std::string& path) override;
  bool RemoveFile(const std::string& path) override;

  bool CreateDirectory(const std::string& path) override;

 private:
  Logger<> logger_;
};
