#pragma once

#include <cstdint>

#include <chrono>
#include <memory>
#include <string>

#include "config/server_machine_config.h"
#include "network/ping_interface.h"
#include "network/shutdown_interface.h"
#include "network/wol_interface.h"
#include "utils/sd_journal_logger.hpp"

class ServerController {
  static const char kFileOn[];
  static const char kFileOff[];
  static const char kFileKeepOn[];
  static const char kFileKeepOff[];

 public:
  ServerController(const ServerMachineConfig& config,
                   std::shared_ptr<WolInterface> wol,
                   std::shared_ptr<PingInterface> ping,
                   std::shared_ptr<ShutdownInterface> shutdown);
  explicit ServerController(ServerController&& other);

  void DoWork();

 private:
  const ServerMachineConfig config_;
  std::shared_ptr<WolInterface> wol_;
  std::shared_ptr<PingInterface> ping_;
  std::shared_ptr<ShutdownInterface> shutdown_;

  SdJournalLogger<std::string> logger_;
  bool running_;

  std::chrono::system_clock::time_point last_control_;
  std::chrono::system_clock::time_point last_client_;

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

  bool CheckFile(const std::string& filepath) const;
  bool CreateFile(const std::string& filepath) const;
};
