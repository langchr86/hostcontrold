#pragma once

#include <gmock/gmock.h>

#include "logic/state_signal_interface.h"

class StateSignalInterfaceMock : public StateSignalInterface {
 public:
  MOCK_METHOD(void, InitState, (bool active), (override));
  MOCK_METHOD(void, NotifyState, (bool active), (override));
  MOCK_METHOD(bool, IsActive, (), (const override));
};
