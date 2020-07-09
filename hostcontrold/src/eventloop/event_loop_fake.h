#pragma once

#include <memory>

#include "eventloop/event_loop_interface.h"
#include "eventloop/internal/event_controller_interface.h"

class EventLoopFake : public EventLoopInterface {
 public:
  EventLoopFake();

  std::unique_ptr<Event> CreateOneShotTimer(TimerHandler&& handler, const Duration& delay) override;
  std::unique_ptr<Event> CreateRecurringTimer(TimerHandler&& handler, const Duration& interval) override;

 private:
  std::shared_ptr<EventControllerInterface> controller_;
};
