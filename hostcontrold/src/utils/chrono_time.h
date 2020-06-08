#pragma once

#include "utils/time_interface.h"

class ChronoTime : public TimeInterface {
 public:
  TimePoint Now() const override;
};
