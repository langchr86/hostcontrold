#pragma once

#include <string>

class ShutdownInterface {
 public:
  virtual ~ShutdownInterface() = default;
  virtual bool SendShutdownCommand(const std::string& host_ipv4_address, const std::string& ssh_user) const = 0;
};
