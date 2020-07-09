#pragma once

class ServerObserverInterface {
 public:
  ~ServerObserverInterface() = default;

  virtual void NotifyServerState(bool active) = 0;
};
