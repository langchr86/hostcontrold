#include "clients_state.h"

ClientsState::ClientsState(const std::vector<ClientMachineConfig>& clients,
                           std::shared_ptr<ClientsObserverInterface> observer,
                           std::shared_ptr<PingInterface> ping)
    : clients_(clients)
      , observer_(observer)
      , ping_(ping)
      , logger_(__FILE__, "ClientsState", {}) {
}

void ClientsState::DoWork() {
  for (const auto& client : clients_) {
    const auto result = ping_->PingHost(client.ip);
    switch (result) {
      case PingResult::kHostActive:
        logger_.SdLogDebug("client-ping(%s): host is running. Skip other pings.", client.name.c_str());
        observer_->NotifyClientsActive(true);
        return;
      case PingResult::kHostInactive:
        logger_.SdLogDebug("client-ping(%s): host does not answer.", client.name.c_str());
        break;
      default:
        logger_.SdLogErr("client-ping(%s): failed", client.name.c_str());
        break;
    }
  }

  // no clients running
  observer_->NotifyClientsActive(false);
}
