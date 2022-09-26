#include "dbscan/pbbs/parallel.h"
#include "dbscan/capi.h"

// Deprecated Caller class API
namespace Wrapper {
  class [[deprecated("If you must have a function that is able to work on data of unknown dimension, use the DBSCAN in dbscan/capi.h. I would highly consider using dbscan/algo.h instead if you know the number of dimensions at compile time.")]]
  Caller {
  public:
    int n;
    int dim;
    double* PF;
    Caller();
    Caller(double* PF, int dim, int n);
    ~Caller();
    int* computeDBSCAN(double epsilon, int minPts, bool* coreFlag, int* cluster);
  };
}
