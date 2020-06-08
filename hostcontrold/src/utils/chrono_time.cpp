#include "chrono_time.h"

ChronoTime::TimePoint ChronoTime::Now() const {
  return Clock::now().time_since_epoch();
}
