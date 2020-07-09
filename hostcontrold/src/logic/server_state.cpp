#include "server_state.h"

ServerState::ServerState(const std::string& server_ip,
                         const std::string& server_name,
                         std::shared_ptr<ServerObserverInterface> observer,
                         std::shared_ptr<PingInterface> ping)
    : server_ip_(server_ip)
      , server_name_(server_name)
      , observer_(observer)
      , ping_(ping)
      , logger_(__FILE__, "ServerState", {"HOST=%s"}, &server_name_) {
}

void ServerState::DoWork() {
  const auto result = ping_->PingHost(server_ip_);
  switch (result) {
    case PingResult::kHostActive:logger_.SdLogDebug("server-ping: host is running.");
      observer_->NotifyServerState(true);
      break;
    case PingResult::kHostInactive:logger_.SdLogDebug("server-ping: host does not answer.");
      observer_->NotifyServerState(false);
      break;
    default:logger_.SdLogErr("server-ping: failed");
      break;
  }
}
