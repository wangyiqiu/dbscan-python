#pragma once

#include "dbscan/pbbs/parallel.h"

namespace Wrapper {
  class Caller {
  public:
    intT n;
    int dim;
    floatT* PF;
    Caller();
    Caller(floatT* PF, int dim, intT n);
    ~Caller();
    intT* computeDBSCAN(floatT epsilon, intT minPts, bool* coreFlag, intT* cluster);
  };
}
