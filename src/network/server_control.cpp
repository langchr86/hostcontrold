#include "server_control.h"

#include <oping.h>

#include "network/wake_on_lan.h"
#include "utils/filehandling.h"

// #define DEBUG

const int ServerControl::kServerPort = 5555;
const string ServerControl::kFileOn = "on";
const string ServerControl::kFileOff = "off";
const string ServerControl::kFileKeepOn = "force_on";
const string ServerControl::kFileKeepOff = "force_off";

ServerControl::ServerControl(const string& control_dir, const Config& config, const ClientList& client_list)
    : logger_(control_dir + "/log"),
      control_dir_(control_dir + "/"),
      config_(config),
      client_list_(client_list) {
  // create directory
  const int status = mkdir(control_dir_.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (status != 0) {
    // error or already there
  }

  // remove old files
  remove((control_dir_ + kFileOn).c_str());
  remove((control_dir_ + kFileOff).c_str());

  // force reset server state to off
  running_ = true;
  CheckAndSignalServerState(false);

  // ensure shutdown timeout will be respected if service starts fresh
  last_client_ = system_clock::now();
}

ServerControl::~ServerControl() {
}

void ServerControl::DoWork() {
  // do work only in defined interval
  const system_clock::time_point current_time = system_clock::now();
  if (last_control_ + config_.control_interval > current_time) {
    return;
  }
  last_control_ = current_time;

  // check server state
  PingServer();

  // check force_on-file and start server if not already running
  // force_on has higher priority than force_off
  if (checkFile((control_dir_ + kFileKeepOn).c_str())) {
#ifdef DEBUG
    logger_.Log("[debug]\tforce_on file available");
#endif
    StartServerIfNotRunning();
    last_client_ = current_time;
    return;
  }

  // check force_off-file and stop server if running
  if (checkFile((control_dir_ + kFileKeepOff).c_str())) {
#ifdef DEBUG
    logger_.Log("[debug]\tforce_off file available");
#endif
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
    logger_.Log("[error]\tWOL failed!");
  }

#ifdef DEBUG
  logger_.Log("[debug]\tWOL sent");
#endif
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
  const string ssh_login = string("ssh ")
      + config_.ssh_user + string("@") + config_.ip;
#ifdef DEBUG
  logger_.Log("[debug]\tSSH login command: " + ssh_login);
#endif
  FILE* ssh = popen(ssh_login.c_str(), "w");
  if (ssh == NULL) {
    logger_.Log("[error]\tpopen failed!");
    return;
  }

  fputs("sudo shutdown -h now", ssh);

  pclose(ssh);
#ifdef DEBUG
  logger_.Log("[debug]\tshutdown command via SSH executed");
#endif
  return;
}

void ServerControl::PingServer() {
  // check if server is running
  const int pingRes = Ping(config_.ip);

  // server running
  if (pingRes > 0) {
#ifdef DEBUG
    logger_.Log("[debug]\tserver-ping > 0:\t" + config_.ip);
#endif
    CheckAndSignalServerState(true);

    // server not running
  } else if (pingRes == 0) {
#ifdef DEBUG
    logger_.Log("[debug]\tserver-ping == 0:\t" + config_.ip);
#endif
    CheckAndSignalServerState(false);

    // log failed ping
  } else {
    logger_.LogWithInt("[error]\tserver-ping failed!:\t" + config_.ip, pingRes);
  }
}

bool ServerControl::CheckClients() {
  // check if any client is runnning
  for (ClientList::const_iterator it = client_list_.begin(); it != client_list_.end(); ++it) {
    const int pingRes = Ping(it->ip);

    // client answer
    if (pingRes > 0) {
#ifdef DEBUG
      logger_.Log("[debug]\tclient-ping > 0:\t" + it->description);
      logger_.Log("skip other pings");
#endif
      return true;

      // no answer
    } else if (pingRes == 0) {
#ifdef DEBUG
      logger_.Log("[debug]\tclient-ping == 0:\t" + it->description);
#endif

      // log failed ping
    } else {
      logger_.LogWithInt("[error]\tclient-ping failed!:\t" + it->description, pingRes);
    }
  }

  // if no clients are running
  return false;
}

void ServerControl::CheckAndSignalServerState(const bool& newState) {
  if (newState) {
    // server changed state to running
    if (running_ == false) {
      logger_.Log("[info]\tserver started");

      // write on-file and delete off-file
      createFile((control_dir_ + kFileOn).c_str());
      remove((control_dir_ + kFileOff).c_str());
      running_ = true;
    }
  } else {
    // server changed state to stopped
    if (running_) {
      logger_.Log("[info]\tserver stopped");

      // write off-file and delete on-file
      createFile((control_dir_ + kFileOff).c_str());
      remove((control_dir_ + kFileOn).c_str());
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

  if (iter == NULL) {
    return -4;    // error
  }
  if (ping_iterator_get_info(iter, PING_INFO_LATENCY, &latency, &buffer_len) < 0) {
    return -5;    // error
  }

  // delete ressources
  ping_destroy(obj);

  // return result
  if (latency < 0.0) {
    return 0;    // timeout
  } else {
    return 1;    // ping reply recieved
  }
}

