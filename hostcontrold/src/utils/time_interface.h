#pragma once

#include <chrono>

class TimeInterface {
 public:
  using Clock = std::chrono::system_clock;
  using TimePoint = std::chrono::nanoseconds;
  using Duration = std::chrono::nanoseconds;

  virtual ~TimeInterface() = default;
  virtual TimePoint Now() const = 0;
};
