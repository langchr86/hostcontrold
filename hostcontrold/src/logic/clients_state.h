#pragma once

#include <memory>
#include <string>

#include "config/client_machine_config.h"
#include "logic/clients_observer_interface.h"
#include "network/ping_interface.h"
#include "utils/sd_journal_logger.hpp"

class ClientsState {
 public:

  ClientsState(const std::vector<ClientMachineConfig>& clients,
              std::shared_ptr<ClientsObserverInterface> observer,
              std::shared_ptr<PingInterface> ping);

  void DoWork();

 private:
  const std::vector<ClientMachineConfig> clients_;

  std::shared_ptr<ClientsObserverInterface> observer_;
  std::shared_ptr<PingInterface> ping_;

  SdJournalLogger<> logger_;
};
