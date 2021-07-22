#include <iostream>

#include "TresGather.h"

int main() {
  NVML::TresNvml tres_nvml;
  auto *ptres_nvml = &tres_nvml;
  NVML::TresGatherNvml tres_acct(ptres_nvml);

  if (tres_acct.Init() != SlurmxErr::kOk) {
    return 1;
  }

  if (tres_acct.TresGatherProcInfo() != SlurmxErr::kOk) {
    tres_acct.Fini();

    return 1;
  }

  std::string info;

  if (tres_acct.ConvertProcInfoToString(info) != SlurmxErr::kOk) {
    tres_acct.Fini();

    return 1;
  }
  printf("%s\n", info.c_str());

  if (tres_acct.Fini() != SlurmxErr::kOk) {
    return 1;
  }

  return 0;
}
