#pragma once

#include <gmock/gmock.h>

#include "network/shutdown_interface.h"

class ShutdownInterfaceMock : public ShutdownInterface {
 public:
  MOCK_METHOD(bool, SendShutdownCommand, (const std::string&, const std::string&), (const override));
};
