#pragma once

#include <string>

enum class PingResult {
  kFailed,
  kHostActive,
  kHostInactive,
};

class PingInterface {
 public:
  virtual ~PingInterface() = default;
  virtual PingResult PingHost(const std::string& host_ipv4_address) const = 0;
};
