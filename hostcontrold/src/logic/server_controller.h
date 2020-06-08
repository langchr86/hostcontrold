#pragma once

#include <cstdint>

#include <chrono>
#include <memory>
#include <string>

#include "config/server_machine_config.h"
#include "network/ping_interface.h"
#include "network/shutdown_interface.h"
#include "network/wol_interface.h"
#include "utils/file_interface.h"
#include "utils/sd_journal_logger.hpp"
#include "utils/time_interface.h"

class ServerController {
  static const char kFileOn[];
  static const char kFileOff[];
  static const char kFileKeepOn[];
  static const char kFileKeepOff[];

 public:
  ServerController(const ServerMachineConfig& config,
                   std::shared_ptr<TimeInterface> time,
                   std::shared_ptr<FileInterface> file,
                   std::shared_ptr<WolInterface> wol,
                   std::shared_ptr<PingInterface> ping,
                   std::shared_ptr<ShutdownInterface> shutdown);

  ServerController(const ServerController& other) = delete;
  ServerController(ServerController&& other) = delete;

  void DoWork();

 private:
  const ServerMachineConfig config_;
  std::shared_ptr<TimeInterface> time_;
  std::shared_ptr<FileInterface> file_;
  std::shared_ptr<WolInterface> wol_;
  std::shared_ptr<PingInterface> ping_;
  std::shared_ptr<ShutdownInterface> shutdown_;

  SdJournalLogger<std::string> logger_;
  bool running_;

  TimeInterface::TimePoint last_control_;
  TimeInterface::TimePoint last_client_;

  void StartServerIfNotRunning();
  void ShutdownServerIfRunning();

  void PingServer();
  bool CheckClients();

  /*
   *	\brief	Checks if state of server has changed and handles the
   *			indicator files and logging of the server state.
   *
   *	\param	newState	The current state of the server at call time.
   */
  void CheckAndSignalServerState(const bool& newState);
};
