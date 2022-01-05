#include "pinger.h"

#include "utils/exec.h"

Pinger::Pinger()
    : logger_("Pinger", {}) {}

PingResult Pinger::PingHost(const std::string& ip) const {
  const auto command = std::string("ping -c1 -s1 -q ") + ip;

  const auto ret = Exec::Command(command);

  if (ret < 0) {
    logger_.LogErr("failed to execute command: '%s' because of: %s (%d)", command.c_str(), std::strerror(errno), ret);
    return PingResult::kFailed;
  }

  if (ret > 0) {
    return PingResult::kHostInactive;
  }

  return PingResult::kHostActive;
}
