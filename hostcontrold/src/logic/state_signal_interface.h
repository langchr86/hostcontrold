#pragma once

class StateSignalInterface {
 public:
  ~StateSignalInterface() = default;

  virtual void InitState(bool active) = 0;

  /// \brief  Checks if state of server has changed and handles the
  ///         notification of this new state.
  virtual void NotifyState(bool active) = 0;

  virtual bool GetState() const = 0;
};
