#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logic/state_files.h"
#include "utils/file_interface_mock.h"

using ::testing::Return;
using ::testing::StrictMock;
using ::testing::_;

static constexpr char kServerName[] = "server";
static constexpr char kControlDir[] = "/dir/server";
static constexpr char kOnFile[] = "/dir/server/on";
static constexpr char kOffFile[] = "/dir/server/off";

class StateFilesTest : public ::testing::Test {
 protected:
  StateFilesTest() : file_(std::make_shared<StrictMock<FileInterfaceMock>>()) {
    state_ = std::make_shared<StateFiles>(file_, kServerName, kControlDir);
  }

  void ExpectStatusFilesChangeToOn() {
    EXPECT_CALL(*file_, CreateEmptyFile(kOnFile));
    EXPECT_CALL(*file_, RemoveFile(kOffFile));
  }

  void ExpectStatusFilesChangeToOff() {
    EXPECT_CALL(*file_, CreateEmptyFile(kOffFile));
    EXPECT_CALL(*file_, RemoveFile(kOnFile));
  }

  void ExpectNoStatusFileChanges() {
    EXPECT_CALL(*file_, CreateEmptyFile(kOnFile)).Times(0);
    EXPECT_CALL(*file_, CreateEmptyFile(kOffFile)).Times(0);
    EXPECT_CALL(*file_, RemoveFile(kOnFile)).Times(0);
    EXPECT_CALL(*file_, RemoveFile(kOffFile)).Times(0);
  }

  std::shared_ptr<FileInterfaceMock> file_;
  std::shared_ptr<StateFiles> state_;
};

TEST_F(StateFilesTest, WhenForceInitializeToInactiveThenAlwaysChangeFiles) {
  ExpectStatusFilesChangeToOff();
  state_->InitState(false);
  ASSERT_FALSE(state_->IsActive());
}

TEST_F(StateFilesTest, WhenForceInitializeToActiveThenAlwaysChangeFiles) {
  ExpectStatusFilesChangeToOn();
  state_->InitState(true);
  ASSERT_TRUE(state_->IsActive());
}

TEST_F(StateFilesTest, GivenInactiveWhenNotifyInactiveThenStatusFilesDoNotChange) {
  ExpectStatusFilesChangeToOff();
  state_->InitState(false);

  ExpectNoStatusFileChanges();
  state_->NotifyState(false);
  ASSERT_FALSE(state_->IsActive());
}

TEST_F(StateFilesTest, GivenInactiveWhenNotifyActiveThenStatusFilesChangeToOn) {
  ExpectStatusFilesChangeToOff();
  state_->InitState(false);

  ExpectStatusFilesChangeToOn();
  state_->NotifyState(true);
  ASSERT_TRUE(state_->IsActive());
}

TEST_F(StateFilesTest, GivenActiveWhenNotifyActiveThenStatusFilesDoNotChange) {
  ExpectStatusFilesChangeToOn();
  state_->InitState(true);

  ExpectNoStatusFileChanges();
  state_->NotifyState(true);
  ASSERT_TRUE(state_->IsActive());
}

TEST_F(StateFilesTest, GivenActiveWhenNotifyInactiveThenStatusFilesChangeToOff) {
  ExpectStatusFilesChangeToOn();
  state_->InitState(true);

  ExpectStatusFilesChangeToOff();
  state_->NotifyState(false);
  ASSERT_FALSE(state_->IsActive());
}
