#include "server_control_config.h"

void to_json(nlohmann::json& j, const ServerControlConfig::Machine& p) {
  j = nlohmann::json{ {"name", p.name}, {"ip", p.ip} };
}

void from_json(const nlohmann::json& j, ServerControlConfig::Machine& p) {
  p.name = j.at("name").get<std::string>();
  p.ip = j.at("ip").get<std::string>();
}

void to_json(nlohmann::json& j, const ServerControlConfig& p) {
  j = nlohmann::json{{"name", p.name},
                     {"ip", p.ip},
                     {"mac", p.mac},
                     {"ssh_user", p.ssh_user},
                     {"control_dir", p.control_dir},
                     {"control_interval_sec", p.control_interval.count()},
                     {"shutdown_timeout_sec", p.shutdown_timeout.count()},
                     {"clients", p.clients}
  };
}

void from_json(const nlohmann::json& j, ServerControlConfig& p) {
  p.name = j.at("name").get<std::string>();
  p.ip = j.at("ip").get<std::string>();
  p.mac = j.at("mac").get<std::string>();
  p.ssh_user = j.at("ssh_user").get<std::string>();
  p.control_dir = j.at("control_dir").get<std::string>() + "/";
  p.control_interval = std::chrono::seconds{j.at("control_interval_sec").get<std::chrono::seconds::rep>()};
  p.shutdown_timeout = std::chrono::seconds{j.at("shutdown_timeout_sec").get<std::chrono::seconds::rep>()};
  p.clients = j.at("clients").get<std::vector<ServerControlConfig::Machine>>();
}
