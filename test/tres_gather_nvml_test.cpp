#include "../src/tres_gather_nvml.h"

#include <thread>

#include "gmock//gmock.h"
#include "gtest/gtest.h"
#include "nvml_mock.h"

using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::HasSubstr;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArrayArgument;

/**
 * Mock NVML API, and test basic running
 */
// Simulate three devices and each devices has two processes
TEST(NVML_MOCK, mock_3devices_6procs) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(true));

  // Simulate 3 devices
  unsigned int device_count = 3;
  EXPECT_CALL(*mocknvml, _nvml_get_device_count(::testing::_))
      .Times(1)
      .WillOnce(DoAll(SetArgPointee<0>(device_count), Return(true)));

  EXPECT_CALL(*mocknvml, _nvml_get_handle(::testing::_, ::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  // Simulate 3 devices' minor number
  unsigned int minor1 = 1;
  unsigned int minor2 = 2;
  unsigned int minor3 = 3;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_minor_number(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(minor1), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor2), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor3), Return(true)));

  // Simulate first device need to set mode
  nvmlEnableState_t mode = NVML_FEATURE_DISABLED;
  nvmlEnableState_t mode1 = NVML_FEATURE_ENABLED;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_accounting_mode(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(mode), Return(true)))
      .WillRepeatedly(DoAll(SetArgPointee<1>(mode1), Return(true)));
  EXPECT_CALL(*mocknvml,
              _nvml_set_device_accounting_mode(::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*mocknvml, _nvml_device_clear_accounting_pids(::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  constexpr unsigned int buffersize = 10;
  EXPECT_CALL(*mocknvml, _nvml_device_get_accounting_buffersize(::testing::_,
                                                                ::testing::_))
      .Times(3)
      .WillRepeatedly(DoAll(SetArgPointee<1>(buffersize), Return(true)));

  // Simulate each devices' processes
  unsigned int count = 2;
  unsigned int pid1[buffersize] = {123, 567};
  unsigned int pid2[buffersize] = {234, 678};
  unsigned int pid3[buffersize] = {345, 789};

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_pids(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid1, pid1 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid2, pid2 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid3, pid3 + buffersize),
                      Return(true)));

  // Simulate each processes' stats
  nvmlAccountingStats_t stats;
  stats.maxMemoryUsage = 134217728;
  stats.startTime = 1619343198016349;

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_stats(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(6)
      .WillRepeatedly(DoAll(SetArgPointee<2>(stats), Return(true)));

  // Simulate each processes' name
  char task_name[Tres_nvml::TASKNAMESIZE] = "mocktask";
  EXPECT_CALL(*mocknvml, getNameByPid(::testing::_, ::testing::_))
      .Times(6)
      .WillRepeatedly(
          SetArrayArgument<1>(task_name, task_name + Tres_nvml::TASKNAMESIZE));

  EXPECT_CALL(*mocknvml, _nvml_shutdown()).Times(1).WillOnce(Return(true));

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);

  auto info = new std::string;
  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_STREQ(
      info->c_str(),
      "+-----+-----+--------------------+-------------+------------------+\n"
      "| GPU | Pid | MaxMemoryUsage(MB) | ProcessName |      StartAt     |\n"
      "+-----+-----+--------------------+-------------+------------------+\n"
      "|  1  | 123 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  1  | 567 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  2  | 234 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  2  | 678 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 345 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 789 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "+-----+-----+--------------------+-------------+------------------+");
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete info;
  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete mocknvml;
}

// Simulate _nvml_init function return false, and test
TEST(NVML_MOCK, init_false) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(false));

  SlurmxErr err = tres_gather_nvml->Init();

  EXPECT_EQ(err, SlurmxErr::kTresFailure);

  delete tres_gather_nvml;
  delete mocknvml;
}

// Simulate _nvml_get_device_count function false, and test
TEST(NVML_MOCK, count_false) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*mocknvml, _nvml_get_device_count(::testing::_))
      .Times(1)
      .WillOnce(Return(false));

  SlurmxErr err = tres_gather_nvml->Init();

  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kTresFailure);

  delete tres_gather_nvml;
  delete mocknvml;
}

// simulat first device get_handle function return false, and test
TEST(NVML_MOCK, first_gethandle_false) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(true));

  // Simulate 3 devices
  unsigned int device_count = 3;
  EXPECT_CALL(*mocknvml, _nvml_get_device_count(::testing::_))
      .Times(1)
      .WillOnce(DoAll(SetArgPointee<0>(device_count), Return(true)));

  // The first device's get_handle function return false
  EXPECT_CALL(*mocknvml, _nvml_get_handle(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));

  // Simulate 3 devices' minor number,first one is never used
  unsigned int minor1 = 1;
  unsigned int minor2 = 2;
  unsigned int minor3 = 3;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_minor_number(::testing::_, ::testing::_))
      .Times(2)
      .WillOnce(DoAll(SetArgPointee<1>(minor2), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor3), Return(true)));

  // Simulate first device need to set mode
  nvmlEnableState_t mode = NVML_FEATURE_DISABLED;
  nvmlEnableState_t mode1 = NVML_FEATURE_ENABLED;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_accounting_mode(::testing::_, ::testing::_))
      .Times(2)
      .WillOnce(DoAll(SetArgPointee<1>(mode), Return(true)))
      .WillRepeatedly(DoAll(SetArgPointee<1>(mode1), Return(true)));
  EXPECT_CALL(*mocknvml,
              _nvml_set_device_accounting_mode(::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*mocknvml, _nvml_device_clear_accounting_pids(::testing::_))
      .Times(2)
      .WillRepeatedly(Return(true));

  constexpr unsigned int buffersize = 10;
  EXPECT_CALL(*mocknvml, _nvml_device_get_accounting_buffersize(::testing::_,
                                                                ::testing::_))
      .Times(2)
      .WillRepeatedly(DoAll(SetArgPointee<1>(buffersize), Return(true)));

  // Simulate each devices' processes,first one is never used
  unsigned int count = 2;
  unsigned int pid1[buffersize] = {123, 567};
  unsigned int pid2[buffersize] = {234, 678};
  unsigned int pid3[buffersize] = {345, 789};

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_pids(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(2)
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid2, pid2 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid3, pid3 + buffersize),
                      Return(true)));

  // Simulate each processes' stats
  nvmlAccountingStats_t stats;
  stats.maxMemoryUsage = 134217728;
  stats.startTime = 1619343198016349;

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_stats(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(4)
      .WillRepeatedly(DoAll(SetArgPointee<2>(stats), Return(true)));

  // Simulate each processes' name
  char task_name[Tres_nvml::TASKNAMESIZE] = "mocktask";
  EXPECT_CALL(*mocknvml, getNameByPid(::testing::_, ::testing::_))
      .Times(4)
      .WillRepeatedly(
          SetArrayArgument<1>(task_name, task_name + Tres_nvml::TASKNAMESIZE));

  EXPECT_CALL(*mocknvml, _nvml_shutdown()).Times(1).WillOnce(Return(true));

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);

  auto info = new std::string;
  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_STREQ(
      info->c_str(),
      "+-----+-----+--------------------+-------------+------------------+\n"
      "| GPU | Pid | MaxMemoryUsage(MB) | ProcessName |      StartAt     |\n"
      "+-----+-----+--------------------+-------------+------------------+\n"
      "|  2  | 234 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  2  | 678 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 345 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 789 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "+-----+-----+--------------------+-------------+------------------+");
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete info;
  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete mocknvml;
}

// simulat first device get_minor function return false, and test
TEST(NVML_MOCK, first_minor_false) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(true));

  // Simulate 3 devices
  unsigned int device_count = 3;
  EXPECT_CALL(*mocknvml, _nvml_get_device_count(::testing::_))
      .Times(1)
      .WillOnce(DoAll(SetArgPointee<0>(device_count), Return(true)));

  EXPECT_CALL(*mocknvml, _nvml_get_handle(::testing::_, ::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  // Simulate 3 devices' minor number
  // first one device's get_minor function return false, and the first minor is
  // never used
  unsigned int minor1 = 1;
  unsigned int minor2 = 2;
  unsigned int minor3 = 3;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_minor_number(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(Return(false))
      .WillOnce(DoAll(SetArgPointee<1>(minor2), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor3), Return(true)));

  // Simulate first device need to set mode
  nvmlEnableState_t mode = NVML_FEATURE_DISABLED;
  nvmlEnableState_t mode1 = NVML_FEATURE_ENABLED;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_accounting_mode(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(mode), Return(true)))
      .WillRepeatedly(DoAll(SetArgPointee<1>(mode1), Return(true)));
  EXPECT_CALL(*mocknvml,
              _nvml_set_device_accounting_mode(::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*mocknvml, _nvml_device_clear_accounting_pids(::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  constexpr unsigned int buffersize = 10;
  EXPECT_CALL(*mocknvml, _nvml_device_get_accounting_buffersize(::testing::_,
                                                                ::testing::_))
      .Times(3)
      .WillRepeatedly(DoAll(SetArgPointee<1>(buffersize), Return(true)));

  // Simulate each devices' processes
  unsigned int count = 2;
  unsigned int pid1[buffersize] = {123, 567};
  unsigned int pid2[buffersize] = {234, 678};
  unsigned int pid3[buffersize] = {345, 789};

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_pids(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid1, pid1 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid2, pid2 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid3, pid3 + buffersize),
                      Return(true)));

  // Simulate each processes' stats
  nvmlAccountingStats_t stats;
  stats.maxMemoryUsage = 134217728;
  stats.startTime = 1619343198016349;

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_stats(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(6)
      .WillRepeatedly(DoAll(SetArgPointee<2>(stats), Return(true)));

  // Simulate each processes' name
  char task_name[Tres_nvml::TASKNAMESIZE] = "mocktask";
  EXPECT_CALL(*mocknvml, getNameByPid(::testing::_, ::testing::_))
      .Times(6)
      .WillRepeatedly(
          SetArrayArgument<1>(task_name, task_name + Tres_nvml::TASKNAMESIZE));

  EXPECT_CALL(*mocknvml, _nvml_shutdown()).Times(1).WillOnce(Return(true));

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);

  auto info = new std::string;
  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_STREQ(
      info->c_str(),
      "+-------+-----+--------------------+-------------+------------------+\n"
      "|  GPU  | Pid | MaxMemoryUsage(MB) | ProcessName |      StartAt     |\n"
      "+-------+-----+--------------------+-------------+------------------+\n"
      "| 65535 | 123 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "| 65535 | 567 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|   2   | 234 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|   2   | 678 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|   3   | 345 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|   3   | 789 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "+-------+-----+--------------------+-------------+------------------+");
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete info;
  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete mocknvml;
}

// simulat first device clear accounting function return false, and test
TEST(NVML_MOCK, first_clear_accounting_false) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(true));

  // Simulate 3 devices
  unsigned int device_count = 3;
  EXPECT_CALL(*mocknvml, _nvml_get_device_count(::testing::_))
      .Times(1)
      .WillOnce(DoAll(SetArgPointee<0>(device_count), Return(true)));

  EXPECT_CALL(*mocknvml, _nvml_get_handle(::testing::_, ::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  // Simulate 3 devices' minor number
  unsigned int minor1 = 1;
  unsigned int minor2 = 2;
  unsigned int minor3 = 3;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_minor_number(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(minor1), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor2), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor3), Return(true)));

  // Simulate first device need to set mode
  nvmlEnableState_t mode = NVML_FEATURE_DISABLED;
  nvmlEnableState_t mode1 = NVML_FEATURE_ENABLED;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_accounting_mode(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(mode), Return(true)))
      .WillRepeatedly(DoAll(SetArgPointee<1>(mode1), Return(true)));
  EXPECT_CALL(*mocknvml,
              _nvml_set_device_accounting_mode(::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*mocknvml, _nvml_device_clear_accounting_pids(::testing::_))
      .Times(3)
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));

  constexpr unsigned int buffersize = 10;
  EXPECT_CALL(*mocknvml, _nvml_device_get_accounting_buffersize(::testing::_,
                                                                ::testing::_))
      .Times(2)
      .WillRepeatedly(DoAll(SetArgPointee<1>(buffersize), Return(true)));

  // Simulate each devices' processes
  unsigned int count = 2;
  unsigned int pid1[buffersize] = {123, 567};
  unsigned int pid2[buffersize] = {234, 678};
  unsigned int pid3[buffersize] = {345, 789};

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_pids(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(2)
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid2, pid2 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid3, pid3 + buffersize),
                      Return(true)));

  // Simulate each processes' stats
  nvmlAccountingStats_t stats;
  stats.maxMemoryUsage = 134217728;
  stats.startTime = 1619343198016349;

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_stats(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(4)
      .WillRepeatedly(DoAll(SetArgPointee<2>(stats), Return(true)));

  // Simulate each processes' name
  char task_name[Tres_nvml::TASKNAMESIZE] = "mocktask";
  EXPECT_CALL(*mocknvml, getNameByPid(::testing::_, ::testing::_))
      .Times(4)
      .WillRepeatedly(
          SetArrayArgument<1>(task_name, task_name + Tres_nvml::TASKNAMESIZE));

  EXPECT_CALL(*mocknvml, _nvml_shutdown()).Times(1).WillOnce(Return(true));

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);

  auto info = new std::string;
  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_STREQ(
      info->c_str(),
      "+-----+-----+--------------------+-------------+------------------+\n"
      "| GPU | Pid | MaxMemoryUsage(MB) | ProcessName |      StartAt     |\n"
      "+-----+-----+--------------------+-------------+------------------+\n"
      "|  2  | 234 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  2  | 678 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 345 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 789 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "+-----+-----+--------------------+-------------+------------------+");
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete info;
  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete mocknvml;
}

// simulat first device get_accounting_pids function return false, and test
TEST(NVML_MOCK, first_get_accounting_pids_false) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(true));

  // Simulate 3 devices
  unsigned int device_count = 3;
  EXPECT_CALL(*mocknvml, _nvml_get_device_count(::testing::_))
      .Times(1)
      .WillOnce(DoAll(SetArgPointee<0>(device_count), Return(true)));

  EXPECT_CALL(*mocknvml, _nvml_get_handle(::testing::_, ::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  // Simulate 3 devices' minor number
  unsigned int minor1 = 1;
  unsigned int minor2 = 2;
  unsigned int minor3 = 3;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_minor_number(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(minor1), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor2), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor3), Return(true)));

  // Simulate first device need to set mode
  nvmlEnableState_t mode = NVML_FEATURE_DISABLED;
  nvmlEnableState_t mode1 = NVML_FEATURE_ENABLED;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_accounting_mode(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(mode), Return(true)))
      .WillRepeatedly(DoAll(SetArgPointee<1>(mode1), Return(true)));
  EXPECT_CALL(*mocknvml,
              _nvml_set_device_accounting_mode(::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*mocknvml, _nvml_device_clear_accounting_pids(::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  constexpr unsigned int buffersize = 10;
  EXPECT_CALL(*mocknvml, _nvml_device_get_accounting_buffersize(::testing::_,
                                                                ::testing::_))
      .Times(3)
      .WillRepeatedly(DoAll(SetArgPointee<1>(buffersize), Return(true)));

  // Simulate each devices' processes,
  // The first device get_accounting function return false,and pid1 is never
  // used
  unsigned int count = 2;
  unsigned int pid1[buffersize] = {123, 567};
  unsigned int pid2[buffersize] = {234, 678};
  unsigned int pid3[buffersize] = {345, 789};

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_pids(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(Return(false))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid2, pid2 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid3, pid3 + buffersize),
                      Return(true)));

  // Simulate each processes' stats
  nvmlAccountingStats_t stats;
  stats.maxMemoryUsage = 134217728;
  stats.startTime = 1619343198016349;

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_stats(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(4)
      .WillRepeatedly(DoAll(SetArgPointee<2>(stats), Return(true)));

  // Simulate each processes' name
  char task_name[Tres_nvml::TASKNAMESIZE] = "mocktask";
  EXPECT_CALL(*mocknvml, getNameByPid(::testing::_, ::testing::_))
      .Times(4)
      .WillRepeatedly(
          SetArrayArgument<1>(task_name, task_name + Tres_nvml::TASKNAMESIZE));

  EXPECT_CALL(*mocknvml, _nvml_shutdown()).Times(1).WillOnce(Return(true));

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);

  auto info = new std::string;
  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_STREQ(
      info->c_str(),
      "+-----+-----+--------------------+-------------+------------------+\n"
      "| GPU | Pid | MaxMemoryUsage(MB) | ProcessName |      StartAt     |\n"
      "+-----+-----+--------------------+-------------+------------------+\n"
      "|  2  | 234 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  2  | 678 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 345 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 789 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "+-----+-----+--------------------+-------------+------------------+");
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete info;
  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete mocknvml;
}

// simulat first device's first process get_stats function return false, and
// test
TEST(NVML_MOCK, first_get_stats_false) {
  auto *mocknvml = new Mocknvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(mocknvml);

  EXPECT_CALL(*mocknvml, _nvml_init()).Times(1).WillOnce(Return(true));

  // Simulate 3 devices
  unsigned int device_count = 3;
  EXPECT_CALL(*mocknvml, _nvml_get_device_count(::testing::_))
      .Times(1)
      .WillOnce(DoAll(SetArgPointee<0>(device_count), Return(true)));

  EXPECT_CALL(*mocknvml, _nvml_get_handle(::testing::_, ::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  // Simulate 3 devices' minor number
  unsigned int minor1 = 1;
  unsigned int minor2 = 2;
  unsigned int minor3 = 3;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_minor_number(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(minor1), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor2), Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(minor3), Return(true)));

  // Simulate first device need to set mode
  nvmlEnableState_t mode = NVML_FEATURE_DISABLED;
  nvmlEnableState_t mode1 = NVML_FEATURE_ENABLED;

  EXPECT_CALL(*mocknvml,
              _nvml_get_device_accounting_mode(::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(mode), Return(true)))
      .WillRepeatedly(DoAll(SetArgPointee<1>(mode1), Return(true)));
  EXPECT_CALL(*mocknvml,
              _nvml_set_device_accounting_mode(::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*mocknvml, _nvml_device_clear_accounting_pids(::testing::_))
      .Times(3)
      .WillRepeatedly(Return(true));

  constexpr unsigned int buffersize = 10;
  EXPECT_CALL(*mocknvml, _nvml_device_get_accounting_buffersize(::testing::_,
                                                                ::testing::_))
      .Times(3)
      .WillRepeatedly(DoAll(SetArgPointee<1>(buffersize), Return(true)));

  // Simulate each devices' processes
  unsigned int count = 2;
  unsigned int pid1[buffersize] = {123, 567};
  unsigned int pid2[buffersize] = {234, 678};
  unsigned int pid3[buffersize] = {345, 789};

  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_pids(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(3)
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid1, pid1 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid2, pid2 + buffersize),
                      Return(true)))
      .WillOnce(DoAll(SetArgPointee<1>(count),
                      SetArrayArgument<2>(pid3, pid3 + buffersize),
                      Return(true)));

  // Simulate each processes' stats
  nvmlAccountingStats_t stats;
  stats.maxMemoryUsage = 134217728;
  stats.startTime = 1619343198016349;

  // Simulate the first process get_stats function return false
  EXPECT_CALL(*mocknvml, _nvml_get_device_accounting_stats(
                             ::testing::_, ::testing::_, ::testing::_))
      .Times(6)
      .WillOnce(Return(false))
      .WillRepeatedly(DoAll(SetArgPointee<2>(stats), Return(true)));

  // Simulate each processes' name
  char task_name[Tres_nvml::TASKNAMESIZE] = "mocktask";
  EXPECT_CALL(*mocknvml, getNameByPid(::testing::_, ::testing::_))
      .Times(6)
      .WillRepeatedly(
          SetArrayArgument<1>(task_name, task_name + Tres_nvml::TASKNAMESIZE));

  EXPECT_CALL(*mocknvml, _nvml_shutdown()).Times(1).WillOnce(Return(true));

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);

  auto info = new std::string;
  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_STREQ(
      info->c_str(),
      "+-----+-----+--------------------+-------------+------------------+\n"
      "| GPU | Pid | MaxMemoryUsage(MB) | ProcessName |      StartAt     |\n"
      "+-----+-----+--------------------+-------------+------------------+\n"
      "|  1  | 123 |          0         |   mocktask  |                  |\n"
      "|  1  | 567 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  2  | 234 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  2  | 678 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 345 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "|  3  | 789 |         128        |   mocktask  | 2021-04-25 17:33 |\n"
      "+-----+-----+--------------------+-------------+------------------+");
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete info;
  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete mocknvml;
}

/**
 * test real NVML API with normal condition
 */
// test without process,Requires root/admin permissions.
TEST(NVML, run_without_process) {
  auto *tresnvml = new Tres_nvml::tres_nvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(tresnvml);

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);

  auto info = new std::string;

  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_EQ(err, SlurmxErr::kOk);
  EXPECT_STREQ(info->c_str(), "There is no process to print!");
  delete info;

  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete tresnvml;
}

// test with process,Requires root/admin permissions.
TEST(NVML, run_with_process) {
  auto *tresnvml = new Tres_nvml::tres_nvml();
  auto *tres_gather_nvml = new Tres_gather_nvml::tres_gather_nvml(tresnvml);

  SlurmxErr err = tres_gather_nvml->Init();
  EXPECT_EQ(err, SlurmxErr::kOk);

  // run a cuda process thread
  auto thread_function = []() { system("/home/xgy/cuda_proc/test1"); };
  std::thread threadObj(thread_function);

  // sleep a second, prevent TresGatherProcInfo() from starting too fast
  sleep(1);

  err = tres_gather_nvml->TresGatherProcInfo();
  EXPECT_EQ(err, SlurmxErr::kOk);
  auto info = new std::string;
  // wait for the thread end
  threadObj.join();

  err = tres_gather_nvml->ConvertProcInfoToString(info);
  EXPECT_EQ(err, SlurmxErr::kOk);
  ASSERT_THAT(info->c_str(), HasSubstr("test1"));

  delete info;

  err = tres_gather_nvml->Fini();
  EXPECT_EQ(err, SlurmxErr::kOk);

  delete tres_gather_nvml;
  delete tresnvml;
}