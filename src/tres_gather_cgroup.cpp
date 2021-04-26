#include "tres_gather_cgroup.h"

namespace tres_cgroup {

SlurmxErr tres_gather_cgroup::Init() {
  plugin_name += "/cgroup";

  printf("loading %s\n", plugin_name.c_str());
  return SlurmxErr::kOk;
}

SlurmxErr tres_gather_cgroup::Fini() {
  printf("unloaded %s\n", plugin_name.c_str());
  return SlurmxErr::kOk;
}

SlurmxErr tres_gather_cgroup::TresGatherProcInfo() { return SlurmxErr::kOk; }

SlurmxErr tres_gather_cgroup::ConvertProcInfoToString(std::string *PrecInfo) const{
  return SlurmxErr::kOk;
}
}  // namespace tres_cgroup