// This code is part of the project "Theoretically Efficient and Practical
// Parallel DBSCAN"
// Copyright (c) 2020 Yiqiu Wang, Yan Gu, Julian Shun
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "cell.h"
#include "point.h"
#include "shared.h"
#include "kdTree.h"
#include "kdNode.h"
#include "pbbs/sequence.h"
#include "pbbs/ndHash.h"
#include "pbbs/sampleSort.h"
#include "pbbs/quickSort.h"
#include "pbbs/parallel.h"

//a less comparator based on grid
template<int dim, class pointT, class geoPointT>
inline bool pointGridCmp(pointT p1, pointT p2, geoPointT pMin, floatT r) {
  for(int i=0; i<dim; ++i) {
    intT xx1 = (intT) floor((p1[i]-pMin[i])/r);
    intT xx2 = (intT) floor((p2[i]-pMin[i])/r);
    if (xx1 != xx2) {
      if (xx1 > xx2) return false;
      else return true;}
  }
  return false;
}

/**
  *   A grid class, that puts dim-dimensional axis-aligned box cells on a point set.
  */
template<int dim, class objT>
struct grid {
  typedef grid<dim, objT> gridT;
  typedef double floatT;
  typedef point<dim> geoPointT;
  typedef cell<dim, objT> cellT;
  typedef hashFloatToCell<dim> cellHashT;
  typedef Table<cellHash<dim, objT>,intT> tableT;
  typedef Table<aFloatHash<dim, objT>,intT> objTableT;
  typedef kdTree<dim, cellT> treeT;
// #ifdef USEJEMALLOC
//   typedef vector<cellT*, je_allocator<intT>> cellBuf;
// #else
  typedef vector<cellT*> cellBuf;
  //#endif

  static const bool noRandom = true;

  floatT r;
  geoPointT pMin;
  cellT* cells;
  intT numCells, cellCapacity;
  cellHashT* myHash=NULL;// generic hash function
  tableT* table=NULL;
  treeT* tree=NULL;
  intT totalPoints;
  cellBuf **nbrCache;

  /**
  *   Grid constructor.
  *   @param cellMax projected maximum number of points inserted.
  *   @param pMinn global coordinate minimum.
  *   @param r box cell size.
  */
  grid(intT cellMax, geoPointT pMinn, floatT rr):
    r(rr), pMin(pMinn), cellCapacity(cellMax), totalPoints(0) {

    cells = newA(cellT, cellCapacity);
    nbrCache = newA(cellBuf*, cellCapacity);
    parallel_for(0, cellCapacity, [&](intT i) {
	nbrCache[i] = NULL;
	cells[i].init();
      });
    numCells = 0;

    myHash = new cellHashT(pMinn, r);
    table = new tableT(cellMax*2, cellHash<dim, objT>(myHash));//todo load
  }

  ~grid() {
    free(cells);
    parallel_for(0, numCells, [&](intT i) {
	if(nbrCache[i]) delete nbrCache[i];
      });
    free(nbrCache);
    if(myHash) delete myHash;
    if(table) {
      table->del();
      delete table;}
    if(tree) delete tree;
  }

  inline cellT* getCell(floatT* coord) {
    cellT bait = cellT(geoPointT(coord));
    cellT* found = table->find(&bait);
    return found;}

  inline cellT* getCell(intT i) {
    return &cells[i];}

  inline intT numCell() {return numCells;}

  inline intT size() {
    return totalPoints;
  }

  template<class func>
  inline void nghPointMap(floatT* center, func& f) {
    auto bait = getCell(center);//center must be there
    if (!bait) {
      cout << "error, nghPointMap mapped to a non-existent point, abort" << endl;
      abort();}
    auto fStop = [&](){return false;};
    auto fWrap = [&](cellT* nbr) {
                   if (!nbr->isEmpty()
                       && nbr->actualSize()>0) {
                     for(intT jj=0;jj<nbr->size();++jj) {
                         if(f(nbr->getItem(jj))) return true;
                     }
                   }
                   return false;};//todo, optimize
    if (nbrCache[bait-cells]) {
      auto accum = nbrCache[bait-cells];
      for (auto accum_i : *accum) {
        if(fWrap(accum_i)) break;
      }
    } else {
      floatT hop = sqrt(dim + 3) * 1.0000001;
      nbrCache[bait-cells] = tree->rangeNeighbor(bait, r * hop, fStop, fWrap, true, nbrCache[bait-cells]);
    }
  }

  template<class func>
  inline void nghCellMap(cellT* bait, func& f) {
    auto fStop = [&](){return false;};
    auto fWrap = [&](cellT* cell){
                   if(!cell->isEmpty())
                     return f(cell);
                   return false;
                 };
    if (nbrCache[bait-cells]) {
      auto accum = nbrCache[bait-cells];
      for (auto accum_i : *accum) {
        if(fWrap(accum_i)) break;
      }
    } else {
      floatT hop = sqrt(dim + 3) * 1.0000001;
      nbrCache[bait-cells] = tree->rangeNeighbor(bait, r * hop, fStop, fWrap, true, nbrCache[bait-cells]);
    }
  }

  void insertParallel(objT* P, objT* PP, intT nn, intT* I, intT* flag=NULL) {
    if (nn==0) return;

    bool freeFlag=false;
    if (!flag) {
      flag=newA(intT, nn+1);//todo size
      freeFlag=true;}

    parallel_for(0, nn, [&](intT i){I[i] = i;});
    auto ipLess = [&] (intT a, intT b) {
                   return pointGridCmp<dim, objT, geoPointT>(P[a], P[b], pMin, r);};
    sampleSort(I, nn, ipLess);
    parallel_for(0, nn, [&](intT i){PP[i] = P[I[i]];});

    flag[0] = 1;
    parallel_for(1, nn, [&](intT i) {
	if (table->hashStruct.diffCell(PP[i].coordinate(), PP[i-1].coordinate())) {
	  flag[i] = 1;
	} else {
	  flag[i] = 0;}
      });

    numCells = sequence::prefixSum(flag, 0, nn);
    flag[nn] = numCells;

    if (numCells > cellCapacity) {
      cout << "error, grid insert exceeded cell capacity, abort()" << endl;abort();}

    parallel_for(0, nn, [&](intT i) {
	if (flag[i] != flag[i+1]) {
	  auto c = &cells[flag[i]];
	  c->P = &PP[i];
	  c->computeCoord(pMin, r);
	  table->insert(c);
	}
      });
    parallel_for(0, numCells-1, [&](intT i) {
	cells[i].numPoints = cells[i+1].P - cells[i].P;
      });
    cells[numCells-1].numPoints = &PP[nn] - cells[numCells-1].P;

    tree = new treeT(&cells[0], numCells, true);
    if(freeFlag) free(flag);
  }

};
