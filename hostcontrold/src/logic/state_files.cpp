#include "state_files.h"

const char StateFiles::kFileOn[] = "on";
const char StateFiles::kFileOff[] = "off";

StateFiles::StateFiles(std::shared_ptr<FileInterface> file,
                       const std::string& host_name,
                       const std::string& control_dir_path)
    : file_(file)
      , host_name_(host_name)
      , on_file_path_(control_dir_path + "/" + kFileOn)
      , off_file_path_(control_dir_path + "/" + kFileOff)
      , logger_(__FILE__, "StateFiles", {"HOST=%s"}, &host_name_)
      , active_(false) {}

void StateFiles::InitState(bool active) {
  active_ = active;
  if (active) {
    logger_.SdLogInfo("initialized state to: started");
    file_->CreateEmptyFile(on_file_path_);
    file_->RemoveFile(off_file_path_);
  } else {
    logger_.SdLogInfo("initialized state to: stopped");
    file_->CreateEmptyFile(off_file_path_);
    file_->RemoveFile(on_file_path_);
  }
}

void StateFiles::NotifyState(bool active) {
  if (active) {
    // server changed state to running
    if (active_ == false) {
      logger_.SdLogInfo("started");
      file_->CreateEmptyFile(on_file_path_);
      file_->RemoveFile(off_file_path_);
      active_ = true;
    }
  } else {
    // server changed state to stopped
    if (active_) {
      logger_.SdLogInfo("stopped");
      file_->CreateEmptyFile(off_file_path_);
      file_->RemoveFile(on_file_path_);
      active_ = false;
    }
  }
}

bool StateFiles::IsActive() const {
  return active_;
}
