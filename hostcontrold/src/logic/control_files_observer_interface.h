#pragma once

#include "logic/control_files_action.h"

class ControlFilesObserverInterface {
 public:
  ~ControlFilesObserverInterface() = default;

  virtual void NotifyForceAction(const ControlFilesAction& active_action) = 0;
};
