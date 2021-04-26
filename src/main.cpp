#include <iostream>
#include "tres_gather_nvml.h"

int main() {
  auto *tresnvml = new Tres_nvml::tres_nvml();
  auto *tres_acct = new Tres_gather_nvml::tres_gather_nvml(tresnvml);

  if(tres_acct->Init()!=SlurmxErr::kOk){
    delete tres_acct;
    delete tresnvml;
    return 1;
  }

  if(tres_acct->TresGatherProcInfo()!=SlurmxErr::kOk){
    tres_acct->Fini();
    delete tres_acct;
    delete tresnvml;
    return 1;
  }

  auto info = new std::string;

  if(tres_acct->ConvertProcInfoToString(info)!=SlurmxErr::kOk){
    delete info;
    tres_acct->Fini();
    delete tres_acct;
    delete tresnvml;
    return 1;
  }
  printf("%s\n", info->c_str());
  delete info;

  if(tres_acct->Fini()!=SlurmxErr::kOk){
    delete tres_acct;
    delete tresnvml;
    return 1;
  }

  delete tres_acct;
  delete tresnvml;
  return 0;
}
