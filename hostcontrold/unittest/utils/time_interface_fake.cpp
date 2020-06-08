#include "time_interface_fake.h"

TimeInterfaceFake::TimeInterfaceFake()
    : now_(std::chrono::hours(1)) {}

TimeInterfaceFake::TimePoint TimeInterfaceFake::Now() const {
  return now_;
}

void TimeInterfaceFake::Advance(const TimePoint& difference) {
  now_ += difference;
}
