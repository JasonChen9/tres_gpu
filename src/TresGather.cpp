#include "TresGather.h"

namespace NVML {

constexpr int kTimeBias = 8;
constexpr uint32_t kTimeSize = 20;
constexpr uint64_t M = 1024 * 1024;
constexpr uint64_t kTimeStampToTimet = 1000000;

using namespace tabulate;

SlurmxErr TresGatherNvml::Init() {
  plugin_name += "/nvml";
  if (!m_nvml_ptr_->NvmlInit()) {
    return SlurmxErr::kTresFailure;
  } else {
    printf("loading %s\n", plugin_name.c_str());
    return SlurmxErr::kOk;
  }
}

SlurmxErr TresGatherNvml::Fini() {
  if (!m_nvml_ptr_->NvmlShutdown()) {
    return SlurmxErr::kTresFailure;
  } else
    printf("unloaded %s\n", plugin_name.c_str());
  return SlurmxErr::kOk;
}

/**
 * @brief Find how mang processes are running, and gather accounting information
 *
 * @return SlurmxErr code
 */
SlurmxErr TresGatherNvml::TresGatherProcInfo() {
  if (!m_device_count_) {
   uint32_t device_count = 0;
    if (!m_nvml_ptr_->NvmlGetDeviceCount(&device_count))
      return SlurmxErr::kTresFailure;
    printf("Found %d device%s\n", device_count, device_count != 1 ? "s" : "");
    m_device_count_ = device_count;
  }
  uint32_t i;
  printf("Accounting processes info ...\n");
  auto Errprint = [](uint32_t index) {
    printf("Failde index:%d\n", index);
  };
  std::vector<nvmlDevice_t> devices;
  devices.resize(m_device_count_);
  for (i = 0; i < m_device_count_; i++) {
    if (!m_nvml_ptr_->NvmlGetHandle(i, &devices[i])) {
      Errprint(i);
      continue;
    }

    uint32_t minor = kMaxMinor;
    if (!m_nvml_ptr_->NvmlGetDeviceMinorNumber(devices[i], &minor)) {
      Errprint(i);
    }

    nvmlEnableState_t mode;
    if (!m_nvml_ptr_->NvmlGetDeviceAccountingMode(devices[i], &mode)) {
      Errprint(i);
      continue;
    }

    if (mode != 1)
      if (!m_nvml_ptr_->NvmlSetDeviceAccountingMode(devices[i],
                                                 NVML_FEATURE_ENABLED)) {
        Errprint(i);
        continue;
      }

    // Clears accounting information about all processes that have already
    // terminated, Requires root/admin permissions.
    if (!m_nvml_ptr_->NvmlDeviceClearAccountingPids(devices[i])) {
      Errprint(i);
      continue;
    }

    uint32_t buffersize = 0;

    // Returns the number of processes that the circular buffer with accounting
    // pids can hold
    if (!m_nvml_ptr_->NvmlDeviceGetAccountingBuffersize(devices[i], &buffersize)) {
      Errprint(i);
      continue;
    }

    uint32_t count = buffersize;
    uint32_t pids[buffersize];
    if (!m_nvml_ptr_->NvmlGetDeviceAccountingPids(devices[i], &count, pids)) {
      Errprint(i);
      count = 0;
    }

    m_proc_count_ += count;
    nvmlAccountingStats_t stats;
    for (int j = 0; j < count; j++) {
//      auto *proc_info = new TresGatherNvml::m_proc_info_;
      TresGatherNvml::m_proc_info_ proc_info;

      proc_info.minor = minor;
      proc_info.pid = pids[j];

      char task_name[NVML::kTaskNameSize];
      m_nvml_ptr_->GetNameByPid(pids[j], task_name);
      proc_info.task_name = std::string(task_name);

      char timebuf[kTimeSize];
      if (!m_nvml_ptr_->NvmlGetDeviceAccountingStats(devices[i], pids[j],
                                                  &stats)) {
        Errprint(i);
        m_proc_info_vector_.push_back(proc_info);
        continue;
      }
      auto rawtime = (time_t)(stats.startTime / kTimeStampToTimet);
      struct tm *const ptm = gmtime(&rawtime);
      int time_bias = kTimeBias;

      snprintf(timebuf, kTimeSize, "%d-%02d-%02d %2d:%02d", ptm->tm_year + 1900,
               ptm->tm_mon + 1, ptm->tm_mday, (ptm->tm_hour + time_bias) % 24,
               ptm->tm_min);
      proc_info.start_time = timebuf;
      proc_info.max_memory_usage = stats.maxMemoryUsage;

      m_proc_info_vector_.push_back(proc_info);

    }
  }
  if (!m_proc_count_) {
    printf("No running processes found\n");
  } else {
    printf("Found %d process%s\n", m_proc_count_,
           m_proc_count_ != 1 ? "es" : "");
  }
  return SlurmxErr::kOk;
}

/**
 * @brief Retrieves running process information of all the accessible gpu device
 * @param PrecInfo (IN/OUT) Pointer in which to return the running process
 * information string
 *
 * @details  String format:
 * @verbatim
 * +-----+-----+--------------------+-------------+------------------+
 * | GPU | Pid | MaxMemoryUsage(MB) | ProcessName |      StartAt     |
 * +-----+-----+--------------------+-------------+------------------+
 * |  0  | 537 |         255        |    test1    | 2021-04-21 17:42 |
 * |  0  | 538 |         369        |    test2    | 2021-04-21 17:42 |
 * |...  | ... |         ...        |    ...      | ...              |
 * +-----+-----+--------------------+-------------+------------------+
 * If there is no running process, return "There is no process to print!"
 * @endverbatim
 * @return SlurmxErr code
 */
SlurmxErr TresGatherNvml::ConvertProcInfoToString(std::string &PrecInfo) const {
  auto beg = m_proc_info_vector_.begin();
  auto end = m_proc_info_vector_.end();
  std::string proc_info_str;
  if (!m_proc_count_) {
    proc_info_str += "There is no process to print!";
  } else {
    Table table;
    table.add_row(
        {"GPU", "Pid", "MaxMemoryUsage(MB)", "ProcessName", "StartAt"});
    for (auto itr = beg; itr != end; itr++) {
      table.add_row({std::to_string(itr->minor), std::to_string(itr->pid),
                     std::to_string(itr->max_memory_usage / M),
                     itr->task_name.c_str(), itr->start_time.c_str()});
    }
    table.format().font_align(tabulate::FontAlign::center).hide_border_top();
    table[0].format().show_border_top();
    table[1].format().show_border_top();

    proc_info_str += table.str();
  }

  PrecInfo = proc_info_str;

  return SlurmxErr::kOk;
}

}  // namespace NVML

namespace CGROUP {

SlurmxErr TresGatherCgroup::Init() {
  plugin_name += "/cgroup";

  printf("loading %s\n", plugin_name.c_str());
  return SlurmxErr::kOk;
}

SlurmxErr TresGatherCgroup::Fini() {
  printf("unloaded %s\n", plugin_name.c_str());
  return SlurmxErr::kOk;
}

SlurmxErr TresGatherCgroup::TresGatherProcInfo() { return SlurmxErr::kOk; }

SlurmxErr TresGatherCgroup::ConvertProcInfoToString(
    std::string &PrecInfo) const {
  return SlurmxErr::kOk;
}
}  // namespace CGROUP

namespace CUDA {
SlurmxErr GetVersion() {
  int version;
  cuInit(0);
  cuDriverGetVersion(&version);
  printf("%d", version);
  return SlurmxErr::kOk;
}
}  // namespace CUDA