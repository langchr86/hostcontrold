#pragma once

#include <gmock/gmock.h>

#include "network/ping_interface.h"

class PingInterfaceMock : public PingInterface {
 public:
  MOCK_METHOD(PingResult, PingHost, (const std::string&), (const override));
};
