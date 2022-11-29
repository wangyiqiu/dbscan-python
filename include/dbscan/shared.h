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

#include "point.h"
#include "pbbs/parallel.h"
#include "pbbs/sequence.h"

// *************************************************************
//    Point manipulation in a grid
// *************************************************************

//hash function for float array to a cell
template<int dim>
struct hashFloatToCell {
  typedef double floatT;
  typedef point<dim> pointT;
  static const unsigned int prime = -5;
  static const unsigned int mask = -1;
  static const unsigned int range = (1 << 29);
  static const bool noRandom = false;
  int rands[10] = {846930886, 1681692777, 1714636915, 1957747793, 424238335, 719885386, 1649760492, 596516649, 1189641421, 120120309};
  int randInt[dim];
  floatT r;
  pointT pMin;
  hashFloatToCell(pointT pMinn, floatT rr): r(rr), pMin(pMinn) {
    srand(time(NULL));
    for (intT i = 0; i < dim; i++) {
      if(noRandom) randInt[i] = rands[i] % range + 1;
      else randInt[i] = rand() % range + 1;}
  }
  inline uintT primeHash(intT* x, intT n) {
    unsigned long long temp = 0;
    uintT key = 0;
    for (intT i=0; i<n; i++) {
      temp = (long long) x[i] * (long long) randInt[i];
      temp = (temp & mask) + 5 * (temp >> 32);
      if (temp >= prime) temp -= prime;
      temp += key;
      if (temp >= prime) temp -= prime;
      key = (uintT) temp;
    }
    return key;}
  //+1: later cell, -1 earlier cell, 0 same cell
  inline int compareCell(floatT* x1, floatT* x2) {
    for(int i=0; i<dim; ++i) {
      intT xx1 = (intT) floor((x1[i]-pMin[i])/r);
      intT xx2 = (intT) floor((x2[i]-pMin[i])/r);
      if (xx1 != xx2) {
        if (xx1 > xx2) return 1;
        else return -1;
      }}
    return 0;}
  //+1: larger point, -1 smaller point, 0 same point
  inline int comparePoint(floatT* x1, floatT* x2) {
    for(int i=0; i<dim; ++i) {
      if (x1[i] != x2[i]) {
        if (x1[i] > x2[i]) return 1;
        else return -1;
      }}
    return 0;}
  inline uintT hash(floatT *x) {
    intT xx[dim];
    for(int i=0; i<dim; ++i) {
      xx[i] = (intT) floor((x[i]-pMin[i])/r);}
    return primeHash(xx, dim);}
};

//hashStruct for float array object supporting ->coordinate, ../common/ndHash.h
template<int dim, class objT>
struct aFloatHash {
  typedef double floatT;
  typedef hashFloatToCell<dim> hashFunc;
  typedef objT* eType;
  typedef objT* kType;
  hashFunc* hashF;
  objT* e;
  aFloatHash(hashFunc* hashFF):hashF(hashFF) {
    e = new objT();}
  ~aFloatHash() {}
  eType empty() {return e;}
  kType getKey(eType v) {return v;}
  uintT hash(kType c) {
    return hashF->hash(c->coordinate());
  }
  int cmp(kType c1, kType c2) {
    if (c1->isEmpty() || c2->isEmpty()) return 1;
    return hashF->comparePoint(c1->coordinate(), c2->coordinate());
  }
  //inline int diffPoint(floatT* p1, floatT* p2) {return hashF->comparePoint(p1, p2);}
  bool replaceQ(eType c1, eType c2) {return 1;}
};

// *************************************************************
//   Misc
// *************************************************************
template<int dim>
point<dim> pMinSerial(point<dim>* items, intT n) {
  point<dim> pMin = point<dim>(items[0].x);
  for(intT p=0; p<n; p++) {
    pMin.minCoords(items[p].x);}
  return pMin;
}

template<int dim>
point<dim> pMinParallel(point<dim>* items, intT n) {
  point<dim> pMin = point<dim>(items[0].x);
  // intT P = getWorkers()*8;
  static const intT P = 36 * 8;
  intT blockSize = (n+P-1)/P;
  point<dim> localMin[P];
  for (intT i=0; i<P; ++i) {
    localMin[i] = point<dim>(items[0].x);}
  parallel_for(0, P, [&](intT p) {
      intT s = p*blockSize;
      intT e = min((p+1)*blockSize,n);
      for (intT j=s; j<e; ++j) {
	localMin[p].minCoords(items[j].x);}
    });
  pMin = point<dim>(items[0].x);
  for(intT p=0; p<P; ++p) {
    pMin.minCoords(localMin[p].x);}
  return pMin;
}
