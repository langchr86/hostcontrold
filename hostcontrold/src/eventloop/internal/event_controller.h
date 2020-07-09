#pragma once

#include "eventloop/internal/event_controller_interface.h"

#include <unordered_map>

class EventController : public EventControllerInterface {
 public:
  bool Start(EventIndex index) override;
  void Stop(EventIndex index) override;
  bool IsRunning(EventIndex index) const override;

 private:
  std::unordered_map<EventIndex, bool> events_;
};
