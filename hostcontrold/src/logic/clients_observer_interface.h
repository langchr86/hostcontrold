#pragma once

class ClientsObserverInterface {
 public:
  ~ClientsObserverInterface() = default;

  virtual void NotifyClientsActive(bool some_are_active) = 0;
};
