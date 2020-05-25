#pragma once

#include "network/ping_interface.h"
#include "utils/sd_journal_logger.hpp"

class Pinger : public PingInterface {
 public:
  Pinger();

  PingResult PingHost(const std::string& host_ipv4_address) const override;

 private:
  int PingIntern(const std::string& ip) const;

  SdJournalLogger<> logger_;
};
