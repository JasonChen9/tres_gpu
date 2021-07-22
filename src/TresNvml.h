#pragma once
#include <nvml.h>

#include <iostream>
namespace NVML {
constexpr uint32_t kTaskNameSize = 1024;

class Nvml {
 public:
  virtual ~Nvml() = default;

  virtual bool NvmlInit() = 0;
  virtual bool NvmlShutdown() = 0;
  virtual bool NvmlGetHandle(uint32_t index, nvmlDevice_t *device) = 0;
  virtual bool NvmlGetDeviceCount(uint32_t *deviceCount) = 0;
  virtual bool NvmlGetDeviceMinorNumber(nvmlDevice_t device,
                                        uint32_t *minor) = 0;
  virtual bool NvmlSetDeviceAccountingMode(nvmlDevice_t device,
                                           nvmlEnableState_t mode) = 0;
  virtual bool NvmlGetDeviceAccountingMode(nvmlDevice_t device,
                                           nvmlEnableState_t *mode) = 0;
  virtual bool NvmlGetDeviceAccountingPids(nvmlDevice_t device,
                                           uint32_t *count,
                                           uint32_t *pids) = 0;
  virtual bool NvmlGetDeviceAccountingStats(nvmlDevice_t device,
                                            uint32_t pid,
                                            nvmlAccountingStats_t *stats) = 0;
  virtual bool NvmlDeviceClearAccountingPids(nvmlDevice_t device) = 0;
  virtual bool NvmlDeviceGetAccountingBuffersize(nvmlDevice_t device,
                                                 uint32_t *bufferSize) = 0;
  virtual void GetNameByPid(uint32_t pid, char *task_name) = 0;
};

class TresNvml : public Nvml {
 public:
  TresNvml() = default;
  bool NvmlInit() override;
  bool NvmlShutdown() override;
  bool NvmlGetHandle(uint32_t index, nvmlDevice_t *device) override;
  bool NvmlGetDeviceCount(uint32_t *deviceCount) override;
  bool NvmlGetDeviceMinorNumber(nvmlDevice_t device,
                                uint32_t *minor) override;
  bool NvmlSetDeviceAccountingMode(nvmlDevice_t device,
                                   nvmlEnableState_t mode) override;
  bool NvmlGetDeviceAccountingMode(nvmlDevice_t device,
                                   nvmlEnableState_t *mode) override;
  bool NvmlGetDeviceAccountingPids(nvmlDevice_t device, uint32_t *count,
                                   uint32_t *pids) override;
  bool NvmlGetDeviceAccountingStats(nvmlDevice_t device, uint32_t pid,
                                    nvmlAccountingStats_t *stats) override;
  bool NvmlDeviceClearAccountingPids(nvmlDevice_t device) override;
  bool NvmlDeviceGetAccountingBuffersize(nvmlDevice_t device,
                                         uint32_t *bufferSize) override;
  void GetNameByPid(uint32_t pid, char *task_name) override;

  // means for gres
  bool NvmlGetName(nvmlDevice_t device, char *name, uint32_t length);
  bool NvmlGetDevicePciInfo(nvmlDevice_t device, nvmlPciInfo_t *pci);
  bool NvmlGetDeviceMemoryInfo(nvmlDevice_t device,
                               nvmlMemory_t *GPUmemoryInfo);
  bool NvmlGetDeviceUuid(nvmlDevice_t device, char *uuid, uint32_t length);
  bool NvmlGetDeviceCudaComputeCapability(nvmlDevice_t device, int *major,
                                          int *minor);
};
}  // namespace NVML
