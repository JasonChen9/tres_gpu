#pragma once
#include <ctime>
#include <iostream>
#include <tabulate/table.hpp>
#include <vector>

#include "TresGather.h"
#include "TresNvml.h"
#include "fmt/format.h"
enum class SlurmxErr : uint16_t {
  kOk = 0,

  kTresFailure,

  __ERR_SIZE
};

class TresGather {
 public:
  TresGather() = default;

  virtual SlurmxErr Init() = 0;
  virtual SlurmxErr Fini() = 0;
  virtual SlurmxErr TresGatherProcInfo() = 0;
  virtual SlurmxErr ConvertProcInfoToString(std::string &PrecInfo) const = 0;

 protected:
  mutable std::string plugin_name = "tres_gather";
};

namespace NVML {
constexpr uint32_t kMaxMinor = 65535;

class TresGatherNvml : TresGather {
 public:
  explicit TresGatherNvml(Nvml *mvnl_ptr) : m_nvml_ptr_(mvnl_ptr) {}

  SlurmxErr Init() override;
  SlurmxErr Fini() override;

  SlurmxErr TresGatherProcInfo() override;
  SlurmxErr ConvertProcInfoToString(std::string &PrecInfo) const override;

 private:
  struct m_proc_info_ {
    uint32_t minor = kMaxMinor;
    uint32_t pid = 0;
    uint64_t max_memory_usage = 0;
    std::string task_name;
    std::string start_time;
  };

  std::vector<m_proc_info_> m_proc_info_vector_;
  uint32_t m_proc_count_ = 0;
  uint32_t m_device_count_ = 0;
  Nvml *m_nvml_ptr_;
};

}  // namespace NVML

namespace CGROUP {
class TresGatherCgroup : TresGather {
  SlurmxErr Init() override;
  SlurmxErr Fini() override;

  SlurmxErr TresGatherProcInfo() override;
  SlurmxErr ConvertProcInfoToString(std::string &PrecInfo) const override;
};
}  // namespace CGROUP

#include <cuda.h>
namespace CUDA {
SlurmxErr GetVersion();
}
