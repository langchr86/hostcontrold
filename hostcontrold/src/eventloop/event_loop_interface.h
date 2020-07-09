#pragma once

#include <functional>
#include <memory>

#include "eventloop/event.h"
#include "utils/time_point.h"
#include "utils/duration.h"

class EventLoopInterface {
 public:
  virtual ~EventLoopInterface() = default;

  using TimerHandler = std::function<bool(const TimePoint& now)>;
  virtual std::unique_ptr<Event> CreateOneShotTimer(TimerHandler&& handler, const Duration& delay) = 0;
  virtual std::unique_ptr<Event> CreateRecurringTimer(TimerHandler&& handler, const Duration& interval) = 0;
};
