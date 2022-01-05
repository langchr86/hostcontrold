#include "utils/exec.h"

#include <csignal>

int Exec::Command(const std::string& command) {
  // block all signals
  sigset_t set;
  sigset_t old_set;
  sigfillset(&set);
  sigprocmask(SIG_BLOCK, &set, &old_set);

  const int ret = system((command + " > /dev/null 2>&1").c_str());

  // restore old signal set
  sigprocmask(SIG_SETMASK, &old_set, nullptr);

  return ret;
}
