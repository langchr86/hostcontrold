#pragma once

#include <cstddef>

class EventControllerInterface {
 public:
  using EventIndex = std::size_t;

  virtual ~EventControllerInterface() = default;

  virtual bool Start(EventIndex index) = 0;
  virtual void Stop(EventIndex index) = 0;
  virtual bool IsRunning(EventIndex index) const = 0;
};
