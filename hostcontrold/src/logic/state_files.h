#pragma once

#include <memory>
#include <string>

#include "utils/file_interface.h"
#include "utils/sd_journal_logger.hpp"

#include "logic/state_signal_interface.h"

class StateFiles : public StateSignalInterface {
 public:
  static const char kFileOn[];
  static const char kFileOff[];

  StateFiles(std::shared_ptr<FileInterface> file, const std::string& host_name, const std::string& control_dir_path);

  void InitState(bool active) override;

  void NotifyState(bool active) override;

  bool IsActive() const override;

 private:
  std::shared_ptr<FileInterface> file_;
  const std::string host_name_;
  const std::string on_file_path_;
  const std::string off_file_path_;

  SdJournalLogger<std::string> logger_;
  bool active_;
};
