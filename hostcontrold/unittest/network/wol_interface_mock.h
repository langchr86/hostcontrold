#pragma once

#include <gmock/gmock.h>

#include "network/wol_interface.h"

class WolInterfaceMock : public WolInterface {
 public:
  MOCK_METHOD(bool, SendMagicPacket, (const std::string&), (const override));
};
