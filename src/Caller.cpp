#include "Caller.h"
#include "capi.cpp"

// Deprecated Caller class API
namespace Wrapper {

  // Default constructor
  Caller::Caller () {}

  // Overloaded constructor
  Caller::Caller (double* PF, intT dim, intT n) {
    this->PF = PF;
    this->n = n;
    this->dim = dim;
  }

  // Destructor
  Caller::~Caller () {}

  intT* Caller::computeDBSCAN(double epsilon, intT minPts, bool* coreFlag, intT* labels) {
    DBSCAN(dim, n, PF, epsilon, minPts, coreFlag, labels);
    return labels;
  }
}
