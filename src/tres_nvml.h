#pragma once
#include <nvml.h>
#include <iostream>
namespace Tres_nvml {
constexpr unsigned int TASKNAMESIZE = 1024;

class nvml {
 public:
  virtual ~nvml() = default;

  virtual bool _nvml_init() = 0;
  virtual bool _nvml_shutdown() = 0;
  virtual bool _nvml_get_handle(unsigned int index, nvmlDevice_t *device) = 0;
  virtual bool _nvml_get_device_count(unsigned int *deviceCount) = 0;
  virtual bool _nvml_get_device_minor_number(nvmlDevice_t device,
                                             unsigned int *minor) = 0;
  virtual bool _nvml_set_device_accounting_mode(nvmlDevice_t device,
                                                nvmlEnableState_t mode) = 0;
  virtual bool _nvml_get_device_accounting_mode(nvmlDevice_t device,
                                                nvmlEnableState_t *mode) = 0;
  virtual bool _nvml_get_device_accounting_pids(nvmlDevice_t device,
                                                unsigned int *count,
                                                unsigned int *pids) = 0;
  virtual bool _nvml_get_device_accounting_stats(
      nvmlDevice_t device, unsigned int pid, nvmlAccountingStats_t *stats) = 0;
  virtual bool _nvml_device_clear_accounting_pids(nvmlDevice_t device) = 0;
  virtual bool _nvml_device_get_accounting_buffersize( nvmlDevice_t device, unsigned int* bufferSize)=0;
  virtual void getNameByPid(unsigned int pid, char *task_name) = 0;
};

class tres_nvml : public nvml {
 public:
  tres_nvml() = default;
  bool _nvml_init() override;
  bool _nvml_shutdown() override;
  bool _nvml_get_handle(unsigned int index, nvmlDevice_t *device) override;
  bool _nvml_get_device_count(unsigned int *deviceCount) override;
  bool _nvml_get_device_minor_number(nvmlDevice_t device,
                                     unsigned int *minor) override;
  bool _nvml_set_device_accounting_mode(nvmlDevice_t device,
                                        nvmlEnableState_t mode) override;
  bool _nvml_get_device_accounting_mode(nvmlDevice_t device,
                                        nvmlEnableState_t *mode) override;
  bool _nvml_get_device_accounting_pids(nvmlDevice_t device,
                                        unsigned int *count,
                                        unsigned int *pids) override;
  bool _nvml_get_device_accounting_stats(nvmlDevice_t device, unsigned int pid,
                                         nvmlAccountingStats_t *stats) override;
  bool _nvml_device_clear_accounting_pids(nvmlDevice_t device) override;
  bool _nvml_device_get_accounting_buffersize( nvmlDevice_t device, unsigned int* bufferSize) override;
  void getNameByPid(unsigned int pid, char *task_name) override;

  //means for gres
  bool _nvml_get_name(nvmlDevice_t device, char *name, unsigned int length);
  bool _nvml_get_device_pci_info(nvmlDevice_t device, nvmlPciInfo_t *pci);
  bool _nvml_get_device_memory_info(nvmlDevice_t device,
                                    nvmlMemory_t *GPUmemoryInfo);
  bool _nvml_get_device_uuid(nvmlDevice_t device, char *uuid,
                             unsigned int length);
  bool _nvml_get_device_cuda_compute_capability(nvmlDevice_t device, int *major,
                                                int *minor);
};
}  // namespace Tres_nvml
