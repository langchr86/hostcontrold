#ifndef SRC_SERVER_CONTROL_H_
#define SRC_SERVER_CONTROL_H_

#include <cstdint>

#include <string>
#include <vector>
#include <chrono>

#include "utils/logger.h"

using std::string;
using std::vector;
using std::chrono::seconds;
using std::chrono::system_clock;

class ServerControl {
  static const int kServerPort;
  static const string kFileOn;
  static const string kFileOff;
  static const string kFileKeepOn;
  static const string kFileKeepOff;

 public:
  struct Config {
    string ip;
    string mac;
    string ssh_user;
    seconds control_interval;
    seconds shutdown_timeout;

  };

  struct Machine {
    Machine(const string& i, const string& d)
        : ip(i), description(d) {
    }
    string ip;
    string description;
  };

  typedef vector<Machine> ClientList;

  ServerControl(const string& control_dir, const Config& config, const ClientList& client_list);
  virtual ~ServerControl();

  /**
   * \brief The main work method for each instance. The method returns after each iteration.
   */
  void DoWork();

 private:
  Logger logger_;

  const string control_dir_;
  const Config config_;
  const ClientList client_list_;

  bool running_;

  system_clock::time_point last_control_;
  system_clock::time_point last_client_;

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

#endif /* SRC_SERVER_CONTROL_H_ */
