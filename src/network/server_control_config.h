#ifndef SRC_NETWORK_SERVER_CONTROL_CONFIG_H_
#define SRC_NETWORK_SERVER_CONTROL_CONFIG_H_

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
    Machine(const std::string& name, const std::string& ip) : name(name), ip(ip) {}
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

void to_json(nlohmann::json& j, const ServerControlConfig::Machine& p);
void from_json(const nlohmann::json& j, ServerControlConfig::Machine& p);

void to_json(nlohmann::json& j, const ServerControlConfig& p);
void from_json(const nlohmann::json& j, ServerControlConfig& p);

#endif  // SRC_NETWORK_SERVER_CONTROL_CONFIG_H_
