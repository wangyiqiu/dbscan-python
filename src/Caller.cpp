#include <iostream>
#include "Caller.h"

#include "dbscan/algo.h"
#include "dbscan/pbbs/parallel.h"
#include "dbscan/pbbs/utils.h"

intT* DBSCAN(floatT* PF, intT dim, intT n, double epsilon, intT minPts, bool* coreFlag, intT* labels) {
  auto coreFlag2 = newA(intT, n);
  if (dim == 2) {DBSCAN<2>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 3) {DBSCAN<3>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 4) {DBSCAN<4>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 5) {DBSCAN<5>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 6) {DBSCAN<6>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 7) {DBSCAN<7>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 8) {DBSCAN<8>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 9) {DBSCAN<9>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 10) {DBSCAN<10>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 11) {DBSCAN<11>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 12) {DBSCAN<12>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 13) {DBSCAN<13>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 14) {DBSCAN<14>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 15) {DBSCAN<15>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 16) {DBSCAN<16>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 17) {DBSCAN<17>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 18) {DBSCAN<18>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 19) {DBSCAN<19>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else if (dim == 20) {DBSCAN<20>(PF, n, epsilon, minPts, coreFlag, coreFlag2, labels);}
  else {
    cout << "Error: dimension >20 is not supported." << endl;
  }
  free(coreFlag2);
  return labels;
}

