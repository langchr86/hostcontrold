#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "config/server_machine_config.h"
#include "logic/server_controller.h"
#include "network/ping_interface_mock.h"
#include "network/shutdown_interface_mock.h"
#include "network/wol_interface_mock.h"
#include "utils/file_interface_mock.h"
#include "utils/time_interface_fake.h"

using namespace std::chrono_literals;   // NOLINT[build/namespaces]

using ::testing::Return;
using ::testing::StrictMock;
using ::testing::_;

static constexpr char kServerName[] = "server";
static constexpr char kServerIp[] = "ServerIp";
static constexpr char kServerMac[] = "MAC";
static constexpr char kServerUser[] = "user";
static constexpr char kClientName[] = "client";
static constexpr char kClientIp[] = "ClientIp";
static constexpr char kControlDir[] = "/dir/server/";
static constexpr char kOnFile[] = "/dir/server/on";
static constexpr char kOffFile[] = "/dir/server/off";
static constexpr char kForceOnFile[] = "/dir/server/force_on";
static constexpr char kForceOffFile[] = "/dir/server/force_off";
static constexpr auto kControlInterval = 5s;
static constexpr auto kShutdownTimeout = 10min;

class ServerControllerTest : public ::testing::Test {
 protected:
  ServerControllerTest()
      : time_(std::make_shared<StrictMock<TimeInterfaceFake>>())
        , file_(std::make_shared<StrictMock<FileInterfaceMock>>())
        , wol_(std::make_shared<StrictMock<WolInterfaceMock>>())
        , ping_(std::make_shared<StrictMock<PingInterfaceMock>>())
        , shutdown_(std::make_shared<StrictMock<ShutdownInterfaceMock>>()) {}

  void SetupWithConfig() {
    // create control dir
    EXPECT_CALL(*file_, CreateDirectory(kControlDir));

    // clean up and initial check
    EXPECT_CALL(*file_, RemoveFile(kOnFile)).Times(2);
    EXPECT_CALL(*file_, RemoveFile(kOffFile));
    EXPECT_CALL(*file_, CreateEmptyFile(kOffFile));

    ServerMachineConfig config =
        {kServerName, kServerIp, kServerMac, kServerUser, kControlDir, kControlInterval, kShutdownTimeout, {}};
    config.clients.emplace_back(kClientName, kClientIp);
    controller_ = std::make_shared<ServerController>(config, time_, file_, wol_, ping_, shutdown_);
  }

  void SetupWithActiveServer() {
    SetupWithConfig();
    ExpectActiveServerPing();
    ExpectStatusFilesChangeToOn();
  }

  void SetupWithStoppedServer() {
    SetupWithConfig();
    ExpectInactiveServerPing();
  }

  void IgnoreForceFileChecks() {
    EXPECT_CALL(*file_, CheckFileExists(_)).Times(2);
  }

  void ExpectActiveServerPing() {
    EXPECT_CALL(*ping_, PingHost(kServerIp)).WillOnce(Return(PingResult::kHostActive));
  }

  void ExpectInactiveServerPing() {
    EXPECT_CALL(*ping_, PingHost(kServerIp)).WillOnce(Return(PingResult::kHostInactive));
  }

  void ExpectOneActiveClientPing() {
    EXPECT_CALL(*ping_, PingHost(kClientIp)).WillOnce(Return(PingResult::kHostActive));
  }

  void ExpectNoActiveClientPing() {
    EXPECT_CALL(*ping_, PingHost(kClientIp)).WillOnce(Return(PingResult::kHostInactive));
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

  void ExpectNoServerChange() {
    EXPECT_CALL(*wol_, SendMagicPacket(_)).Times(0);
    EXPECT_CALL(*shutdown_, SendShutdownCommand(_, _)).Times(0);
  }

  void ExpectServerStart() {
    EXPECT_CALL(*wol_, SendMagicPacket(kServerMac));
    EXPECT_CALL(*shutdown_, SendShutdownCommand(_, _)).Times(0);
  }

  void ExpectServerStop() {
    EXPECT_CALL(*wol_, SendMagicPacket(_)).Times(0);
    EXPECT_CALL(*shutdown_, SendShutdownCommand(kServerIp, kServerUser));
  }

  std::shared_ptr<TimeInterfaceFake> time_;
  std::shared_ptr<FileInterfaceMock> file_;
  std::shared_ptr<WolInterfaceMock> wol_;
  std::shared_ptr<PingInterfaceMock> ping_;
  std::shared_ptr<ShutdownInterfaceMock> shutdown_;

  std::shared_ptr<ServerController> controller_;
};

TEST_F(ServerControllerTest, GivenServerIsInactiveWhenServerPingReturnsInactiveThenStatusFilesDoNotChange) {
  SetupWithStoppedServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoStatusFileChanges();
  controller_->DoWork();

  time_->Advance(kControlInterval);

  ExpectInactiveServerPing();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoStatusFileChanges();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsInactiveWhenServerPingReturnsActiveThenStatusFilesChangeToOn) {
  SetupWithStoppedServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoStatusFileChanges();
  controller_->DoWork();

  time_->Advance(kControlInterval);

  ExpectActiveServerPing();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectStatusFilesChangeToOn();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsActiveWhenServerPingReturnsActiveThenStatusFilesDoNotChange) {
  SetupWithActiveServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  controller_->DoWork();

  time_->Advance(kControlInterval);

  ExpectActiveServerPing();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoStatusFileChanges();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsActiveWhenServerPingReturnsInactiveThenStatusFilesChangeToOff) {
  SetupWithActiveServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  controller_->DoWork();

  time_->Advance(kControlInterval);

  ExpectInactiveServerPing();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectStatusFilesChangeToOff();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsInactiveWhenOneClientIsActiveThenServerStarts) {
  SetupWithStoppedServer();
  IgnoreForceFileChecks();
  ExpectOneActiveClientPing();

  ExpectServerStart();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsInactiveThenNoClientIsActiveThenNothingHappens) {
  SetupWithStoppedServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();

  ExpectNoServerChange();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsActiveThenOneClientIsActiveThenNothingHappens) {
  SetupWithActiveServer();
  IgnoreForceFileChecks();
  ExpectOneActiveClientPing();

  ExpectNoServerChange();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsActiveThenNoClientIsActiveRightBeforeShutdownTimeoutThenNothingHappens) {
  SetupWithActiveServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();

  ExpectNoServerChange();
  time_->Advance(kShutdownTimeout - 1ns);
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsActiveThenNoClientIsActiveRightAtShutdownTimeoutThenServerStops) {
  SetupWithActiveServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();

  ExpectServerStop();
  time_->Advance(kShutdownTimeout);
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsActiveThenNoClientIsActivebutTogglesThenTimeoutIsRestarted) {
  SetupWithConfig();
  ExpectStatusFilesChangeToOn();

  // client inactive
  ExpectActiveServerPing();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoServerChange();
  time_->Advance(kShutdownTimeout / 2);
  controller_->DoWork();

  // client active
  ExpectActiveServerPing();
  IgnoreForceFileChecks();
  ExpectOneActiveClientPing();
  ExpectNoServerChange();
  time_->Advance(kShutdownTimeout / 2);
  controller_->DoWork();

  // client inactive
  ExpectActiveServerPing();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoServerChange();
  time_->Advance(kShutdownTimeout / 2);
  controller_->DoWork();

  // client inactive
  ExpectActiveServerPing();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectServerStop();
  time_->Advance(kShutdownTimeout / 2);
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsInactiveThenForceOnFileExistsThenServerStarts) {
  SetupWithStoppedServer();

  EXPECT_CALL(*file_, CheckFileExists(kForceOnFile)).WillOnce(Return(true));

  ExpectServerStart();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsInactiveThenForceOnAndOffFileExistsThenOffFileIsIgnored) {
  SetupWithStoppedServer();

  EXPECT_CALL(*file_, CheckFileExists(kForceOnFile)).WillOnce(Return(true));
  EXPECT_CALL(*file_, CheckFileExists(kForceOffFile)).Times(0);

  ExpectServerStart();
  controller_->DoWork();
}

TEST_F(ServerControllerTest, GivenServerIsActiveThenForceOffFileExistsThenServerStops) {
  SetupWithActiveServer();

  EXPECT_CALL(*file_, CheckFileExists(kForceOnFile)).WillOnce(Return(false));
  EXPECT_CALL(*file_, CheckFileExists(kForceOffFile)).WillOnce(Return(true));

  ExpectServerStop();
  controller_->DoWork();
}
