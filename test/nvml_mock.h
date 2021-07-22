#pragma once
#include <nvml.h>

#include "../src/TresNvml.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class Mocknvml : public NVML::Nvml {
 public:
  MOCK_METHOD0(NvmlInit, bool());
  MOCK_METHOD0(NvmlShutdown, bool());
  MOCK_METHOD2(NvmlGetHandle,
               bool(uint32_t index, nvmlDevice_t *device));
  MOCK_METHOD1(NvmlGetDeviceCount, bool(uint32_t *deviceCount));
  MOCK_METHOD2(NvmlGetDeviceMinorNumber,
               bool(nvmlDevice_t device, uint32_t *minor));
  MOCK_METHOD2(NvmlSetDeviceAccountingMode,
               bool(nvmlDevice_t device, nvmlEnableState_t mode));
  MOCK_METHOD2(NvmlGetDeviceAccountingMode,
               bool(nvmlDevice_t device, nvmlEnableState_t *mode));
  MOCK_METHOD3(NvmlGetDeviceAccountingPids,
               bool(nvmlDevice_t device, uint32_t *count,
                   uint32_t *pids));

  MOCK_METHOD3(NvmlGetDeviceAccountingStats,
               bool(nvmlDevice_t device, uint32_t pid,
                    nvmlAccountingStats_t *stats));
  MOCK_METHOD1(NvmlDeviceClearAccountingPids, bool(nvmlDevice_t device));
  MOCK_METHOD2(NvmlDeviceGetAccountingBuffersize,
               bool(nvmlDevice_t device, uint32_t *bufferSize));
  MOCK_METHOD2(GetNameByPid, void(uint32_t pid, char *task_name));
};