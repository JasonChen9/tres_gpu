#pragma once

#include <ctime>
#include <iostream>
#include <tabulate/table.hpp>
#include <vector>
#include "fmt/format.h"
#include "tres_gather.h"
#include "tres_nvml.h"

namespace Tres_gather_nvml {
constexpr unsigned int MAXMINOR = 65535;
using namespace Tres_nvml;
class tres_gather_nvml : tres_gather {
 public:
  explicit tres_gather_nvml(nvml *mvnl_ptr) : nvml_ptr(mvnl_ptr) {}

  SlurmxErr Init() override;
  SlurmxErr Fini() override;

  SlurmxErr TresGatherProcInfo() override;
  SlurmxErr ConvertProcInfoToString(std::string *PrecInfo) const override;

 private:
  struct m_proc_info_ {
    unsigned int minor=MAXMINOR;
    unsigned int pid=0;
    unsigned long long int max_memory_usage=0;
    std::string task_name="";
    std::string start_time="";
  };

  std::vector<m_proc_info_> m_proc_info_vector_;
  unsigned int m_proc_count_ = 0;
  unsigned int m_device_count_ = 0;
  nvml *nvml_ptr;
};

}  // namespace Tres_gather_nvml
