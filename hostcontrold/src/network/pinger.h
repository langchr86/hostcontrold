#pragma once

#include "network/ping_interface.h"
#include "utils/logger.hpp"

class Pinger : public PingInterface {
 public:
  Pinger();

  PingResult PingHost(const std::string& host_ipv4_address) const override;

 private:
  Logger<> logger_;
};
