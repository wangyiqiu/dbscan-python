// Copyright (c) 2020 Yiqiu Wang
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

#include "shared.h"
#include "point.h"
#include "pbbs/ndHash.h"

#ifdef USEJEMALLOC
#include<jemalloc/jemalloc.h>
#define jeNewA(__E,__n) (__E*) je_custom_prefix_malloc((__n)*sizeof(__E))
#define jeFree(__E) je_custom_prefix_free(__E)
#endif

template<int dim, class objT> struct grid;
template<int dim, class objT> struct cellHash;

/**
 *  A cell class that represents each box in the grid.
 */
template<int dim, class objT>
struct cell {
  typedef double floatT;
  typedef point<dim> geoPointT;
  typedef cell<dim, objT> cellT;
  typedef Table<cellHash<dim, objT>,intT> tableT;
  typedef grid<dim, objT> gridT;

  static const intT defaultCapacity = 10;
  static const intT resizeFactor = 2;

  objT *P=NULL;
  geoPointT coordP;
  intT numPoints;

  /**
  *   Constructor 1, that initializes an empty cell.
  */
  cell(): numPoints(0) {};

  /**
  *   Constructor 2, that initializes cell with external point array.
  *   @param PP external point array.
  *   @param nn size of PP.
  */
  cell(objT* PP, intT nn): P(PP), numPoints(nn) {};

  /**
  *   Constructor 3, that only initializes the coordinate of the cell base on an external point.
  *   @param coordPP external point.
  */
  cell(geoPointT coordPP): coordP(coordPP), numPoints(0) {}

  cell(floatT* coordIn): numPoints(0) {
    for(intT i=0; i<dim; ++i) coordP.updateCoordinate(i, coordIn[i]);
  }//only used as bait

  inline void init() {
    numPoints = 0;}

  inline objT* getItem() {return P;}
  inline objT* getItem(intT i) {return &P[i];}
  inline objT* getCoordObj() {return &coordP;}

  /**
  *   Computes the coordinate of the cell base on P, and assign to coordP.
  *   The coordinate is suppose to be the center of the cell.
  *   @param pMin the global point minimum.
  *   @param r the grid size.
  */
  void computeCoord(geoPointT pMin, double r) {
    for(int i=0; i<dim; ++i) {
      coordP.x[i] = r/2+pMin[i]+(double)floor((P[0][i]-pMin[i])/r)*r;}
  }

  /**
  *   The number of inserted points, not considering deletion.
  *   @return number.
  */
  inline intT size() {return numPoints;}

  /**
  *   The number of inserted points (&& not deleted yet).
  *   @return number.
  */
  inline intT actualSize() {return size();}

  /**
  *   Whether the cell is valid (initialized). It has nothing to do with the number of inserted points.
  *   @return yes or no.
  */
  inline bool isEmpty() {return coordP.isEmpty();}

  floatT *coordinate() {
    if (isEmpty()) return NULL;
    else return coordP.x;
  }

  floatT coordinate(int i) {return coordP.x[i];}

  template<class func>
  void pointMap(func& f) {
    for(intT i=0; i<numPoints; ++i) {
      if(!P[i].isEmpty()) {
        if (f(&P[i])) break;
      }
    }
  }
};

/**
  *   Hash function for cell, for pbbs/ndHash.h
  */
template<int dim, class objT>
struct cellHash {
  typedef double floatT;
  typedef cell<dim, objT> cellT;
  typedef hashFloatToCell<dim> hashFunc;
  typedef cellT* eType;
  typedef cellT* kType;

  hashFunc* hashF;
  cellT* e;

  cellHash(hashFunc* hashFF):hashF(hashFF) {
    e = new cellT();}

  cellHash(const cellHash& rhs) {
      hashF = rhs.hashF;
      e = new cellT();
      *e = *rhs.e;
  }

  ~cellHash() {
      delete e;
  }

  eType empty() {return e;}

  kType getKey(eType v) {return v;}

  uintT hash(kType c) {
    return hashF->hash(c->coordinate());
  }

  int cmp(kType c1, kType c2) {
    if (c1->isEmpty() || c2->isEmpty()) return 1;
    return hashF->compareCell(c1->coordinate(), c2->coordinate());
  }

  inline int diffCell(floatT* c1, floatT* c2) {return hashF->compareCell(c1, c2);}

  bool replaceQ(eType c1, eType c2) {return 0;}

  bool cas(eType* p, eType o, eType n) {
    return std::atomic_compare_exchange_strong_explicit(
      reinterpret_cast<std::atomic<eType>*>(p), &o, n, std::memory_order_relaxed, std::memory_order_relaxed);
  }
};
