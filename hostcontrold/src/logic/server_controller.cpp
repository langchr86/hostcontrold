#include "server_controller.h"

const char ServerController::kFileOn[] = "on";
const char ServerController::kFileOff[] = "off";
const char ServerController::kFileKeepOn[] = "force_on";
const char ServerController::kFileKeepOff[] = "force_off";

ServerController::ServerController(bool* stop_execution,
                                   const ServerMachineConfig& config,
                                   std::shared_ptr<TimeInterface> time,
                                   std::shared_ptr<FileInterface> file,
                                   std::shared_ptr<WolInterface> wol,
                                   std::shared_ptr<PingInterface> ping,
                                   std::shared_ptr<ShutdownInterface> shutdown)
    : stop_execution_(stop_execution)
    , config_(config)
    , time_(time)
    , file_(file)
    , wol_(wol)
    , ping_(ping)
    , shutdown_(shutdown)
    , logger_("ServerController", {"HOST=%s"}, &config_.name)
    , last_control_(0) {
  logger_.LogInfo("Start controlling host: %s", config_.name.c_str());

  // create control directory
  file_->CreateDirectory(config_.control_dir);

  // remove old files
  file_->RemoveFile(config_.control_dir + kFileOn);
  file_->RemoveFile(config_.control_dir + kFileOff);

  // force reset server state to off
  running_ = true;
  CheckAndSignalServerState(false);

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
    logger_.LogDebug("force_on file available");
    StartServerIfNotRunning();
    last_client_ = current_time;
    return;
  }

  // check force_off-file and stop server if running
  if (file_->CheckFileExists(config_.control_dir + kFileKeepOff)) {
    logger_.LogDebug("force_off file available");
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
  if (running_) {
    return;
  }

  // send WOL packet
  if (wol_->SendMagicPacket(config_.mac) == false) {
    logger_.LogErr("WOL failed!");
  }

  logger_.LogDebug("WOL sent");
}

void ServerController::ShutdownServerIfRunning() {
  if (running_ == false) {
    return;
  }
  shutdown_->SendShutdownCommand(config_.ip, config_.ssh_user);
}

void ServerController::PingServer() {
  const auto result = ping_->PingHost(config_.ip);
  switch (result) {
    case PingResult::kHostActive:
      logger_.LogDebug("server-ping: host is running");
      CheckAndSignalServerState(true);
      break;
    case PingResult::kHostInactive:
      logger_.LogDebug("server-ping: host does not answer");
      CheckAndSignalServerState(false);
      break;
    default:
      logger_.LogErr("server-ping: failed");
      break;
  }
}

bool ServerController::CheckClients() {
  for (const auto& client : config_.clients) {
    if (*stop_execution_) {
      return false;
    }

    const auto result = ping_->PingHost(client.ip);
    switch (result) {
      case PingResult::kHostActive:
        logger_.LogDebug("Client(%s) has answered. Skip other pings.", client.name.c_str());
        return true;
      case PingResult::kHostInactive:
        logger_.LogDebug("Client(%s) does not answer.", client.name.c_str());
        break;
      default:
        logger_.LogErr("client-ping(%s): failed", client.name.c_str());
        break;
    }
  }

  // no clients running
  return false;
}

void ServerController::CheckAndSignalServerState(const bool& newState) {
  if (newState) {
    // server changed state to running
    if (running_ == false) {
      logger_.LogInfo("server started");

      // write on-file and delete off-file
      file_->CreateEmptyFile(config_.control_dir + kFileOn);
      file_->RemoveFile(config_.control_dir + kFileOff);
      running_ = true;
    }
  } else {
    // server changed state to stopped
    if (running_) {
      logger_.LogInfo("server stopped");

      // write off-file and delete on-file
      file_->CreateEmptyFile(config_.control_dir + kFileOff);
      file_->RemoveFile(config_.control_dir + kFileOn);
      running_ = false;
    }
  }
}
