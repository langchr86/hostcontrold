#pragma once

#include <gmock/gmock.h>

#include "utils/file_interface.h"

class FileInterfaceMock : public FileInterface {
 public:
  MOCK_METHOD(bool, CheckFileExists, (const std::string&), (const override));
  MOCK_METHOD(bool, CreateEmptyFile, (const std::string&), (override));
  MOCK_METHOD(bool, RemoveFile, (const std::string&), (override));
  MOCK_METHOD(bool, CreateDirectory, (const std::string&), (override));
};
