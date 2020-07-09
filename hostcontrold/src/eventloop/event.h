#pragma once

#include <memory>

#include "eventloop/internal/event_controller_interface.h"

class Event {
 public:
  Event(const std::shared_ptr<EventControllerInterface>& controller, EventControllerInterface::EventIndex index);

  bool Start();
  void Stop();
  bool IsRunning() const;

 private:
  std::weak_ptr<EventControllerInterface> controller_;
  EventControllerInterface::EventIndex index_;
};
