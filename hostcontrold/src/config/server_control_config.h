#pragma once

#include <string>
#include <vector>
#include <chrono>

#include "json.hpp"

#include "config/client_machine_config.h"

class ServerControlConfig {
 public:
  std::string name;
  std::string ip;
  std::string mac;
  std::string ssh_user;
  std::string control_dir;
  std::chrono::seconds control_interval;
  std::chrono::seconds shutdown_timeout;
  std::vector<ClientMachineConfig> clients;
};

// conversion functions used by JSON library
void to_json(nlohmann::json& json, const ServerControlConfig& config);              // NOLINT[runtime/references]
void from_json(const nlohmann::json& json, ServerControlConfig& config);            // NOLINT[runtime/references]
