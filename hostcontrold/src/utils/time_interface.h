#pragma once

#include <chrono>

#include "utils/time_point.h"

class TimeInterface {
 public:
  using Clock = std::chrono::system_clock;

  virtual ~TimeInterface() = default;
  virtual TimePoint Now() const = 0;
};
