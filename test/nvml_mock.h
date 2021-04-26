#pragma once
#include <nvml.h>

#include "../src/tres_nvml.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class Mocknvml : public Tres_nvml::nvml {
 public:
  MOCK_METHOD0(_nvml_init, bool());
  MOCK_METHOD0(_nvml_shutdown, bool());
  MOCK_METHOD2(_nvml_get_handle,
               bool(unsigned int index, nvmlDevice_t *device));
  MOCK_METHOD1(_nvml_get_device_count, bool(unsigned int *deviceCount));
  MOCK_METHOD2(_nvml_get_device_minor_number,
               bool(nvmlDevice_t device, unsigned int *minor));
  MOCK_METHOD2(_nvml_set_device_accounting_mode,
               bool(nvmlDevice_t device, nvmlEnableState_t mode));
  MOCK_METHOD2(_nvml_get_device_accounting_mode,
               bool(nvmlDevice_t device, nvmlEnableState_t *mode));
  MOCK_METHOD3(_nvml_get_device_accounting_pids,
               bool(nvmlDevice_t device, unsigned int *count,
                    unsigned int *pids));

  MOCK_METHOD3(_nvml_get_device_accounting_stats,
               bool(nvmlDevice_t device, unsigned int pid,
                    nvmlAccountingStats_t *stats));
  MOCK_METHOD1(_nvml_device_clear_accounting_pids, bool(nvmlDevice_t device));
  MOCK_METHOD2(_nvml_device_get_accounting_buffersize,
               bool(nvmlDevice_t device, unsigned int *bufferSize));
  MOCK_METHOD2(getNameByPid, void(unsigned int pid, char *task_name));
};