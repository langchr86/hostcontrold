#pragma once

#include <memory>
#include <string>

#include "logic/server_observer_interface.h"
#include "network/ping_interface.h"
#include "utils/sd_journal_logger.hpp"

class ServerState {
 public:
  ServerState(const std::string& server_ip,
              const std::string& server_name,
              std::shared_ptr<ServerObserverInterface> observer,
              std::shared_ptr<PingInterface> ping);

  void DoWork();

 private:
  const std::string server_ip_;
  const std::string server_name_;

  std::shared_ptr<ServerObserverInterface> observer_;
  std::shared_ptr<PingInterface> ping_;

  SdJournalLogger<std::string> logger_;
};
