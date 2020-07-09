#include "event.h"

Event::Event(const std::shared_ptr<EventControllerInterface>& controller, EventControllerInterface::EventIndex index)
    : controller_(controller), index_(index) {}

bool Event::Start() {
  if (auto c = controller_.lock()) {
    return c->Start(index_);
  } else {
    return false;
  }
}

void Event::Stop() {
  if (auto c = controller_.lock()) {
    c->Stop(index_);
  }
}

bool Event::IsRunning() const {
  if (auto c = controller_.lock()) {
    return c->IsRunning(index_);
  } else {
    return false;
  }
}
