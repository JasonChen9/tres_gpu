#include "TresNvml.h"

namespace NVML {

/**
 * @brief Initialize the NVML library.
 */
bool TresNvml::NvmlInit() {
  nvmlReturn_t nvml_rc;
  nvml_rc = nvmlInit_v2();
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to initialize NVML: %s", nvmlErrorString(nvml_rc));
    return false;
  } else {
    printf("Successfully initialized NVML\n");
    return true;
  }
}

/**
 * @brief Shutdown the NVML library.
 */
bool TresNvml::NvmlShutdown() {
  nvmlReturn_t nvml_rc;
  nvml_rc = nvmlShutdown();
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to shut down NVML: %s", nvmlErrorString(nvml_rc));
    return false;
  } else {
    printf("Successfully shut down NVML\n");
    return true;
  }
}

/**
 * @brief Get the handle to the GPU for the passed index
 *
 * @param index 	(IN)  The GPU index (corresponds to PCI Bus ID order)
 * @param device	(OUT) The device handle
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetHandle(uint32_t index, nvmlDevice_t *device) {
  nvmlReturn_t nvml_rc;
  nvml_rc = nvmlDeviceGetHandleByIndex_v2(index, device);
  if (nvml_rc != NVML_SUCCESS) {
    printf("NVML: Failed to get device handle for GPU %d: %s\n", index,
           nvmlErrorString(nvml_rc));
    return false;
  }
  return true;
}

/**
 * @brief Get the name of the device
 *
 * @param device 	(IN)     The device handle
 * @param name 	        (IN/OUT) Reference in which to return the product name
 * @param length	(IN)     The maximum allowed length of the string
 * returned in name
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetName(nvmlDevice_t device, char *name,
                           uint32_t length) {
  nvmlReturn_t nvml_rc;
  nvml_rc = nvmlDeviceGetName(device, name, length);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get name of device : %s\n", nvmlErrorString(nvml_rc));
    return false;
  }
  return true;
}

/**
 * @brief Get pci information of the device
 *
 * @param device   (IN)     The device handle
 * @param pci 	   (IN/OUT) Reference in which to return the pci information
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDevicePciInfo(nvmlDevice_t device, nvmlPciInfo_t *pci) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetPciInfo(device, pci);
  if (nvml_rc != NVML_SUCCESS) {
    printf("NVML: Failed to get PCI info of GPU: %s", nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get memory information of the device
 *
 * @param device 	 (IN)     The device handle
 * @param GPUmemoryInfo  (IN/OUT) Reference in which to return the memory
 * information
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceMemoryInfo(nvmlDevice_t device,
                                       nvmlMemory_t *GPUmemoryInfo) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetMemoryInfo(device, GPUmemoryInfo);

  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get memory info: %s\n", nvmlErrorString(nvml_rc));
    return false;
  }
  return true;
}

/**
 * @brief Get the number of GPU devices in the system
 *
 * @param deviceCount   (IN/OUT) Reference in which to return the number of
 * accessible devices
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceCount(uint32_t *deviceCount) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetCount_v2(deviceCount);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to query device count: %s\n", nvmlErrorString(nvml_rc));
    return false;
  }
  return true;
}

/**
 * @brief Get the UUID of the device
 *
 * @param device 	 (IN)     The device handle
 * @param uuid           (IN/OUT) Reference in which to return the uuid
 * @param length         (IN)     The maximum allowed length of the string
 * returned in uuid
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceUuid(nvmlDevice_t device, char *uuid,
                                 uint32_t length) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetUUID(device, uuid, length);

  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get uuid : %s\n", nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get minor number of the device
 *
 * @param device      (IN)     The device handle
 * @param minor       (IN/OUT) Reference in which to return the minor number
 *
 * @details The minor number for the device is such that the Nvidia device node
 * file for each GPU will have the form /dev/nvidia[minor].
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceMinorNumber(nvmlDevice_t device,
                                        uint32_t *minor) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetMinorNumber(device, minor);
  if (nvml_rc != NVML_SUCCESS) {
    printf("NVML: Failed to get minor number of GPU: %s",
           nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get accouting mode of the device
 *
 * @param device      (IN)     The device handle
 * @param mode        (IN/OUT) Reference in which to return the accounting mode
 *
 * @details mode values: NVML_FEATURE_DISABLED = 0, NVML_FEATURE_ENABLED = 1
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceAccountingMode(nvmlDevice_t device,
                                           nvmlEnableState_t *mode) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetAccountingMode(device, mode);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get accounting mode: %s\n", nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Set accouting mode of the device
 *
 * @param device      (IN) The device handle
 * @param mode        (IN) The mode set to the device
 *
 * @details mode values: NVML_FEATURE_DISABLED = 0, NVML_FEATURE_ENABLED = 1
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlSetDeviceAccountingMode(nvmlDevice_t device,
                                           nvmlEnableState_t mode) {
  nvmlReturn_t nvml_rc = nvmlDeviceSetAccountingMode(device, mode);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to set accounting mode: %s\n", nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get processes pids of the device
 *
 * @param device      (IN)     The device handle
 * @param count       (IN/OUT) Reference in which to return the number of the
 * processes
 * @param pids        (IN/OUT) Reference in which to return list of process ids
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceAccountingPids(nvmlDevice_t device,
                                           uint32_t *count,
                                           uint32_t *pids) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetAccountingPids(device, count, pids);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get accounting pids: %s\n", nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get accounting stats of the device
 *
 * @param device      (IN)     The device handle
 * @param pid         (IN)     Process Id of the target process to query stats
 * for
 * @param stats       (IN/OUT) Reference in which to return the process's
 * accounting stats
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceAccountingStats(nvmlDevice_t device,
                                            uint32_t pid,
                                            nvmlAccountingStats_t *stats) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetAccountingStats(device, pid, stats);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get accounting stats: %s\n", nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get the CUDA compute capability of the device.
 *
 * @param device      (IN)     The device handle
 * @param major       (IN/OUT) Reference in which to return the major CUDA
 * compute capability
 * @param minor       (IN/OUT) Reference in which to return the minor CUDA
 * compute capability
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlGetDeviceCudaComputeCapability(nvmlDevice_t device,
                                                  int *major, int *minor) {
  nvmlReturn_t nvml_rc =
      nvmlDeviceGetCudaComputeCapability(device, major, minor);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get cuda compute capability: %s\n",
           nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get accounting information about all processes that have already
 * terminated, Requires root/admin permissions.
 *
 * @param device (IN) The device handle
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlDeviceClearAccountingPids(nvmlDevice_t device) {
  nvmlReturn_t nvml_rc = nvmlDeviceClearAccountingPids(device);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to clear accounting information: %s\n",
           nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get the number of processes that the circular buffer with accounting
 * pids can hold.
 *
 * @param device (IN) The device handle
 * @param bufferSize (IN/OUT) Reference in which to provide the size.
 *
 * @return true if successful, false if not
 */
bool TresNvml::NvmlDeviceGetAccountingBuffersize(nvmlDevice_t device,
                                                 uint32_t *bufferSize) {
  nvmlReturn_t nvml_rc = nvmlDeviceGetAccountingBufferSize(device, bufferSize);
  if (nvml_rc != NVML_SUCCESS) {
    printf("Failed to get accounting buffer size: %s\n",
           nvmlErrorString(nvml_rc));
    return false;
  } else
    return true;
}

/**
 * @brief Get the name of the process with specified pid
 *
 * @param pid (IN)  process's pid
 * @param task_name (IN/OUT) Reference in which to provide the taskname.
 *
 * @return true if successful, false if not
 */
void TresNvml::GetNameByPid(uint32_t pid, char *task_name) {
  char proc_pid_path[kTaskNameSize];
  char buf[kTaskNameSize];

  sprintf(proc_pid_path, "/proc/%d/status", pid);
  FILE *fp = fopen(proc_pid_path, "r");
  if (nullptr != fp) {
    if (fgets(buf, kTaskNameSize - 1, fp) == nullptr) {
      fclose(fp);
    }
    fclose(fp);
    sscanf(buf, "%*s %s", task_name);
  }
}

}  // namespace NVML
