#include "server_controller.h"

const char ServerController::kFileKeepOn[] = "force_on";
const char ServerController::kFileKeepOff[] = "force_off";

ServerController::ServerController(const ServerMachineConfig& config,
                                   std::shared_ptr<StateSignalInterface> state,
                                   std::shared_ptr<TimeInterface> time,
                                   std::shared_ptr<FileInterface> file,
                                   std::shared_ptr<WolInterface> wol,
                                   std::shared_ptr<PingInterface> ping,
                                   std::shared_ptr<ShutdownInterface> shutdown)
    : config_(config)
    , state_(state)
    , time_(time)
    , file_(file)
    , wol_(wol)
    , ping_(ping)
    , shutdown_(shutdown)
    , logger_(__FILE__, "ServerControl", {"HOST=%s"}, &config_.name)
    , last_control_(0) {
  logger_.SdLogInfo("Start controlling host: %s", config_.name.c_str());

  // create control directory
  file_->CreateDirectory(config_.control_dir);

  // force server state to off
  state_->InitState(false);

  // ensure shutdown timeout will be respected if service starts fresh
  last_client_ = time_->Now();
}

void ServerController::DoWork() {
  // do work only in defined interval
  const auto current_time = time_->Now();
  if (last_control_ + config_.control_interval > current_time) {
    return;
  }
  last_control_ = current_time;

  // check server state
  PingServer();

  // check force_on-file and start server if not already running
  // force_on has higher priority than force_off
  if (file_->CheckFileExists(config_.control_dir + kFileKeepOn)) {
    logger_.SdLogDebug("force_on file available");
    StartServerIfNotRunning();
    last_client_ = current_time;
    return;
  }

  // check force_off-file and stop server if running
  if (file_->CheckFileExists(config_.control_dir + kFileKeepOff)) {
    logger_.SdLogDebug("force_off file available");
    ShutdownServerIfRunning();
    return;
  }

  // check if clients around
  if (CheckClients()) {
    last_client_ = current_time;
    StartServerIfNotRunning();

  } else {
    // evaluate if server has to be shutdown
    if (last_client_ + config_.shutdown_timeout <= current_time) {
      ShutdownServerIfRunning();
    }
  }
}

void ServerController::StartServerIfNotRunning() {
  // only if server not running
  if (state_->GetState()) {
    return;
  }

  // send WOL packet
  if (wol_->SendMagicPacket(config_.mac) == false) {
    logger_.SdLogErr("WOL failed!");
  }

  logger_.SdLogDebug("WOL sent");
}

void ServerController::ShutdownServerIfRunning() {
  if (state_->GetState() == false) {
    return;
  }
  shutdown_->SendShutdownCommand(config_.ip, config_.ssh_user);
}

void ServerController::PingServer() {
  const auto result = ping_->PingHost(config_.ip);
  switch (result) {
    case PingResult::kHostActive:
      logger_.SdLogDebug("server-ping: host is running");
      state_->NotifyState(true);
      break;
    case PingResult::kHostInactive:
      logger_.SdLogDebug("server-ping: host does not answer");
      state_->NotifyState(false);
      break;
    default:
      logger_.SdLogErr("server-ping: failed");
      break;
  }
}

bool ServerController::CheckClients() {
  for (const auto& client : config_.clients) {
    const auto result = ping_->PingHost(client.ip);
    switch (result) {
      case PingResult::kHostActive:
        logger_.SdLogDebug("Client(%s) has answered. Skip other pings.", client.name.c_str());
        return true;
      case PingResult::kHostInactive:
        logger_.SdLogDebug("Client(%s) does not answer.", client.name.c_str());
        break;
      default:
        logger_.SdLogErr("client-ping(%s): failed", client.name.c_str());
        break;
    }
  }

  // no clients running
  return false;
}
