#pragma once
#include <iostream>

enum class SlurmxErr : uint16_t {
  kOk = 0,

  kTresFailure,

  __ERR_SIZE
};

class tres_gather {
 public:
  tres_gather() = default;

  virtual SlurmxErr Init() = 0;
  virtual SlurmxErr Fini() = 0;
  virtual SlurmxErr TresGatherProcInfo() = 0;
  virtual SlurmxErr ConvertProcInfoToString(std::string *PrecInfo) const = 0;

 protected:
  mutable std::string plugin_name = "tres_gather";
};
