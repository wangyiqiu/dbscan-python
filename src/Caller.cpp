#include <iostream>
#include "Caller.h"

#include "dbscan/point.h"
#include "dbscan/shared.h"
#include "dbscan/grid.h"
#include "dbscan/coreBccp.h"
#include "dbscan/pbbs/gettime.h"
#include "dbscan/pbbs/parallel.h"
#include "dbscan/pbbs/sampleSort.h"
#include "dbscan/pbbs/unionFind.h"

// #define VERBOSE

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

  template<int dim>
  intT* computeDBSCANInternal(point<dim>* PRead, intT n, double epsilon, intT minPts, bool* coreFlagOut, intT* coreFlag, intT* cluster) {
    typedef point<dim> pointT;
    typedef grid<dim, pointT> gridT;
    typedef cell<dim, pointT> cellT;

#ifdef VERBOSE
    cout << "Input: " << n << " points, dimension " << dim << endl;
    printScheduler();
    timing tt; tt.start();
    timing t0; t0.start();
#endif

    floatT epsSqr = epsilon*epsilon;
    pointT pMin = pMinParallel(PRead, n);

    auto P = newA(pointT, n);
    // parallel_for(0, n, [&](intT i){P[i] = PRead[i];});
    auto G = new gridT(n+1, pMin, epsilon/sqrt(dim));
    auto I = newA(intT, n);
    G->insertParallel(PRead, P, n, I);
#ifdef VERBOSE
    cout << "num-cell = " << G->numCell() << endl;
    cout << "compute-grid = " << t0.next() << endl;
#endif
    //mark core
    parallel_for(0, n, [&](intT i) {coreFlag[i] = -1;});

    auto isCore = [&](pointT *p) {
		    coreFlag[p-P] = 1;
		    return false;
		  };

    parallel_for(0, G->numCell(), [&](intT i) {
	cellT* c = G->getCell(i);
	if (c->size() >= minPts) c->pointMap(isCore);
      });

    parallel_for(0, n, [&](intT i) {
			 if (coreFlag[i] < 0) {
			   intT count = 0;
			   auto isCore = [&] (pointT *p) {
					   if(count >= minPts) return true;
					   if(p->distSqr(P[i]) <= epsSqr) {//todo sqrt opt
					     count ++;}
					   return false;};
			   G->nghPointMap(P[i].coordinate(), isCore);
			   if (count >= minPts) coreFlag[i] = 1;
			   else coreFlag[i] = 0;
			 }
		       });
#ifdef VERBOSE
    cout << "mark-core-time = " << t0.next() << endl;
#endif
    //cluster core
    auto ccFlag = newA(intT, G->numCell());
    parallel_for(0, G->numCell(), [&](intT i) {
	auto ci = G->getCell(i);
	ccFlag[i] = 0;
	auto hasCore = [&](pointT *p) {
			 if (coreFlag[p-P]) {
			   ccFlag[i] = 1;
			   return true;
			 }
			 return false;
		       };
	ci->pointMap(hasCore);
      });

    typedef kdTree<dim, pointT> treeT;
    typedef kdNode<dim, pointT> nodeT;
    typedef typename nodeT::bcp bcpT;
    auto trees = newA(treeT*, G->numCell());
    parallel_for(0, G->numCell(), [&](intT i) {trees[i] = NULL;});

    // auto degCmp = [&](intT i, intT j) {
    //                 return G->getCell(i)->size() < G->getCell(j)->size();
    //               };
    // auto ordering = newA(intT, G->numCell());
    // par_for(intT i=0; i<G->numCell(); ++i) ordering[i] = i;
    //sampleSort(ordering, G->numCell(), degCmp);

    auto uf = unionFind(G->numCell());

    floatT bcpTotalTime = 0; timing t1;
    parallel_for(0, G->numCell(), [&](intT i) {
	if (ccFlag[i]) {
	  auto ti = trees[i];
	  auto procTj = [&](cellT* cj) {
			  intT j = cj - G->getCell(0);
			  auto tj = trees[j];
			  if (j < i && ccFlag[j] &&
			      uf.find(i) != uf.find(j)) {
			    if(hasEdge<cellT, treeT, pointT>(i, j, coreFlag, P, epsilon, G->getCell(0), trees)) {
			      uf.link(i, j);
			    }
			  }
			  return false;
			};
	  //G->nghCellMap(G->getCell(ordering[i]), procTj);
	  G->nghCellMap(G->getCell(i), procTj);
	}
      });

    parallel_for(0, G->numCell(), [&](intT i) {
	if (trees[i]) delete trees[i];
      });

    parallel_for(0, n, [&](intT i) {cluster[i] = -1;});

    parallel_for(0, G->numCell(), [&](intT i) {
	auto cid = G->getCell(uf.find(i))->getItem() - P;//id of first point
	auto clusterCore = [&](pointT* p){
			     if (coreFlag[p - P])
			       cluster[p - P] = cid;
			     return false;
			   };
	G->getCell(i)->pointMap(clusterCore);
      });
#ifdef VERBOSE
    cout << "cluster-core-time = " << t0.next() << endl;
#endif
    //cluster border to closest core point
    parallel_for(0, n, [&](intT i) {
			 if (!coreFlag[i]) {
			   intT cid = -1;
			   floatT cDistSqr = floatMax();
			   auto closestCore = [&] (pointT* p) {
						if (coreFlag[p-P]) {
						  auto dist = p->distSqr(P[i]);
						  if (dist <= epsSqr && dist < cDistSqr) {
						    cDistSqr = dist;
						    cid = cluster[p-P];}
						}
						return false;};
			   G->nghPointMap(P[i].coordinate(), closestCore);
			   cluster[i] = cid;
			 }
		       });
#ifdef VERBOSE
    cout << "cluster-border-time = " << t0.next() << endl;
    cout << ">> total-clustering-time = " << tt.next() << endl;
#endif
    uf.del();
    free(ccFlag);
    free(trees);
    delete G;

    //improving cluster representation
    auto cluster2 = newA(intT, n);
    auto flag = newA(intT, n+1);
    parallel_for(0, n, [&](intT i){cluster2[i] = cluster[i];});
    sampleSort(cluster, n, std::less<intT>());

    flag[0] = 1;
    parallel_for(1, n, [&](intT i){
			 if (cluster[i] != cluster[i-1])
			   flag[i] = 1;
			 else
			   flag[i] = 0;
		       });
    flag[n] = sequence::prefixSum(flag, 0, n);

    typedef Table<hashSimplePair,intT> tableT;
    auto T = new tableT(n, hashSimplePair());
    parallel_for(0, n, [&] (intT i) {
			 if (flag[i] != flag[i+1]) {
			   T->insert(make_pair(cluster[i], flag[i]));
			 }
		       });

    if(T->find(-1).second < 0) {
      parallel_for(0, n, [&](intT i){
			   cluster2[i] = T->find(cluster2[i]).second;
			 });
    } else {
      parallel_for(0, n, [&](intT i){
			   if (cluster2[i] > 0)
			     cluster2[i] = T->find(cluster2[i]).second-1;
			 });
    }

    //restoring order
    parallel_for(0, n, [&](intT i){
			 cluster[i] = cluster2[I[i]];
		       });
    parallel_for(0, n, [&](intT i){
			 coreFlagOut[i] = coreFlag[I[i]];
		       });

    free(I);
    free(cluster2);
    free(flag);
    T->del(); // Required to clean-up T's internals
    delete T;
    free(P);
#ifdef VERBOSE
    cout << "output-time = " << tt.stop() << endl;
#endif
    return cluster;
  }

  intT* Caller::computeDBSCAN(double epsilon, intT minPts, bool* coreFlag, intT* labels) {
    auto coreFlag2 = newA(intT, n);
    if (this->dim == 2) {computeDBSCANInternal<2>((point<2>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 3) {computeDBSCANInternal<3>((point<3>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 4) {computeDBSCANInternal<4>((point<4>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 5) {computeDBSCANInternal<5>((point<5>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 6) {computeDBSCANInternal<6>((point<6>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 7) {computeDBSCANInternal<7>((point<7>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 8) {computeDBSCANInternal<8>((point<8>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 9) {computeDBSCANInternal<9>((point<9>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 10) {computeDBSCANInternal<10>((point<10>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 11) {computeDBSCANInternal<11>((point<11>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 12) {computeDBSCANInternal<12>((point<12>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 13) {computeDBSCANInternal<13>((point<13>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 14) {computeDBSCANInternal<14>((point<14>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 15) {computeDBSCANInternal<15>((point<15>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 16) {computeDBSCANInternal<16>((point<16>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 17) {computeDBSCANInternal<17>((point<17>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 18) {computeDBSCANInternal<18>((point<18>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 19) {computeDBSCANInternal<19>((point<19>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else if (this->dim == 20) {computeDBSCANInternal<20>((point<20>*) PF, this->n, epsilon, minPts, coreFlag, coreFlag2, labels);}
    else {
      cout << "Error: dimension >20 is not supported." << endl;
    }
    free(coreFlag2);
    return labels;
  }

}
