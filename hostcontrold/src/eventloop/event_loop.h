#pragma once

#include "eventloop/event_loop_interface.h"
#include "eventloop/internal/event_controller_interface.h"

class EventLoop : public EventLoopInterface {
 public:

 private:
  std::shared_ptr<EventControllerInterface> controller_;
};
