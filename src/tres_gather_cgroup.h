#pragma once
#include "tres_gather.h"
namespace tres_cgroup {
class tres_gather_cgroup : tres_gather {
  SlurmxErr Init() override;
  SlurmxErr Fini() override;

  SlurmxErr TresGatherProcInfo() override;
  SlurmxErr ConvertProcInfoToString(std::string *PrecInfo) const override;
};
}  // namespace tres_cgroup
