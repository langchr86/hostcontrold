#include "server_machine_config.h"

void to_json(nlohmann::json& json, const ServerMachineConfig& config) {             // NOLINT[runtime/references]
  json = nlohmann::json{{"name", config.name},
                        {"ip", config.ip},
                        {"mac", config.mac},
                        {"ssh_user", config.ssh_user},
                        {"control_dir", config.control_dir},
                        {"control_interval_sec", config.control_interval.count()},
                        {"shutdown_timeout_sec", config.shutdown_timeout.count()},
                        {"clients", config.clients}
  };
}

void from_json(const nlohmann::json& json, ServerMachineConfig& config) {           // NOLINT[runtime/references]
  config.name = json.at("name").get<std::string>();
  config.ip = json.at("ip").get<std::string>();
  config.mac = json.at("mac").get<std::string>();
  config.ssh_user = json.at("ssh_user").get<std::string>();
  config.control_dir = json.at("control_dir").get<std::string>() + "/";
  config.control_interval = std::chrono::seconds{json.at("control_interval_sec").get<std::chrono::seconds::rep>()};
  config.shutdown_timeout = std::chrono::seconds{json.at("shutdown_timeout_sec").get<std::chrono::seconds::rep>()};
  config.clients = json.at("clients").get<std::vector<ClientMachineConfig>>();
}
