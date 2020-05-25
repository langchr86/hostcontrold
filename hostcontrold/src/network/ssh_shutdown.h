#pragma once

#include "network/shutdown_interface.h"


class SshShutdown : public ShutdownInterface {
 public:
  bool SendShutdownCommand(const std::string& host_ipv4_address, const std::string& ssh_user) const override;
};

