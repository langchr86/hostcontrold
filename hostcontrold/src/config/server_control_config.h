#pragma once

#include <string>
#include <vector>
#include <chrono>

#include "json.hpp"

class ServerControlConfig {
 public:
  struct Machine {
    std::string name;
    std::string ip;
    Machine() = default;
    Machine(const std::string& name, const std::string& ip)
        : name(name)
        , ip(ip) {}
  };

  std::string name;
  std::string ip;
  std::string mac;
  std::string ssh_user;
  std::string control_dir;
  std::chrono::seconds control_interval;
  std::chrono::seconds shutdown_timeout;
  std::vector<Machine> clients;
};

// conversion functions used by JSON library
void to_json(nlohmann::json& json, const ServerControlConfig::Machine& config);     // NOLINT[runtime/references]
void from_json(const nlohmann::json& json, ServerControlConfig::Machine& config);   // NOLINT[runtime/references]
void to_json(nlohmann::json& json, const ServerControlConfig& config);              // NOLINT[runtime/references]
void from_json(const nlohmann::json& json, ServerControlConfig& config);            // NOLINT[runtime/references]
