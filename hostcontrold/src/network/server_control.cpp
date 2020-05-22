#include "server_control.h"

#include <sys/stat.h>
#include <fstream>

#include <oping.h>

#include "network/wake_on_lan.h"

const char ServerControl::kFileOn[] = "on";
const char ServerControl::kFileOff[] = "off";
const char ServerControl::kFileKeepOn[] = "force_on";
const char ServerControl::kFileKeepOff[] = "force_off";

ServerControl::ServerControl(const ServerMachineConfig& config)
    : config_(config)
    , logger_(__FILE__, "ServerControl", {"HOST=%s"}, &config_.name) {
  logger_.SdLogInfo("Start controlling host: %s", config_.name.c_str());

  // create directory
  const int status = mkdir(config_.control_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (status != 0) {
    logger_.SdLogDebug("Folder already exists: %s", config_.control_dir.c_str());
  }

  // remove old files
  remove((config_.control_dir + kFileOn).c_str());
  remove((config_.control_dir + kFileOff).c_str());

  // force reset server state to off
  running_ = true;
  CheckAndSignalServerState(false);

  // ensure shutdown timeout will be respected if service starts fresh
  last_client_ = std::chrono::system_clock::now();
}

ServerControl::ServerControl(ServerControl&& other)
    : config_(other.config_)
    , logger_(__FILE__, "ServerControl", {"HOST=%s"}, &config_.name)
    , running_(other.running_) {}

void ServerControl::DoWork() {
  // do work only in defined interval
  const auto current_time = std::chrono::system_clock::now();
  if (last_control_ + config_.control_interval > current_time) {
    return;
  }
  last_control_ = current_time;

  // check server state
  PingServer();

  // check force_on-file and start server if not already running
  // force_on has higher priority than force_off
  if (CheckFile(config_.control_dir + kFileKeepOn)) {
    logger_.SdLogDebug("force_on file available");
    StartServerIfNotRunning();
    last_client_ = current_time;
    return;
  }

  // check force_off-file and stop server if running
  if (CheckFile(config_.control_dir + kFileKeepOff)) {
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

void ServerControl::StartServerIfNotRunning() {
  // only if server not running
  if (running_) {
    return;
  }

  // send WOL packet
  if (WakeOnLan::SendWol(config_.mac) == false) {
    logger_.SdLogErr("WOL failed!");
  }

  logger_.SdLogDebug("WOL sent");
}

void ServerControl::ShutdownServerIfRunning() {
  if (running_ == false) {
    return;
  }
  ShutdownWithSsh();
}

void ServerControl::ShutdownWithSsh() {
  // do use SSH if available
  // This needs a correct hash in ~/.ssh/known_hosts. This can be ensured by
  // once manually connect via ssh as root to the remote host. In addition the root user needs a not
  // passphrase secured rsa-key that allowes him to connect to the remote. Ensure to configure
  // the correct user of the remote.
  const auto ssh_login = std::string("ssh ") + config_.ssh_user + "@" + config_.ip;
  logger_.SdLogDebug("SSH login command: %s", ssh_login.c_str());
  FILE* ssh = popen(ssh_login.c_str(), "w");
  if (ssh == nullptr) {
    logger_.SdLogErr("popen failed!");
    return;
  }

  fputs("sudo shutdown -h now", ssh);

  pclose(ssh);
  logger_.SdLogDebug("shutdown command via SSH executed");
}

void ServerControl::PingServer() {
  // check if server is running
  const int pingRes = Ping(config_.ip);

  // server running
  if (pingRes > 0) {
    logger_.SdLogDebug("server-ping: host is running");
    CheckAndSignalServerState(true);

    // server not running
  } else if (pingRes == 0) {
    logger_.SdLogDebug("server-ping: host does not answer");
    CheckAndSignalServerState(false);

    // log failed ping
  } else {
    logger_.SdLogErr("server-ping failed: %i", pingRes);
  }
}

bool ServerControl::CheckClients() {
  // check if any client is runnning
  for (auto it = config_.clients.cbegin(); it != config_.clients.cend(); ++it) {
    const int pingRes = Ping(it->ip);

    // client answer
    if (pingRes > 0) {
      logger_.SdLogDebug("Client(%s) has answered. Skip other pings.", it->name.c_str());
      return true;

      // no answer
    } else if (pingRes == 0) {
      logger_.SdLogDebug("Client(%s) does not answer.", it->name.c_str());

      // ping failed
    } else {
      logger_.SdLogErr("client-ping(%s) failed: %i", it->name.c_str(), pingRes);
    }
  }

  // if no clients are running
  return false;
}

void ServerControl::CheckAndSignalServerState(const bool& newState) {
  if (newState) {
    // server changed state to running
    if (running_ == false) {
      logger_.SdLogInfo("server started");

      // write on-file and delete off-file
      CreateFile(config_.control_dir + kFileOn);
      remove((config_.control_dir + kFileOff).c_str());
      running_ = true;
    }
  } else {
    // server changed state to stopped
    if (running_) {
      logger_.SdLogInfo("server stopped");

      // write off-file and delete on-file
      CreateFile(config_.control_dir + kFileOff);
      remove((config_.control_dir + kFileOn).c_str());
      running_ = false;
    }
  }
}

int ServerControl::Ping(const std::string& ip) const {
  // create object
  pingobj_t* obj = ping_construct();

  // add host to object
  if (ping_host_add(obj, ip.c_str()) < 0) {
    return -1;    // error
  }

  // send ICMP
  int res = ping_send(obj);
  if (res < 0) {
    return -2;    // error
  } else if (res == 0) {
    // no echo replies
  }

  // receive info
  pingobj_iter_t* iter = ping_iterator_get(obj);
  double latency = -1.0;
  size_t buffer_len = sizeof(latency);

  if (iter == nullptr) {
    return -4;    // error
  }
  if (ping_iterator_get_info(iter, PING_INFO_LATENCY, &latency, &buffer_len) < 0) {
    return -5;    // error
  }

  // delete resources
  ping_destroy(obj);

  // return result
  if (latency < 0.0) {
    return 0;    // timeout
  } else {
    return 1;    // ping reply received
  }
}

bool ServerControl::CheckFile(const std::string& filepath) const {
  std::ifstream file(filepath);
  return static_cast<bool>(file);
}

bool ServerControl::CreateFile(const std::string& filepath) const {
  std::ifstream file(filepath);
  bool check = !file;
  if (check) {
    std::ofstream newFile(filepath);
    newFile.close();
  }
  return check;
}
