#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "config/server_machine_config.h"
#include "logic/server_controller.h"
#include "logic/state_signale_interface_mock.h"
#include "network/ping_interface_mock.h"
#include "network/shutdown_interface_mock.h"
#include "network/wol_interface_mock.h"
#include "utils/file_interface_mock.h"
#include "utils/time_interface_fake.h"

using namespace std::chrono_literals;   // NOLINT[build/namespaces]

using ::testing::NiceMock;
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
static constexpr char kForceOnFile[] = "/dir/server/force_on";
static constexpr char kForceOffFile[] = "/dir/server/force_off";
static constexpr auto kControlInterval = 5s;
static constexpr auto kShutdownTimeout = 10min;

class ServerControllerTest : public ::testing::Test {
 protected:
  ServerControllerTest()
      : state_(std::make_shared<NiceMock<StateSignalInterfaceMock>>())
      , time_(std::make_shared<StrictMock<TimeInterfaceFake>>())
        , file_(std::make_shared<StrictMock<FileInterfaceMock>>())
        , wol_(std::make_shared<StrictMock<WolInterfaceMock>>())
        , ping_(std::make_shared<StrictMock<PingInterfaceMock>>())
        , shutdown_(std::make_shared<StrictMock<ShutdownInterfaceMock>>()) {}

  void SetupWithConfig() {
    // create control dir
    EXPECT_CALL(*file_, CreateDirectory(kControlDir));

    ServerMachineConfig config =
        {kServerName, kServerIp, kServerMac, kServerUser, kControlDir, kControlInterval, kShutdownTimeout, {}};
    config.clients.emplace_back(kClientName, kClientIp);
    controller_ = std::make_shared<ServerController>(config, state_, time_, file_, wol_, ping_, shutdown_);
  }

  void SetupWithActiveServer() {
    SetupWithConfig();
    ExpectActiveServer();
  }

  void SetupWithStoppedServer() {
    SetupWithConfig();
    ExpectInactiveServer();
  }

  void IgnoreForceFileChecks() {
    EXPECT_CALL(*file_, CheckFileExists(_)).Times(2);
  }

  void ExpectActiveServer() {
    EXPECT_CALL(*state_, IsActive()).WillRepeatedly(Return(true));
  }

  void ExpectInactiveServer() {
    EXPECT_CALL(*state_, IsActive()).WillRepeatedly(Return(false));
  }

  void ExpectOneActiveClientPing() {
    EXPECT_CALL(*ping_, PingHost(kClientIp)).WillOnce(Return(PingResult::kHostActive));
  }

  void ExpectNoActiveClientPing() {
    EXPECT_CALL(*ping_, PingHost(kClientIp)).WillOnce(Return(PingResult::kHostInactive));
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

  std::shared_ptr<StateSignalInterfaceMock> state_;
  std::shared_ptr<TimeInterfaceFake> time_;
  std::shared_ptr<FileInterfaceMock> file_;
  std::shared_ptr<WolInterfaceMock> wol_;
  std::shared_ptr<PingInterfaceMock> ping_;
  std::shared_ptr<ShutdownInterfaceMock> shutdown_;

  std::shared_ptr<ServerController> controller_;
};

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

  // client inactive
  ExpectActiveServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoServerChange();
  time_->Advance(kShutdownTimeout / 2);
  controller_->DoWork();

  // client active
  ExpectActiveServer();
  IgnoreForceFileChecks();
  ExpectOneActiveClientPing();
  ExpectNoServerChange();
  time_->Advance(kShutdownTimeout / 2);
  controller_->DoWork();

  // client inactive
  ExpectActiveServer();
  IgnoreForceFileChecks();
  ExpectNoActiveClientPing();
  ExpectNoServerChange();
  time_->Advance(kShutdownTimeout / 2);
  controller_->DoWork();

  // client inactive
  ExpectActiveServer();
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
