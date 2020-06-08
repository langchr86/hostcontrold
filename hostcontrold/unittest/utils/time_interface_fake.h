#pragma once

#include "utils/time_interface.h"

class TimeInterfaceFake : public TimeInterface {
 public:
  TimeInterfaceFake();

  TimePoint Now() const override;

  void Advance(const TimePoint& difference = TimePoint(1));

 private:
  TimePoint now_;
};

