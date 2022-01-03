#include "ssh_shutdown.h"

#include <cerrno>
#include <cstdio>

#include "utils/sd_journal_logger.hpp"

static const SdJournalLogger<>& GetLogger() {
  static SdJournalLogger<> logger("SshShutdown", {});
  return logger;
}

bool SshShutdown::SendShutdownCommand(const std::string& host_ipv4_address, const std::string& ssh_user) const {
  // This needs a correct hash in ~/.ssh/known_hosts. This can be ensured by
  // once manually connect via ssh as root to the remote host.

  // do use SSH if available
  // This needs a correct hash in ~/.ssh/known_hosts. This can be ensured by
  // once manually connect via ssh as root to the remote host. In addition the root user needs a not
  // passphrase secured rsa-key that allows him to connect to the remote. Ensure to configure
  // the correct user of the remote.
  const auto ssh_login = std::string("ssh ") + ssh_user + "@" + host_ipv4_address;
  GetLogger().SdLogDebug("SSH login command: %s", ssh_login.c_str());

  FILE* ssh = popen(ssh_login.c_str(), "w");
  if (ssh == nullptr) {
    GetLogger().SdLogErr("popen('%s', 'w') failed with: %s", ssh_login.c_str(), std::strerror(errno));
    return false;
  }

  const int res = fputs("sudo shutdown -h now", ssh);
  pclose(ssh);

  if (res == EOF) {
    GetLogger().SdLogErr("fputs('sudo shutdown -h now', ssh); failed with: %s", std::strerror(errno));
    return false;
  }

  GetLogger().SdLogDebug("shutdown command via SSH executed");
  return true;
}
