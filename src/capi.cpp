/*
A C API that instantiates templates for the C++ DBSCAN function. This API is
also useful in C++ because it allows for the selection of the number of
dimensions in the data.
*/

#include "dbscan/algo.h"
#include "dbscan/capi.h"
#include "dbscan/pbbs/parallel.h"
#include "dbscan/pbbs/utils.h"

// https://artificial-mind.net/blog/2020/10/31/constexpr-for
template <auto Start, auto End, auto Inc, class F>
static constexpr void constexpr_for(F&& f)
{
    if constexpr (Start < End)
    {
        f(std::integral_constant<decltype(Start), Start>());
        constexpr_for<Start + Inc, End, Inc>(f);
    }
}

extern "C" int DBSCAN(intT dim, intT n, floatT* PF, double epsilon, intT minPts, bool* coreFlag, intT* labels) {
  auto coreFlag2 = newA(intT, n);
  int error = 1; // didn't match number of dimensions
  constexpr_for<DBSCAN_MIN_DIMS, DBSCAN_MAX_DIMS+1, 1>([&](auto i){
    if (dim == i) {
      error = DBSCAN<i>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);
      free(coreFlag2);
    }
  });
  return error;
}

/*
Equivalent to the following, but can be controlled via compile time flags

extern "C" int DBSCAN(intT dim, intT n, floatT* PF, double epsilon, intT minPts, bool* coreFlag, intT* labels) {
  auto coreFlag2 = newA(intT, n);
  if (dim == 2) {DBSCAN<2>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 3) {DBSCAN<3>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 4) {DBSCAN<4>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 5) {DBSCAN<5>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 6) {DBSCAN<6>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 7) {DBSCAN<7>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 8) {DBSCAN<8>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 9) {DBSCAN<9>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 10) {DBSCAN<10>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 11) {DBSCAN<11>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 12) {DBSCAN<12>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 13) {DBSCAN<13>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 14) {DBSCAN<14>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 15) {DBSCAN<15>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 16) {DBSCAN<16>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 17) {DBSCAN<17>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 18) {DBSCAN<18>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 19) {DBSCAN<19>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 20) {DBSCAN<20>(n, PF, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else {
    return 1;
  }
  free(coreFlag2);
  return 0;
}
*/
