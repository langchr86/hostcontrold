#pragma once

#include <cstdint>

#include <string>
#include <vector>
#include <chrono>

#include <utils/sd_journal_logger.hpp>

#include "config/server_machine_config.h"
#include "network/wol_interface.h"

class ServerController {
  static const char kFileOn[];
  static const char kFileOff[];
  static const char kFileKeepOn[];
  static const char kFileKeepOff[];

 public:
  ServerController(const ServerMachineConfig& config, std::shared_ptr<WolInterface> wol);
  explicit ServerController(ServerController&& other);

  /**
   * \brief The main work method for each instance. The method returns after each iteration.
   */
  void DoWork();

 private:
  const ServerMachineConfig config_;
  std::shared_ptr<WolInterface> wol_;

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

  /**
   * \brief Evaluate server state.
   */
  void PingServer();

  /**
   * \brief Try to ping all clients and return true if any client could be reached.
   */
  bool CheckClients();

  /*
   *	\brief	Checks if state of server has changed and handles the
   *			indicator files and logging of the server state.
   *
   *	\param	newState	The current state of the server at call time.
   */
  void CheckAndSignalServerState(const bool& newState);

  int Ping(const std::string& ip) const;
  bool CheckFile(const std::string& filepath) const;
  bool CreateFile(const std::string& filepath) const;
};
