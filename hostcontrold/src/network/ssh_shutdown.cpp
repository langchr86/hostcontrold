#include "ssh_shutdown.h"

#include <cerrno>
#include <cstdio>

#include "utils/exec.h"
#include "utils/logger.hpp"

static const Logger<>& GetLogger() {
  static Logger<> logger("SshShutdown", {});
  return logger;
}

bool SshShutdown::SendShutdownCommand(const std::string& host_ipv4_address, const std::string& ssh_user) const {
  const auto remote_command = "sudo shutdown -h now";
  const auto command = std::string("ssh -o ConnectTimeout=1 -o StrictHostKeyChecking=no ")
          + ssh_user + "@" + host_ipv4_address + " '" + remote_command + "'";

  const auto ret = Exec::Command(command);

  if (ret < 0) {
    GetLogger().LogErr("failed to execute command: '%s' because of: %s (%d)",
                       command.c_str(), std::strerror(errno), ret);
    return false;
  }

  if (ret == 255) {
    GetLogger().LogErr("failed to executed remote command: '%s' via SSH", remote_command);
    return false;
  }

  GetLogger().LogInfo("shutting down: %s", host_ipv4_address.c_str());
  return true;
}
