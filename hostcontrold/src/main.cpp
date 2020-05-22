#include <thread>
#include <fstream>

#include <json.hpp>

#include "utils/ignore.hpp"
#include "utils/sd_journal_logger.hpp"
#include "utils/sd_journal_logger_core.h"
#include "network/server_control.h"
#include "config/server_machine_config.h"

using json = nlohmann::json;
using namespace std::chrono_literals;   // NOLINT[build/namespaces]

static constexpr char config_path[] = "/etc/hostcontrold.conf";

int main(int argc, char* argv[]) {
  ignore_unused(argc, argv);

  SdJournalLoggerCore::SetMaxLogPriority(LOG_INFO);

  SdJournalLogger<> logger(__FILE__, "Main", {});

  // read config file
  json config;
  std::vector<ServerControl> controllers;
  std::ifstream config_stream(config_path);

  // create example config if no file exists
  if (config_stream.is_open() == false) {
    logger.SdLogErr("Found no config file under: %s", config_path);

    ServerMachineConfig example_server_config =
        {"lang-nas16", "192.168.0.6", "40:8D:5C:B6:E6:52", "clang", "/share/lang-nas16/", 5s, 10min, {} };
    example_server_config.clients.emplace_back("lang-xps13", "192.168.0.213");
    example_server_config.clients.emplace_back("lang-ct2014", "192.168.0.25");
    config.push_back(example_server_config);

    std::ofstream new_config_stream(config_path);
    new_config_stream << config.dump(2);
    logger.SdLogInfo("Created example config file under: %s", config_path);
    return 1;
  }

  // parse config file
  logger.SdLogInfo("Found existing config file under: %s", config_path);
  try {
    config_stream >> config;
  } catch (const nlohmann::detail::parse_error& e) {
    logger.SdLogErr("Config parse error: %s", e.what());
    return 1;
  }

  // create configured controllers
  for (const auto& config_object : config) {
    const ServerMachineConfig control_config(config_object);
    controllers.emplace_back(control_config);
  }

  // main loop
  while (true) {
    for (auto& controller : controllers) {
      controller.DoWork();
    }
    std::this_thread::sleep_for(1s);
  }
}
