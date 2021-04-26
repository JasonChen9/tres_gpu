#include "tres_gather_nvml.h"

namespace Tres_gather_nvml {

constexpr unsigned int TIMESIZE = 20;
constexpr int TIME_BIAS = 8;
constexpr unsigned long long M = 1024 * 1024;
constexpr unsigned long long TIMESTAMPToTimet = 1000000;
using namespace tabulate;
SlurmxErr tres_gather_nvml::Init() {
  plugin_name += "/nvml";
  if (!nvml_ptr->_nvml_init()) {
    return SlurmxErr::kTresFailure;
  } else {
    printf("loading %s\n", plugin_name.c_str());
    return SlurmxErr::kOk;
  }
}

SlurmxErr tres_gather_nvml::Fini() {
  if (!nvml_ptr->_nvml_shutdown()) {
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
SlurmxErr tres_gather_nvml::TresGatherProcInfo() {
  if (!m_device_count_) {
    unsigned int device_count = 0;
    if (!nvml_ptr->_nvml_get_device_count(&device_count))
      return SlurmxErr::kTresFailure;
    printf("Found %d device%s\n", device_count, device_count != 1 ? "s" : "");
    m_device_count_ = device_count;
  }
  unsigned int i;
  printf("Accounting processes info ...\n");
  auto Errprint = [](unsigned int index) {
    printf("Failde index:%d\n", index);
  };
  std::vector<nvmlDevice_t> devices;
  devices.resize(m_device_count_);
  for (i = 0; i < m_device_count_; i++) {
    if (!nvml_ptr->_nvml_get_handle(i, &devices[i])) {
      Errprint(i);
      continue;
    }

    unsigned int minor=MAXMINOR;
    if (!nvml_ptr->_nvml_get_device_minor_number(devices[i], &minor)) {
      Errprint(i);
    }

    nvmlEnableState_t mode;
    if (!nvml_ptr->_nvml_get_device_accounting_mode(devices[i], &mode)) {
      Errprint(i);
      continue;
    }

    if (mode != 1)
      if (!nvml_ptr->_nvml_set_device_accounting_mode(devices[i],
                                                      NVML_FEATURE_ENABLED)) {
        Errprint(i);
        continue;
      }

    // Clears accounting information about all processes that have already
    // terminated, Requires root/admin permissions.
    if (!nvml_ptr->_nvml_device_clear_accounting_pids(devices[i])) {
      Errprint(i);
      continue;
    }

    unsigned int buffersize = 0;

    // Returns the number of processes that the circular buffer with accounting
    // pids can hold
    if (!nvml_ptr->_nvml_device_get_accounting_buffersize(devices[i], &buffersize)) {
      Errprint(i);
      continue;
    }

    unsigned int count = buffersize;
    unsigned int pids[buffersize];
    if (!nvml_ptr->_nvml_get_device_accounting_pids(devices[i], &count, pids)) {
      Errprint(i);
      count = 0;
    }

    m_proc_count_ += count;
    nvmlAccountingStats_t stats;
    for (int j = 0; j < count; j++) {
      auto *proc_info = new tres_gather_nvml::m_proc_info_;
      proc_info->minor = minor;
      proc_info->pid = pids[j];

      char task_name[Tres_nvml::TASKNAMESIZE];
      nvml_ptr->getNameByPid(pids[j], task_name);
      proc_info->task_name = std::string(task_name);

      char timebuf[TIMESIZE];
      if (!nvml_ptr->_nvml_get_device_accounting_stats(devices[i], pids[j],
                                                       &stats)) {
        Errprint(i);
        m_proc_info_vector_.push_back(*proc_info);
        delete proc_info;
        continue;
      }
      auto rawtime = (time_t)(stats.startTime/TIMESTAMPToTimet);
      struct tm *const ptm = gmtime(&rawtime);
      int time_bias = TIME_BIAS;

      snprintf(timebuf, TIMESIZE, "%d-%02d-%02d %2d:%02d", ptm->tm_year + 1900,
               ptm->tm_mon + 1, ptm->tm_mday, (ptm->tm_hour + time_bias) % 24,
               ptm->tm_min);
      proc_info->start_time = timebuf;

      proc_info->max_memory_usage = stats.maxMemoryUsage;

      m_proc_info_vector_.push_back(*proc_info);

      delete proc_info;
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
SlurmxErr tres_gather_nvml::ConvertProcInfoToString(
    std::string *PrecInfo) const {
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

  *PrecInfo = proc_info_str;

  return SlurmxErr::kOk;
}

}  // namespace Tres_gather_nvml
