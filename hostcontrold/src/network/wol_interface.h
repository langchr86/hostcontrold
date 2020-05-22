#pragma once

#include <string>

class WolInterface {
 public:
  virtual ~WolInterface() = default;
  virtual bool SendMagicPacket(const std::string& mac_address) const = 0;
};
