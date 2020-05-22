#include "client_machine_config.h"

ClientMachineConfig::ClientMachineConfig(const std::string& name, const std::string& ip)
    : name(name)
    , ip(ip) {}

void to_json(nlohmann::json& json, const ClientMachineConfig& config) {    // NOLINT[runtime/references]
  json = nlohmann::json{{"name", config.name}, {"ip", config.ip}};
}

void from_json(const nlohmann::json& json, ClientMachineConfig& config) {  // NOLINT[runtime/references]
  config.name = json.at("name").get<std::string>();
  config.ip = json.at("ip").get<std::string>();
}
