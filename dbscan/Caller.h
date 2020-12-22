#ifndef CALLER_H
#define CALLER_H

#include "source/pbbs/parallel.h"

// #define VERBOSE

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

#endif
