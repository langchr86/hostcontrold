#pragma once

#include <string>

#include "json.hpp"

class ClientMachineConfig {
 public:
  ClientMachineConfig() = default;
  ClientMachineConfig(const std::string& name, const std::string& ip);

  std::string name;
  std::string ip;
};

// conversion functions used by JSON library
void to_json(nlohmann::json& json, const ClientMachineConfig& config);     // NOLINT[runtime/references]
void from_json(const nlohmann::json& json, ClientMachineConfig& config);   // NOLINT[runtime/references]
