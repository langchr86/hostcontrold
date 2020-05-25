#pragma once

#include <cstdint>

#include <chrono>
#include <memory>
#include <string>

#include "config/server_machine_config.h"
#include "network/ping_interface.h"
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
                   std::shared_ptr<PingInterface> ping);
  explicit ServerController(ServerController&& other);

  /**
   * \brief The main work method for each instance. The method returns after each iteration.
   */
  void DoWork();

 private:
  const ServerMachineConfig config_;
  std::shared_ptr<WolInterface> wol_;
  std::shared_ptr<PingInterface> ping_;

  SdJournalLogger<std::string> logger_;
  bool running_;

  std::chrono::system_clock::time_point last_control_;
  std::chrono::system_clock::time_point last_client_;

  /*
   *	\brief	Method uses a WOL-package to start the server.
   */
  void StartServerIfNotRunning();

  /*
   *	\brief	Method uses a UDP-package to indicate the ShutdownDaemon on the server to shutdown the host.
   */
  void ShutdownServerIfRunning();

  /**
   * \brief	Do use SSH for remote shutdown.
   *
   * 	This needs a correct hash in ~/.ssh/known_hosts. This can be ensured by
   * 	once manually connect via ssh as root to the remote host.
   */
  void ShutdownWithSsh();

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
