#include "tres_gather_cuda.h"

SlurmxErr get_version() {
  int version;
  cuInit(0);
  cuDriverGetVersion(&version);
  printf("%d",version);
  return SlurmxErr::kOk;
}
